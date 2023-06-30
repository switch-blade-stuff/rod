/*
 * Created by switchblade on 2023-05-18.
 */

#ifdef __unix__

#include "file.hpp"

#include <sys/param.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <climits>
#include <limits>

namespace rod::detail
{
	static result<std::string, std::error_code> get_fd_path(int fd)
	{
		auto path = std::string(MAXPATHLEN, '\0');
#ifdef F_GETPATH
		if (const auto res = ::fcntl(fd, F_GETPATH, path.data()); res >= 0) [[likely]]
			return {path};
		else if (res != ENOENT) [[unlikely]]
			return std::error_code{errno, std::system_category()};
#else
		char proc_fd[15 + std::numeric_limits<int>::digits10];
		::snprintf(proc_fd, sizeof(proc_fd), "/proc/self/fd/%i", fd);
		if (::readlink(proc_fd, path.data(), path.size()) < 0) [[unlikely]]
			return std::error_code{errno, std::system_category()};
		else
			return path;
#endif
	}

	result<system_file, std::error_code> system_file::reopen(native_handle_type fd, int mode) noexcept
	{
		int flags = O_CLOEXEC | O_NONBLOCK;
		switch (mode & (openmode::in | openmode::out))
		{
			case openmode::out | openmode::in:
				flags |= O_RDWR;
				break;
			case openmode::out:
				flags |= O_WRONLY;
				break;
			default:
				flags |= O_RDONLY;
				break;
		}
		if (mode & openmode::app) flags |= O_APPEND;
		if (mode & openmode::direct) flags |= O_DIRECT;

		/* Get file path from the descriptor & re-open the file. */
		try
		{
			const auto path = get_fd_path(fd);
			if (!path.has_value()) [[unlikely]]
				return {path.error_or({})};

			/* Save old flags. */
			if (const auto getfl = ::fcntl(fd, F_GETFL); getfl >= 0)
				[[likely]] flags |= getfl;
			else
				goto fail;

			if (const auto new_fd = ::open(path->c_str(), flags); new_fd >= 0) [[likely]]
			{
				if ((mode & openmode::ate) && ::lseek(new_fd, 0, SEEK_END) < 0) [[unlikely]]
				{
					::close(new_fd);
					goto fail;
				}
				return system_file{new_fd};
			}
		fail:
			return std::error_code{errno, std::system_category()};
		}
		catch (const std::bad_alloc &)
		{
			return std::make_error_code(std::errc::not_enough_memory);
		}
	}
	result<system_file, std::error_code> system_file::open(const char *path, int mode, int prot) noexcept
	{
		int flags = O_CLOEXEC | O_NONBLOCK;
		switch (mode & (openmode::in | openmode::out))
		{
			case openmode::out | openmode::in:
				flags |= O_RDWR;
				break;
			case openmode::out:
				flags |= O_WRONLY;
				break;
			default:
				flags |= O_RDONLY;
				break;
		}

		if (mode & openmode::app) flags |= O_APPEND;
		if (mode & openmode::trunc) flags |= O_TRUNC;
		if (mode & openmode::direct) flags |= O_DIRECT;
		if (mode & openmode::noreplace) flags |= O_EXCL;
		if (!(mode & openmode::nocreate)) flags |= O_CREAT;

		const auto fd = (mode & openmode::_sharedfd) ? ::shm_open(path, flags, prot) : ::open(path, flags, prot);
		if (fd < 0) [[unlikely]] goto fail;

		if ((mode & openmode::ate) && ::lseek(fd, 0, SEEK_END) < 0) [[unlikely]]
		{
			::close(fd);
			goto fail;
		}

		return system_file{fd};
	fail:
		return std::error_code{errno, std::system_category()};
	}
	result<system_file, std::error_code> system_file::open(const wchar_t *path, int mode, int prot) noexcept
	{
		try
		{
			auto state = std::mbstate_t{};
			auto res = std::wcsrtombs(nullptr, &path, 0, &state);
			if (res == static_cast<std::size_t>(-1)) [[unlikely]]
				return std::error_code{errno, std::system_category()};

			auto buff = std::string(res, '\0');
			res = std::wcsrtombs(buff.data(), &path, buff.size(), &state);
			if (res == static_cast<std::size_t>(-1)) [[unlikely]]
				return std::error_code{errno, std::system_category()};
			else
				return open(buff.c_str(), mode, prot);
		}
		catch (const std::bad_alloc &)
		{
			return std::make_error_code(std::errc::not_enough_memory);
		}
	}

	result<std::size_t, std::error_code> system_file::size() const noexcept
	{
		if (struct stat stat = {}; ::fstat(native_handle(), &stat)) [[unlikely]]
			return std::error_code{errno, std::system_category()};
		else
			return static_cast<std::size_t>(stat.st_size);
	}
	result<std::size_t, std::error_code> system_file::tell() const noexcept
	{
#if PTRDIFF_MAX >= INT64_MAX
		const auto res = ::lseek64(native_handle(), 0, seekdir::beg);
#else
		const auto res = ::lseek(native_handle(), 0, seekdir::beg);
#endif
		if (res < 0) [[unlikely]]
			return std::error_code{errno, std::system_category()};
		else
			return static_cast<std::size_t>(res);
	}
	result<std::size_t, std::error_code> system_file::resize(std::size_t n) noexcept
	{
#if SIZE_MAX >= UINT64_MAX
		const auto res = ::ftruncate64(native_handle(), static_cast<off64_t>(n));
#else
		const auto res = ::ftruncate(native_handle(), static_cast<off_t>(n));
#endif
		if (res) [[unlikely]]
			return std::error_code{errno, std::system_category()};
		else
			return static_cast<std::size_t>(n);
	}
	result<std::size_t, std::error_code> system_file::seek(std::ptrdiff_t off, int dir) noexcept
	{
#if PTRDIFF_MAX >= INT64_MAX
		const auto res = ::lseek64(native_handle(), static_cast<off64_t>(off), dir);
#else
		const auto res = ::lseek(native_handle(), static_cast<off_t>(off), dir);
#endif
		if (res < 0) [[unlikely]]
			return std::error_code{errno, std::system_category()};
		else
			return static_cast<std::size_t>(res);
	}

	result<std::filesystem::path, std::error_code> system_file::path() const noexcept
	{
		try { return get_fd_path(native_handle()); }
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
	}

	std::error_code system_file::sync() noexcept
	{
		if (::fsync(native_handle())) [[unlikely]]
			return {errno, std::system_category()};
		else
			return {};
	}
	result<std::size_t, std::error_code> system_file::sync_read(void *dst, std::size_t n) noexcept
	{
		result<std::size_t, std::error_code> result;
		while (!result.has_error())
		{
			result = unique_descriptor::read(dst, n);
			if (const auto code = result.error_or({}).value(); code == EAGAIN || code == EWOULDBLOCK || code == EPERM)
				result = unique_descriptor::poll_read(-1);
			else
				break;
		}
		return result;
	}
	result<std::size_t, std::error_code> system_file::sync_write(const void *src, std::size_t n) noexcept
	{
		result<std::size_t, std::error_code> result;
		while (!result.has_error())
		{
			result = unique_descriptor::write(src, n);
			if (const auto code = result.error_or({}).value(); code == EAGAIN || code == EWOULDBLOCK || code == EPERM)
				result = unique_descriptor::poll_write(-1);
			else
				break;
		}
		return result;
	}
	result<std::size_t, std::error_code> system_file::sync_read_at(void *dst, std::size_t n, std::size_t off) noexcept
	{
		result<std::size_t, std::error_code> result;
		while (!result.has_error())
		{
			result = unique_descriptor::read_at(dst, n, off);
			if (const auto code = result.error_or({}).value(); code == EAGAIN || code == EWOULDBLOCK || code == EPERM)
				result = unique_descriptor::poll_read(-1);
			else
				break;
		}
		return result;
	}
	result<std::size_t, std::error_code> system_file::sync_write_at(const void *src, std::size_t n, std::size_t off) noexcept
	{
		result<std::size_t, std::error_code> result;
		while (!result.has_error())
		{
			result = unique_descriptor::write_at(src, n, off);
			if (const auto code = result.error_or({}).value(); code == EAGAIN || code == EWOULDBLOCK || code == EPERM)
				result = unique_descriptor::poll_write(-1);
			else
				break;
		}
		return result;
	}
}
#endif

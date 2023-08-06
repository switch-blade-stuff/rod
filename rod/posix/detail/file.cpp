/*
 * Created by switchblade on 2023-05-18.
 */

#ifdef __unix__

#include "file.hpp"

#include <sys/param.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <climits>
#include <limits>

namespace rod::_detail
{
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

		const auto path = basic_descriptor{fd}.path(fd);
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
		return std::error_code{errno, std::system_category()};
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

		const auto fd = ::open(path, flags, prot);
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
			auto name_size = std::wcsrtombs(nullptr, &path, 0, &state);
			if (name_size == static_cast<std::size_t>(-1)) [[unlikely]]
				return std::error_code{errno, std::system_category()};

			auto name_buff = std::string(name_size, '\0');
			name_size = std::wcsrtombs(buff.data(), &path, name_buff.size(), &state);
			if (name_size == static_cast<std::size_t>(-1)) [[unlikely]]
				return std::error_code{errno, std::system_category()};
			else
				return open(name_buff.c_str(), mode, prot);
		}
		catch (const std::bad_alloc &)
		{
			return std::make_error_code(std::errc::not_enough_memory);
		}
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

	result<system_mmap, std::error_code> system_file::map(std::size_t off, std::size_t n, int mode) const noexcept
	{
		/* Align the offset to page size & resize if needed. */
		const auto base_off = native_mmap::pagesize_off(off);
		if (base_off.has_error())
			[[unlikely]] return base_off.error();
		else if (std::error_code err; (mode & system_mmap::mapmode::reserve) && (err = reserve(n + off)))
			[[unlikely]] return err;

		int flags = 0;
		if (mode & system_mmap::mapmode::copy)
			flags = MAP_PRIVATE;

		int prot = 0;
		if (mode & system_mmap::mapmode::read)
			prot |= PROT_READ;
		if (mode & system_mmap::mapmode::write)
			prot |= PROT_WRITE;

#if PTRDIFF_MAX >= INT64_MAX
		const auto data = ::mmap64(nullptr, n + off - *base_off, prot, flags, fd, static_cast<off64_t>(*base_off));
#else
		const auto data = ::mmap(nullptr, n + off - *base_off, prot, flags, fd, static_cast<off64_t>(*base_off));
#endif
		if (data) [[likely]]
			return system_mmap{data, off - *base_off, n + off - *base_off};
		else
			return std::error_code{errno, std::system_category()};
	}
}
#endif

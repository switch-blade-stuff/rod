/*
 * Created by switchblade on 2023-05-18.
 */

#ifdef __unix__

#include "file.hpp"

#include <unistd.h>
#include <climits>
#include <limits>

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	system_file system_file::reopen(native_handle_type fd, int mode, std::error_code &err) noexcept
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
		char path[PATH_MAX];
#ifdef F_GETPATH
		if (::fcntl(fd, F_GETPATH, path) < 0) [[unlikely]]
			goto fail;
#else
		char proc_fd[15 + std::numeric_limits<int>::digits10];
		::snprintf(proc_fd, sizeof(proc_fd), "/proc/self/fd/%i", fd);
		if (::readlink(proc_fd, path, PATH_MAX) < 0) [[unlikely]]
			goto fail;
#endif

		/* Save old flags. */
		if (const auto getfl = ::fcntl(fd, F_GETFL); getfl >= 0) [[likely]]
				                                                         flags |= getfl;
		else
			goto fail;

		if (const auto new_fd = ::open(path, flags); new_fd >= 0) [[likely]]
		{
			if ((mode & openmode::ate) && ::lseek(new_fd, 0, SEEK_END) < 0) [[unlikely]]
			{
				::close(new_fd);
				goto fail;
			}
			return (err = {}, system_file{new_fd});
		}
	fail:
		return (err = {errno, std::system_category()}, system_file{});
	}
	system_file system_file::open(const char *path, int mode, int prot, std::error_code &err) noexcept
	{
		int flags = O_CREAT | O_CLOEXEC | O_NONBLOCK;
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

		const auto fd = ::open(path, flags, prot);
		if (fd < 0) [[unlikely]] goto fail;

		if ((mode & openmode::ate) && ::lseek(fd, 0, SEEK_END) < 0) [[unlikely]]
		{
			::close(fd);
			goto fail;
		}

		return (err = {}, system_file{fd});
	fail:
		return (err = {errno, std::system_category()}, system_file{});
	}
	system_file system_file::open(const wchar_t *path, int mode, int prot, std::error_code &err) noexcept
	{
		auto state = std::mbstate_t{};
		auto res = std::wcsrtombs(nullptr, &path, 0, &state);
		if (res == static_cast<std::size_t>(-1)) [[unlikely]]
			return (err = {errno, std::system_category()}, system_file{});

		auto buff = std::string(res, '\0');
		res = std::wcsrtombs(buff.data(), &path, buff.size(), &state);
		if (res == static_cast<std::size_t>(-1)) [[unlikely]]
			return (err = {errno, std::system_category()}, system_file{});
		else
			return open(buff.c_str(), mode, prot, err);
	}

	std::size_t system_file::tell(std::error_code &err) const noexcept
	{
#if SIZE_MAX >= UINT64_MAX
		const auto res = ::lseek64(native_handle(), 0, SEEK_CUR);
#else
		const auto res = ::lseek(native_handle(), 0, SEEK_CUR);
#endif
		if (res < 0) [[unlikely]]
			return (err = {errno, std::system_category()}, 0);
		else
			return (err = {}, static_cast<std::size_t>(res));
	}
	std::size_t system_file::seek(std::ptrdiff_t off, int dir, std::error_code &err) noexcept
	{
#if PTRDIFF_MAX >= INT64_MAX
		const auto res = ::lseek64(native_handle(), static_cast<off64_t>(off), dir);
#else
		const auto res = ::lseek(native_handle(), static_cast<off_t>(off), dir);
#endif
		if (res < 0) [[unlikely]]
			return (err = {errno, std::system_category()}, 0);
		else
			return (err = {}, static_cast<std::size_t>(res));
	}

	std::error_code system_file::flush() noexcept
	{
		if (::fsync(native_handle())) [[unlikely]]
			return {errno, std::system_category()};
		else
			return {};
	}
	std::size_t system_file::sync_read(void *dst, std::size_t n, std::error_code &err) noexcept
	{
		for (;;)
		{
			const auto res = unique_descriptor::read(dst, n, err);
			if (const auto code = err.value(); code == EAGAIN || code == EWOULDBLOCK || code == EPERM)
			{
				unique_descriptor::poll_read(-1, err);
				continue;
			}
			return res;
		}
	}
	std::size_t system_file::sync_write(const void *src, std::size_t n, std::error_code &err) noexcept
	{
		for (;;)
		{
			const auto res = unique_descriptor::write(src, n, err);
			if (const auto code = err.value(); code == EAGAIN || code == EWOULDBLOCK || code == EPERM)
			{
				unique_descriptor::poll_read(-1, err);
				continue;
			}
			return res;
		}
	}
	std::size_t system_file::sync_read_at(void *dst, std::size_t n, std::ptrdiff_t pos, std::error_code &err) noexcept
	{
		for (;;)
		{
			const auto res = unique_descriptor::read_at(dst, n, pos, err);
			if (const auto code = err.value(); code == EAGAIN || code == EWOULDBLOCK || code == EPERM)
			{
				unique_descriptor::poll_read(-1, err);
				continue;
			}
			return res;
		}
	}
	std::size_t system_file::sync_write_at(const void *src, std::size_t n, std::ptrdiff_t pos, std::error_code &err) noexcept
	{
		for (;;)
		{
			const auto res = unique_descriptor::write_at(src, n, pos, err);
			if (const auto code = err.value(); code == EAGAIN || code == EWOULDBLOCK || code == EPERM)
			{
				unique_descriptor::poll_read(-1, err);
				continue;
			}
			return res;
		}
	}
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

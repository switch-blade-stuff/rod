/*
 * Created by switchblade on 2023-05-20.
 */

#ifdef __unix__

#include "descriptor.hpp"

#include <sys/stat.h>
#include <unistd.h>
#include <poll.h>

#include <climits>
#include <limits>

namespace rod::_detail
{
	std::error_code basic_descriptor::close() noexcept
	{
		if (::close(release())) [[unlikely]]
			return {errno, std::system_category()};
		else
			return {};
	}

	result<std::size_t, std::error_code> basic_descriptor::size() const noexcept
	{
		if (struct stat stat = {}; ::fstat(native_handle(), &stat)) [[unlikely]]
			return std::error_code{errno, std::system_category()};
		else
			return static_cast<std::size_t>(stat.st_size);
	}
	result<std::size_t, std::error_code> basic_descriptor::resize(std::size_t n) noexcept
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

	std::error_code unlink() noexcept
	{
		auto buff = std::string(MAXPATHLEN, '\0');
#ifdef F_GETPATH
		if (const auto res = ::fcntl(fd, F_GETPATH, buff.data()); res < 0 && res != ENOENT) [[unlikely]]
			return std::error_code{errno, std::system_category()};
		else if (res == ENOENT)
#else
		{
			char proc_fd[15 + std::numeric_limits<int>::digits10];
			::snprintf(proc_fd, sizeof(proc_fd), "/proc/self/fd/%i", fd);
			if (::readlink(proc_fd, buff.data(), buff.size()) < 0) [[unlikely]]
				return std::error_code{errno, std::system_category()};
		}
#endif
		if (::unlink(buff.c_str()) != 0) [[unlikely]]
			return std::error_code{errno, std::system_category()};
		else
			return {};
	}
	result<std::filesystem::path, std::error_code> basic_descriptor::path() const noexcept
	{
		auto buff = std::string(MAXPATHLEN, '\0');
#ifdef F_GETPATH
		if (const auto res = ::fcntl(fd, F_GETPATH, buff.data()); res < 0 && res != ENOENT) [[unlikely]]
			return std::error_code{errno, std::system_category()};
		else if (res == ENOENT)
#else
		{
			char proc_fd[15 + std::numeric_limits<int>::digits10];
			::snprintf(proc_fd, sizeof(proc_fd), "/proc/self/fd/%i", fd);
			if (::readlink(proc_fd, buff.data(), buff.size()) < 0) [[unlikely]]
				return std::error_code{errno, std::system_category()};
		}
#endif
		return buff.c_str();
	}

	result<bool, std::error_code> basic_descriptor::poll_read(int timeout) noexcept
	{
		pollfd fds = {.fd = _fd, .events = POLLIN, .revents = 0};
		if (const auto res = ::poll(&fds, 1, timeout); res < 0) [[unlikely]]
			return std::error_code{errno, std::system_category()};
		else
			return static_cast<bool>(res);
	}
	result<bool, std::error_code> basic_descriptor::poll_write(int timeout) noexcept
	{
		pollfd fds = {.fd = _fd, .events = POLLOUT, .revents = 0};
		if (const auto res = ::poll(&fds, 1, timeout); res < 0) [[unlikely]]
			return std::error_code{errno, std::system_category()};
		else
			return static_cast<bool>(res);
	}
	result<bool, std::error_code> basic_descriptor::poll_error(int timeout) noexcept
	{
		pollfd fds = {.fd = _fd, .events = POLLPRI | POLLERR | POLLHUP, .revents = 0};
		if (const auto res = ::poll(&fds, 1, timeout); res < 0) [[unlikely]]
			return std::error_code{errno, std::system_category()};
		else
			return static_cast<bool>(res);
	}

	result<std::size_t, std::error_code> basic_descriptor::read(void *dst, std::size_t n) noexcept
	{
		if (const auto res = ::read(_fd, dst, n); res < 0) [[unlikely]]
			return std::error_code{errno, std::system_category()};
		else
			return static_cast<std::size_t>(res);
	}
	result<std::size_t, std::error_code> basic_descriptor::write(const void *src, std::size_t n) noexcept
	{
		if (const auto res = ::write(_fd, src, n); res < 0) [[unlikely]]
			return std::error_code{errno, std::system_category()};
		else
			return static_cast<std::size_t>(res);
	}

	result<std::size_t, std::error_code> basic_descriptor::read_at(void *dst, std::size_t n, std::size_t off) noexcept
	{
#if PTRDIFF_MAX >= INT64_MAX
		const auto res = ::pread64(_fd, dst, n, static_cast<off64_t>(off));
#else
		const auto res = ::pread(_fd, dst, n, static_cast<off_t>(off));
#endif
		if (res < 0) [[unlikely]]
			return std::error_code{errno, std::system_category()};
		else
			return static_cast<std::size_t>(res);
	}
	result<std::size_t, std::error_code> basic_descriptor::write_at(const void *src, std::size_t n, std::size_t off) noexcept
	{
#if PTRDIFF_MAX >= INT64_MAX
		const auto res = ::pwrite64(_fd, src, n, static_cast<off64_t>(off));
#else
		const auto res = ::pwrite(_fd, src, n, static_cast<off_t>(off));
#endif
		if (res < 0) [[unlikely]]
			return std::error_code{errno, std::system_category()};
		else
			return static_cast<std::size_t>(res);
	}

	unique_descriptor::~unique_descriptor() { if (is_open()) ::close(release()); }
}
#endif

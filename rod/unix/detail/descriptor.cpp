/*
 * Created by switchblade on 2023-05-20.
 */

#ifdef __unix__

#include "descriptor.hpp"

#include <unistd.h>
#include <poll.h>

namespace rod::detail
{
	std::error_code basic_descriptor::close() noexcept
	{
		if (::close(release())) [[unlikely]]
			return {errno, std::system_category()};
		else
			return {};
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

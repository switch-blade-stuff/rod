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

	bool basic_descriptor::poll_read(int timeout, std::error_code &err) noexcept
	{
		pollfd fds = {.fd = _fd, .events = POLLIN, .revents = 0};
		if (const auto res = ::poll(&fds, 1, timeout); res < 0) [[unlikely]]
			return (err = {errno, std::system_category()}, 0);
		else
			return (err = {}, res);
	}
	bool basic_descriptor::poll_write(int timeout, std::error_code &err) noexcept
	{
		pollfd fds = {.fd = _fd, .events = POLLOUT, .revents = 0};
		if (const auto res = ::poll(&fds, 1, timeout); res < 0) [[unlikely]]
			return (err = {errno, std::system_category()}, 0);
		else
			return (err = {}, res);
	}
	bool basic_descriptor::poll_error(int timeout, std::error_code &err) noexcept
	{
		pollfd fds = {.fd = _fd, .events = POLLPRI | POLLERR | POLLHUP, .revents = 0};
		if (const auto res = ::poll(&fds, 1, timeout); res < 0) [[unlikely]]
			return (err = {errno, std::system_category()}, 0);
		else
			return (err = {}, res);
	}

	std::size_t basic_descriptor::read(void *dst, std::size_t n, std::error_code &err) noexcept
	{
		if (const auto res = ::read(_fd, dst, n); res < 0) [[unlikely]]
			return (err = {errno, std::system_category()}, 0);
		else
			return (err = {}, static_cast<std::size_t>(res));
	}
	std::size_t basic_descriptor::write(const void *src, std::size_t n, std::error_code &err) noexcept
	{
		if (const auto res = ::write(_fd, src, n); res < 0) [[unlikely]]
			return (err = {errno, std::system_category()}, 0);
		else
			return (err = {}, static_cast<std::size_t>(res));
	}

	std::size_t basic_descriptor::read_at(void *dst, std::size_t n, std::size_t off, std::error_code &err) noexcept
	{
#if PTRDIFF_MAX >= INT64_MAX
		const auto res = ::pread64(_fd, dst, n, static_cast<off64_t>(off));
#else
		const auto res = ::pread(_fd, dst, n, static_cast<off_t>(off));
#endif
		if (res < 0) [[unlikely]]
			return (err = {errno, std::system_category()}, 0);
		else
			return (err = {}, static_cast<std::size_t>(res));
	}
	std::size_t basic_descriptor::write_at(const void *src, std::size_t n, std::size_t off, std::error_code &err) noexcept
	{
#if PTRDIFF_MAX >= INT64_MAX
		const auto res = ::pwrite64(_fd, src, n, static_cast<off64_t>(off));
#else
		const auto res = ::pwrite(_fd, src, n, static_cast<off_t>(off));
#endif
		if (res < 0) [[unlikely]]
			return (err = {errno, std::system_category()}, 0);
		else
			return (err = {}, static_cast<std::size_t>(res));
	}

	unique_descriptor::~unique_descriptor() { if (is_open()) ::close(release()); }
}
#endif

/*
 * Created by switchblade on 2023-05-20.
 */

#include "descriptor.hpp"

#include <unistd.h>

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	descriptor_handle::~descriptor_handle() { ::close(m_fd); }
	std::error_code descriptor_handle::close() noexcept
	{
		if (::close(release())) [[unlikely]]
			return errno_code();
		else
			return {};
	}

	std::size_t descriptor_handle::read(void *dst, std::size_t n, std::error_code &err) noexcept
	{
		if (const auto res = ::read(m_fd, dst, n); res >= 0) [[likely]]
			return (err = {}, static_cast<std::size_t>(res));
		else
			return (err = errno_code(), 0);
	}
	std::size_t descriptor_handle::write(const void *src, std::size_t n, std::error_code &err) noexcept
	{
		if (const auto res = ::write(m_fd, src, n); res >= 0) [[likely]]
			return (err = {}, static_cast<std::size_t>(res));
		else
			return (err = errno_code(), 0);
	}

	std::size_t descriptor_handle::read_at(void *dst, std::size_t n, std::ptrdiff_t off, std::error_code &err) noexcept
	{
#if PTRDIFF_MAX >= INT64_MAX
		const auto res = ::pread64(m_fd, dst, n, static_cast<off64_t>(off));
#else
		const auto res = ::pread(m_fd, dst, n, static_cast<off_t>(off));
#endif
		if (res >= 0) [[likely]]
			return (err = {}, static_cast<std::size_t>(res));
		else
			return (err = errno_code(), 0);
	}
	std::size_t descriptor_handle::write_at(const void *src, std::size_t n, std::ptrdiff_t off, std::error_code &err) noexcept
	{
#if PTRDIFF_MAX >= INT64_MAX
		const auto res = ::pwrite64(m_fd, src, n, static_cast<off64_t>(off));
#else
		const auto res = ::pwrite(m_fd, src, n, static_cast<off_t>(off));
#endif
		if (res >= 0) [[likely]]
			return (err = {}, static_cast<std::size_t>(res));
		else
			return (err = errno_code(), 0);
	}

	std::size_t descriptor_handle::seek(std::ptrdiff_t off, int dir, std::error_code &err) noexcept
	{
#if PTRDIFF_MAX >= INT64_MAX
		const auto res = ::lseek64(m_fd, static_cast<off64_t>(off), dir);
#else
		const auto res = ::lseek(m_fd, static_cast<off_t>(off), dir);
#endif
		if (res >= 0) [[likely]]
			return (err = {}, static_cast<std::size_t>(res));
		else
			return (err = errno_code(), 0);
	}
	std::size_t descriptor_handle::tell(std::error_code &err) const noexcept
	{
#if SIZE_MAX >= UINT64_MAX
		const auto res = ::lseek64(m_fd, 0, SEEK_CUR);
#else
		const auto res = ::lseek(m_fd, 0, SEEK_CUR);
#endif
		if (res >= 0) [[likely]]
			return (err = {}, static_cast<std::size_t>(res));
		else
			return (err = errno_code(), 0);
	}
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

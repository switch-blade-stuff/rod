/*
 * Created by switchblade on 2023-05-20.
 */

#include "descriptor.hpp"

#ifdef __unix__

#include <unistd.h>
#include <utility>

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	descriptor_handle::~descriptor_handle() { ::close(m_fd); }
	std::error_code descriptor_handle::close() noexcept
	{
		if (::close(std::exchange(m_fd, -1))) [[unlikely]]
			return std::make_error_code(static_cast<std::errc>(errno));
		else
			return {};
	}

	std::size_t descriptor_handle::read(void *dst, std::size_t n, std::error_code &err) noexcept
	{
		if (const auto res = ::read(m_fd, dst, n); res < 0) [[unlikely]]
			return (err = std::make_error_code(static_cast<std::errc>(errno)), 0);
		else
			return (err = {}, static_cast<std::size_t>(res));
	}
	std::size_t descriptor_handle::write(const void *src, std::size_t n, std::error_code &err) noexcept
	{
		if (const auto res = ::write(m_fd, src, n); res < 0) [[likely]]
			return (err = std::make_error_code(static_cast<std::errc>(errno)), 0);
		else
			return (err = {}, static_cast<std::size_t>(res));
	}

	std::size_t descriptor_handle::seek(std::ptrdiff_t off, int dir, std::error_code &err) noexcept
	{
#if PTRDIFF_MAX >= INT64_MAX || SIZE_MAX >= UINT64_MAX
		const auto res = ::lseek64(m_fd, static_cast<off64_t>(off), dir);
#else
		const auto res = ::lseek(m_fd, static_cast<off_t>(off), dir);
#endif
		if (res < 0) [[likely]]
			return (err = std::make_error_code(static_cast<std::errc>(errno)), 0);
		else
			return (err = {}, static_cast<std::size_t>(res));
	}
	std::size_t descriptor_handle::tell(std::error_code &err) const noexcept
	{
#if SIZE_MAX >= UINT64_MAX
		const auto res = ::lseek64(m_fd, 0, SEEK_CUR);
#else
		const auto res = ::lseek(m_fd, 0, SEEK_CUR);
#endif
		if (res < 0) [[likely]]
			return (err = std::make_error_code(static_cast<std::errc>(errno)), 0);
		else
			return (err = {}, static_cast<std::size_t>(res));
	}
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

/*
 * Created by switchblade on 2023-05-30.
 */

#ifdef __unix__

#include <unistd.h>

#include "mmap.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	static std::size_t get_pagesize(std::error_code &err) noexcept
	{
		static const auto size = [&]() noexcept -> std::size_t
		{
#ifdef _SC_PAGE_SIZE
			const auto size = sysconf(_SC_PAGE_SIZE);
#else
			const auto size = sysconf(_SC_PAGESIZE);
#endif
			if (size < 0) [[unlikely]]
				return (err = {errno, std::system_category()}, 0);
			else
				return (err = {}, static_cast<std::size_t>(size));
		}();
		return size;
	}
	template<typename I>
	static I align_pagesize(bool up, I req, std::error_code &err) noexcept
	{
		if (const auto mult = get_pagesize(err); !mult) [[unlikely]]
			return mult;
		else if (const auto rem = static_cast<I>(req % mult); rem && up)
			return req + static_cast<I>(mult) - rem;
		else
			return req - rem;
	}

	system_mmap system_mmap::map(void *hint, std::size_t size, int fd, std::ptrdiff_t off, int mode, int prot, std::error_code &err) noexcept
	{
		/* Align the file offset and use the difference as the base offset. */
		const auto base_off = off - align_pagesize(false, off, err);
		if (err || !(size = align_pagesize(true, size + base_off, err))) [[unlikely]]
			return system_mmap{};

#if PTRDIFF_MAX >= INT64_MAX
		const auto data = ::mmap64(hint, size, prot, mode, fd, static_cast<off64_t>(off - base_off));
#else
		const auto data = ::mmap(hint, size, prot, mode, fd, static_cast<off64_t>(off - base_off));
#endif
		if (data) [[likely]]
			return system_mmap{data, size, static_cast<std::size_t>(base_off)};
		else
			return (err = {errno, std::system_category()}, system_mmap{});
	}

	std::error_code system_mmap::unmap() noexcept
	{
		if (!empty() && ::munmap(std::exchange(m_data, {}), m_size)) [[unlikely]]
			return {errno, std::system_category()};
		else
			return {};
	}
	std::error_code system_mmap::remap(std::size_t new_size) noexcept
	{
		/* Align the file offset and use the difference as the base offset. */
		const auto off = m_data - m_base;
		if (std::error_code err = {}; !(new_size = align_pagesize(true, new_size + off, err))) [[unlikely]]
			return err;
		if (const auto new_base = static_cast<std::byte *>(::mremap(m_base, m_size, new_size, 0)); !new_base) [[unlikely]]
			return {errno, std::system_category()};
		else
		{
			m_data = new_base + off;
			m_base = new_base;
			m_size = new_size;
			return {};
		}
	}

	system_mmap::~system_mmap() { if (!empty()) ::munmap(std::exchange(m_base, {}), m_size); }
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

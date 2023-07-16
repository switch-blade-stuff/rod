/*
 * Created by switchblade on 2023-05-30.
 */

#ifdef __unix__

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <atomic>

#include "mmap.hpp"

namespace rod::detail
{
	result<std::size_t, std::error_code> mmap_handle::get_pagesize() noexcept
	{
		static std::atomic<std::size_t> size = {};
		if (const auto old = size.load(); old != 0) [[likely]]
			return old;

#ifdef _SC_PAGE_SIZE
		const auto res = sysconf(_SC_PAGE_SIZE);
#else
		const auto res = sysconf(_SC_PAGESIZE);
#endif
		if (res < 0) [[unlikely]]
			return std::error_code{errno, std::system_category()};
		else
			return size = static_cast<std::size_t>(res);
	}

	std::error_code mmap_handle::unmap() noexcept
	{
		const auto [data, size] = release();
		if (data && ::munmap(data, size)) [[unlikely]]
			return {errno, std::system_category()};
		else
			return {};
	}
	mmap_handle::~mmap_handle()
	{
		const auto [data, size] = release();
		if (data) ::munmap(data, size);
	}
}
#endif
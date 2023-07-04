/*
 * Created by switchblade on 2023-05-30.
 */

#ifdef __unix__

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mmap.hpp"

namespace rod::detail
{
	result<std::size_t, std::error_code> mmap_handle::get_pagesize() noexcept
	{
		result<std::size_t, std::error_code> result;
		static const auto size = [&]() noexcept -> std::size_t
		{
#ifdef _SC_PAGE_SIZE
			const auto size = sysconf(_SC_PAGE_SIZE);
#else
			const auto size = sysconf(_SC_PAGESIZE);
#endif
			if (size < 0) [[unlikely]]
				return (result = std::error_code{errno, std::system_category()}, 0);
			else
				return static_cast<std::size_t>(size);
		}();
		return result.empty() ? size : result;
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
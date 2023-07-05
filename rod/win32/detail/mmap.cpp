/*
 * Created by switch_blade on 2023-07-03.
 */

#ifdef _WIN32

#include "mmap.hpp"

#define NOMINMAX
#include <windows.h>

namespace rod::detail
{
	std::size_t system_mmap::get_pagesize() noexcept
	{
		static std::atomic<std::size_t> size = {};
		if (const auto old = size.load(); old != 0) [[likely]]
			return old;

		SYSTEM_INFO sysinfo;
		::GetSystemInfo(&sysinfo);
		return size = sysinfo.dwAllocationGranularity;
	}

	system_mmap::~system_mmap() { if (_data) ::UnmapViewOfFile(_data); }
}
#endif

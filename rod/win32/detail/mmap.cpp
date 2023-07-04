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
		SYSTEM_INFO sysinfo;
		::GetSystemInfo(&sysinfo);
		return sysinfo.dwAllocationGranularity;
	}

	system_mmap::~system_mmap() { ::UnmapViewOfFile(_data); }
}
#endif

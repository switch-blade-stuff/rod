/*
 * Created by switch_blade on 2023-07-15.
 */

#pragma once

#include "filetime_clock.hpp"

#ifdef ROD_WIN32

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

namespace rod
{
	filetime_clock::time_point filetime_clock::now() noexcept
	{
		FILETIME time = {};
		::GetSystemTimeAsFileTime(&time);
		return {time.dwHighDateTime, time.dwLowDateTime};
	}
	filetime_clock::time_point filetime_clock::create_time(void *hnd) noexcept
	{
		FILETIME time = {};
		::GetFileTime(hnd, &time, nullptr, nullptr);
		return {time.dwHighDateTime, time.dwLowDateTime};
	}
	filetime_clock::time_point filetime_clock::access_time(void *hnd) noexcept
	{
		FILETIME time = {};
		::GetFileTime(hnd, nullptr, &time, nullptr);
		return {time.dwHighDateTime, time.dwLowDateTime};
	}
	filetime_clock::time_point filetime_clock::modify_time(void *hnd) noexcept
	{
		FILETIME time = {};
		::GetFileTime(hnd, nullptr, nullptr, &time);
		return {time.dwHighDateTime, time.dwLowDateTime};
	}
}
#endif

/*
 * Created by switch_blade on 2023-07-15.
 */

#pragma once

#include "../file_clock.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

rod::file_clock::time_point rod::file_clock::now() noexcept
{
	union { FILETIME ft = {}; std::int64_t qw; };
	::GetSystemTimeAsFileTime(&ft);
	return {qw};
}

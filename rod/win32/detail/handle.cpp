/*
 * Created by switchblade on 2023-05-20.
 */

#ifdef _WIN32

#include "handle.hpp"

#define NOMINMAX
#include <windows.h>

namespace rod::detail
{
	std::error_code basic_handle::close() noexcept
	{
		if (is_open() && !::CloseHandle(_handle)) [[unlikely]]
			return {static_cast<int>(::GetLastError()), std::system_category()};
		else
			return {};
	}
	std::error_code basic_handle::poll_wait() const noexcept
	{
		for (auto &winapi = winapi::instance;;)
		{
			if (const auto status = winapi.NtWaitForSingleObject(native_handle(), true, nullptr); !status)
				return {};
			else if (status != STATUS_USER_APC && status != 0x00000102 /* STATUS_TIMEOUT */)
				return std::error_code{static_cast<int>(winapi.RtlNtStatusToDosError(status)), std::system_category()};
		}
	}

	unique_handle::~unique_handle() { if (is_open()) ::CloseHandle(release()); }
}
#endif
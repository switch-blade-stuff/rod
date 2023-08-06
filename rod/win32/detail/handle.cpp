/*
 * Created by switchblade on 2023-05-20.
 */

#include "handle.hpp"

#ifdef ROD_WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace rod::_detail
{
	std::error_code basic_handle::close() noexcept
	{
		if (is_open() && !::CloseHandle(_handle)) [[unlikely]]
			return {static_cast<int>(::GetLastError()), std::system_category()};
		else
			return {};
	}
	std::error_code basic_handle::wait() const noexcept
	{
		for (;;)
		{
			if (const auto status = ntapi::instance.NtWaitForSingleObject(native_handle(), true, nullptr); !status)
				return {};
			else if (status != STATUS_USER_APC && status != 0x00000102 /* STATUS_TIMEOUT */)
				return std::error_code{static_cast<int>(ntapi::instance.RtlNtStatusToDosError(status)), std::system_category()};
		}
	}
}
#endif
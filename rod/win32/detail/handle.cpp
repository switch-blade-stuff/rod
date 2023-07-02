/*
 * Created by switchblade on 2023-05-20.
 */

#ifdef _WIN32

#include "handle.hpp"

#define NOMINMAX
#include <windows.h>

namespace rod::detail
{
	unique_handle::~unique_handle() { if (is_open()) ::CloseHandle(release()); }
	std::error_code basic_handle::close() noexcept
	{
		if (is_open() && !::CloseHandle(_handle)) [[unlikely]]
			return {static_cast<int>(::GetLastError()), std::system_category()};
		else
			return {};
	}
}
#endif

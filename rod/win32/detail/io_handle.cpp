/*
 * Created by switchblade on 2023-05-20.
 */

#ifdef _WIN32

#include "io_handle.hpp"

#define NOMINMAX
#include <windows.h>

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	unique_io_handle::~unique_io_handle() { if (is_open()) ::CloseHandle(release()); }
	std::error_code basic_io_handle::close() noexcept
	{
		if (is_open() && !::CloseHandle(_handle)) [[unlikely]]
			return {static_cast<int>(::GetLastError()), std::system_category()};
		else
			return {};
	}
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

/*
 * Created by switchblade on 2023-05-20.
 */

#include "io_handle.hpp"

#include <Windows.h>

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	std::error_code basic_io_handle::close() noexcept { return ::CloseHandle(m_handle) ? std::error_code{} : std::error_code{static_cast<int>(::GetLastError()), std::system_category()}; }
	unique_io_handle::~unique_io_handle() { if (is_open()) ::CloseHandle(release()); }
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

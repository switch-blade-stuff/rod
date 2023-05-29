/*
 * Created by switchblade on 2023-05-20.
 */

#include "io_handle.hpp"

#include <Windows.h>

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	std::error_code basic_io_handle::close() noexcept
	{
		if (!::CloseHandle(m_handle)) [[unlikely]]
			return {static_cast<int>(::GetLastError()), std::system_category()};
		else
			return {};
	}
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

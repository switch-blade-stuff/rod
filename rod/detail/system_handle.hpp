/*
 * Created by switch_blade on 2023-07-10.
 */

#pragma once

#include "config.hpp"

#if defined(ROD_WIN32)
#include "../win32/detail/handle.hpp"
#elif defined(ROD_POSIX)
#include "../posix/detail/handle.hpp"
#else
#error No valid implementation of system_handle
#endif

namespace rod::_detail
{
#if defined(ROD_WIN32)
	using system_handle = _win32::unique_handle;
#elif defined(ROD_POSIX)
	using system_handle = _posix::unique_handle;
#endif
}

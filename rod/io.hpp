/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#include "detail/io_handle.hpp"
#include "file.hpp"

#ifdef ROD_WIN32
#include "win32/iocp_context.hpp"
#endif
#ifdef __linux__
#include "linux/epoll_context.hpp"
#include "linux/io_uring_context.hpp"
#endif

namespace rod
{
#if defined(ROD_WIN32)
	using system_context = iocp_context;
#elif defined(ROD_HAS_LIBURING)
	using system_context = io_uring_context;
#elif defined(ROD_HAS_EPOLL)
	using system_context = epoll_context;
#else
	using system_context = run_loop;
#endif
}

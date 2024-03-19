/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#include "scheduling.hpp"
#include "path.hpp"
#include "file.hpp"
#include "mmap.hpp"

#if defined(ROD_WIN32)
#include "win32/iocp_context.hpp"
#endif
#if defined(__linux__) && 0
#include "linux/epoll_context.hpp"
#include "linux/io_uring_context.hpp"
#endif

namespace rod
{
#if defined(ROD_WIN32)
	using system_context = iocp_context;
#elif defined(ROD_HAS_LIBURING) && 0
	using system_context = io_uring_context;
#elif defined(ROD_HAS_EPOLL) && 0
	using system_context = epoll_context;
#else
	using system_context = run_loop;
#endif
}

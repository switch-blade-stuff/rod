/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#include "detail/io_buffer.hpp"
#include "detail/adaptors/read_some.hpp"
#include "detail/adaptors/write_some.hpp"

#ifdef __linux__
#include "linux/epoll_context.hpp"
#include "linux/io_uring_context.hpp"
#endif

#include "thread_pool.hpp"
#include "file.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
#if defined(ROD_HAS_LIBURING)
	using system_context = io_uring_context;
#elif defined(ROD_HAS_EPOLL)
	using system_context = epoll_context;
#else
	using system_context = run_loop;
#endif
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

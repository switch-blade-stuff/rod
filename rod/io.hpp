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

#include "file.hpp"

namespace rod
{
#if defined(ROD_HAS_LIBURING)
	using io_context = io_uring_context;
#elif defined(ROD_HAS_EPOLL)
	using io_context = epoll_context;
#else
	using io_context = run_loop;
#endif
}

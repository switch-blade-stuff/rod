/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#include "detail/io_buffer.hpp"
#include "detail/adaptors/read_some.hpp"
#include "detail/adaptors/write_some.hpp"

#ifdef __linux__
#include "linux/epoll_context.hpp"
#endif

#include "file.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
#ifdef ROD_HAS_EPOLL
	using system_context = epoll_context;
#else
	using system_context = run_loop;
#endif
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

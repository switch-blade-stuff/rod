/*
 * Created by switchblade on 2023-06-01.
 */

#pragma once

#include "../../detail/adaptors/read_some.hpp"
#include "../../detail/adaptors/write_some.hpp"

#if defined(ROD_HAS_EPOLL) || defined(ROD_HAS_LIBURING)

#include <utility>
#include <thread>
#include <chrono>

#include "../../detail/priority_queue.hpp"
#include "../../detail/atomic_queue.hpp"
#include "../../detail/basic_queue.hpp"

#include "../../unix/monotonic_clock.hpp"
#include "../../unix/detail/file.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::_system_ctx
{
	enum flags_t { stop_possible = 1, stop_requested = 4, dispatched = 8 };
	enum class io_type { read, write, /*rcv_data, rcv_err, send_data*/ };

	using clock = monotonic_clock;
	using time_point = typename clock::time_point;

	template<typename Env>
	concept stoppable_env = stoppable_token<stop_token_of_t<Env &>>;

	template<typename Ctx>
	struct operation_base
	{
		using _notify_func_t = void (*)(operation_base *);

		operation_base(operation_base &&) = delete;
		operation_base &operator=(operation_base &&) = delete;

		constexpr operation_base() noexcept = default;

		void _notify() noexcept { std::exchange(_notify_func, {})(this); }

		_notify_func_t _notify_func = {};
		operation_base *_next = {};
	};
	template<typename Ctx>
	struct timer_node : operation_base<Ctx>
	{
		constexpr timer_node(Ctx &ctx, time_point timeout, bool can_stop) noexcept : _flags(can_stop ? stop_possible : 0), _timeout(timeout), _ctx(ctx) {}

		timer_node *_timer_prev = {};
		timer_node *_timer_next = {};
		std::atomic<int> _flags = {};
		time_point _timeout = {};
		Ctx &_ctx;
	};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

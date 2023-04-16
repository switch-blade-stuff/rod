/*
 * Created by switchblade on 2023-04-10.
 */

#pragma once

#include <functional>
#include <chrono>
#include <atomic>

#include "scheduling.hpp"

namespace rod
{
	/** Exception type thrown on failure to create a timer. */
	class timer_schedule_error
	{

	};

	namespace detail
	{
		template<typename Clock, typename F = void>
		class platform_timer;
		template<typename Clock, std::invocable F>
		class platform_timer<Clock, F> : platform_timer<Clock, void>, ebo_helper<F>
		{
			using node_base = platform_timer<Clock, void>;
			using ebo_base = ebo_helper<F>;

			static void expire_signal(void *ptr) noexcept
			{
				std::invoke(static_cast<platform_timer *>(ptr)->value());
				std::destroy_at(static_cast<platform_timer *>(ptr));
			}

		public:
			template<typename D>
			platform_timer(platform_timer *&base, std::chrono::time_point<Clock, D> tp, F &&f) : node_base(base, tp, expire_signal), ebo_base(std::forward<F>(f)) {}
			template<typename R, typename P>
			platform_timer(platform_timer *&base, std::chrono::duration<R, P> dur, F &&f) : node_base(base, dur, expire_signal), ebo_base(std::forward<F>(f)) {}
		};
	}
}

#include "detail/unix/timer.hpp"
#include "detail/win32/timer.hpp"

namespace rod
{
	/** @brief Scheduler type used to schedule time-bound execution of tasks using platform-specific timers.
	 *
	 * Internally, a timer maintains a handle to a platform-dependent timer and an instance of `run_loop`.
	 * When
	 *
	 * @tparam Clock Clock type used by the timer. */
	template<typename Clock>
	class timer
	{
		static_assert(std::chrono::is_clock_v<Clock>);
		using node_t = detail::platform_timer<Clock>;

	public:
		constexpr timer(timer &&other) noexcept
		{
			other.m_nodes->rebase(m_nodes);
		}

		/** Blocks until every scheduled task is completed. */
		void wait();

	private:
		void do_schedule_at()
		{

		}

		std::atomic<node_t *> m_nodes = nullptr;
	};

	/** Alias for timer execution context with `std::chrono::system_clock` clock type. */
	using system_timer = timer<std::chrono::system_clock>;
	/** Alias for timer execution context with `std::chrono::steady_clock` clock type. */
	using steady_timer = timer<std::chrono::steady_clock>;
}

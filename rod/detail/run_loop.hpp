/*
 * Created by switchblade on 2023-04-10.
 */

#pragma once

#include <condition_variable>
#include <chrono>

#include "../stop_token.hpp"

#include "queries/may_block.hpp"
#include "queries/scheduler.hpp"
#include "queries/progress.hpp"
#include "priority_queue.hpp"
#include "atomic_queue.hpp"
#include "tid_lock.hpp"

namespace rod
{
	namespace _run_loop
	{
		class operation_base;
		class scheduler;
		class run_loop;

		using clock = std::chrono::system_clock;
		using time_point = typename clock::time_point;

		template<typename, typename>
		struct operation { class type; };

		struct env { run_loop *_loop; };

		class operation_base
		{
			friend class run_loop;

		protected:
			using notify_func_t = void (*)(operation_base *) noexcept;

			constexpr operation_base(run_loop *loop, notify_func_t notify) noexcept : _notify_func(notify), _loop(loop) {}

			void notify() noexcept { std::exchange(_notify_func, nullptr)(this); }
			inline void start() noexcept;

			notify_func_t _notify_func;
			operation_base *_next = {};
			run_loop *_loop;
		};
		class timer_operation_base : public operation_base
		{
			friend class run_loop;

		protected:
			constexpr timer_operation_base(run_loop *loop, notify_func_t notify, time_point tp) noexcept : operation_base(loop, notify), _tp(tp) {}

			inline void start() noexcept;

		private:
			timer_operation_base *_timer_prev = {};
			timer_operation_base *_timer_next = {};
			time_point _tp;
		};
		template<typename F>
		struct callback_operation : operation_base, empty_base<F>
		{
			static void notify(operation_base *ptr) noexcept { static_cast<callback_operation *>(ptr)->value()(); }
			constexpr callback_operation(run_loop *loop, F &&func) noexcept : operation_base(loop, notify), empty_base<F>(std::forward<F>(func)) {}
		};

		template<typename Base, typename Rcv>
		class operation<Base, Rcv>::type : Base, empty_base<Rcv>
		{
			static void notify_complete(operation_base *p) noexcept
			{
				auto &rcv = *static_cast<type *>(p)->empty_base<Rcv>::get();
				if (rod::get_stop_token(get_env(rcv)).stop_requested())
					set_stopped(std::move(rcv));
				else
					set_value(std::move(rcv));
			}

		public:
			type() = delete;
			type(const type &) = delete;

			template<typename Rcv2, typename... Args>
			constexpr explicit type(run_loop *loop, Rcv2 &&rcv, Args &&...args) : Base(loop, notify_complete, std::forward<Args>(args)...), empty_base<Rcv>(std::forward<Rcv2>(rcv)) {}

			friend void tag_invoke(start_t, type &op) noexcept { op.Base::start(); }
		};

		template<typename OpBase, typename... Args>
		class sender : std::tuple<Args...>
		{
			friend class scheduler;

		public:
			using is_sender = std::true_type;

		private:
			template<typename Rcv>
			using operation_t = typename operation<OpBase, std::decay_t<Rcv>>::type;
			using signs_t = completion_signatures<set_value_t(), set_stopped_t()>;

			constexpr sender(run_loop *loop, Args ...args) noexcept : std::tuple<Args...>(std::move(args)...), _loop(loop) {}

		public:
			friend constexpr env tag_invoke(get_env_t, const sender &s) noexcept { return {s._loop}; }
			template<decays_to_same<sender> T, typename Env>
			friend constexpr signs_t tag_invoke(get_completion_signatures_t, T &&, Env) { return {}; }

			template<decays_to_same<sender> T, rod::receiver Rcv>
			friend constexpr operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv) noexcept(std::is_nothrow_move_constructible_v<Rcv> && (_detail::nothrow_decay_copyable<Args>::value && ...)) { return s.connect(std::forward<Rcv>(rcv)); }

		private:
			template<typename Rcv>
			[[nodiscard]] constexpr operation_t<Rcv> connect(Rcv &&rcv)
			{
				const auto factory = [&](auto &...args) { return operation_t<Rcv>(_loop, std::forward<Rcv>(rcv), std::move(args)...); };
				return std::apply(factory, static_cast<std::tuple<Args...> &>(*this));
			}

			run_loop *_loop;
		};

		class scheduler
		{
			using sender_t = sender<operation_base>;
			using timer_sender_t = sender<timer_operation_base, time_point>;

		public:
			constexpr scheduler(run_loop *loop) noexcept : _loop(loop) {}

			/** Returns the current time point of the clock used by the run loop. */
			[[nodiscard]] time_point now() const noexcept { return clock::now(); }

			[[nodiscard]] friend constexpr bool operator==(const scheduler &, const scheduler &) noexcept = default;

		public:
			friend constexpr bool tag_invoke(execute_may_block_caller_t, const scheduler &) noexcept { return true; }
			friend constexpr auto tag_invoke(get_forward_progress_guarantee_t, const scheduler &) noexcept { return forward_progress_guarantee::parallel; }

			template<decays_to_same<scheduler> T>
			friend constexpr sender_t tag_invoke(schedule_t, T &&s) noexcept { return s.schedule(); }
			template<decays_to_same<scheduler> T, typename Tp>
			friend constexpr timer_sender_t tag_invoke(schedule_at_t, T &&s, Tp &&tp) noexcept { return s.schedule_at(std::forward<Tp>(tp)); }
			template<decays_to_same<scheduler> T, typename Dur>
			friend constexpr timer_sender_t tag_invoke(schedule_after_t, T &&s, Dur &&dur) noexcept { return s.schedule_at(s.now() + dur); }

		private:
			auto schedule() noexcept { return sender_t{_loop}; }
			template<typename Tp> requires decays_to_same<Tp, time_point>
			auto schedule_at(Tp &&tp) noexcept { return timer_sender_t{_loop, std::forward<Tp>(tp)}; }
			template<typename Tp> requires(!decays_to_same<Tp, time_point>)
			auto schedule_at(Tp &&tp) noexcept { return timer_sender_t{_loop, clock::now() + tp.time_since_epoch()}; }

			run_loop *_loop;
		};

		/** Generic execution context used to schedule work on a FIFO queue. */
		class run_loop
		{
			friend class operation_base;
			friend class timer_operation_base;

		public:
			using time_point = _run_loop::time_point;
			using clock = _run_loop::clock;

		private:
			struct timer_cmp { constexpr bool operator()(const timer_operation_base &a, const timer_operation_base &b) const noexcept { return a._tp <= b._tp; }};
			using timer_queue_t = _detail::priority_queue<timer_operation_base, timer_cmp, &timer_operation_base::_timer_prev, &timer_operation_base::_timer_next>;
			using task_queue_t = _detail::basic_queue<operation_base, &operation_base::_next>;

		public:
			run_loop(run_loop &&) = delete;
			run_loop(const run_loop &) = delete;

			run_loop() noexcept { _active.test_and_set(std::memory_order_relaxed); }

			/** Returns a scheduler used to schedule operations and timers to be executed by the run loop. */
			[[nodiscard]] constexpr scheduler get_scheduler() noexcept { return {this}; }

			/** Returns reference to the stop source associated with the run loop. */
			[[nodiscard]] constexpr in_place_stop_source &get_stop_source() noexcept { return _stop_src; }
			/** Returns a stop token of the stop source associated with the run loop. */
			[[nodiscard]] constexpr in_place_stop_token get_stop_token() const noexcept { return _stop_src.get_token(); }

			/** Returns `true` if the run loop is active, and `false` if `finish` has been called. */
			[[nodiscard]] bool active() const noexcept { return _active.test(std::memory_order_acquire); }

			/** Dispatches scheduled operations from the consumer queue.
			 * @return Amount of scheduled operations dispatched.
			 * @note Returned number might not be the same as the one returned by `poll`. */
			ROD_API_PUBLIC std::size_t run_once();
			/** Acquires pending operations and elapsed timers, and schedules them to the consumer queue. Blocks if there are no pending operations or timers and \a block is equal to `true`.
			 * @param block If set to `true` blocks the thread until new operations or timers are scheduled to the consumer queue, or the run loop is terminated via `finish`.
			 * @return Amount of operations and timers scheduled to the consumer queue to be executed on next call to `run_once`. */
			ROD_API_PUBLIC std::size_t poll(bool block = true);

			/** Blocks the current thread until `finish` is called and executes scheduled operations and timers. */
			void run() { run(never_stop_token{}); }
			/** Blocks the current thread until `finish` is called or stopped via \a tok and executes scheduled operations and timers.
			 * @param token Stop token used to stop execution of the event loop. */
			template<stoppable_token Token>
			void run(Token &&token)
			{
				auto flag = std::atomic_flag();
				auto op = callback_operation(this, [&flag] { flag.test_and_set(std::memory_order_release); });
				auto cb = _detail::make_stop_callback(std::forward<Token>(token), [&op]() { op._loop->schedule(&op); });

				for (std::size_t pending = 0;;)
				{
					if (pending != 0) [[likely]]
					{
						if (const auto n = run_once(); n <= pending)
							pending -= n;
						else
							pending = 0;
					}
					if (!flag.test(std::memory_order_acquire) && active())
						pending += poll(pending == 0);
					else
						break;
				}
			}

			/** Changes the internal state to stopped and unblocks waiting threads. Any in-progress work will run to completion. */
			void finish() { _active.clear(std::memory_order_release), _cnd.notify_all(); }
			/** Sends a stop request to the stop source associated with the run loop. */
			void request_stop() { _stop_src.request_stop(); }

		private:
			[[nodiscard]] bool is_consumer_thread() const noexcept { return _consumer_lock.tid.load(std::memory_order_acquire) == std::this_thread::get_id(); }
			[[nodiscard]] bool has_pending() const noexcept { return !_consumer_queue.empty() || !_producer_queue.empty(); }

			ROD_API_PUBLIC void schedule(operation_base *node);
			ROD_API_PUBLIC void schedule_timer(timer_operation_base *node);

			void acquire_producer_queue() noexcept;
			void acquire_elapsed_timers() noexcept;

			mutable std::mutex _mtx;
			std::condition_variable _cnd;
			_detail::tid_lock _consumer_lock;

			std::atomic_flag _active;
			in_place_stop_source _stop_src;

			task_queue_t _consumer_queue;
			task_queue_t _producer_queue;
			timer_queue_t _timer_queue;
			time_point _next_timeout;
		};

		constexpr in_place_stop_token tag_invoke(get_stop_token_t, const env &s) noexcept { return s._loop->get_stop_token(); }
		template<typename T>
		constexpr scheduler tag_invoke(get_completion_scheduler_t<T>, const env &e) noexcept { return e._loop->get_scheduler(); }

		void operation_base::start() noexcept { _loop->schedule(this); }
		void timer_operation_base::start() noexcept { _loop->schedule_timer(this); }
	}

	using _run_loop::run_loop;

	static_assert(scheduler<decltype(std::declval<run_loop>().get_scheduler())>);
}

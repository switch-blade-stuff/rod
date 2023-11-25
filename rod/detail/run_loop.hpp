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
			using task_queue_t = _detail::basic_queue<operation_base, &operation_base::_next>;

		public:
			/** Structure containing scoped thread context used for iterative execution and synchronization. */
			class thread_context
			{
				friend class run_loop;

			public:
				/** Initializes thread context for run loop \a loop. */
				thread_context(run_loop &loop) : _loop(&loop) { _loop->push_ctx(this); }
				~thread_context() { _loop->pop_ctx(this); }

				/** Unblocks and stops the parent thread of this context. Does not stop other threads or the run loop itself unlike `run_loop::finish`. */
				void finish() noexcept
				{
					/* notify_all has to be used since all threads are waiting on the same condition variable. */
					_is_stopped.test_and_set();
					_loop->_cnd.notify_all();
				}
				/** Returns `true` if the thread context is active, and `false` if `finish` has been called and the thread should terminate it's event loop.
				 * @note Individual contexts are not stopped when `run_loop::finish` is called. */
				[[nodiscard]] bool active() const noexcept { return !_is_stopped.test(); }

			private:
				thread_context *_next_node = nullptr;
				thread_context **_this_ptr = nullptr;
				std::atomic_flag _is_stopped;

				task_queue_t _local_queue;
				run_loop *_loop;
			};

		private:
			struct timer_cmp { constexpr bool operator()(const timer_operation_base &a, const timer_operation_base &b) const noexcept { return a._tp <= b._tp; }};
			using timer_queue_t = _detail::priority_queue<timer_operation_base, timer_cmp, &timer_operation_base::_timer_prev, &timer_operation_base::_timer_next>;

		public:
			run_loop(run_loop &&) = delete;
			run_loop(const run_loop &) = delete;

			run_loop() noexcept = default;

			/** Returns a scheduler used to schedule operations and timers to be executed by the run loop. */
			[[nodiscard]] constexpr scheduler get_scheduler() noexcept { return {this}; }
			/** Returns copy of the stop source associated with the run loop. */
			[[nodiscard]] constexpr in_place_stop_source &get_stop_source() noexcept { return _stop_src; }
			/** Returns a stop token of the stop source associated with the run loop. */
			[[nodiscard]] constexpr in_place_stop_token get_stop_token() const noexcept { return _stop_src.get_token(); }

			/** Returns `true` if the run loop is active, and `false` if `run_loop::finish` has been called. */
			[[nodiscard]] bool active() const noexcept { return !_is_stopped.test(); }
			/** Returns `true` if the run loop has does not have any pending scheduled operations or timers. */
			[[nodiscard]] bool empty() const noexcept { return ((void) std::unique_lock(_mtx), _task_queue.empty() || _timer_queue.empty()); }

			/** Dispatches scheduled operations of context `ctx`.
			 * @return Amount of operations executed. */
			ROD_API_PUBLIC std::size_t run_once(thread_context &ctx);
			/** Acquires pending operations and elapsed timers for context `ctx` and blocks if the thread context queue is empty and `block` is equal to `false`.
			 * @return Amount of operations scheduled to execute on next call to `run_once`.
			 * @note Associated thread context must be locked to avoid thread contention. */
			ROD_API_PUBLIC std::size_t poll(thread_context &ctx, bool block = true);

			/** Blocks the current thread until `finish` is called and executes scheduled operations and timers. */
			void run()
			{
				auto ctx = thread_context(*this);
				run_ctx(ctx);
			}
			/** Blocks the current thread until `finish` is called or stopped via \a tok and executes scheduled operations and timers.
			 * @param token Stop token used to stop execution of the event loop. */
			template<stoppable_token Token>
			void run(Token &&token)
			{
				auto ctx = thread_context(*this);
				auto cb = _detail::make_stop_callback(std::forward<Token>(token), [&ctx]() noexcept { ctx.finish(); });
				run_ctx(ctx);
			}

			/** Changes the internal state to stopped and unblocks waiting threads. Any in-progress work will run to completion. */
			void finish()
			{
				_is_stopped.test_and_set();
				_cnd.notify_all();
			}
			/** Sends a stop request to the stop source associated with the run loop. */
			void request_stop() { _stop_src.request_stop(); }

		private:
			ROD_API_PUBLIC void run_ctx(thread_context &ctx);
			ROD_API_PUBLIC void pop_ctx(thread_context *ctx);
			ROD_API_PUBLIC void push_ctx(thread_context *ctx);

			ROD_API_PUBLIC void schedule(operation_base *node);
			ROD_API_PUBLIC void schedule_timer(timer_operation_base *node);

			mutable std::mutex _mtx;
			std::condition_variable _cnd;
			std::atomic_flag _is_stopped;

			in_place_stop_source _stop_src;
			thread_context *_ctx_queue = {};
			timer_queue_t _timer_queue = {};
			task_queue_t _task_queue = {};
			time_point _next_timeout = {};
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

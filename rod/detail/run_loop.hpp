/*
 * Created by switchblade on 2023-04-10.
 */

#pragma once

#include <condition_variable>
#include <chrono>

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
			template<decays_to<sender> T, typename Env>
			friend constexpr signs_t tag_invoke(get_completion_signatures_t, T &&, Env) { return {}; }

			template<decays_to<sender> T, rod::receiver Rcv>
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

			template<decays_to<scheduler> T>
			friend constexpr sender_t tag_invoke(schedule_t, T &&s) noexcept { return s.schedule(); }
			template<decays_to<scheduler> T, typename Tp>
			friend constexpr timer_sender_t tag_invoke(schedule_at_t, T &&s, Tp &&tp) noexcept { return s.schedule_at(std::forward<Tp>(tp)); }
			template<decays_to<scheduler> T, typename Dur>
			friend constexpr timer_sender_t tag_invoke(schedule_after_t, T &&s, Dur &&dur) noexcept { return s.schedule_at(s.now() + dur); }

		private:
			auto schedule() noexcept { return sender_t{_loop}; }
			template<typename Tp> requires decays_to<Tp, time_point>
			auto schedule_at(Tp &&tp) noexcept { return timer_sender_t{_loop, std::forward<Tp>(tp)}; }
			template<typename Tp> requires(!decays_to<Tp, time_point>)
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

			run_loop() noexcept = default;

			/** Returns a scheduler used to schedule work to be executed on the run loop. */
			[[nodiscard]] constexpr scheduler get_scheduler() noexcept { return {this}; }

			/** Blocks the current thread until `finish` is called and executes scheduled operations. */
			void run()
			{
				for (task_queue_t local_queue;;)
				{
					while (!local_queue.empty())
					{
						auto *node = local_queue.pop_front();
						node->_notify_func(node);
					}

					auto g = std::unique_lock{_mtx};

					/* Acquire tasks from the producer queue. */
					while (!_task_queue.empty())
					{
						auto *node = _task_queue.pop_front();
						local_queue.push_back(node);
					}

					/* Dispatch elapsed timers. */
					if (!_timer_queue.empty())
						for (const auto now = clock::now(); !_timer_queue.empty();)
						{
							if (_timer_queue.front()->_tp > now)
							{
								_next_timeout = _timer_queue.front()->_tp;
								break;
							}
							auto *node = _timer_queue.pop_front();
							local_queue.push_back(node);
						}

					/* Do not wait if we have tasks to dispatch. */
					if (!local_queue.empty())
						continue;
					else if (_is_stopping && is_idle())
						return;

					if (_timer_queue.empty())
						_cnd.wait(g, [&]() { return !is_idle() || _is_stopping; });
					else
						_cnd.wait_until(g, _next_timeout);
				}
			}
			/** Blocks the current thread until stopped via \a tok and executes scheduled operations.
			 * @param tok Stop token used to stop execution of the event loop. */
			template<stoppable_token Tok>
			void run(Tok &&tok)
			{
				const auto do_stop = [&]() { finish(); };
				const auto cb = stop_callback_for_t<Tok, decltype(do_stop)>{std::forward<Tok>(tok), do_stop};
				run();
			}

			/** Changes the internal state to stopped and unblocks waiting threads. Any in-progress work will run to completion. */
			void finish()
			{
				const auto g = std::unique_lock{_mtx};
				_is_stopping = true;
				_cnd.notify_all();
			}

			/** Returns copy of the stop source associated with the run loop. */
			[[nodiscard]] constexpr in_place_stop_source &get_stop_source() noexcept { return _stop_src; }
			/** Returns a stop token of the stop source associated with the run loop. */
			[[nodiscard]] constexpr in_place_stop_token get_stop_token() const noexcept { return _stop_src.get_token(); }
			/** Sends a stop request to the stop source associated with the run loop. */
			void request_stop() { _stop_src.request_stop(); }

		private:
			constexpr bool is_idle() const noexcept { return _timer_queue.empty() && _task_queue.empty(); }

			void schedule(operation_base *node) noexcept
			{
				const auto g = std::unique_lock{_mtx};
				_task_queue.push_back(node);
				_cnd.notify_one();
			}
			void schedule_timer(timer_operation_base *node) noexcept
			{
				const auto g = std::unique_lock{_mtx};
				_timer_queue.insert(node);
				_cnd.notify_one();
			}

			std::mutex _mtx;
			std::condition_variable _cnd;

			in_place_stop_source _stop_src;
			timer_queue_t _timer_queue;
			task_queue_t _task_queue;
			time_point _next_timeout;
			bool _is_stopping = {};
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

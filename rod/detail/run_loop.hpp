/*
 * Created by switchblade on 2023-04-10.
 */

#pragma once

#include <condition_variable>
#include <chrono>

#include "queries/may_block.hpp"
#include "queries/scheduler.hpp"
#include "queries/progress.hpp"
#include "../stop_token.hpp"
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

		template<typename>
		struct timer_operation { class type; };
		template<typename>
		struct operation { class type; };
		class timer_sender;
		class sender;

		struct env { run_loop *_loop; };

		class operation_base
		{
			friend class run_loop;

			using notify_func_t = void (*)(operation_base *, time_point) noexcept;

		protected:
			constexpr operation_base(run_loop *loop, notify_func_t notify = {}) noexcept : _notify_func(notify), _loop(loop) {}

		private:
			void _notify(time_point now) noexcept { _notify_func(this, now); }

		protected:
			notify_func_t _notify_func;
			operation_base *_next = {};
			run_loop *_loop;
		};

		template<typename R>
		class operation<R>::type : operation_base
		{
			friend sender;

			static void notify_complete(operation_base *p, time_point) noexcept
			{
				auto &rcv = static_cast<type *>(p)->_rcv;
				if (rod::get_stop_token(get_env(rcv)).stop_requested())
					set_stopped(std::move(rcv));
				else
					set_value(std::move(rcv));
			}

			constexpr type(run_loop *loop, R rcv) noexcept(std::is_nothrow_move_constructible_v<R>) : operation_base(loop, notify_complete), _rcv(std::move(rcv)) {}

		public:
			type() = delete;
			type(const type &) = delete;

			friend void tag_invoke(start_t, type &op) noexcept { op.start(); }

		private:
			inline void start() noexcept;

			ROD_NO_UNIQUE_ADDRESS R _rcv;
		};
		template<typename R>
		class timer_operation<R>::type : operation_base
		{
			friend timer_sender;

			static void notify_complete(operation_base *p, time_point now) noexcept { static_cast<type *>(p)->complete(now); }
			constexpr type(run_loop *loop, time_point timeout, R rcv) noexcept(std::is_nothrow_move_constructible_v<R>) : operation_base(loop, notify_complete), _rcv(std::move(rcv)), _timeout(timeout) {}

		public:
			type() = delete;
			type(const type &) = delete;

			friend void tag_invoke(start_t, type &op) noexcept { op.start(); }

		private:
			inline void start() noexcept;
			inline void complete(time_point now) noexcept;

			ROD_NO_UNIQUE_ADDRESS R _rcv;
			time_point _timeout;
		};

		class sender
		{
			friend scheduler;

		public:
			using is_sender = std::true_type;

		private:
			using signs_t = completion_signatures<set_value_t(), set_stopped_t()>;
			template<typename R>
			using operation_t = typename operation<std::decay_t<R>>::type;

			constexpr sender(run_loop *loop) noexcept : _loop(loop) {}

		public:
			friend constexpr env tag_invoke(get_env_t, const sender &s) noexcept { return {s._loop}; }
			template<decays_to<sender> T, typename E>
			friend constexpr signs_t tag_invoke(get_completion_signatures_t, T &&, E) { return {}; }

			template<decays_to<sender> T, typename Rcv>
			friend constexpr operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv) noexcept(detail::nothrow_decay_copyable<Rcv>::value) { return s.connect(std::forward<Rcv>(rcv)); }

		private:
			template<typename Rcv>
			operation_t<Rcv> connect(Rcv &&rcv) { return {_loop, std::forward<Rcv>(rcv)}; }

			run_loop *_loop;
		};
		class timer_sender
		{
			friend scheduler;

		public:
			using is_sender = std::true_type;

		private:
			using signs_t = completion_signatures<set_value_t(), set_stopped_t()>;
			template<typename R>
			using operation_t = typename timer_operation<std::decay_t<R>>::type;

			constexpr timer_sender(run_loop *loop, time_point timeout) noexcept : _timeout(timeout), _loop(loop) {}

		public:
			friend constexpr env tag_invoke(get_env_t, const timer_sender &s) noexcept { return {s._loop}; }
			template<decays_to<timer_sender> T, typename E>
			friend constexpr signs_t tag_invoke(get_completion_signatures_t, T &&, E) { return {}; }

			template<decays_to<timer_sender> T, typename Rcv>
			friend constexpr operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv) noexcept(detail::nothrow_decay_copyable<Rcv>::value) { return s.connect(std::forward<Rcv>(rcv)); }

		private:
			template<typename Rcv>
			operation_t<Rcv> connect(Rcv &&rcv) { return {_loop, _timeout, std::forward<Rcv>(rcv)}; }

			time_point _timeout;
			run_loop *_loop;
		};

		class scheduler
		{
		public:
			constexpr scheduler(run_loop *loop) noexcept : _loop(loop) {}

			/** Returns the current time point of the clock used by the run loop. */
			[[nodiscard]] time_point now() const noexcept { return clock::now(); }

			constexpr bool operator==(const scheduler &) const noexcept = default;

			friend constexpr bool tag_invoke(execute_may_block_caller_t, const scheduler &) noexcept { return true; }
			friend constexpr auto tag_invoke(get_forward_progress_guarantee_t, const scheduler &) noexcept { return forward_progress_guarantee::parallel; }

			template<decays_to<scheduler> T>
			friend constexpr sender tag_invoke(schedule_t, T &&s) noexcept { return s.schedule(); }
			template<decays_to<scheduler> T, decays_to<time_point> Tp>
			friend constexpr timer_sender tag_invoke(schedule_at_t, T &&s, Tp &&tp) noexcept { return s.schedule_at(std::forward<Tp>(tp)); }
			template<decays_to<scheduler> T, typename Dur>
			friend constexpr timer_sender tag_invoke(schedule_after_t, T &&s, Dur &&dur) noexcept { return s.schedule_at(std::forward<T>(s), s.now() + dur); }

		private:
			auto schedule() noexcept { return sender{_loop}; }
			auto schedule_at(time_point tp) noexcept { return timer_sender{_loop, tp}; }

			run_loop *_loop;
		};

		/** Generic execution context used to schedule work on a FIFO queue. */
		class run_loop
		{
			template<typename>
			friend struct _run_loop::operation;
			template<typename>
			friend struct _run_loop::timer_operation;

		public:
			using time_point = _run_loop::time_point;
			using clock = _run_loop::clock;

		private:
			using producer_queue_t = detail::atomic_queue<operation_base, &operation_base::_next>;
			using consumer_queue_t = detail::basic_queue<operation_base, &operation_base::_next>;

		public:
			run_loop(run_loop &&) = delete;
			run_loop(const run_loop &) = delete;

			run_loop() noexcept = default;

			/** Returns a scheduler used to schedule work to be executed on the run loop. */
			[[nodiscard]] constexpr scheduler get_scheduler() noexcept { return {this}; }

			/** Blocks the current thread until `finish` is called and executes scheduled operations. */
			void run()
			{
				for (consumer_queue_t consumer_queue;;)
				{
					time_point now;
					if (_timers.load(std::memory_order_acquire))
						now = clock::now();
					while (!consumer_queue.empty())
						consumer_queue.pop_front()->_notify(now);

					if (const auto front = _producer_queue.front(); !front)
						_producer_queue.wait();
					else if (front == _producer_queue.sentinel())
						break;
					else
					{
						consumer_queue = std::move(_producer_queue);
						_producer_queue.notify_all();
					}
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
				_producer_queue.terminate();
				_producer_queue.notify_all();
			}

			/** Returns copy of the stop source associated with the run loop. */
			[[nodiscard]] constexpr in_place_stop_source &get_stop_source() noexcept { return _stop_source; }
			/** Returns a stop token of the stop source associated with the run loop. */
			[[nodiscard]] constexpr in_place_stop_token get_stop_token() const noexcept { return _stop_source.get_token(); }
			/** Sends a stop request to the stop source associated with the run loop. */
			void request_stop() { _stop_source.request_stop(); }

		private:
			void schedule(operation_base *node) noexcept
			{
				_producer_queue.push(node);
				_producer_queue.notify_one();
			}

			in_place_stop_source _stop_source = {};
			producer_queue_t _producer_queue = {};
			std::atomic<std::size_t> _timers = {};
		};

		constexpr in_place_stop_token tag_invoke(get_stop_token_t, const env &s) noexcept { return s._loop->get_stop_token(); }
		template<typename T>
		constexpr scheduler tag_invoke(get_completion_scheduler_t<T>, const env &e) noexcept { return e._loop->get_scheduler(); }

		template<typename R>
		void operation<R>::type::start() noexcept { _loop->schedule(this); }
		template<typename R>
		void timer_operation<R>::type::start() noexcept
		{
			_loop->_timers.fetch_add(1, std::memory_order_acq_rel);
			_loop->schedule(this);
		}
		template<typename R>
		void timer_operation<R>::type::complete(time_point now) noexcept
		{
			if (rod::get_stop_token(get_env(_rcv)).stop_requested())
			{
				_loop->_timers.fetch_sub(1, std::memory_order_acq_rel);
				set_stopped(std::move(_rcv));
			}
			else if (now >= _timeout)
			{
				_loop->_timers.fetch_sub(1, std::memory_order_acq_rel);
				set_value(std::move(_rcv));
			}
			else
				_loop->schedule(this);
		}
	}

	using _run_loop::run_loop;

	static_assert(scheduler<decltype(std::declval<run_loop>().get_scheduler())>);
}

/*
 * Created by switchblade on 2023-04-10.
 */

#pragma once

#include <condition_variable>

#include "queries/may_block.hpp"
#include "queries/scheduler.hpp"
#include "queries/progress.hpp"
#include "../stop_token.hpp"
#include "atomic_queue.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	namespace _run_loop
	{
		class run_loop;

		class sender;
		class scheduler;
		template<typename = void>
		class operation;

		template<>
		class operation<void>
		{
		public:
			constexpr operation(run_loop *loop, void (*invoke)(operation *) noexcept) noexcept : invoke_func(invoke), loop(loop) {}

			void invoke() noexcept { invoke_func(this); }

			void (*invoke_func)(operation *) noexcept;
			union
			{
				operation *next;
				run_loop *loop;
			};
		};
		template<typename R>
		class operation : operation<>
		{
			friend class sender;

			static void invoke(operation<> *p) noexcept
			{
				auto &rcv = static_cast<operation<R> *>(p)->m_rcv;
				detail::rcv_try_invoke(std::move(rcv), [&rcv]()
				{
					if (rod::get_stop_token(get_env(rcv)).stop_requested())
						set_stopped(std::move(rcv));
					else
						set_value(std::move(rcv));
				});
			}

			constexpr operation(run_loop *loop, R &&rcv) noexcept(std::is_nothrow_move_constructible_v<R>) : operation<>(loop, invoke), m_rcv(std::forward<R>(rcv)) {}

		public:
			operation() = delete;
			operation(const operation &) = delete;

			friend void tag_invoke(start_t, operation &t) noexcept { t.start(); }

		private:
			inline void start() noexcept;

			[[ROD_NO_UNIQUE_ADDRESS]] R m_rcv;
		};

		class sender
		{
			friend class scheduler;

			using signs = completion_signatures<set_value_t(), set_error_t(std::exception_ptr), set_stopped_t()>;
			template<typename R>
			using operation_t = operation<std::remove_cvref_t<R>>;

			class env
			{
				friend class sender;

				constexpr env(run_loop *loop) noexcept : m_loop(loop) {}

			public:
				template<typename T>
				friend constexpr auto tag_invoke(get_completion_scheduler_t<T>, const env &e) noexcept { return e.get_scheduler<T>(); }

			private:
				template<typename T>
				[[nodiscard]] constexpr scheduler get_scheduler() const noexcept;

				run_loop *m_loop;
			};

		public:
			using is_sender = std::true_type;

		private:
			constexpr sender(run_loop *loop) noexcept : m_loop(loop) {}

		public:
			friend constexpr env tag_invoke(get_env_t, const sender &s) noexcept { return s.get_env(); }
			template<detail::decays_to<sender> T, typename E>
			friend constexpr signs tag_invoke(get_completion_signatures_t, T &&, E) { return {}; }

			template<typename R>
			friend constexpr operation_t<R> tag_invoke(connect_t, sender &s, R &&rcv) noexcept(std::is_nothrow_constructible_v<operation_t<R>, R>) { return {s.m_loop, std::forward<R>(rcv)};; }

		private:
			[[nodiscard]] constexpr env get_env() const noexcept { return {m_loop}; }

			run_loop *m_loop;
		};
		class scheduler
		{
			friend class run_loop;

			constexpr scheduler(run_loop *loop) noexcept : m_loop(loop) {}

		public:
			friend constexpr auto tag_invoke(get_forward_progress_guarantee_t, const scheduler &) noexcept { return forward_progress_guarantee::parallel; }
			friend constexpr bool tag_invoke(execute_may_block_caller_t, const scheduler &) noexcept { return true; }

			friend constexpr in_place_stop_token tag_invoke(get_stop_token_t, const scheduler &s) noexcept { return s.get_stop_token(); }
			friend constexpr sender tag_invoke(schedule_t, const scheduler &s) noexcept { return s.schedule(); }

			constexpr bool operator==(const scheduler &) const noexcept = default;

		private:
			[[nodiscard]] constexpr in_place_stop_token get_stop_token() const noexcept;
			[[nodiscard]] constexpr sender schedule() const noexcept { return {m_loop}; }

			run_loop *m_loop;
		};

		/** Generic execution context used to schedule work on a FIFO queue. */
		class run_loop
		{
			template<typename>
			friend class _run_loop::operation;

		public:
			run_loop(run_loop &&) = delete;
			run_loop(const run_loop &) = delete;

			run_loop() noexcept = default;

			/** Returns a scheduler used to schedule work to be executed on the run loop. */
			[[nodiscard]] constexpr scheduler get_scheduler() noexcept { return {this}; }

			/** Repeatedly blocks until the run loop is stopped or any work is available. */
			void run() { run_impl(true); }
			/** Blocks until the run loop is stopped or any work is available.
			 * @return `true` if a task has been executed, `false` if the run loop was stopped. */
			bool run_one() { return run_one_impl(true); }

			/** Runs all currently available tasks until the queue is empty or the run loop is stopped without blocking.
			 * @return Total number of tasks executed. */
			void poll() { run_impl(false); }
			/** Runs a single available task and returns without blocking.
			 * @return `true` if a task has been executed, `false` if the queue was empty or stopped. */
			bool poll_one() { return run_one_impl(false); }

			/** Changes the internal state to stopped and unblocks waiting threads. Any in-progress work will run to completion. */
			void finish() { m_queue.terminate(); }

			/** Returns copy of the stop source associated with the run loop. */
			[[nodiscard]] constexpr in_place_stop_source &get_stop_source() noexcept { return m_stop_source; }
			/** Returns a stop token of the stop source associated with the run loop. */
			[[nodiscard]] constexpr in_place_stop_token get_stop_token() const noexcept { return m_stop_source.get_token(); }
			/** Sends a stop request to the stop source associated with the run loop. */
			void request_stop() { m_stop_source.request_stop(); }

		private:
			void run_impl(bool block)
			{
				while (auto node = m_queue.pop(block))
					node->invoke();
			}
			bool run_one_impl(bool block)
			{
				if (auto node = m_queue.pop(block); node)
					return (node->invoke(), true);
				else
					return false;
			}

			detail::atomic_queue<operation<>, &operation<>::next> m_queue;
			in_place_stop_source m_stop_source;
		};

		template<typename R>
		void operation<R>::start() noexcept try { loop->m_queue.push(this); } catch (...) { set_error(std::move(m_rcv), std::current_exception()); }

		template<typename T>
		constexpr scheduler sender::env::get_scheduler() const noexcept { return m_loop->get_scheduler(); }
		constexpr in_place_stop_token scheduler::get_stop_token() const noexcept { return m_loop->get_stop_token(); }
	}

	using _run_loop::run_loop;

	static_assert(scheduler<decltype(std::declval<run_loop>().get_scheduler())>);
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

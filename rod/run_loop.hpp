/*
 * Created by switchblade on 2023-04-10.
 */

#pragma once

#include <condition_variable>

#include "detail/atomic_queue.hpp"
#include "scheduling.hpp"

namespace rod
{
	/** Generic execution context used to schedule work on a FIFO queue. */
	class run_loop
	{
		class sender;
		class scheduler;
		template<typename = void>
		class operation;

		template<typename>
		friend class operation;

		template<>
		class operation<void>
		{
		public:
			constexpr operation(run_loop *loop, void (*invoke)(operation *) noexcept) noexcept : loop(loop), invoke_func(invoke) {}

			friend constexpr void tag_invoke(detail::next_t, operation &node, operation *ptr) noexcept { node.next = ptr; }
			friend constexpr auto tag_invoke(detail::next_t, operation &node) noexcept { return node.next; }

			void invoke() noexcept { invoke_func(this); }

			union
			{
				operation *next;
				run_loop *loop;
			};
			void (*invoke_func)(operation *) noexcept;
		};
		template<typename R>
		class operation : operation<>, detail::ebo_helper<R>
		{
			friend class sender;

			static void invoke(operation<> *p) noexcept
			{
				rcv_try_invoke(std::move(static_cast<operation<R> *>(p)->receiver()), [](R &&rcv)
				{
					if (rod::get_stop_token(get_env(rcv)).stop_requested())
						set_stopped(std::forward<R>(rcv));
					else
						set_value(std::forward<R>(rcv));
				});
			}

			constexpr operation(run_loop *loop, R &&rcv) : operation<>(loop, invoke), detail::ebo_helper<R>(std::forward<R>(rcv)) {}

		public:
			operation() = delete;
			operation(const operation &) = delete;

			friend void tag_invoke(start_t, operation &t) noexcept
			{
				try { t.loop->push_node(t); }
				catch (...) { set_error(std::move(t.receiver()), std::current_exception()); }
			}

		private:
			[[nodiscard]] constexpr auto &receiver() noexcept { return detail::ebo_helper<R>::value(); }
		};

		class sender
		{
			friend class scheduler;

			class env
			{
				friend class sender;

				constexpr env(run_loop *loop) noexcept : m_loop(loop) {}

			public:
				template<typename T>
				[[nodiscard]] friend scheduler tag_invoke(get_completion_scheduler_t<T>, const env &e) noexcept { return e.m_loop->get_scheduler(); }

			private:
				run_loop *m_loop;
			};

		public:
			using completion_signatures = completion_signatures<set_value_t(), set_error_t(std::exception_ptr), set_stopped_t()>;

		private:
			sender(run_loop *loop) noexcept : m_loop(loop) {}

		public:
			template<typename R>
			[[nodiscard]] friend operation<std::remove_cvref_t<R>> tag_invoke(connect_t, sender &s, R &&rcv) { return s.connect(std::forward<R>(rcv)); }
			[[nodiscard]] friend env tag_invoke(get_env_t, const sender &s) noexcept { return {s.m_loop}; }

		private:
			template<typename R>
			[[nodiscard]] operation<std::remove_cvref_t<R>> connect(R &&rcv) { return {m_loop, std::forward<R>(rcv)}; }

			run_loop *m_loop;
		};
		class scheduler
		{
			friend class run_loop;

			scheduler(run_loop *loop) noexcept : m_loop(loop) {}

		public:
			friend constexpr auto tag_invoke(get_forward_progress_guarantee_t, const scheduler &) noexcept { return forward_progress_guarantee::parallel; }
			friend constexpr bool tag_invoke(execute_may_block_caller_t, const scheduler &) noexcept { return true; }

			[[nodiscard]] friend in_place_stop_token tag_invoke(get_stop_token_t, const scheduler &s) noexcept { return s.get_stop_token(); }
			[[nodiscard]] friend sender tag_invoke(schedule_t, const scheduler &s) noexcept { return s.schedule(); }

			constexpr bool operator==(const scheduler &) const noexcept = default;

		private:
			[[nodiscard]] in_place_stop_token get_stop_token() const { return m_loop->get_stop_token(); }
			[[nodiscard]] sender schedule() const noexcept { return {m_loop}; }

			run_loop *m_loop;
		};

	public:
		run_loop(run_loop &&) = delete;
		run_loop(const run_loop &) = delete;

		run_loop() noexcept = default;

		/** Returns a scheduler used to schedule work to be executed on the run loop. */
		[[nodiscard]] scheduler get_scheduler() noexcept { return {this}; }

		/** Blocks until work is available (or finish is called), then runs all scheduled work until the queue is empty or the run loop is cancelled. */
		void run() { while (auto node = pop_node()) node->invoke(); }
		/** Changes the internal state to stopped. Any in-progress work will run to completion. */
		void finish() { m_queue.terminate(); }

		/** Returns copy of the stop source associated with the run loop. */
		[[nodiscard]] in_place_stop_source &get_stop_source() { return m_stop_source; }
		/** Returns a stop token of the stop source associated with the run loop. */
		[[nodiscard]] in_place_stop_token get_stop_token() const { return m_stop_source.get_token(); }
		/** Sends a stop request to the stop source associated with the run loop. */
		void request_stop() { m_stop_source.request_stop(); }

	private:
		void push_node(operation<> &node) { m_queue.push(&node); }
		[[nodiscard]] operation<> *pop_node() { return m_queue.pop(); }

		in_place_stop_source m_stop_source;
		detail::atomic_queue<operation<>> m_queue;
	};

	static_assert(scheduler<decltype(std::declval<run_loop>().get_scheduler())>);
}

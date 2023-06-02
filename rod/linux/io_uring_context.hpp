/*
 * Created by switchblade on 2023-05-29.
 */

#pragma once

#include "detail/context_fwd.hpp"

#ifdef ROD_HAS_LIBURING

#include <liburing.h>

#include "../unix/detail/mmap.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	namespace _io_uring
	{
		using namespace _system_ctx;

		struct scheduler;
		class context;

		class context
		{
			struct timer_cmp { constexpr bool operator()(const timer_node<context> &a, const timer_node<context> &b) const noexcept { return a._timeout <= b._timeout; }};
			using timer_queue_t = detail::priority_queue<timer_node<context>, timer_cmp, &timer_node<context>::_timer_prev, &timer_node<context>::_timer_next>;
			using producer_queue_t = detail::atomic_queue<operation_base<context>, &operation_base<context>::_next>;
			using consumer_queue_t = detail::basic_queue<operation_base<context>, &operation_base<context>::_next>;

			struct queue_state_t
			{
				std::uint32_t size;
				std::uint32_t mask;
				std::uint32_t pending;
			};
			struct cq_state_t : queue_state_t
			{
				unsigned *head = {};
				unsigned *tail = {};
				unsigned *overflow = {};
				io_uring_cqe *entries;
			};
			struct sq_state_t : queue_state_t
			{
				unsigned *head;
				unsigned *tail;
				unsigned *flags;
				unsigned *dropped;
				unsigned *idx_data;
				io_uring_sqe *entries;
			};

		public:
			context(context &&) = delete;
			context &operator=(context &&) = delete;

			/** Initializes the io_uring execution context with a default queue size.
			 * @throw std::system_error On failure to initialize descriptors or memory mappings. */
			ROD_PUBLIC context();
			/** Initializes the EPOLL execution context with the specified queue size.
			 * @param entries Number of entries in the io_uring queues.
			 * @throw std::system_error On failure to initialize descriptors or memory mappings. */
			ROD_PUBLIC explicit context(std::size_t entries);
			ROD_PUBLIC ~context();

			/** Changes the internal state to stopped and unblocks consumer thread. Any in-progress work will run to completion. */
			ROD_PUBLIC void finish();

			/** Returns copy of the stop source associated with the context. */
			[[nodiscard]] constexpr in_place_stop_source &get_stop_source() noexcept { return m_stop_source; }
			/** Returns a stop token of the stop source associated with the context. */
			[[nodiscard]] constexpr in_place_stop_token get_stop_token() const noexcept { return m_stop_source.get_token(); }
			/** Sends a stop request to the stop source associated with the context. */
			ROD_PUBLIC void request_stop();

		private:
			template<typename F>
			bool submit_sqe(F &&init) noexcept
			{
				if (m_cq.pending + m_sq.pending >= m_cq.size)
					return false;

				const auto head = std::atomic_ref{*m_sq.head}.load(std::memory_order_acquire);
				const auto tail = std::atomic_ref{*m_sq.tail}.load(std::memory_order_acquire);
				if (m_cq.size <= tail - head)
					return false;

				const auto idx = tail & m_sq.mask;
				if constexpr (std::is_void_v<std::invoke_result_t<F, io_uring_sqe &>>)
					init(m_sq.entries[idx]);
				else if (!init(m_sq.entries[idx]))
					return false;

				std::atomic_ref{*m_sq.tail}.store(tail + 1, std::memory_order_release);
				m_sq.idx_data[idx] = idx;
				m_sq.pending++;
				return true;
			}

			[[nodiscard]] ROD_PUBLIC bool is_consumer_thread() const noexcept;

			void schedule(operation_base<context> *node, std::error_code &err) noexcept
			{
				if (!is_consumer_thread())
					schedule_producer(node, err);
				else
					schedule_consumer(node);
			}
			ROD_PUBLIC void schedule_producer(operation_base<context> *node, std::error_code &err) noexcept;
			ROD_PUBLIC void schedule_consumer(operation_base<context> *node) noexcept;
			ROD_PUBLIC void schedule_waitlist(operation_base<context> *node) noexcept;

			ROD_PUBLIC void add_timer(timer_node<context> *node) noexcept;
			ROD_PUBLIC void del_timer(timer_node<context> *node) noexcept;

			bool submit_timer_event(time_point timeout) noexcept;
			bool cancel_timer_event() noexcept;

			/* TID of the current consumer thread. */
			std::atomic<std::thread::id> m_consumer_tid = {};

			/* Descriptors used for io_uring notifications. */
			detail::unique_descriptor m_uring_fd;
			detail::unique_descriptor m_event_fd;

			/* Memory mappings of io_uring queues. */
			detail::system_mmap m_cq_mmap;
			detail::system_mmap m_sq_mmap;
			detail::system_mmap m_sqe_mmap;

			/* State of io_uring queues. */
			cq_state_t m_cq = {};
			sq_state_t m_sq = {};

			in_place_stop_source m_stop_source;
			/* Queue of operations waiting for more space in the IO queues. */
			consumer_queue_t m_waitlist_queue;
			/* Queue of operations pending for dispatch by consumer thread. */
			consumer_queue_t m_consumer_queue;
			/* Queue of operation pending for acquisition by consumer thread. */
			producer_queue_t m_producer_queue;
			/* Priority queue of pending timers. */
			timer_queue_t m_timers;

			std::uint32_t m_active_timers = 0;
			time_point m_next_timeout = {};
			__kernel_timespec m_ktime = {};

			bool m_timer_pending = false;
			bool m_wait_pending = false;
			bool m_stop_pending = false;
		};
	}

	/** Linux-specific execution context implemented via io_uring. */
	using io_uring_context = _io_uring::context;

	//static_assert(scheduler<decltype(std::declval<io_uring_context>().get_scheduler())>);
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

/*
 * Created by switchblade on 2023-05-29.
 */

#pragma once

#include "detail/context_fwd.hpp"

#ifdef ROD_HAS_LIBURING

#include "../unix/detail/mmap.hpp"

/* Forward declare these to avoid including liburing.h */
extern "C"
{
struct io_uring_cqe;
struct io_uring_sqe;
}

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	namespace _io_uring
	{
		struct kernel_timespec_t
		{
			int64_t tv_sec;
			long long tv_nsec;
		};

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

		class context : _system_ctx::operation_base<context>
		{
			template<typename, typename>
			friend struct _system_ctx::operation;
			template<typename, typename>
			friend struct _system_ctx::timer_operation;
			template<typename, typename, typename>
			friend struct _system_ctx::io_operation;

		public:
			using time_point = _system_ctx::time_point;
			using clock = _system_ctx::clock;

		private:
			using operation_base_t = typename _system_ctx::operation_base<context>;
			using timer_node_t = typename _system_ctx::timer_node<context>;

			struct timer_cmp { constexpr bool operator()(const timer_node_t &a, const timer_node_t &b) const noexcept { return a._timeout <= b._timeout; }};
			using timer_queue_t = detail::priority_queue<timer_node_t, timer_cmp, &timer_node_t::_timer_prev, &timer_node_t::_timer_next>;
			using producer_queue_t = detail::atomic_queue<operation_base_t, &operation_base_t::_next>;
			using consumer_queue_t = detail::basic_queue<operation_base_t, &operation_base_t::_next>;

			using scheduler_t = typename _system_ctx::scheduler<context>::type;

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

			/** Returns a scheduler used to schedule work to be executed on the context. */
			[[nodiscard]] constexpr scheduler_t get_scheduler() noexcept;

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
				if constexpr (std::is_void_v<std::invoke_result_t<F, io_uring_sqe *, decltype(idx)>>)
					init(m_sq.entries, idx);
				else if (!init(m_sq.entries, idx))
					return false;

				std::atomic_ref{*m_sq.tail}.store(tail + 1, std::memory_order_release);
				m_sq.idx_data[idx] = idx;
				m_sq.pending++;
				return true;
			}

			[[nodiscard]] ROD_PUBLIC bool is_consumer_thread() const noexcept;

			void schedule(operation_base_t *node, std::error_code &err) noexcept
			{
				if (!is_consumer_thread())
					schedule_producer(node, err);
				else
					schedule_consumer(node);
			}
			ROD_PUBLIC void schedule_producer(operation_base_t *node, std::error_code &err) noexcept;
			ROD_PUBLIC void schedule_consumer(operation_base_t *node) noexcept;
			ROD_PUBLIC void schedule_waitlist(operation_base_t *node) noexcept;

			ROD_PUBLIC bool submit_io_event(operation_base_t *node, _system_ctx::io_id id, const _system_ctx::io_cmd<> &cmd) noexcept;
			ROD_PUBLIC bool cancel_io_event(operation_base_t *node) noexcept;

			ROD_PUBLIC void add_timer(timer_node_t *node) noexcept;
			ROD_PUBLIC void del_timer(timer_node_t *node) noexcept;

			bool submit_timer_event(_system_ctx::time_point timeout) noexcept;
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
			kernel_timespec_t m_ktime = {};
			time_point m_next_timeout = {};

			bool m_timer_pending = false;
			bool m_wait_pending = false;
			bool m_stop_pending = false;
		};
	}

	/** Linux-specific execution context implemented via io_uring. */
	using io_uring_context = _io_uring::context;

	constexpr auto io_uring_context::get_scheduler() noexcept -> scheduler_t { return {this}; }

	template<>
	struct _system_ctx::io_cmd<schedule_read_some_t, io_uring_context> : _system_ctx::io_cmd<schedule_read_some_t>
	{
		std::size_t operator()(std::error_code &err) noexcept { return fd.read(buff.data, buff.size, err); }
		using io_cmd<schedule_read_some_t>::io_cmd;
	};
	template<>
	struct _system_ctx::io_cmd<schedule_write_some_t, io_uring_context> : _system_ctx::io_cmd<schedule_write_some_t>
	{
		std::size_t operator()(std::error_code &err) noexcept { return fd.write(buff.data, buff.size, err); }
		using io_cmd<schedule_write_some_t>::io_cmd;
	};
	template<>
	struct _system_ctx::io_cmd<schedule_read_some_at_t, io_uring_context> : _system_ctx::io_cmd<schedule_read_some_at_t>
	{
		std::size_t operator()(std::error_code &err) noexcept { return fd.read_at(buff.data, buff.size, off, err); }
		using io_cmd<schedule_read_some_at_t>::io_cmd;
	};
	template<>
	struct _system_ctx::io_cmd<schedule_write_some_at_t, io_uring_context> : _system_ctx::io_cmd<schedule_write_some_at_t>
	{
		std::size_t operator()(std::error_code &err) noexcept { return fd.write_at(buff.data, buff.size, off, err); }
		using io_cmd<schedule_write_some_at_t>::io_cmd;
	};

	template<typename Op, typename Rcv>
	struct _system_ctx::io_operation<Op, io_uring_context, Rcv>
	{
		struct type : private complete_base<io_uring_context>, private stop_base<io_uring_context>
		{
			using _operation_base_t = operation_base<io_uring_context>;

			static void _notify_start(_operation_base_t *ptr) noexcept { static_cast<type *>(static_cast<complete_base *>(ptr))->_start_consumer(); }
			static void _notify_value(_operation_base_t *ptr) noexcept { static_cast<type *>(static_cast<complete_base *>(ptr))->_complete_value(); }
			static void _notify_stopped(_operation_base_t *ptr) noexcept { static_cast<type *>(static_cast<stop_base *>(ptr))->_complete_stopped(); }
			static void _notify_request_stop(_operation_base_t *ptr) noexcept { static_cast<type *>(static_cast<stop_base *>(ptr))->_request_stop(); }

			template<typename Rcv1>
			type(io_uring_context &ctx, Rcv1 &&rcv, io_cmd<Op, io_uring_context> cmd) : _rcv(std::forward<Rcv1>(rcv)), _ctx(ctx), _cmd(std::move(cmd)) {}

			friend void tag_invoke(start_t, type &op) noexcept { op._start(); }

			void _complete_value() noexcept
			{
				if constexpr (stoppable_env<env_of_t<Rcv>>)
					_stop_cb.reset();
				if (_flags.fetch_or(flags_t::dispatched, std::memory_order_acq_rel) & flags_t::stop_requested)
					return;

				if (const auto err = static_cast<int>(_cmd.buff.iov_len); err == -ECANCELED)
					set_error(std::move(_rcv), std::error_code{-err, std::system_category()});
				else if (err >= 0)
					[[likely]] set_value(std::move(_rcv), _cmd.buff.iov_len);
				else
					set_stopped(std::move(_rcv));
			}
			void _complete_stopped() noexcept
			{
				if (complete_base::_notify_func)
				{
					stop_base::_notify_func = _notify_stopped;
					_ctx.schedule_consumer(static_cast<stop_base *>(this));
				}
				if constexpr (stoppable_env<env_of_t<Rcv >>)
					set_stopped(std::move(_rcv));
				else
					std::terminate();
			}

			void _start() noexcept
			{
				if (_ctx.is_consumer_thread())
					_start_consumer();
				else
				{
					std::error_code err = {};
					complete_base::_notify_func = _notify_start;
					_ctx.schedule_producer(static_cast<complete_base *>(this), err);
					if (err) [[unlikely]] set_error(std::move(_rcv), err);
				}
			}
			void _start_consumer() noexcept
			{
				if (_flags.load(std::memory_order_acquire) & flags_t::stop_requested)
					return; /* Already stopped on a different thread. */

				complete_base::_notify_func = _notify_value;
				if (!_ctx.submit_io_event(static_cast<complete_base *>(this), io_cmd<Op, io_uring_context>::id, _cmd))
				{
					complete_base::_notify_func = _notify_start;
					_ctx.schedule_waitlist(this);
				}
			}
			void _request_stop() noexcept
			{
				if (_flags.fetch_or(flags_t::stop_requested, std::memory_order_acq_rel) & flags_t::dispatched)
					return; /* Already completed on a different thread */

				stop_base::_notify_func = _notify_stopped;
				if (!_ctx.cancel_io_event(static_cast<stop_base *>(this)))
				{
					stop_base::_notify_func = _notify_request_stop;
					_ctx.schedule_waitlist(static_cast<stop_base *>(this));
				}
			}

			using _stop_cb_t = stop_cb<env_of_t<Rcv>, &type::_request_stop>;

			[[ROD_NO_UNIQUE_ADDRESS]] _stop_cb_t _stop_cb;
			[[ROD_NO_UNIQUE_ADDRESS]] Rcv _rcv;
			io_cmd<Op, io_uring_context> _cmd;
			std::atomic<int> _flags = {};
			io_uring_context &_ctx;
		};
	};

	//static_assert(rod::scheduler<decltype(std::declval<context>().get_scheduler())>);
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

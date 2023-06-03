/*
 * Created by switchblade on 2023-05-21.
 */

#pragma once

#include "detail/system_ctx.hpp"

#ifdef ROD_HAS_EPOLL
ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	namespace _epoll
	{
		class context : _system_ctx::operation_base<context>
		{
			template<typename, typename>
			friend struct _system_ctx::timer_operation;
			template<typename, typename>
			friend struct _system_ctx::operation;
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

			/** Initializes the EPOLL execution context with a default max number of events.
			 * @throw std::system_error On failure to initialize descriptors.
			 * @throw std::bad_alloc On failure to allocate event buffer. */
			ROD_PUBLIC context();
			/** Initializes the EPOLL execution context with the specified max number of events.
			 * @param max Maximum size of the internal EPOLL event buffer.
			 * @throw std::system_error On failure to initialize descriptors.
			 * @throw std::bad_alloc On failure to allocate event buffer. */
			ROD_PUBLIC explicit context(std::size_t max);
			ROD_PUBLIC ~context();

			/** Returns a scheduler used to schedule work to be executed on the context. */
			[[nodiscard]] constexpr scheduler_t get_scheduler() noexcept;

			/** Blocks the current thread until `finish` is called and executes scheduled operations.
			 * @throw std::system_error On implementation-defined internal failures.
			 * @note Only one thread may call `run`. */
			ROD_PUBLIC void run();
			/** Blocks the current thread until stopped via \a tok and executes scheduled operations.
			 * @param tok Stop token used to stop execution of the EPOLL event loop.
			 * @throw std::system_error On implementation-defined internal failures.
			 * @note Only one thread may call `run`. */
			template<stoppable_token Tok>
			void run(Tok &&tok)
			{
				const auto do_stop = [&]() { finish(); };
				const auto cb = stop_callback_for_t<Tok, decltype(do_stop)>{std::forward<Tok>(tok), do_stop};
				run();
			}

			/** Changes the internal state to stopped and unblocks consumer thread. Any in-progress work will run to completion. */
			ROD_PUBLIC void finish();

			/** Returns copy of the stop source associated with the context. */
			[[nodiscard]] constexpr in_place_stop_source &get_stop_source() noexcept { return m_stop_source; }
			/** Returns a stop token of the stop source associated with the context. */
			[[nodiscard]] constexpr in_place_stop_token get_stop_token() const noexcept { return m_stop_source.get_token(); }
			/** Sends a stop request to the stop source associated with the context. */
			ROD_PUBLIC void request_stop();

		private:
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

			ROD_PUBLIC void add_io(operation_base_t *node, _system_ctx::io_id id, const _system_ctx::io_cmd<> &cmd) noexcept;
			ROD_PUBLIC void del_io(const _system_ctx::io_cmd<> &cmd) noexcept;

			ROD_PUBLIC void add_timer(timer_node_t *node) noexcept;
			ROD_PUBLIC void del_timer(timer_node_t *node) noexcept;

			bool acquire_producer_queue() noexcept;
			void epoll_wait();

			/* TID of the current consumer thread. */
			std::atomic<std::thread::id> m_consumer_tid = {};

			/* Descriptors used for EPOLL notifications. */
			detail::unique_descriptor m_epoll_fd;
			detail::unique_descriptor m_timer_fd;
			detail::unique_descriptor m_event_fd;

			/* EPOLL event buffer. */
			std::size_t m_buff_size;
			void *m_event_buff;

			in_place_stop_source m_stop_source;
			/* Queue of operations pending for dispatch by consumer thread. */
			consumer_queue_t m_consumer_queue;
			/* Queue of operation pending for acquisition by consumer thread. */
			producer_queue_t m_producer_queue;
			/* Priority queue of pending timers. */
			timer_queue_t m_timers;

			_system_ctx::time_point m_next_timeout = {};
			bool m_timer_started = false;
			bool m_timer_pending = false;
			bool m_wait_pending = false;
			bool m_stop_pending = false;
		};
	}

	/** Linux-specific execution context implemented via EPOLL. */
	using epoll_context = _epoll::context;

	constexpr auto epoll_context::get_scheduler() noexcept -> scheduler_t { return {this}; }

	template<>
	struct _system_ctx::io_cmd<schedule_read_some_t, epoll_context> : _system_ctx::io_cmd<schedule_read_some_t>
	{
		std::size_t operator()(std::error_code &err) noexcept { return fd.read(buff.data, buff.size, err); }
		using io_cmd<schedule_read_some_t>::io_cmd;
	};
	template<>
	struct _system_ctx::io_cmd<schedule_write_some_t, epoll_context> : _system_ctx::io_cmd<schedule_write_some_t>
	{
		std::size_t operator()(std::error_code &err) noexcept { return fd.write(buff.data, buff.size, err); }
		using io_cmd<schedule_write_some_t>::io_cmd;
	};
	template<>
	struct _system_ctx::io_cmd<schedule_read_some_at_t, epoll_context> : _system_ctx::io_cmd<schedule_read_some_at_t>
	{
		std::size_t operator()(std::error_code &err) noexcept { return fd.read_at(buff.data, buff.size, off, err); }
		using io_cmd<schedule_read_some_at_t>::io_cmd;
	};
	template<>
	struct _system_ctx::io_cmd<schedule_write_some_at_t, epoll_context> : _system_ctx::io_cmd<schedule_write_some_at_t>
	{
		std::size_t operator()(std::error_code &err) noexcept { return fd.write_at(buff.data, buff.size, off, err); }
		using io_cmd<schedule_write_some_at_t>::io_cmd;
	};

	template<typename Op, typename Rcv>
	struct _system_ctx::io_operation<Op, epoll_context, Rcv>
	{
		struct type : private complete_base<epoll_context>, private stop_base<epoll_context>
		{
			using _operation_base_t = operation_base<epoll_context>;

			static void _notify_start(_operation_base_t *ptr) noexcept { static_cast<type *>(static_cast<complete_base *>(ptr))->_start_consumer(); }
			static void _notify_value(_operation_base_t *ptr) noexcept { static_cast<type *>(static_cast<complete_base *>(ptr))->_complete_value(); }
			static void _notify_stopped(_operation_base_t *ptr) noexcept { static_cast<type *>(static_cast<stop_base *>(ptr))->_complete_stopped(); }

			template<typename Rcv1>
			explicit type(epoll_context &ctx, Rcv1 &&rcv, io_cmd<Op, epoll_context> func) : _rcv(std::forward<Rcv1>(rcv)), _cmd(std::move(func)), _ctx(ctx) {}

			friend void tag_invoke(start_t, type &op) noexcept { op._start(); }

			void _complete_value() noexcept
			{
				if constexpr (stoppable_env<env_of_t<Rcv>>)
					_stop_cb.reset();
				if (_flags.fetch_or(flags_t::dispatched, std::memory_order_acq_rel) & flags_t::stop_requested)
					return;

				_ctx.del_io(_cmd);
				std::error_code err = {};
				const auto res = _cmd(err);
				_complete_value(res, err);
			}
			void _complete_value(std::size_t res, std::error_code err) noexcept
			{
				if (!err)
					[[likely]] set_value(std::move(_rcv), res);
				else if (err.value() != ECANCELED)
					set_error(std::move(_rcv), err);
				else
					set_stopped(std::move(_rcv));
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
				std::error_code err = {};
				const auto res = _cmd(err);
				if (const auto code = err.value(); code == EAGAIN || code == EWOULDBLOCK || code == EPERM)
				{
					/* Schedule read operation via EPOLL. */
					complete_base::_notify_func = _notify_value;
					if constexpr (stoppable_env<env_of_t<Rcv>>)
						_stop_cb.init(get_env(_rcv), this);

					_ctx.add_io(static_cast<complete_base *>(this), io_cmd<Op, epoll_context>::id, _cmd);
					return;
				}

				if (_flags.fetch_or(flags_t::dispatched, std::memory_order_acq_rel) & flags_t::stop_requested)
					return; /* Already stopped on a different thread. */

				_complete_value(res, err);
			}

			void _request_stop() noexcept
			{
				if (!(_flags.fetch_or(flags_t::stop_requested, std::memory_order_acq_rel) & flags_t::dispatched))
				{
					_ctx.del_io(_cmd);
					stop_base::_notify_func = _notify_stopped;

					std::error_code err = {};
					_ctx.schedule_producer(static_cast<stop_base *>(this), err);
					if (err) [[unlikely]] std::terminate();
				}
			}
			void _complete_stopped() noexcept
			{
				if (complete_base::_notify_func)
				{
					stop_base::_notify_func = _notify_stopped;
					_ctx.schedule_consumer(static_cast<stop_base *>(this));
				}
				if constexpr (stoppable_env<env_of_t<Rcv>>)
					set_stopped(std::move(_rcv));
				else
					std::terminate();
			}

			using _stop_cb_t = stop_cb<env_of_t<Rcv>, &type::_request_stop>;

			[[ROD_NO_UNIQUE_ADDRESS]] _stop_cb_t _stop_cb;
			[[ROD_NO_UNIQUE_ADDRESS]] Rcv _rcv;
			io_cmd<Op, epoll_context> _cmd;
			std::atomic<int> _flags = {};
			epoll_context &_ctx;
		};
	};

	static_assert(rod::scheduler<decltype(std::declval<epoll_context>().get_scheduler())>);
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

/*
 * Created by switchblade on 2023-05-21.
 */

#pragma once

#include "../detail/config.hpp"

#ifdef ROD_HAS_EPOLL

#include <thread>
#include <chrono>

#include <sys/epoll.h>
#include <sys/uio.h>

#include "../detail/priority_queue.hpp"
#include "../detail/atomic_queue.hpp"
#include "../detail/basic_queue.hpp"

#include "../unix/monotonic_clock.hpp"
#include "../unix/detail/file.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	namespace _epoll
	{
		class context;

		using clock = monotonic_clock;
		using time_point = typename clock::time_point;

		template<typename Env>
		concept stoppable_env = stoppable_token<stop_token_of_t<Env> &>;
		template<typename Snd, typename Env>
		using make_signs = make_completion_signatures<Snd, Env, completion_signatures<set_error_t(std::exception_ptr)>>;

		struct operation_base
		{
			operation_base(operation_base &&) = delete;
			operation_base &operator=(operation_base &&) = delete;

			constexpr operation_base() noexcept = default;

			void _notify() noexcept { _notify_func(this); }

			void (*_notify_func)(operation_base *) noexcept = {};
			operation_base *_next = {};
		};
		struct timer_node : operation_base
		{
			enum _flags_t { stop_possible = 1, stop_requested = 4, dispatched = 8 };

			constexpr timer_node(context &ctx, time_point timeout, bool can_stop) noexcept : _flags(can_stop ? stop_possible : 0), _timeout(timeout), _ctx(ctx) {}

			timer_node *_timer_prev = {};
			timer_node *_timer_next = {};
			std::atomic<int> _flags = {};
			time_point _timeout = {};
			context &_ctx;
		};

		struct scheduler;
		template<typename Rcv>
		struct timer_operation { struct type; };
		template<typename Rcv>
		struct operation { struct type; };
		template<typename T>
		struct sender { struct type; };

		template<typename, auto>
		struct stop_cb {};
		template<typename Env, typename Op, void (Op::*Stop)() noexcept> requires stoppable_env<Env>
		struct stop_cb<Env, Stop>
		{
			struct callback
			{
				void operator()() const noexcept { op->*Stop(); }
				Op *op;
			};

			constexpr void init(auto &&env, Op *ptr) noexcept { data.emplace(get_stop_token(env), callback{ptr}); }
			constexpr void reset() noexcept { data.reset(); }

			std::optional<stop_callback_for_t<stop_token_of_t<Env &>, callback>> data;
		};

		template<typename Rcv>
		struct operation<Rcv>::type : operation_base
		{
			static void _bind_notify(operation_base *ptr) noexcept
			{
				auto &op = *static_cast<type *>(ptr);

				/* Stop the top-level receiver if necessary. */
				if constexpr (stoppable_env<env_of_t<Rcv>>)
					if (get_stop_token(get_env(op._rcv)).stop_requested())
					{
						set_stopped(std::move(op._rcv));
						return;
					}

				/* Complete the top-level receiver. */
				if constexpr (!detail::nothrow_callable<set_value_t, Rcv>)
					try { set_value(std::move(op._rcv)); } catch (...) { set_error(std::move(op._rcv), std::current_exception()); }
				else
					set_value(std::move(op._rcv));
			}

			template<typename Rcv1>
			constexpr type(context &ctx, Rcv1 &&rcv) : _ctx(ctx), _rcv(std::forward<Rcv1>(rcv)) { this->_notify = _bind_notify; }

			friend void tag_invoke(start_t, type &op) noexcept { op._start(); }

			inline void _start() noexcept;

			[[ROD_NO_UNIQUE_ADDRESS]] Rcv _rcv;
			context &_ctx;
		};
		template<typename Rcv>
		struct timer_operation<Rcv>::type : timer_node
		{
			static void _notify_value(operation_base *ptr) noexcept { static_cast<type *>(ptr)->_complete_value(); }
			static void _notify_stopped(operation_base *ptr) noexcept { static_cast<type *>(ptr)->_complete_stopped(); }

			constexpr type(context &ctx, time_point timeout, Rcv rcv) noexcept(std::is_nothrow_move_constructible_v<Rcv>) : timer_node(ctx, timeout, get_stop_token(get_env(rcv)).stop_possible()), _rcv(std::move(rcv)) {}

			friend void tag_invoke(start_t, type &op) noexcept { op._start(); }

			void _complete_value() noexcept
			{
				/* Handle spontaneous stop requests. */
				if constexpr (stoppable_env<env_of_t<Rcv>>)
				{
					_stop_cb.reset();
					if (get_stop_token(get_env(_rcv)).stop_requested())
					{
						_complete_stopped();
						return;
					}
				}

				if constexpr (detail::nothrow_callable<set_value_t, Rcv>)
					try { set_value(std::move(_rcv)); } catch (...) { set_error(std::move(_rcv), std::current_exception()); }
				else
					set_value(std::move(_rcv));
			}
			void _complete_stopped() noexcept
			{
				if constexpr (stoppable_env<env_of_t<Rcv>>)
					set_stopped(std::move(_rcv));
				else
					std::terminate();
			}

			inline void _start() noexcept;
			inline void _start_consumer() noexcept;
			inline void _request_stop() noexcept;

			using _stop_cb_t = stop_cb<env_of_t<Rcv>, &type::_request_stop>;

			[[ROD_NO_UNIQUE_ADDRESS]] _stop_cb_t _stop_cb;
			[[ROD_NO_UNIQUE_ADDRESS]] Rcv _rcv;
		};

		class context
		{
			template<typename>
			friend struct timer_operation;
			template<typename>
			friend struct operation;

			struct timer_cmp { constexpr bool operator()(const timer_node &a, const timer_node &b) const noexcept { return a._timeout <= b._timeout; }};
			using timer_queue_t = detail::priority_queue<timer_node, timer_cmp, &timer_node::_timer_prev, &timer_node::_timer_next>;
			using producer_queue_t = detail::atomic_queue<operation_base, &operation_base::_next>;
			using consumer_queue_t = detail::basic_queue<operation_base, &operation_base::_next>;

			/* A special stop operation is used in order to ensure that consumer thread exists only
			 * after all pending operations at the time that stop is requested are complete. */
			struct stop_trigger : operation_base
			{
				constexpr explicit stop_trigger(bool &flag) noexcept : flag(flag)
				{
					_notify_func = [](operation_base *ptr) noexcept { static_cast<stop_trigger *>(ptr)->flag = true; };
				}
				bool &flag;
			};

		public:
			/** Initializes the EPOLL execution context with a default max number of events.
			 * @throw std::system_error On failure to initialize EPOLL descriptors.
			 * @throw std::bad_alloc On failure to allocate EPOLL event buffer. */
			ROD_PUBLIC context();
			/** Initializes the EPOLL execution context with the specified max number of events.
			 * @param max Maximum size of the internal EPOLL event buffer.
			 * @throw std::system_error On failure to initialize EPOLL descriptors.
			 * @throw std::bad_alloc On failure to allocate EPOLL event buffer. */
			ROD_PUBLIC explicit context(std::size_t max);
			ROD_PUBLIC ~context();

			/** Blocks the current thread until stopped via \a tok and handles EPOLL operations.
			 * @throw std::system_error On implementation-defined internal failures.
			 * @note Only one thread may call `run`. */
			template<stoppable_token Tok>
			void run(Tok &&tok)
			{
				/* Make sure only one thread is allowed to run at a given time. */
				if (std::thread::id id = {}; !m_consumer_tid.compare_exchange_strong(id, std::this_thread::get_id(), std::memory_order_acq_rel))
					throw std::system_error(std::make_error_code(std::errc::device_or_resource_busy), "Only one thread may invoke `epoll_context::run`");

				auto trigger = stop_trigger{m_stop_pending};
				const auto on_stop = [&]() { schedule(&trigger); };
				auto cb = stop_callback_for_t<Tok, decltype(on_stop)>{std::forward<Tok>(tok), on_stop};
				run_impl();
			}

		private:
			void schedule(operation_base *node)
			{
				if (m_consumer_tid.load(std::memory_order_acquire) != std::this_thread::get_id())
					schedule_producer(node);
				else
					schedule_consumer(node);
			}

			ROD_PUBLIC void schedule_producer(operation_base *node);
			ROD_PUBLIC void schedule_consumer(operation_base *node) noexcept;

			ROD_PUBLIC void insert_timer(timer_node *node) noexcept;
			ROD_PUBLIC void erase_timer(timer_node *node) noexcept;

			bool acquire_producer_queue() noexcept;
			void epoll_wait();

			ROD_PUBLIC void run_impl();

			/* TID of the current consumer thread. */
			std::atomic<std::thread::id> m_consumer_tid = {};

			/* Descriptors used for EPOLL notifications. */
			detail::descriptor_handle m_epoll_fd = {};
			detail::descriptor_handle m_timer_fd = {};
			detail::descriptor_handle m_event_fd = {};

			/* EPOLL event buffer. */
			std::span<epoll_event> m_events = {};

			/* Queue of operations pending for dispatch by consumer thread. */
			consumer_queue_t m_consumer_queue = {};
			/* Queue of operation pending for acquisition by consumer thread. */
			producer_queue_t m_producer_queue = {};
			/* Priority queue of pending timers. */
			timer_queue_t m_timers = {};

			/* Time point when `m_timer_fd` will elapse. */
			time_point m_next_timeout = {};
			bool m_timer_fd_started = {};

			/* Flags used to indicate status of pending operations. */
			bool m_epoll_pending = {};
			bool m_timer_pending = {};
			bool m_stop_pending = {};
		};

		template<typename Rcv>
		void operation<Rcv>::type::_start() noexcept
		{
			try { _ctx.schedule(this); }
			catch (...) { set_error(std::move(_rcv), std::current_exception()); }
		}
		template<typename Rcv>
		void timer_operation<Rcv>::type::_start() noexcept
		{
			if (_ctx.m_consumer_tid.load(std::memory_order_acquire) != std::this_thread::get_id())
			{
				_notify_func = [](operation_base *p) noexcept { static_cast<type *>(p)->_start_consumer(); };
				try { _ctx.schedule_producer(this); }
				catch (...) { set_error(std::move(_rcv), std::current_exception()); }
			}
			else
				_start_consumer();
		}
		template<typename Rcv>
		void timer_operation<Rcv>::type::_start_consumer() noexcept
		{
			/* Bail if a stop has already been requested. */
			if constexpr (stoppable_env<env_of_t<Rcv>>)
				if (get_stop_token(get_env(_rcv)).stop_requested())
				{
					_notify_func = _notify_stopped;
					_ctx.schedule_consumer(this);
					return;
				}

			_notify_func = _notify_value;
			_ctx.insert_timer(this);

			/* Initialize the stop callback for stoppable environments. */
			if constexpr (stoppable_env<env_of_t<Rcv>>)
				_stop_cb.init(get_env(_rcv));
		}
		template<typename Rcv>
		void timer_operation<Rcv>::type::_request_stop() noexcept
		{
			if constexpr (!stoppable_env<env_of_t<Rcv>>)
				std::terminate();
			else
			{
				if (_ctx.m_consumer_tid.load(std::memory_order_acquire) == std::this_thread::get_id())
				{
					_stop_cb.reset();
					_notify_func = _notify_stopped;
					if (!(_flags.load(std::memory_order_relaxed) & _flags_t::dispatched))
					{
						/* Timer has not yet been dispatched, schedule stop completion. */
						_ctx.erase_timer(this);
						_ctx.schedule_consumer(this);
					}
				}
				else if (!(_flags.fetch_add(_flags_t::stop_requested, std::memory_order_acq_rel) & _flags_t::dispatched))
				{
					/* Timer has not yet been dispatched, schedule stop completion on producer thread. */
					_notify_func = [](operation_base *ptr) noexcept
					{
						const auto op = static_cast<type *>(ptr);
						op->_stop_cb.reset();

						/* Make sure to erase the timer if it has not already been dispatched. */
						if (!(op->_flags.load(std::memory_order_relaxed) & _flags_t::dispatched))
							op->_ctx.erase_timer(&op);

						op->_complete_stopped();
					};

					/* Complete with error if failed to schedule. */
					try { _ctx.schedule_producer(this); }
					catch (...) { set_error(std::move(_rcv), std::current_exception()); }
				}
			}
		}
	}

	/** Linux-specific execution context implemented via EPOLL. */
	using epoll_context = _epoll::context;
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

/*
 * Created by switchblade on 2023-05-29.
 */

#pragma once

#include "../detail/adaptors/read_some.hpp"
#include "../detail/adaptors/write_some.hpp"

#ifdef ROD_HAS_LIBURING

#include <utility>
#include <thread>
#include <chrono>

#include "../detail/priority_queue.hpp"
#include "../detail/atomic_queue.hpp"
#include "../detail/basic_queue.hpp"

#include "../unix/monotonic_clock.hpp"
#include "../unix/detail/file.hpp"
#include "../unix/detail/mmap.hpp"

/* Forward declare these to avoid including liburing.h */
extern "C"
{
struct io_uring_cqe;
struct io_uring_sqe;
}

namespace rod
{
	namespace _io_uring
	{
		enum flags_t { stop_possible = 1, stop_requested = 4, dispatched = 8 };

		using clock = monotonic_clock;
		using time_point = typename clock::time_point;

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

		class context;
		struct scheduler;

		struct env { context *_ctx; };

		template<typename, auto>
		struct stop_cb {};
		template<typename Env, typename Op, void (Op::*Stop)() noexcept> requires detail::stoppable_env<Env>
		struct stop_cb<Env, Stop>
		{
			struct callback
			{
				void operator()() const noexcept { (op->*Stop)(); }
				Op *op;
			};

			constexpr void init(auto &&env, Op *ptr) noexcept { data.emplace(get_stop_token(env), callback{ptr}); }
			constexpr void reset() noexcept { data.reset(); }

			std::optional<stop_callback_for_t<stop_token_of_t<Env &>, callback>> data;
		};

		template<typename>
		struct io_cmd { struct type; };
		template<>
		struct io_cmd<schedule_read_some_t>::type
		{
			template<typename T>
			constexpr type(int fd, std::span<T> buff) noexcept : fd(fd), buff(std::begin(buff), std::end(buff)) {}

			detail::basic_descriptor fd;
			std::span<std::byte> buff;
		};
		template<>
		struct io_cmd<schedule_write_some_t>::type
		{
			template<typename T>
			constexpr type(int fd, std::span<T> buff) noexcept : fd(fd), buff(std::cbegin(buff), std::cend(buff)) {}

			detail::basic_descriptor fd;
			std::span<const std::byte> buff;
		};
		template<>
		struct io_cmd<schedule_read_some_at_t>::type : io_cmd<schedule_read_some_t>::type
		{
			template<typename T>
			constexpr type(int fd, std::size_t off, std::span<T> buff) noexcept : io_cmd<schedule_read_some_t>::type(fd, buff), off(off) {}

			std::size_t off;
		};
		template<>
		struct io_cmd<schedule_write_some_at_t>::type : io_cmd<schedule_write_some_t>::type
		{
			template<typename T>
			constexpr type(int fd, std::size_t off, std::span<T> buff) noexcept : io_cmd<schedule_write_some_t>::type(fd, buff), off(off) {}

			std::size_t off;
		};

		struct operation_base
		{
			using _notify_func_t = void (*)(operation_base *);

			operation_base(operation_base &&) = delete;
			operation_base &operator=(operation_base &&) = delete;

			constexpr operation_base() noexcept = default;

			void _notify() noexcept { std::exchange(_notify_func, {})(this); }

			_notify_func_t _notify_func = {};
			operation_base *_next = {};
			std::ptrdiff_t _res = {};
		};
		struct timer_base : operation_base
		{
			constexpr timer_base(context &ctx, time_point tp, bool can_stop) noexcept : _flags(can_stop ? flags_t::stop_possible : 0), _tp(tp), _ctx(ctx) {}

			[[nodiscard]] bool _stop_possible() const noexcept { return _flags.load(std::memory_order_relaxed) & flags_t::stop_possible; }

			timer_base *_timer_prev = {};
			timer_base *_timer_next = {};
			std::atomic<int> _flags = {};
			time_point _tp = {};
			context &_ctx;
		};

		struct complete_base : operation_base {};
		struct stop_base : operation_base {};

		struct sender;
		template<typename Rcv>
		struct operation { struct type; };

		struct timer_sender;
		template<typename Rcv>
		struct timer_operation { struct type; };

		template<typename Op>
		struct io_sender { struct type; };
		template<typename Op, typename Rcv>
		struct io_operation { struct type; };

		class context : operation_base
		{
			template<typename>
			friend struct operation;
			template<typename>
			friend struct timer_operation;
			template<typename, typename>
			friend struct io_operation;

		public:
			using time_point = _io_uring::time_point;
			using clock = _io_uring::clock;

		private:
			struct timer_cmp { constexpr bool operator()(const timer_base &a, const timer_base &b) const noexcept { return a._tp <= b._tp; }};
			using timer_queue_t = detail::priority_queue<timer_base, timer_cmp, &timer_base::_timer_prev, &timer_base::_timer_next>;
			using producer_queue_t = detail::atomic_queue<operation_base, &operation_base::_next>;
			using consumer_queue_t = detail::basic_queue<operation_base, &operation_base::_next>;

			template<typename Op>
			using io_cmd_t = typename io_cmd<Op>::type;

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

			/** Blocks the current thread until `finish` is called and executes scheduled operations.
			 * @throw std::system_error On implementation-defined internal failures.
			 * @note Only one thread may call `run`. */
			ROD_PUBLIC void run();
			/** Blocks the current thread until stopped via \a tok and executes scheduled operations.
			 * @param tok Stop token used to stop execution of the internal event loop.
			 * @throw std::system_error On implementation-defined internal failures.
			 * @note Only one thread may call `run`. */
			template<stoppable_token Tok>
			void run(Tok &&tok)
			{
				const auto do_stop = [&]() { finish(); };
				const auto cb = stop_callback_for_t<Tok, decltype(do_stop)>{std::forward<Tok>(tok), do_stop};
				run();
			}

			/** Returns a scheduler used to schedule work to be executed on the context. */
			[[nodiscard]] constexpr scheduler get_scheduler() noexcept;

			/** Changes the internal state to stopped and unblocks consumer thread. Any in-progress work will run to completion. */
			ROD_PUBLIC void finish();

			/** Returns copy of the stop source associated with the context. */
			[[nodiscard]] constexpr in_place_stop_source &get_stop_source() noexcept { return _stop_source; }
			/** Returns a stop token of the stop source associated with the context. */
			[[nodiscard]] constexpr in_place_stop_token get_stop_token() const noexcept { return _stop_source.get_token(); }
			/** Sends a stop request to the stop source associated with the context. */
			ROD_PUBLIC void request_stop();

		private:
			template<typename F>
			bool submit_sqe(F &&init) noexcept
			{
				if (_cq.pending + _sq.pending >= _cq.size)
					return false;

				const auto tail = std::atomic_ref{*_sq.tail}.load(std::memory_order_acquire);
				const auto head = std::atomic_ref{*_sq.head}.load(std::memory_order_acquire);
				if (_cq.size <= tail - head)
					return false;

				const auto idx = tail & _sq.mask;
				if constexpr (std::is_void_v<std::invoke_result_t<F, io_uring_sqe *, decltype(idx)>>)
					init(_sq.entries, idx);
				else if (!init(_sq.entries, idx))
					return false;

				_sq.pending++;
				_sq.idx_data[idx] = idx;
				std::atomic_ref{*_sq.tail}.store(tail + 1, std::memory_order_release);
				return true;
			}

			[[nodiscard]] ROD_PUBLIC bool is_consumer_thread() const noexcept;

			ROD_PUBLIC void schedule_producer(operation_base *node, std::error_code &err) noexcept;
			ROD_PUBLIC void schedule_consumer(operation_base *node) noexcept;
			ROD_PUBLIC void schedule_waitlist(operation_base *node) noexcept;
			ROD_PUBLIC void schedule_producer(operation_base *node);

			void schedule(operation_base *node, std::error_code &err) noexcept
			{
				if (!is_consumer_thread())
					schedule_producer(node, err);
				else
					schedule_consumer(node);
			}
			void schedule(operation_base *node)
			{
				if (!is_consumer_thread())
					schedule_producer(node);
				else
					schedule_consumer(node);
			}

			bool submit_io_event(int op, auto *data, int fd, auto *addr, std::size_t n, std::size_t off) noexcept;
			bool submit_timer_event(time_point timeout) noexcept;
			bool submit_queue_event() noexcept;
			bool cancel_timer_event() noexcept;

			ROD_PUBLIC bool submit_io_event(operation_base *node, io_cmd_t<schedule_read_some_t> cmd) noexcept;
			ROD_PUBLIC bool submit_io_event(operation_base *node, io_cmd_t<schedule_write_some_t> cmd) noexcept;
			ROD_PUBLIC bool submit_io_event(operation_base *node, io_cmd_t<schedule_read_some_at_t> cmd) noexcept;
			ROD_PUBLIC bool submit_io_event(operation_base *node, io_cmd_t<schedule_write_some_at_t> cmd) noexcept;
			ROD_PUBLIC bool cancel_io_event(operation_base *node) noexcept;

			ROD_PUBLIC void add_timer(timer_base *node) noexcept;
			ROD_PUBLIC void del_timer(timer_base *node) noexcept;

			void acquire_consumer_queue();
			void acquire_elapsed_timers();
			void uring_enter();

			/* TID of the current consumer thread. */
			std::atomic<std::thread::id> _consumer_tid = {};

			/* Descriptors used for io_uring notifications. */
			detail::unique_descriptor _uring_fd = {};
			detail::unique_descriptor _event_fd = {};

			/* Memory mappings of io_uring queues. */
			detail::system_mmap _cq_mmap = {};
			detail::system_mmap _sq_mmap = {};
			detail::system_mmap _sqe_mmap = {};

			/* State of io_uring queues. */
			cq_state_t _cq = {};
			sq_state_t _sq = {};

			in_place_stop_source _stop_source;
			/* Queue of operations waiting for more space in the IO queues. */
			consumer_queue_t _waitlist_queue;
			/* Queue of operations pending for dispatch by consumer thread. */
			consumer_queue_t _consumer_queue;
			/* Queue of operation pending for acquisition by consumer thread. */
			producer_queue_t _producer_queue;
			/* Priority queue of pending timers. */
			timer_queue_t _timers;

			kernel_timespec_t _ktime = {};
			time_point _next_timeout = {};

			std::uint32_t _active_timers = 0;
			bool _timer_started = false;
			bool _timer_pending = false;
			bool _wait_pending = false;
			bool _stop_pending = false;
		};

		template<typename Rcv>
		struct operation<Rcv>::type : operation_base
		{
			static void _notify_complete(operation_base *ptr) noexcept
			{
				auto &op = *static_cast<type *>(ptr);
				if (get_stop_token(get_env(op._rcv)).stop_requested())
					set_stopped(std::move(op._rcv));
				else
					set_value(std::move(op._rcv));
			}

			template<typename Rcv2>
			constexpr type(context &ctx, Rcv2 &&rcv) : _rcv(std::forward<Rcv2>(rcv)), _ctx(ctx) { _notify_func = _notify_complete; }

			friend void tag_invoke(start_t, type &op) noexcept { op._start(); }

			void _start() noexcept
			{
				std::error_code err = {};
				_ctx.schedule(this, err);
				if (err) [[unlikely]] set_error(std::move(_rcv), err);
			}

			ROD_NO_UNIQUE_ADDRESS Rcv _rcv;
			context &_ctx;
		};
		template<typename Rcv>
		struct timer_operation<Rcv>::type : timer_base
		{
			static void _notify_value(operation_base *ptr) noexcept { static_cast<type *>(ptr)->_complete_value(); }
			static void _notify_stopped(operation_base *ptr) noexcept { static_cast<type *>(ptr)->_complete_stopped(); }
			static void _notify_request_stop(operation_base *ptr) noexcept { static_cast<type *>(ptr)->_request_stop_consumer(); }

			constexpr type(context &ctx, time_point timeout, Rcv rcv) noexcept(std::is_nothrow_move_constructible_v<Rcv>) : timer_base(ctx, timeout, get_stop_token(get_env(rcv)).stop_possible()), _rcv(std::move(rcv)) {}

			friend void tag_invoke(start_t, type &op) noexcept { op._start(); }

			void _complete_value() noexcept
			{
				/* Handle spontaneous stop requests. */
				if constexpr (detail::stoppable_env<env_of_t<Rcv>>)
				{
					_stop_cb.reset();
					if (get_stop_token(get_env(_rcv)).stop_requested())
					{
						_complete_stopped();
						return;
					}
				}
				set_value(std::move(_rcv));
			}
			void _complete_stopped() noexcept
			{
				if constexpr (detail::stoppable_env<env_of_t<Rcv>>)
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
					_notify_func = [](operation_base *p) noexcept { static_cast<type *>(p)->_start_consumer(); };
					_ctx.schedule_producer(this, err);
					if (err) [[unlikely]] set_error(std::move(_rcv), err);
				}
			}
			void _start_consumer() noexcept
			{
				/* Bail if a stop has already been requested. */
				if constexpr (detail::stoppable_env<env_of_t<Rcv>>)
					if (get_stop_token(get_env(_rcv)).stop_requested())
					{
						_notify_func = _notify_stopped;
						_ctx.schedule_consumer(this);
						return;
					}

				_notify_func = _notify_value;
				_ctx.add_timer(this);

				/* Initialize the stop callback for stoppable environments. */
				if constexpr (detail::stoppable_env<env_of_t<Rcv>>)
					_stop_cb.init(get_env(_rcv), this);
			}

			void _request_stop() noexcept
			{
				if constexpr (!detail::stoppable_env<env_of_t<Rcv>>)
					std::terminate();
				else if (_ctx.is_consumer_thread())
					_request_stop_consumer();
				else if (!(_flags.fetch_or(flags_t::stop_requested, std::memory_order_acq_rel) & flags_t::dispatched))
				{
					/* Timer has not yet been dispatched, schedule stop request. */
					_notify_func = _notify_request_stop;
					_ctx.schedule_producer(this);
				}
			}
			void _request_stop_consumer() noexcept
			{
				if constexpr (detail::stoppable_env<env_of_t<Rcv>>)
					_stop_cb.reset();

				_notify_func = _notify_stopped;
				if (!(_flags.load(std::memory_order_relaxed) & flags_t::dispatched))
				{
					/* Timer has not yet been dispatched, schedule stop completion. */
					_ctx.del_timer(this);
					_ctx.schedule_consumer(this);
				}
			}

			using _stop_cb_t = stop_cb<env_of_t<Rcv>, &type::_request_stop>;

			ROD_NO_UNIQUE_ADDRESS _stop_cb_t _stop_cb;
			ROD_NO_UNIQUE_ADDRESS Rcv _rcv;
		};
		template<typename Op, typename Rcv>
		struct io_operation<Op, Rcv>::type : private complete_base, private stop_base
		{
			using _io_cmd_t = typename io_cmd<Op>::type;

			static void _notify_start(operation_base *ptr) noexcept { static_cast<type *>(static_cast<complete_base *>(ptr))->_start_consumer(); }
			static void _notify_value(operation_base *ptr) noexcept { static_cast<type *>(static_cast<complete_base *>(ptr))->_complete_value(); }
			static void _notify_stopped(operation_base *ptr) noexcept { static_cast<type *>(static_cast<stop_base *>(ptr))->_complete_stopped(); }
			static void _notify_request_stop(operation_base *ptr) noexcept { static_cast<type *>(static_cast<stop_base *>(ptr))->_request_stop(); }

			template<typename Rcv2>
			type(context &ctx, Rcv2 &&rcv, _io_cmd_t cmd) : _rcv(std::forward<Rcv2>(rcv)), _cmd(std::move(cmd)), _ctx(ctx) {}

			friend void tag_invoke(start_t, type &op) noexcept { op._start(); }

			void _complete_value() noexcept
			{
				if constexpr (detail::stoppable_env<env_of_t<Rcv>>)
					_stop_cb.reset();
				if (_flags.fetch_or(flags_t::dispatched, std::memory_order_acq_rel) & flags_t::stop_requested)
					return;

				if (complete_base::_res >= 0)
					[[likely]] set_value(std::move(_rcv), static_cast<std::size_t>(complete_base::_res));
				else if (const auto err = static_cast<int>(-complete_base::_res); err != ECANCELED)
					set_error(std::move(_rcv), std::error_code{err, std::system_category()});
				else
					set_stopped(std::move(_rcv));
			}
			void _complete_stopped() noexcept
			{
				if constexpr (detail::stoppable_env<env_of_t<Rcv>>)
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
				if (!_ctx.submit_io_event(static_cast<complete_base *>(this), _cmd))
				{
					complete_base::_notify_func = _notify_start;
					_ctx.schedule_waitlist(static_cast<complete_base *>(this));
				}
			}
			void _request_stop() noexcept
			{
				if (_flags.fetch_or(flags_t::stop_requested, std::memory_order_acq_rel) & flags_t::dispatched)
					return; /* Already completed on a different thread */

				stop_base::_notify_func = _notify_stopped;
				if (!_ctx.is_consumer_thread())
				{
					stop_base::_notify_func = _notify_request_stop;
					_ctx.schedule_producer(static_cast<stop_base *>(this));
				}
				else if (!_ctx.cancel_io_event(static_cast<stop_base *>(this)))
				{
					stop_base::_notify_func = _notify_request_stop;
					_ctx.schedule_waitlist(static_cast<stop_base *>(this));
				}
			}

			using _stop_cb_t = stop_cb<env_of_t<Rcv>, &type::_request_stop>;

			ROD_NO_UNIQUE_ADDRESS _stop_cb_t _stop_cb;
			ROD_NO_UNIQUE_ADDRESS Rcv _rcv;
			std::atomic<int> _flags = {};
			_io_cmd_t _cmd;
			context &_ctx;
		};

		struct sender
		{
			using is_sender = std::true_type;

			template<typename Rcv>
			using _operation_t = typename operation<std::decay_t<Rcv>>::type;

			template<typename Env>
			using _stop_signs_t = std::conditional_t<detail::stoppable_env<Env>, completion_signatures<set_stopped_t()>, completion_signatures<>>;
			template<typename Env>
			using _signs_t = detail::concat_tuples_t<completion_signatures<set_value_t(), set_error_t(std::error_code)>, _stop_signs_t<Env>>;

			friend constexpr env tag_invoke(get_env_t, const sender &s) noexcept { return {&s._ctx}; }
			template<decays_to<sender> T, typename Env>
			friend constexpr _signs_t<Env> tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }

			template<decays_to<sender> T, typename Rcv>
			friend constexpr _operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<std::decay_t<Rcv>, Rcv>)
			{
				static_assert(receiver_of<Rcv, _signs_t<env_of_t<Rcv>>>);
				return _operation_t<Rcv>{s._ctx, std::forward<Rcv>(rcv)};
			}

			context &_ctx;
		};
		struct timer_sender
		{
			using is_sender = std::true_type;

			template<typename Rcv>
			using _operation_t = typename timer_operation<std::decay_t<Rcv>>::type;

			template<typename Env>
			using _stop_signs_t = std::conditional_t<detail::stoppable_env<Env>, completion_signatures<set_stopped_t()>, completion_signatures<>>;
			template<typename Env>
			using _signs_t = detail::concat_tuples_t<completion_signatures<set_value_t(), set_error_t(std::error_code)>, _stop_signs_t<Env>>;

			friend constexpr env tag_invoke(get_env_t, const timer_sender &s) noexcept { return {&s._ctx}; }
			template<decays_to<timer_sender> T, typename Env>
			friend constexpr _signs_t<Env> tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }

			template<decays_to<timer_sender> T, typename Rcv>
			friend constexpr _operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<std::decay_t<Rcv>, Rcv>)
			{
				static_assert(receiver_of<Rcv, _signs_t<env_of_t<Rcv>>>);
				return _operation_t<Rcv>{s._ctx, s._tp, std::forward<Rcv>(rcv)};
			}

			time_point _tp;
			context &_ctx;
		};
		template<typename Op>
		struct io_sender<Op>::type
		{
			using is_sender = std::true_type;
			using _signs_t = completion_signatures<set_value_t(std::size_t), set_error_t(std::error_code), set_stopped_t()>;
			template<typename Rcv>
			using _operation_t = typename io_operation<Op, std::decay_t<Rcv>>::type;
			using _io_cmd_t = typename io_cmd<Op>::type;

			template<typename... Args>
			type(context &ctx, Args &&...args) noexcept : _cmd(std::forward<Args>(args)...), _ctx(ctx) {}

			friend env tag_invoke(get_env_t, const type &s) noexcept { return {&s._ctx}; }
			template<decays_to<type> T, typename Env>
			friend _signs_t tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }
			template<decays_to<type> T, typename Rcv>
			friend _operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<std::decay_t<Rcv>, Rcv>)
			{
				static_assert(receiver_of<Rcv, _signs_t>);
				return _operation_t<Rcv>{s._ctx, std::forward<Rcv>(rcv), std::forward<T>(s)._cmd};
			}

			_io_cmd_t _cmd;
			context &_ctx;
		};

		struct scheduler
		{
			template<typename Op>
			using _io_sender_t = typename io_sender<Op>::type;

			friend constexpr auto tag_invoke(get_forward_progress_guarantee_t, const scheduler &) noexcept { return forward_progress_guarantee::weakly_parallel; }
			friend constexpr bool tag_invoke(execute_may_block_caller_t, const scheduler &) noexcept { return true; }

			template<decays_to<scheduler> T>
			friend constexpr auto tag_invoke(schedule_t, T &&s) noexcept { return sender{*s._ctx}; }
			template<decays_to<scheduler> T, decays_to<time_point> Tp>
			friend constexpr auto tag_invoke(schedule_at_t, T &&s, Tp &&tp) noexcept { return timer_sender{std::forward<Tp>(tp), *s._ctx}; }
			template<decays_to<scheduler> T, typename Dur>
			friend constexpr auto tag_invoke(schedule_after_t, T &&s, Dur &&dur) noexcept { return schedule_at(std::forward<T>(s), s.now() + dur); }

			template<typename Buff>
			friend auto tag_invoke(schedule_read_some_t, scheduler sch, int fd, Buff &&buff) noexcept { return _io_sender_t<schedule_read_some_t>{*sch._ctx, fd, buff}; }
			template<typename Buff>
			friend auto tag_invoke(schedule_write_some_t, scheduler sch, int fd, Buff &&buff) noexcept { return _io_sender_t<schedule_write_some_t>{*sch._ctx, fd, buff}; }
			template<std::convertible_to<std::size_t> Pos, typename Buff>
			friend auto tag_invoke(schedule_read_some_at_t, scheduler sch, int fd, Pos pos, Buff &&buff) noexcept { return _io_sender_t<schedule_read_some_at_t>{*sch._ctx, fd, static_cast<std::ptrdiff_t>(pos), buff}; }
			template<std::convertible_to<std::size_t> Pos, typename Buff>
			friend auto tag_invoke(schedule_write_some_at_t, scheduler sch, int fd, Pos pos, Buff &&buff) noexcept { return _io_sender_t<schedule_write_some_at_t>{*sch._ctx, fd, static_cast<std::ptrdiff_t>(pos), buff}; }

			template<typename Snd, typename Dst>
			friend decltype(auto) tag_invoke(async_read_some_t, scheduler sch, Snd &&snd, int fd, Dst &&dst)
			{
				return let_value(std::forward<Snd>(snd), [sch, fd, dst = std::forward<Dst>(dst)]()
				{
					return schedule_read_some(sch, fd, std::move(dst));
				});
			}
			template<typename Snd, typename Src>
			friend decltype(auto) tag_invoke(async_write_some_t, scheduler sch, Snd &&snd, int fd, Src &&src)
			{
				return let_value(std::forward<Snd>(snd), [sch, fd, src = std::forward<Src>(src)]()
				{
					return schedule_write_some(sch, fd, std::move(src));
				});
			}
			template<typename Snd, std::convertible_to<std::size_t> Pos, typename Dst>
			friend decltype(auto) tag_invoke(async_read_some_at_t, scheduler sch, Snd &&snd, int fd, Pos pos, Dst &&dst)
			{
				return let_value(std::forward<Snd>(snd), [sch, fd, pos, dst = std::forward<Dst>(dst)]()
				{
					return schedule_read_some_at(sch, fd, pos, std::move(dst));
				});
			}
			template<typename Snd, std::convertible_to<std::size_t> Pos, typename Src>
			friend decltype(auto) tag_invoke(async_write_some_at_t, scheduler sch, Snd &&snd, int fd, Pos pos, Src &&src)
			{
				return let_value(std::forward<Snd>(snd), [sch, fd, pos, src = std::forward<Src>(src)]()
				{
					return schedule_write_some_at(sch, fd, pos, std::move(src));
				});
			}

			/** Returns the current time point of the clock used by the context. */
			[[nodiscard]] time_point now() const noexcept { return clock::now(); }

			constexpr bool operator==(const scheduler &) const noexcept = default;

			context *_ctx;
		};

		constexpr scheduler context::get_scheduler() noexcept { return {this}; }
		constexpr auto tag_invoke(get_stop_token_t, const env &e) noexcept { return e._ctx->get_stop_token(); }
		template<typename T>
		constexpr auto tag_invoke(get_completion_scheduler_t<T>, const env &e) noexcept { return e._ctx->get_scheduler(); }
	}

	/** Linux-specific execution context implemented via io_uring. */
	using io_uring_context = _io_uring::context;

	//static_assert(rod::scheduler<decltype(std::declval<context>().get_scheduler())>);
}
#endif

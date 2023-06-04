/*
 * Created by switchblade on 2023-05-21.
 */

#pragma once

#include "../detail/adaptors/read_some.hpp"
#include "../detail/adaptors/write_some.hpp"

#ifdef ROD_HAS_EPOLL

#include <utility>
#include <thread>
#include <chrono>

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
		enum flags_t { stop_possible = 1, stop_requested = 4, dispatched = 8 };

		using clock = monotonic_clock;
		using time_point = typename clock::time_point;

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
			
			std::size_t operator()(std::error_code &err) noexcept { return fd.read(buff.data(), buff.size(), err); }

			detail::basic_descriptor fd;
			std::span<std::byte> buff;
		};
		template<>
		struct io_cmd<schedule_write_some_t>::type
		{
			template<typename T>
			constexpr type(int fd, std::span<T> buff) noexcept : fd(fd), buff(std::cbegin(buff), std::cend(buff)) {}
			
			std::size_t operator()(std::error_code &err) noexcept { return fd.write(buff.data(), buff.size(), err); }

			detail::basic_descriptor fd;
			std::span<const std::byte> buff;
		};
		template<>
		struct io_cmd<schedule_read_some_at_t>::type : io_cmd<schedule_read_some_t>::type
		{
			template<typename T>
			constexpr type(int fd, std::ptrdiff_t off, std::span<T> buff) noexcept : io_cmd<schedule_read_some_t>::type(fd, buff), off(off) {}
			
			std::size_t operator()(std::error_code &err) noexcept { return fd.read_at(buff.data(), buff.size(), off, err); }
			
			std::ptrdiff_t off;
		};
		template<>
		struct io_cmd<schedule_write_some_at_t>::type : io_cmd<schedule_write_some_t>::type
		{
			template<typename T>
			constexpr type(int fd, std::ptrdiff_t off, std::span<T> buff) noexcept : io_cmd<schedule_write_some_t>::type(fd, buff), off(off) {}

			std::size_t operator()(std::error_code &err) noexcept { return fd.write_at(buff.data(), buff.size(), off, err); }

			std::ptrdiff_t off;
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
		};
		struct timer_node : operation_base
		{
			constexpr timer_node(context &ctx, time_point tp, bool can_stop) noexcept : _flags(can_stop ? flags_t::stop_possible : 0), _tp(tp), _ctx(ctx) {}

			[[nodiscard]] bool _stop_possible() const noexcept { return _flags.load(std::memory_order_relaxed) & flags_t::stop_possible; }

			timer_node *_timer_prev = {};
			timer_node *_timer_next = {};
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
			friend struct timer_operation;
			template<typename>
			friend struct operation;
			template<typename, typename>
			friend struct io_operation;

		public:
			using time_point = _epoll::time_point;
			using clock = _epoll::clock;

		private:
			struct timer_cmp { constexpr bool operator()(const timer_node &a, const timer_node &b) const noexcept { return a._tp <= b._tp; }};
			using timer_queue_t = detail::priority_queue<timer_node, timer_cmp, &timer_node::_timer_prev, &timer_node::_timer_next>;
			using producer_queue_t = detail::atomic_queue<operation_base, &operation_base::_next>;
			using consumer_queue_t = detail::basic_queue<operation_base, &operation_base::_next>;
			
			template<typename Op>
			using io_cmd_t = typename io_cmd<Op>::type;

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
			[[nodiscard]] constexpr scheduler get_scheduler() noexcept;

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

			void schedule(operation_base *node, std::error_code &err) noexcept
			{
				if (!is_consumer_thread())
					schedule_producer(node, err);
				else
					schedule_consumer(node);
			}

			ROD_PUBLIC void schedule_producer(operation_base *node, std::error_code &err) noexcept;
			ROD_PUBLIC void schedule_consumer(operation_base *node) noexcept;

			ROD_PUBLIC void add_io(operation_base *node, io_cmd_t<schedule_read_some_t> cmd) noexcept;
			ROD_PUBLIC void add_io(operation_base *node, io_cmd_t<schedule_write_some_t> cmd) noexcept;
			ROD_PUBLIC void add_io(operation_base *node, io_cmd_t<schedule_read_some_at_t> cmd) noexcept;
			ROD_PUBLIC void add_io(operation_base *node, io_cmd_t<schedule_write_some_at_t> cmd) noexcept;
			ROD_PUBLIC void del_io(int fd) noexcept;

			ROD_PUBLIC void add_timer(timer_node *node) noexcept;
			ROD_PUBLIC void del_timer(timer_node *node) noexcept;

			bool acquire_producer_queue() noexcept;
			void epoll_wait();

			/* TID of the current consumer thread. */
			std::atomic<std::thread::id> m_consumer_tid = {};

			/* Descriptors used for EPOLL notifications. */
			detail::unique_descriptor m_epoll_fd = {};
			detail::unique_descriptor m_timer_fd = {};
			detail::unique_descriptor m_event_fd = {};

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

			time_point m_next_timeout = {};
			bool m_timer_started = false;
			bool m_timer_pending = false;
			bool m_wait_pending = false;
			bool m_stop_pending = false;
		};

		template<typename Rcv>
		struct operation<Rcv>::type : operation_base
		{
			static void _bind_notify(operation_base *ptr) noexcept
			{
				auto &op = *static_cast<type *>(ptr);

				/* Stop the top-level receiver if necessary. */
				if constexpr (detail::stoppable_env<env_of_t<Rcv>>)
					if (get_stop_token(get_env(op._rcv)).stop_requested())
					{
						set_stopped(std::move(op._rcv));
						return;
					}

				/* Complete the top-level receiver. */
				set_value(std::move(op._rcv));
			}

			template<typename Rcv1>
			constexpr type(context &ctx, Rcv1 &&rcv) : _rcv(std::forward<Rcv1>(rcv)), _ctx(ctx) { _notify_func = _bind_notify; }

			friend void tag_invoke(start_t, type &op) noexcept { op._start(); }

			void _start() noexcept
			{
				std::error_code err = {};
				_ctx.schedule(this, err);
				if (err) [[unlikely]] set_error(std::move(_rcv), err);
			}

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
				{
					_stop_cb.reset();
					_notify_func = _notify_stopped;
					if (!(_flags.load(std::memory_order_relaxed) & flags_t::dispatched))
					{
						/* Timer has not yet been dispatched, schedule stop completion. */
						_ctx.del_timer(this);
						_ctx.schedule_consumer(this);
					}
				}
				else if (!(_flags.fetch_or(flags_t::stop_requested, std::memory_order_acq_rel) & flags_t::dispatched))
				{
					/* Timer has not yet been dispatched, schedule stop completion on producer thread. */
					_notify_func = [](operation_base *ptr) noexcept
					{
						const auto op = static_cast<type *>(ptr);
						op->_stop_cb.reset();

						/* Make sure to erase the timer if it has not already been dispatched. */
						if (!(op->_flags.load(std::memory_order_relaxed) & flags_t::dispatched))
							op->_ctx.del_timer(op);

						op->_complete_stopped();
					};

					std::error_code err = {};
					_ctx.schedule_producer(this, err);
					if (err) [[unlikely]] std::terminate();
				}
			}

			using _stop_cb_t = stop_cb<env_of_t<Rcv>, &type::_request_stop>;

			[[ROD_NO_UNIQUE_ADDRESS]] _stop_cb_t _stop_cb;
			[[ROD_NO_UNIQUE_ADDRESS]] Rcv _rcv;
		};
		template<typename Op, typename Rcv>
		struct io_operation<Op, Rcv>::type : private complete_base, private stop_base
		{
			using _io_cmd_t = typename io_cmd<Op>::type;

			static void _notify_start(operation_base *ptr) noexcept { static_cast<type *>(static_cast<complete_base *>(ptr))->_start_consumer(); }
			static void _notify_value(operation_base *ptr) noexcept { static_cast<type *>(static_cast<complete_base *>(ptr))->_complete_value(); }
			static void _notify_stopped(operation_base *ptr) noexcept { static_cast<type *>(static_cast<stop_base *>(ptr))->_complete_stopped(); }

			template<typename Rcv1>
			explicit type(context &ctx, Rcv1 &&rcv, _io_cmd_t cmd) : _rcv(std::forward<Rcv1>(rcv)), _cmd(std::move(cmd)), _ctx(ctx) {}

			friend void tag_invoke(start_t, type &op) noexcept { op._start(); }

			void _complete_value() noexcept
			{
				if constexpr (detail::stoppable_env<env_of_t<Rcv>>)
					_stop_cb.reset();
				if (_flags.fetch_or(flags_t::dispatched, std::memory_order_acq_rel) & flags_t::stop_requested)
					return;

				_ctx.add_io(static_cast<complete_base *>(this), _cmd);
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
					if constexpr (detail::stoppable_env<env_of_t<Rcv>>)
						_stop_cb.init(get_env(_rcv), this);

					_ctx.add_io(static_cast<complete_base *>(this), _cmd);
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
					_ctx.del_io(_cmd.fd.native_handle());
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
				if constexpr (detail::stoppable_env<env_of_t<Rcv>>)
					set_stopped(std::move(_rcv));
				else
					std::terminate();
			}

			using _stop_cb_t = stop_cb<env_of_t<Rcv>, &type::_request_stop>;

			[[ROD_NO_UNIQUE_ADDRESS]] _stop_cb_t _stop_cb;
			[[ROD_NO_UNIQUE_ADDRESS]] Rcv _rcv;
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
			friend constexpr bool tag_invoke(execute_may_block_caller_t, const scheduler &) noexcept { return false; }

			template<decays_to<scheduler> T>
			friend constexpr auto tag_invoke(schedule_t, T &&s) noexcept { return sender{*s._ctx}; }
			template<decays_to<scheduler> T, decays_to<time_point> TP>
			friend constexpr auto tag_invoke(schedule_at_t, T &&s, TP &&tp) noexcept { return timer_sender{std::forward<TP>(tp), *s._ctx}; }
			template<decays_to<scheduler> T, typename Dur>
			friend constexpr auto tag_invoke(schedule_in_t, T &&s, Dur &&dur) noexcept { return schedule_at(std::forward<T>(s), s.now() + dur); }

			template<typename Buff>
			friend auto tag_invoke(schedule_read_some_t, scheduler sch, int fd, Buff &&buff) noexcept { return _io_sender_t<schedule_read_some_t>{*sch._ctx, fd, buff}; }
			template<typename Buff>
			friend auto tag_invoke(schedule_write_some_t, scheduler sch, int fd, Buff &&buff) noexcept { return _io_sender_t<schedule_write_some_t>{*sch._ctx, fd, buff}; }
			template<std::convertible_to<std::ptrdiff_t> Pos, typename Buff>
			friend auto tag_invoke(schedule_read_some_at_t, scheduler sch, int fd, Pos pos, Buff &&buff) noexcept { return _io_sender_t<schedule_read_some_at_t>{*sch._ctx, fd, static_cast<std::ptrdiff_t>(pos), buff}; }
			template<std::convertible_to<std::ptrdiff_t> Pos, typename Buff>
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
			template<typename Snd, std::convertible_to<std::ptrdiff_t> Pos, typename Dst>
			friend decltype(auto) tag_invoke(async_read_some_at_t, scheduler sch, Snd &&snd, int fd, Pos pos, Dst &&dst)
			{
				return let_value(std::forward<Snd>(snd), [sch, fd, pos, dst = std::forward<Dst>(dst)]()
				{
					return schedule_read_some_at(sch, fd, pos, std::move(dst));
				});
			}
			template<typename Snd, std::convertible_to<std::ptrdiff_t> Pos, typename Src>
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

	/** Linux-specific execution context implemented via EPOLL. */
	using epoll_context = _epoll::context;

	static_assert(rod::scheduler<decltype(std::declval<epoll_context>().get_scheduler())>);
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

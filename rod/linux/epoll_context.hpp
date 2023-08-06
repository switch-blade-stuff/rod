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

#include "../posix/monotonic_clock.hpp"
#include "../posix/detail/file.hpp"

namespace rod
{
	namespace _epoll
	{
		enum flags_t { stop_possible = 1, stop_requested = 4, dispatched = 8 };

		using clock = monotonic_clock;
		using time_point = typename clock::time_point;

		class context;
		class scheduler;

		struct env { context *_ctx; };

		template<typename Env, auto StopFunc>
		using stop_cb = _detail::stop_cb_adaptor<Env, StopFunc>;

		template<typename, typename>
		struct io_cmd;
		template<typename Buff>
		struct io_cmd<async_read_some_t, Buff>
		{
			_detail::basic_descriptor fd;
			Buff buff;
		};
		template<typename Buff>
		struct io_cmd<async_write_some_t, Buff>
		{
			_detail::basic_descriptor fd;
			Buff buff;
		};
		template<typename Buff>
		struct io_cmd<async_read_some_at_t, Buff>
		{
			_detail::basic_descriptor fd;
			std::size_t pos;
			Buff buff;
		};
		template<typename Buff>
		struct io_cmd<async_write_some_at_t, Buff>
		{
			_detail::basic_descriptor fd;
			std::size_t pos;
			Buff buff;
		};

		struct operation_base
		{
			using notify_func_t = void (*)(operation_base *);

			operation_base(operation_base &&) = delete;
			operation_base &operator=(operation_base &&) = delete;

			constexpr operation_base() noexcept = default;

			void notify() { std::exchange(notify_func, {})(this); }

			notify_func_t notify_func = {};
			operation_base *next = {};
		};
		struct timer_operation_base : operation_base
		{
			constexpr timer_operation_base(context *ctx, time_point tp, bool can_stop) noexcept : flags(can_stop ? flags_t::stop_possible : 0), timeout(tp), ctx(ctx) {}

			[[nodiscard]] bool stop_possible() const noexcept { return flags.load(std::memory_order_relaxed) & flags_t::stop_possible; }

			timer_operation_base *timer_prev = {};
			timer_operation_base *timer_next = {};
			std::atomic<int> flags = {};
			time_point timeout = {};
			context *ctx;
		};

		struct complete_base : operation_base {};
		struct stop_base : operation_base {};

		class sender;
		template<typename Rcv>
		struct operation { class type; };

		class timer_sender;
		template<typename Rcv>
		struct timer_operation { class type; };

		template<typename Op>
		struct io_sender { class type; };
		template<typename Op, typename Rcv>
		struct io_operation { class type; };

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
			struct timer_cmp { constexpr bool operator()(const timer_operation_base &a, const timer_operation_base &b) const noexcept { return a.timeout <= b.timeout; }};
			using timer_queue_t = _detail::priority_queue<timer_operation_base, timer_cmp, &timer_operation_base::timer_prev, &timer_operation_base::timer_next>;
			using producer_queue_t = _detail::atomic_queue<operation_base, &operation_base::next>;
			using consumer_queue_t = _detail::basic_queue<operation_base, &operation_base::next>;

			struct event_buff_t
			{
				std::size_t size;
				void *data;
			};

			template<typename Op>
			using io_cmd_t = typename io_cmd<Op>::type;

		public:
			context(context &&) = delete;
			context &operator=(context &&) = delete;

			/** Initializes the EPOLL execution context with a default max number of events.
			 * @throw std::system_error On failure to initialize descriptors.
			 * @throw std::bad_alloc On failure to allocate event buffer. */
			ROD_API_PUBLIC context();
			/** Initializes the EPOLL execution context with the specified max number of events.
			 * @param max Maximum size of the internal EPOLL event buffer.
			 * @throw std::system_error On failure to initialize descriptors.
			 * @throw std::bad_alloc On failure to allocate event buffer. */
			ROD_API_PUBLIC explicit context(std::size_t max);
			ROD_API_PUBLIC ~context();

			/** Returns a scheduler used to schedule work to be executed on the context. */
			[[nodiscard]] constexpr scheduler get_scheduler() noexcept;

			/** Blocks the current thread until `finish` is called and executes scheduled operations.
			 * @throw std::system_error On implementation-defined internal failures.
			 * @note Only one thread may call `run`. */
			ROD_API_PUBLIC void run();
			/** Changes the internal state to stopped and unblocks consumer thread. Any in-progress work will run to completion. */
			ROD_API_PUBLIC void finish();

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

			/** Returns copy of the stop source associated with the context. */
			[[nodiscard]] constexpr in_place_stop_source &get_stop_source() noexcept { return _stop_src; }
			/** Returns a stop token of the stop source associated with the context. */
			[[nodiscard]] constexpr in_place_stop_token get_stop_token() const noexcept { return _stop_src.get_token(); }
			/** Sends a stop request to the stop source associated with the context. */
			ROD_API_PUBLIC void request_stop();

		private:
			[[nodiscard]] ROD_API_PUBLIC bool is_consumer_thread() const noexcept;

			void schedule(operation_base *node)
			{
				if (!is_consumer_thread())
					schedule_producer(node);
				else
					schedule_consumer(node);
			}
			ROD_API_PUBLIC void schedule_producer(operation_base *node);
			ROD_API_PUBLIC void schedule_consumer(operation_base *node);

			void submit_event(auto *data, int flags, int fd) noexcept;
			ROD_API_PUBLIC void submit_io_event(operation_base *node, io_cmd<async_read_some_t, std::span<std::byte>> cmd) noexcept;
			ROD_API_PUBLIC void submit_io_event(operation_base *node, io_cmd<async_write_some_t, std::span<std::byte>> cmd) noexcept;
			ROD_API_PUBLIC void submit_io_event(operation_base *node, io_cmd<async_read_some_at_t, std::span<std::byte>> cmd) noexcept;
			ROD_API_PUBLIC void submit_io_event(operation_base *node, io_cmd<async_write_some_at_t, std::span<std::byte>> cmd) noexcept;
			ROD_API_PUBLIC void cancel_io_event(int fd) noexcept;

			template<one_of<async_read_some_t, async_write_some_t> Op, typename Buff>
			bool submit_io_event(operation_base *node, io_cmd<Op, Buff> cmd) noexcept
			{
				const auto bytes = rod::as_byte_buffer(const_cast<std::ranges::range_value_t<Buff> *>(std::data(cmd.buff)), std::size(cmd.buff));
				return submit_io_event(node, io_cmd<Op, std::span<std::byte>>{cmd.fd, bytes});
			}
			template<one_of<async_read_some_at_t, async_write_some_at_t> Op, typename Buff>
			bool submit_io_event(operation_base *node, io_cmd<Op, Buff> cmd) noexcept
			{
				const auto bytes = rod::as_byte_buffer(const_cast<std::ranges::range_value_t<Buff> *>(std::data(cmd.buff)), std::size(cmd.buff));
				return submit_io_event(node, io_cmd<Op, std::span<std::byte>>{cmd.fd, cmd.pos, bytes});
			}

			ROD_API_PUBLIC void add_timer(timer_operation_base *node) noexcept;
			ROD_API_PUBLIC void del_timer(timer_operation_base *node) noexcept;

			bool acquire_producer_queue() noexcept;
			void acquire_elapsed_timers();
			bool set_timer(time_point tp);
			void epoll_wait();

			/* TID of the current consumer thread. */
			std::atomic<std::thread::id> _consumer_tid = {};

			/* Descriptors used for EPOLL notifications. */
			_detail::unique_descriptor _epoll_fd = {};
			_detail::unique_descriptor _timer_fd = {};
			_detail::unique_descriptor _event_fd = {};

			/* EPOLL event buffer. */
			event_buff_t _buff;

			in_place_stop_source _stop_src;
			/* Queue of operations pending for dispatch by consumer thread. */
			consumer_queue_t _consumer_queue;
			/* Queue of operation pending for acquisition by consumer thread. */
			producer_queue_t _producer_queue;
			/* Priority queue of pending timers. */
			timer_queue_t _timers;

			time_point _next_timeout = {};
			bool _timer_started = false;
			bool _timer_pending = false;
			bool _queue_active = false;
			bool _stop_pending = false;
		};

		template<typename Rcv>
		class operation<Rcv>::type : operation_base
		{
			static void notify_complete(operation_base *ptr) noexcept
			{
				auto &op = *static_cast<type *>(ptr);
				if (get_stop_token(get_env(op._rcv)).stop_requested())
					set_stopped(std::move(op._rcv));
				else
					set_value(std::move(op._rcv));
			}

		public:
			constexpr explicit type(context *ctx, Rcv &&rcv) noexcept(std::is_nothrow_move_constructible_v<Rcv>) : _rcv(std::forward<Rcv>(rcv)), _ctx(ctx) { notify_func = notify_complete; }

			friend void tag_invoke(start_t, type &op) noexcept { op.start(); }

		private:
			void start() noexcept { _ctx->schedule(this); }

			ROD_NO_UNIQUE_ADDRESS Rcv _rcv;
			context *_ctx;
		};
		template<typename Rcv>
		class timer_operation<Rcv>::type : timer_operation_base
		{
			static void notify_value(operation_base *ptr) { static_cast<type *>(ptr)->complete_value(); }
			static void notify_stopped(operation_base *ptr) { static_cast<type *>(ptr)->complete_stopped(); }
			static void notify_request_stop(operation_base *ptr) { static_cast<type *>(ptr)->request_stop_consumer(); }

		public:
			constexpr explicit type(context *ctx, time_point timeout, Rcv rcv) noexcept(std::is_nothrow_move_constructible_v<Rcv>) : timer_operation_base(ctx, timeout, get_stop_token(get_env(rcv)).stop_possible()), _rcv(std::move(rcv)) {}

			friend void tag_invoke(start_t, type &op) noexcept { op.start(); }

		private:
			void complete_value() noexcept
			{
				/* Handle spontaneous stop requests. */
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
				{
					_stop_cb.reset();
					if (get_stop_token(get_env(_rcv)).stop_requested())
					{
						complete_stopped();
						return;
					}
				}
				set_value(std::move(_rcv));
			}
			void complete_stopped() noexcept
			{
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					set_stopped(std::move(_rcv));
				else
					std::terminate();
			}

			void start() noexcept
			{
				if (ctx->is_consumer_thread())
					start_consumer();
				else
				{
					notify_func = [](operation_base *p) noexcept { static_cast<type *>(p)->start_consumer(); };
					ctx->schedule_producer(this);
				}
			}
			void start_consumer()
			{
				/* Bail if a stop has already been requested. */
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					if (get_stop_token(get_env(_rcv)).stop_requested())
					{
						notify_func = notify_stopped;
						ctx->schedule_consumer(this);
						return;
					}

				notify_func = notify_value;
				ctx->add_timer(this);

				/* Initialize the stop callback for stoppable environments. */
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					_stop_cb.init(get_env(_rcv), this);
			}

			void request_stop()
			{
				if constexpr (!_detail::stoppable_env<env_of_t<Rcv>>)
					std::terminate();
				else if (ctx->is_consumer_thread())
					request_stop_consumer();
				else if (!(flags.fetch_or(flags_t::stop_requested, std::memory_order_acq_rel) & flags_t::dispatched))
				{
					/* Timer has not yet been dispatched, schedule stop request. */
					notify_func = notify_request_stop;
					ctx->schedule_producer(this);
				}
			}
			void request_stop_consumer()
			{
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					_stop_cb.reset();

				notify_func = notify_stopped;
				if (!(flags.load(std::memory_order_relaxed) & flags_t::dispatched))
				{
					/* Timer has not yet been dispatched, schedule stop completion. */
					ctx->del_timer(this);
					ctx->schedule_consumer(this);
				}
			}

			using stop_cb_t = stop_cb<env_of_t<Rcv>, &type::request_stop>;

			ROD_NO_UNIQUE_ADDRESS stop_cb_t _stop_cb;
			ROD_NO_UNIQUE_ADDRESS Rcv _rcv;
		};
		template<typename Op, typename Buff, typename Rcv>
		class io_operation<Op, Buff, Rcv>::type : complete_base, stop_base
		{
			using io_cmd_t = io_cmd<Op, Buff>;

			static void notify_start(operation_base *ptr) { static_cast<type *>(static_cast<complete_base *>(ptr))->start_consumer(); }
			static void notify_stopped(operation_base *ptr) { static_cast<type *>(static_cast<stop_base *>(ptr))->complete_stopped(); }
			static void notify_result(operation_base *ptr) { static_cast<type *>(static_cast<complete_base *>(ptr))->complete_result(); }

		public:
			constexpr explicit type(context *ctx, Rcv &&rcv, io_cmd_t cmd) noexcept(std::is_nothrow_move_constructible_v<Rcv>) : _rcv(std::forward<Rcv>(rcv)), _cmd(std::move(cmd)), _ctx(ctx) {}

			friend void tag_invoke(start_t, type &op) noexcept { op.start(); }

		private:
			void complete_result() noexcept
			{
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					_stop_cb.reset();
				if (_flags.fetch_or(flags_t::dispatched, std::memory_order_acq_rel) & flags_t::stop_requested)
					return;

				complete_result(_cmd());
			}
			void complete_result(const result<std::size_t, std::error_code> &res) noexcept
			{
				if (res.has_value())
					[[likely]] set_value(std::move(_rcv), res.value());
				else if (const auto err = res.error_or({}); err.value() != ECANCELED)
					set_error(std::move(_rcv), err);
				else
					set_stopped(std::move(_rcv));
			}

			void start() noexcept
			{
				if (_ctx->is_consumer_thread())
					start_consumer();
				else
				{
					complete_base::notify_func = notify_start;
					_ctx->schedule_producer(static_cast<complete_base *>(this));
				}
			}
			void start_consumer() noexcept
			{
				if (_flags.load(std::memory_order_acquire) & flags_t::stop_requested)
					return; /* Stopped on a different thread. */

				/* Schedule read operation via EPOLL. */
				complete_base::notify_func = notify_result;
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					_stop_cb.init(get_env(_rcv), this);

				_ctx->submit_io_event(static_cast<complete_base *>(this), _cmd);
			}

			void request_stop()
			{
				if (!(_flags.fetch_or(flags_t::stop_requested, std::memory_order_acq_rel) & flags_t::dispatched))
				{
					_ctx->cancel_io_event(_cmd.fd.native_handle());
					stop_base::notify_func = notify_stopped;
					_ctx->schedule_producer(static_cast<stop_base *>(this));
				}
			}
			void complete_stopped()
			{
				if (complete_base::notify_func)
				{
					stop_base::notify_func = notify_stopped;
					_ctx->schedule_consumer(static_cast<stop_base *>(this));
				}
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					set_stopped(std::move(_rcv));
				else
					std::terminate();
			}

			using stop_cb_t = stop_cb<env_of_t<Rcv>, &type::request_stop>;

			ROD_NO_UNIQUE_ADDRESS stop_cb_t _stop_cb;
			ROD_NO_UNIQUE_ADDRESS Rcv _rcv;
			std::atomic<int> _flags = {};
			io_cmd_t _cmd;
			context *_ctx;
		};

		class sender
		{
		public:
			using is_sender = std::true_type;

		private:
			template<typename Rcv>
			using operation_t = typename operation<std::decay_t<Rcv>>::type;

			template<typename Env>
			using stop_signs_t = std::conditional_t<_detail::stoppable_env<Env>, completion_signatures<set_stopped_t()>, completion_signatures<>>;
			template<typename Env>
			using signs_t = _detail::concat_tuples_t<completion_signatures<set_value_t(), set_error_t(std::error_code)>, stop_signs_t<Env>>;

		public:
			constexpr explicit sender(context *ctx) noexcept : _ctx(ctx) {}

			friend constexpr env tag_invoke(get_env_t, const sender &s) noexcept { return {s._ctx}; }
			template<decays_to<sender> T, typename Env>
			friend constexpr signs_t<Env> tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }

			template<decays_to<sender> T, rod::receiver Rcv> requires receiver_of<Rcv, signs_t<env_of_t<Rcv>>>
			friend constexpr operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<std::decay_t<Rcv>, Rcv>)
			{
				return operation_t<Rcv>{s._ctx, std::forward<Rcv>(rcv)};
			}

		private:
			context *_ctx;
		};
		class timer_sender
		{
		public:
			using is_sender = std::true_type;

		private:
			template<typename Rcv>
			using operation_t = typename timer_operation<std::decay_t<Rcv>>::type;

			template<typename Env>
			using stop_signs_t = std::conditional_t<_detail::stoppable_env<Env>, completion_signatures<set_stopped_t()>, completion_signatures<>>;
			template<typename Env>
			using signs_t = _detail::concat_tuples_t<completion_signatures<set_value_t(), set_error_t(std::error_code)>, stop_signs_t<Env>>;

		public:
			constexpr explicit timer_sender(time_point tp, context *ctx) noexcept : _tp(tp), _ctx(ctx) {}

			friend constexpr env tag_invoke(get_env_t, const timer_sender &s) noexcept { return {s._ctx}; }
			template<decays_to<timer_sender> T, typename Env>
			friend constexpr signs_t<Env> tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }

			template<decays_to<timer_sender> T, rod::receiver Rcv> requires receiver_of<Rcv, signs_t<env_of_t<Rcv>>>
			friend constexpr operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<std::decay_t<Rcv>, Rcv>)
			{
				return operation_t<Rcv>{s._ctx, s._tp, std::forward<Rcv>(rcv)};
			}

		private:
			time_point _tp;
			context *_ctx;
		};
		template<typename Op, typename Buff>
		class io_sender<Op, Buff>::type
		{
		public:
			using is_sender = std::true_type;

		private:
			using signs_t = completion_signatures<set_value_t(std::size_t), set_error_t(std::error_code), set_stopped_t()>;
			template<typename Rcv>
			using operation_t = typename io_operation<Op, Buff, std::decay_t<Rcv>>::type;
			using io_cmd_t = io_cmd<Op, Buff>;

		public:
			template<typename... Args>
			constexpr explicit type(context *ctx, Args &&...args) noexcept : _cmd(std::forward<Args>(args)...), _ctx(ctx) {}

			friend env tag_invoke(get_env_t, const type &s) noexcept { return {s._ctx}; }
			template<decays_to<type> T, typename Env>
			friend signs_t tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }

			template<decays_to<type> T, receiver_of<signs_t> Rcv>
			friend operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<std::decay_t<Rcv>, Rcv>) { return operation_t<Rcv>{s._ctx, std::forward<Rcv>(rcv), std::forward<T>(s)._cmd}; }

		private:
			io_cmd_t _cmd;
			context *_ctx;
		};

		/* epoll scheduler handles. */
		class file_handle : public _file::system_handle
		{
			template<typename Op, typename Buff>
			using io_sender_t = typename io_sender<Op, Buff>::type;

		public:
			file_handle() = delete;

			constexpr file_handle(file_handle &&other) noexcept { swap(other); }
			constexpr file_handle &operator=(file_handle &&other) noexcept { return (swap(other), *this); }

			constexpr explicit file_handle(context *ctx) noexcept : _ctx(ctx) {}

			std::error_code open(const char *path, int mode, int prot) noexcept { return _file::system_handle::open(path, mode, prot | _file::system_handle::nonblock); }
			std::error_code open(const wchar_t *path, int mode, int prot) noexcept { return _file::system_handle::open(path, mode, prot | _file::system_handle::nonblock); }

			constexpr void swap(file_handle &other) noexcept
			{
				_file::system_handle::swap(other);
				std::swap(_ctx, other._ctx);
			}
			friend constexpr void swap(file_handle &a, file_handle &b) noexcept { a.swap(b); }

		public:
			template<reference_to<file_handle> Hnd, typename Buff>
			friend auto tag_invoke(async_read_some_t, Hnd &&hnd, Buff &&buff) noexcept { return io_sender_t<async_read_some_t, Buff>{hnd._ctx, hnd.native_handle(), std::forward<Buff>(buff)}; }
			template<reference_to<file_handle> Hnd, typename Buff>
			friend auto tag_invoke(async_write_some_t, Hnd &&hnd, Buff &&buff) noexcept { return io_sender_t<async_write_some_t, Buff>{hnd._ctx, hnd.native_handle(), std::forward<Buff>(buff)}; }
			template<reference_to<file_handle> Hnd, std::convertible_to<std::size_t> Pos, typename Buff>
			friend auto tag_invoke(async_read_some_at_t, Hnd &&hnd, Pos pos, Buff &&buff) noexcept { return io_sender_t<async_read_some_at_t, Buff>{hnd._ctx, hnd.native_handle(), static_cast<std::size_t>(pos), std::forward<Buff>(buff)}; }
			template<reference_to<file_handle> Hnd, std::convertible_to<std::size_t> Pos, typename Buff>
			friend auto tag_invoke(async_write_some_at_t, Hnd &&hnd, Pos pos, Buff &&buff) noexcept { return io_sender_t<async_write_some_at_t, Buff>{hnd._ctx, hnd.native_handle(), static_cast<std::size_t>(pos), std::forward<Buff>(buff)}; }

		private:
			/* Hide base open so the above overloads will work. */
			using _file::system_handle::open;

			context *_ctx = nullptr;
		};
		/* fifo_handle */
		/* sock_handle */

		class scheduler
		{
			template<typename Op, typename Buff>
			using io_sender_t = typename io_sender<Op, Buff>::type;

		public:
			constexpr explicit scheduler(context *ctx) noexcept : _ctx(ctx) {}

			friend constexpr auto tag_invoke(get_forward_progress_guarantee_t, const scheduler &) noexcept { return forward_progress_guarantee::weakly_parallel; }
			friend constexpr bool tag_invoke(execute_may_block_caller_t, const scheduler &) noexcept { return true; }

			template<decays_to<scheduler> T>
			friend constexpr auto tag_invoke(schedule_t, T &&s) noexcept { return sender{s._ctx}; }
			template<decays_to<scheduler> T, decays_to<time_point> Tp>
			friend constexpr auto tag_invoke(schedule_at_t, T &&s, Tp &&tp) noexcept { return timer_sender{std::forward<Tp>(tp), s._ctx}; }
			template<decays_to<scheduler> T, typename Dur>
			friend constexpr auto tag_invoke(schedule_after_t, T &&s, Dur &&dur) noexcept { return schedule_at(std::forward<T>(s), s.now() + dur); }

			template<decays_to<scheduler> T>
			friend result<_file::basic_file<file_handle>, std::error_code> tag_invoke(open_file_t, T &&sch, auto &&path, int mode, int prot) noexcept
			{
				auto file = _file::basic_file<file_handle>{sch._ctx};
				if (auto err = file.open(path, mode, prot); !err) [[likely]]
					return file;
				else
					return err;
			}

			/** Returns the current time point of the clock used by the context. */
			[[nodiscard]] time_point now() const noexcept { return clock::now(); }

			[[nodiscard]] friend constexpr bool operator==(const scheduler &, const scheduler &) noexcept = default;

		private:
			context *_ctx;
		};

		constexpr scheduler context::get_scheduler() noexcept { return scheduler{this}; }
		constexpr auto tag_invoke(get_stop_token_t, const env &e) noexcept { return e._ctx->get_stop_token(); }
		template<typename T>
		constexpr auto tag_invoke(get_completion_scheduler_t<T>, const env &e) noexcept { return e._ctx->get_scheduler(); }
	}

	/** Linux-specific execution context implemented via EPOLL. */
	using epoll_context = _epoll::context;

	static_assert(rod::scheduler<decltype(std::declval<epoll_context>().get_scheduler())>);
}
#endif

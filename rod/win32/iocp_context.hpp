/*
 * Created by switchblade on 2023-07-04.
 */

#pragma once

#include "../detail/config.hpp"

#ifdef ROD_WIN32

#include <utility>

#include "../detail/io_handle_base.hpp"
#include "../detail/priority_queue.hpp"
#include "../detail/atomic_queue.hpp"
#include "../detail/basic_queue.hpp"
#include "../detail/file_handle.hpp"
#include "../detail/tid_lock.hpp"

#include "../stop_token.hpp"
#include "../detail/queries/may_block.hpp"
#include "../detail/queries/scheduler.hpp"
#include "../detail/queries/progress.hpp"

namespace rod
{
	namespace _iocp
	{
		using extent_type = _handle::extent_type;
		using extent_pair = std::pair<extent_type, extent_type>;

		enum flags_t { timeout_requested = 1, stop_requested = 2 };
		enum event_id { wakeup = 0, timeout = 1 };

		class context;
		class scheduler;
		class file_handle;

		struct env { context *_ctx; };

		template<typename... Signs>
		class sender_base
		{
		public:
			using is_sender = std::true_type;

		private:
			template<typename Env>
			using stop_signs_t = std::conditional_t<_detail::stoppable_env<Env>, completion_signatures<set_stopped_t()>, completion_signatures<>>;
			template<typename Env>
			using signs_t = _detail::concat_tuples_t<completion_signatures<Signs...>, stop_signs_t<Env>>;

		public:
			constexpr explicit sender_base(context *ctx) noexcept : _ctx(ctx) {}

			friend constexpr env tag_invoke(get_env_t, const sender_base &s) noexcept { return {s._ctx}; }
			template<decays_to_derived<sender_base> T, typename Env>
			friend constexpr signs_t<Env> tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }

		protected:
			context *_ctx;
		};

		template<typename Env, auto StopFunc>
		using stop_cb = _detail::stop_cb_adaptor<Env, StopFunc>;

		struct io_operation_base;
		struct timer_operation_base;

		class basic_sender;
		class timer_sender;

		template<typename Rcv>
		struct basic_operation { class type; };
		template<typename Rcv>
		struct timer_operation { class type; };
		template<typename Vst, typename Rcv>
		struct io_operation { class type; };

		struct operation_base
		{
			using notify_func_t = void (*)(operation_base *) noexcept;

			operation_base(operation_base &&) = delete;
			operation_base &operator=(operation_base &&) = delete;

			constexpr operation_base() noexcept = default;

			void notify() noexcept { std::exchange(notify_func, {})(this); }

			notify_func_t notify_func = {};
			operation_base *next = {};
			operation_base *prev = {};
		};
		struct timer_operation_base : operation_base
		{
			constexpr timer_operation_base(fs::file_timeout to) noexcept : to(to) {}

			timer_operation_base *timer_prev = {};
			timer_operation_base *timer_next = {};
			std::atomic<int> flags;
			fs::file_timeout to;
		};
		template<typename F>
		struct callback_operation : operation_base, empty_base<F>
		{
			static void notify(operation_base *ptr) noexcept { static_cast<callback_operation *>(ptr)->value()(); }
			constexpr callback_operation(F &&func) noexcept : empty_base<F>(std::forward<F>(func)) {}
		};

		struct io_event
		{
			io_operation_base *parent;
			io_event *next;

			std::uintptr_t iosb[2];
			std::uintptr_t data;
		};

		inline constexpr std::size_t event_buffer_size = 1024;
		static_assert(sizeof(io_event) * event_buffer_size == sizeof(void *) * 0x1400, "sizeof(io_event) * event_buffer_size = 0x10000 (64-bit) | 0x5000 (32-bit)");

		/* Use 2 operation headers, one to hold state of pending IO requests, another to hold state of stop requests. */
		struct io_stop_operation : timer_operation_base {};
		struct io_operation_base : operation_base
		{
			using event_cb_t = void (*)(io_operation_base *, std::error_code, std::uintptr_t, std::uintptr_t);
			using event_queue_t = _detail::basic_queue<io_event, &io_event::next>;

			io_operation_base(context *ctx, void *hnd) noexcept : ctx(ctx), handle(hnd) {}

			ROD_API_PUBLIC result<> cancel_events() noexcept;
			ROD_API_PUBLIC result<> release_events() noexcept;

			template<auto F>
			void set_event_cb() noexcept { event_cb_func = [](auto *ptr, auto info, auto data) noexcept { std::invoke(F, static_cast<io_operation_base *>(ptr), info, data); }; }

			void event_cb(std::error_code status, std::uintptr_t info, std::uintptr_t data) noexcept { event_cb_func(this, status, info, data); }

			context *ctx;
			void *handle;
			event_cb_t event_cb_func;
			event_queue_t event_queue;
			std::size_t started = 0;
			std::size_t pending = 0;
			std::size_t waiting = 0;
		};

		class context : operation_base
		{
			template<typename>
			friend struct basic_operation;
			template<typename>
			friend struct timer_operation;

			friend struct io_operation_base;
			template<typename, typename>
			friend struct io_operation;

		public:
			using time_point = fs::file_time_point;
			using clock = fs::file_clock;

		private:
			struct timer_cmp { constexpr bool operator()(const timer_operation_base &a, const timer_operation_base &b) const noexcept { return a.to.absolute({}) <= b.to.absolute({}); }};
			using timer_queue_t = _detail::priority_queue<timer_operation_base, timer_cmp, &timer_operation_base::timer_prev, &timer_operation_base::timer_next>;

			using waitlist_queue_t = _detail::basic_queue<operation_base, &operation_base::next, &operation_base::prev>;
			using producer_queue_t = _detail::atomic_queue<operation_base, &operation_base::next>;
			using consumer_queue_t = _detail::basic_queue<operation_base, &operation_base::next>;

			using io_event_pool_t = _detail::basic_queue<io_event, &io_event::next>;
			using io_event_buff_t = malloc_ptr<io_event[]>;

		public:
			context(context &&) = delete;
			context &operator=(context &&) = delete;

			/** Initializes IO context with the default IO event buffer size. */
			ROD_API_PUBLIC context();
			/** Initializes IO context with the specified IO event buffer size. */
			ROD_API_PUBLIC context(std::size_t buff_size);

			/** Returns a scheduler used to schedule operations and timers to be executed by the context. */
			[[nodiscard]] constexpr scheduler get_scheduler() noexcept;

			/** Returns reference to the stop source associated with the context. */
			[[nodiscard]] constexpr in_place_stop_source &get_stop_source() noexcept { return _stop_src; }
			/** Returns a stop token of the stop source associated with the context. */
			[[nodiscard]] constexpr in_place_stop_token get_stop_token() const noexcept { return _stop_src.get_token(); }

			/** Returns `true` if the context is active, and `false` if `finish` has been called. */
			[[nodiscard]] bool active() const noexcept { return _active.test(std::memory_order_acquire); }

			/** Dispatches scheduled operations from the consumer queue.
			 * @return Amount of scheduled operations dispatched.
			 * @note Returned number might not be the same as the one returned by `poll`. */
			ROD_API_PUBLIC std::size_t run_once();
			/** Acquires pending operations and elapsed timers, and schedules them to the consumer queue. Blocks if there are no pending operations or timers and \a block is equal to `true`.
			 * @param block If set to `true` blocks the thread until new operations or timers are scheduled to the consumer queue, or the context is terminated via `finish`.
			 * @return Amount of operations and timers scheduled to the consumer queue to be executed on next call to `run_once`. */
			ROD_API_PUBLIC std::size_t poll(bool block = true);

			/** Blocks the current thread until `finish` is called and executes scheduled operations and timers. */
			void run() { run(never_stop_token{}); }
			/** Blocks the current thread until `finish` is called or stopped via \a tok and executes scheduled operations and timers.
			 * @param token Stop token used to stop execution of the event loop. */
			template<stoppable_token Token>
			void run(Token &&token)
			{
				auto flag = std::atomic_flag();
				auto op = callback_operation([&flag] { flag.test_and_set(std::memory_order_release); });
				auto cb = _detail::make_stop_callback(std::forward<Token>(token), [&op, this]() { schedule(&op); });

				for (std::size_t pending = 0;;)
				{
					if (pending != 0) [[likely]]
					{
						if (const auto n = run_once(); n <= pending)
							pending -= n;
						else
							pending = 0;
					}
					if (!flag.test(std::memory_order_acquire) && active())
						pending += poll(pending == 0);
					else
						break;
				}
			}

			/** Changes the internal state to stopped and unblocks waiting threads. Any in-progress work will run to completion. */
			void finish() { notify_func = [](auto *p) noexcept { static_cast<context *>(p)->_active.clear(std::memory_order_release); }, schedule(this); }
			/** Sends a stop request to the stop source associated with the run loop. */
			void request_stop() { _stop_src.request_stop(); }

		private:
			[[nodiscard]] auto lock_consumer_queue() noexcept { return _consumer_lock.lock(), defer_invoke([&]() { _producer_queue.try_activate(), _consumer_lock.unlock(); }); }
			[[nodiscard]] bool is_consumer_thread() const noexcept { return _consumer_lock.tid.load(std::memory_order_acquire) == std::this_thread::get_id(); }
			[[nodiscard]] bool has_pending() const noexcept { return !_consumer_queue.empty() || !_producer_queue.empty() || _timer_pending; }

			void schedule(operation_base *node) noexcept
			{
				if (!is_consumer_thread())
					schedule_producer(node);
				else
					schedule_consumer(node);
			}
			ROD_API_PUBLIC void schedule_producer(operation_base *node) noexcept;
			ROD_API_PUBLIC void schedule_consumer(operation_base *node) noexcept;
			ROD_API_PUBLIC void schedule_waitlist(operation_base *node) noexcept;
			
			ROD_API_PUBLIC void add_timer(timer_operation_base *node) noexcept;
			ROD_API_PUBLIC void del_timer(timer_operation_base *node) noexcept;

			ROD_API_PUBLIC io_event *request_io_event() noexcept;
			ROD_API_PUBLIC void release_io_event(io_event *) noexcept;

			result<> port_bind(void *hnd) noexcept;
			void set_timer(time_point tp) noexcept;

			void acquire_waiting_events() noexcept;
			void acquire_elapsed_timers() noexcept;
			void acquire_producer_queue() noexcept;

			_detail::tid_lock _consumer_lock;

			basic_handle _iocp;
			io_event_buff_t _io_event_buff;
			io_event_pool_t _io_event_pool;
			std::size_t _io_requested = 0;

			in_place_stop_source _stop_src;
			producer_queue_t _producer_queue;
			consumer_queue_t _consumer_queue;
			waitlist_queue_t _waitlist_queue;

			basic_handle _thread_timer;
			timer_queue_t _timer_queue;
			time_point _next_timeout;

			std::atomic_flag _active;
			bool _timer_started = {};
			bool _timer_pending = {};
		};

		template<typename Rcv>
		class basic_operation<Rcv>::type : operation_base, empty_base<Rcv>
		{
			using rcv_base = empty_base<Rcv>;

		public:
			constexpr type(context *ctx, Rcv &&rcv) noexcept(std::is_nothrow_move_constructible_v<Rcv>) : rcv_base(std::forward<Rcv>(rcv)), _ctx(ctx) {}

			friend void tag_invoke(start_t, type &op) noexcept { op.start(); }

		private:
			void start() noexcept
			{
				notify_func = [](auto *p) noexcept { static_cast<type *>(p)->complete(); };
				_ctx->schedule(this);
			}
			void complete() noexcept
			{
				if (get_stop_token(get_env(rcv_base::value())).stop_requested())
					set_stopped(std::move(rcv_base::value()));
				else
					set_value(std::move(rcv_base::value()));
			}

			context *_ctx;
		};
		template<typename Rcv>
		class timer_operation<Rcv>::type : timer_operation_base, empty_base<Rcv>
		{
			using rcv_base = empty_base<Rcv>;

		public:
			constexpr type(context *ctx, fs::file_time_point tp, Rcv &&rcv) noexcept(std::is_nothrow_move_constructible_v<Rcv>) : timer_operation_base(tp), rcv_base(std::forward<Rcv>(rcv)), _ctx(ctx) {}

			friend void tag_invoke(start_t, type &op) noexcept { op.start(); }

		private:
			void start() noexcept
			{
				if (_ctx->is_consumer_thread())
					start_consumer();
				else
					start_producer();
			}
			void start_consumer() noexcept
			{
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					if (get_stop_token(get_env(rcv_base::value())).stop_requested())
					{
						notify_func = [](auto *p) noexcept { static_cast<type *>(p)->complete_stopped(); };
						_ctx->schedule_consumer(this);
						return;
					}

				notify_func = [](auto *p) noexcept { static_cast<type *>(p)->complete_value(); };
				_ctx->add_timer(this);

				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					_stop_cb.init(get_env(rcv_base::value()), this);
			}
			void start_producer() noexcept
			{
				notify_func = [](auto *p) noexcept { static_cast<type *>(p)->start_consumer(); };
				_ctx->schedule_producer(this);
			}

			void request_stop() noexcept
			{
				if (timer_operation_base::flags.fetch_or(flags_t::stop_requested, std::memory_order_acq_rel) & flags_t::timeout_requested)
					return;
				if (_ctx->is_consumer_thread())
					request_stop_consumer();
				else
					request_stop_producer();
			}
			void request_stop_consumer() noexcept
			{
				notify_func = [](auto *p) noexcept { static_cast<type *>(p)->complete_stopped(); };
				_ctx->del_timer(this);
				_ctx->schedule_consumer(this);
			}
			void request_stop_producer() noexcept
			{
				notify_func = [](auto *p) noexcept { static_cast<type *>(p)->request_stop_consumer(); };
				_ctx->schedule_producer(this);
			}

			void complete_value() noexcept
			{
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					_stop_cb.reset();
				set_value(std::move(rcv_base::value()));
			}
			void complete_stopped() noexcept
			{
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					set_stopped(std::move(empty_base<Rcv>::value()));
				else
					std::terminate();
			}

			context *_ctx;
			stop_cb<env_of_t<Rcv>, &type::request_stop> _stop_cb;
		};

		class basic_sender : public sender_base<set_value_t()>
		{
			template<typename Rcv>
			using operation_t = typename basic_operation<std::decay_t<Rcv>>::type;

		public:
			using sender_base::sender_base;

			template<decays_to_same<basic_sender> T, rod::receiver Rcv> requires receiver_of<Rcv, completion_signatures_of_t<basic_sender, env_of_t<Rcv>>>
			friend constexpr operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<std::decay_t<Rcv>, Rcv>)
			{
				return operation_t<Rcv>{s._ctx, std::forward<Rcv>(rcv)};
			}
		};
		class timer_sender : public sender_base<set_value_t()>
		{
			template<typename Rcv>
			using operation_t = typename timer_operation<std::decay_t<Rcv>>::type;

		public:
			constexpr explicit timer_sender(context *ctx, fs::file_time_point tp) noexcept : sender_base(ctx), _tp(tp) {}

			template<decays_to_same<timer_sender> T, rod::receiver Rcv> requires receiver_of<Rcv, completion_signatures_of_t<timer_sender, env_of_t<Rcv>>>
			friend constexpr operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<std::decay_t<Rcv>, Rcv>)
			{
				return operation_t<Rcv>(s._ctx, s._tp, std::forward<Rcv>(rcv));
			}

		private:
			fs::file_time_point _tp;
		};

		template<typename Vst, typename Rcv>
		class io_operation<Vst, Rcv>::type : io_operation_base, io_stop_operation, empty_base<Vst>, empty_base<Rcv>
		{
			using stop_cb_t = stop_cb<env_of_t<Rcv>, &type::request_stop>;
			using vst_base = empty_base<Vst>;
			using rcv_base = empty_base<Rcv>;

		public:
			type(type &&) = delete;
			type &operator=(type &&) = delete;

			type(const type &) = delete;
			type &operator=(const type &) = delete;

			type(scheduler sch, void *hnd, Vst &&vst, fs::file_timeout to, Rcv &&rcv) noexcept;

			template<std::same_as<start_t> T, decays_to_same<type> Op>
			friend void tag_invoke(T, Op &op) noexcept { op.start(); }

		private:
			template<auto F>
			void set_timer_completion() noexcept { io_stop_operation::notify_func = [](auto *ptr) noexcept { std::invoke(F, static_cast<type *>(static_cast<io_stop_operation *>(ptr))); }; }
			template<auto F>
			void set_primary_completion() noexcept { io_operation_base::notify_func = [](auto *ptr) noexcept { std::invoke(F, static_cast<type *>(static_cast<io_operation_base *>(ptr))); }; }

			void schedule_consumer() noexcept { io_operation_base::ctx->schedule_consumer(static_cast<io_operation_base *>(this)); }
			void schedule_producer() noexcept { io_operation_base::ctx->schedule_producer(static_cast<io_operation_base *>(this)); }
			void schedule_waitlist() noexcept { io_operation_base::ctx->schedule_waitlist(static_cast<io_operation_base *>(this)); }
			void schedule_timer_consumer() noexcept { io_operation_base::ctx->schedule_consumer(static_cast<io_stop_operation *>(this)); }
			void schedule_timer_producer() noexcept { io_operation_base::ctx->schedule_producer(static_cast<io_stop_operation *>(this)); }

			void start() noexcept
			{
				if (io_operation_base::ctx->is_consumer_thread())
					start_consumer();
				else
					start_producer();
			}
			bool start_child_io() noexcept
			{
				set_primary_completion<&type::complete_primary>();
				set_event_cb<&type::io_event_callback>();
				io_operation_base::waiting = 0;

				/* Iteratively request start of IO events. */
				for (io_event *event_ptr = nullptr;;)
				{
					const auto event_pos = io_operation_base::started + io_operation_base::waiting;
					event_ptr = event_ptr ? event_ptr : io_operation_base::ctx->request_io_event();

					/* !started !pending -> no more IO can be started.
					 * !started pending -> requires an async IO event.
					 * started !pending -> synchronous event completion.
					 * started pending -> asynchronous event completion. */
					const auto [started, pending] = vst_base::start_io(event_pos, event_ptr, io_operation_base::handle);
					if (!started && !pending)
					{
						io_operation_base::ctx->release_io_event(event_ptr);
						break;
					}
					if (!started && pending)
					{
						io_operation_base::waiting += event_ptr == nullptr;
						continue;
					}
					if (pending)
					{
						io_operation_base::event_queue.push_back(event_ptr);
						event_ptr = nullptr;
					}
					io_operation_base::started += started;
					io_operation_base::pending += pending;
				}

				if (io_operation_base::waiting != 0)
					schedule_waitlist();
				return io_operation_base::pending + io_operation_base::waiting;
			}
			void start_consumer() noexcept
			{
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					if (get_stop_token(get_env(rcv_base::value())).stop_requested())
					{
						complete_stopped();
						return;
					}

				/* Complete immediately if there are no pending or waiting IO events. */
				if (!start_child_io())
				{
					complete_result();
					return;
				}

				set_timer_completion<schedule_cancellation<&type::complete_timeout>>();
				io_operation_base::ctx->add_timer(this);

				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					_stop_cb.init(get_env(rcv_base::value()), this);
			}
			void start_producer() noexcept
			{
				set_primary_completion<&type::start_consumer>();
				schedule_producer();
			}

			void request_stop() noexcept
			{
				if (io_stop_operation::flags.fetch_or(flags_t::stop_requested, std::memory_order_acq_rel) & flags_t::timeout_requested)
					return;

				if (io_operation_base::ctx->is_consumer_thread())
					request_stop_consumer();
				else
					request_stop_producer();
			}
			void request_stop_consumer() noexcept
			{
				set_timer_completion<schedule_cancellation<&type::complete_stopped>>();
				io_operation_base::ctx->del_timer(this);
				schedule_timer_consumer();
			}
			void request_stop_producer() noexcept
			{
				set_timer_completion<request_stop_consumer>();
				schedule_timer_producer();
			}
			template<auto F>
			void schedule_cancellation() noexcept
			{
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					_stop_cb.reset();

				set_primary_completion<F>();
				if (io_operation_base::pending || io_operation_base::waiting)
					io_operation_base::cancel_events();
				else
					schedule_consumer();
			}

			void complete_primary() noexcept
			{
				if (io_stop_operation::flags.load(std::memory_order_acquire))
					return;
				if (io_operation_base::waiting != 0 && start_child_io())
					return;
				if (io_operation_base::pending != 0)
					return;

				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					_stop_cb.reset();
				complete_result();
			}
			void complete_timeout() noexcept
			{
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					_stop_cb.reset();
				complete_result(std::make_error_code(std::errc::timed_out));
			}
			void complete_stopped() noexcept
			{
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					set_stopped(std::move(rcv_base::value()));
				else
					std::terminate();
			}
			template<typename... Args>
			void complete_result(Args &&...args) noexcept
			{
				const auto into_val = [this]<typename Val>(Val &&val) noexcept { set_value(std::move(rcv_base::value()), std::forward<Val>(val)); };
				const auto into_err = [this]<typename Err>(Err &&err) noexcept { set_error(std::move(rcv_base::value()), std::forward<Err>(err)); };
				vst_base::get_result(std::forward<Args>(args)...).into(into_val, into_err);
			}

			void io_event_callback(std::error_code status, std::uintptr_t info, std::uintptr_t data) noexcept
			{
				if (--pending == 0)
				{
					/* Release current events if this is the last one pending to avoid situations where an operation is waiting for events that never get released. */
					(void) io_operation_base::release_events();
					/* Eagerly schedule completion if we are not part of the waitlist. Otherwise the waitlist will take care of us instead. */
					if (io_operation_base::waiting == 0)
						schedule_consumer();
				}
				vst_base::complete_io(status, info, data);
			}

			stop_cb_t _stop_cb;
		};

		class file_handle : public io_handle_adaptor<file_handle, fs::file_handle, fs::fs_handle_adaptor>
		{
			friend io_handle_adaptor<file_handle, fs::file_handle, fs::fs_handle_adaptor>;
			friend fs::fs_handle_adaptor<file_handle, fs::file_handle>;
			friend handle_adaptor<file_handle, fs::file_handle>;
			friend class scheduler;

			using adaptor = io_handle_adaptor<file_handle, fs::file_handle, fs::fs_handle_adaptor>;

			template<typename Op>
			class io_visitor;

		public:
			template<typename Op>
			using io_buffer = io_buffer_t<fs::file_handle, Op>;
			template<typename Op>
			using io_buffer_sequence = io_buffer_sequence_t<fs::file_handle, Op>;

			template<typename Op>
			using io_result = io_result_t<fs::file_handle, Op>;
			template<typename Op>
			using io_request = io_request_t<fs::file_handle, Op>;

		private:
			template<auto IoFunc, typename Op>
			inline static result<std::size_t> invoke_io_func(io_event &event, void *hnd, io_buffer<Op> &buff, extent_type &off) noexcept;

			ROD_API_PUBLIC static result<std::size_t> start_io(io_event &event, void *hnd, io_buffer<read_some_at_t> &buff, extent_type &off) noexcept;
			ROD_API_PUBLIC static result<std::size_t> start_io(io_event &event, void *hnd, io_buffer<write_some_at_t> &buff, extent_type &off) noexcept;

			file_handle(fs::file_handle &&hnd, context *ctx) noexcept : adaptor(std::forward<fs::file_handle>(hnd)), _ctx(ctx) {}

		public:
			template<decays_to_same<file_handle> Hnd, typename Dur>
			friend result<file_handle> tag_invoke(fs::file_handle::reopen_t, Hnd &&hnd, fs::file_flags flags = fs::file_flags::read, fs::file_caching caching = fs::file_caching::all) noexcept
			{
				if (auto hnd_res = fs::file_handle::reopen(std::forward<Hnd>(hnd).adaptor::base(), flags | fs::file_flags::non_blocking, caching); hnd_res.has_value()) [[likely]]
					return hnd._ctx->port_bind(hnd_res->native_handle()).transform_value([&]() { return file_handle(std::move(*hnd_res), hnd._ctx); });
				else
					return hnd_res.error();
			}

		private:
			io_result<read_some_at_t> do_read_some_at(io_request<read_some_at_t>, const fs::file_timeout &) = delete;
			io_result<write_some_at_t> do_write_some_at(io_request<write_some_at_t>, const fs::file_timeout &) = delete;

			io_result<zero_extents_t> do_zero_extents(io_request<zero_extents_t>, const fs::file_timeout &) = delete;
			io_result<list_extents_t> do_list_extents(io_request<list_extents_t>, const fs::file_timeout &) const = delete;
			io_result<clone_extents_to_t> do_clone_extents_to(io_request<clone_extents_to_t>, const fs::file_timeout &) = delete;

		public:
			file_handle(const file_handle &) = delete;
			file_handle &operator=(const file_handle &) = delete;

			/** Initializes a closed file handle. */
			file_handle() noexcept = default;
			file_handle(file_handle &&other) noexcept : adaptor(std::forward<adaptor>(other)), _ctx(std::exchange(other._ctx, {})) {}
			file_handle &operator=(file_handle &&other) noexcept { return (adaptor::operator=(std::forward<adaptor>(other)), std::swap(_ctx, other._ctx), *this); }

			/** Returns the flags of the file handle. */
			[[nodiscard]] constexpr fs::file_flags flags() const noexcept { return base().flags(); }
			/** Returns the caching mode of the file handle. */
			[[nodiscard]] constexpr fs::file_caching caching() const noexcept { return base().caching(); }

			constexpr void swap(file_handle &other) noexcept
			{
				adl_swap(base(), other.base());
				adl_swap(_ctx, other._ctx);
			}
			friend constexpr void swap(file_handle &a, file_handle &b) noexcept { a.swap(b); }

		private:
			auto do_clone() const noexcept { return clone(base()).transform_value([&](fs::file_handle &&hnd) { return file_handle(std::forward<fs::file_handle>(hnd), _ctx); }); }

			/* TODO: Implement async IO operations. */

			context *_ctx = nullptr;
		};

		template<one_of<read_some_at_t, write_some_at_t> Op>
		class file_handle::io_visitor<Op>
		{
		public:
			template<typename Req>
			constexpr explicit io_visitor(Req &&req) noexcept : _buffs(req.buffs), _off(req.off), _error(), _prt() {}

			auto get_result(std::error_code error) const noexcept
			{
				if ((error = _error != std::error_code() ? _error : error) != std::error_code()) [[unlikely]]
					return io_result_t<file_handle, Op>(in_place_error, error, _prt);
				else
					return io_result_t<file_handle, Op>(in_place_value, _buffs);
			}

			auto start_io(std::size_t pos, io_event *event, void *hnd) noexcept
			{
				if (pos >= _buffs.size()) [[unlikely]]
					return std::make_pair(false, false);
				else if (event == nullptr) [[unlikely]]
					return std::make_pair(false, true);
				else if (_buffs[pos].empty()) [[unlikely]]
					return std::make_pair(true, false);

				const auto res = file_handle::start_io(*event, hnd, _buffs[event->data = pos], _off);
				if (res.has_error()) [[unlikely]]
					return (_error = res.error(), std::make_pair(true, false));
				else if (*res != 0)
					return (_prt += *res, std::make_pair(true, false));
				else
					return std::make_pair(true, true);
			}
			void complete_io(std::error_code error, std::uintptr_t info, std::uintptr_t data) noexcept
			{
				if (info > std::numeric_limits<std::size_t>::max() || data > _buffs.size())
					error = std::make_error_code(std::errc::value_too_large);
				if (_error == std::error_code())
					_error = error;

				const auto len = std::size_t(info);
				const auto idx = std::size_t(data);

				_buffs[idx] = io_buffer<Op>(_buffs[idx].data(), len);
				_prt += len;
			}

		private:
			io_buffer_sequence<Op> _buffs;
			extent_type _off;

			std::error_code _error;
			extent_type _prt;
		};

		class scheduler
		{
			template<typename, typename>
			friend struct io_operation;

		public:
			constexpr explicit scheduler(context *ctx) noexcept : _ctx(ctx) {}

			/** Returns the current time point of the clock used by the context. */
			[[nodiscard]] fs::file_time_point now() const noexcept { return fs::file_clock::now(); }

			[[nodiscard]] friend constexpr bool operator==(const scheduler &a, const scheduler &b) noexcept { return a._ctx == b._ctx; }
			[[nodiscard]] friend constexpr bool operator!=(const scheduler &a, const scheduler &b) noexcept { return a._ctx != b._ctx; }

		private:
			context *_ctx;

		public:
			friend constexpr auto tag_invoke(get_forward_progress_guarantee_t, const scheduler &) noexcept { return forward_progress_guarantee::weakly_parallel; }
			friend constexpr bool tag_invoke(execute_may_block_caller_t, const scheduler &) noexcept { return true; }

			template<decays_to_same<scheduler> T>
			friend constexpr auto tag_invoke(schedule_t, T &&s) noexcept { return basic_sender{s._ctx}; }
			template<decays_to_same<scheduler> T, typename Tp> requires std::constructible_from<fs::file_time_point, Tp>
			friend constexpr auto tag_invoke(schedule_at_t, T &&s, Tp &&tp) noexcept { return timer_sender(s._ctx, fs::file_time_point(std::forward<Tp>(tp))); }
			template<decays_to_same<scheduler> T, typename Dur>
			friend constexpr auto tag_invoke(schedule_after_t, T &&s, Dur &&dur) noexcept { return schedule_at(std::forward<T>(s), s.now() + dur); }

			template<decays_to_same<scheduler> T>
			friend result<file_handle> tag_invoke(fs::file_handle::open_t, T &&s, const fs::path_handle &base, fs::path_view path, fs::file_flags flags = fs::file_flags::read, fs::open_mode mode = fs::open_mode::existing, fs::file_caching caching = fs::file_caching::all, fs::file_perm perm = fs::file_perm::all) noexcept
			{
				if (auto hnd_res = fs::file_handle::open(base, path, flags | fs::file_flags::non_blocking, mode, caching, perm); hnd_res.has_value()) [[likely]]
					return s._ctx->port_bind(hnd_res->native_handle()).transform_value([&]() { return file_handle(std::move(*hnd_res), s._ctx); });
				else
					return hnd_res.error();
			}
			template<decays_to_same<scheduler> T>
			friend result<file_handle> tag_invoke(fs::file_handle::open_anonymous_t, T &&s, const fs::path_handle &base, fs::file_flags flags = fs::file_flags::readwrite, fs::file_caching caching = fs::file_caching::all | fs::file_caching::temporary, fs::file_perm perm = fs::file_perm::all) noexcept
			{
				if (auto hnd_res = fs::file_handle::open_anonymous(base, flags | fs::file_flags::non_blocking, caching, perm); hnd_res.has_value()) [[likely]]
					return s._ctx->port_bind(hnd_res->native_handle()).transform_value([&]() { return file_handle(std::move(*hnd_res), s._ctx); });
				else
					return hnd_res.error();
			}
		};

		template<typename Vst, typename Rcv>
		io_operation<Vst, Rcv>::type::type(scheduler sch, void *hnd, Vst &&vst, fs::file_timeout to, Rcv &&rcv) noexcept : io_operation_base(sch._ctx, hnd), vst_base(std::forward<Vst>(vst)), rcv_base(std::forward<Rcv>(rcv))
		{
			io_stop_operation::to = to.is_infinite() ? to : to.absolute();
		}

		constexpr scheduler context::get_scheduler() noexcept { return scheduler(this); }
		constexpr auto tag_invoke(get_stop_token_t, const env &e) noexcept { return e._ctx->get_stop_token(); }
		template<typename T>
		constexpr auto tag_invoke(get_completion_scheduler_t<T>, const env &e) noexcept { return e._ctx->get_scheduler(); }
	}

	/** Windows-specific execution context implemented via IOCP. */
	using iocp_context = _iocp::context;

	static_assert(rod::scheduler<decltype(std::declval<iocp_context>().get_scheduler())>);
}
#endif
/*
 * Created by switch_blade on 2023-07-04.
 */

#pragma once

#include "../detail/config.hpp"

#ifdef ROD_WIN32

#include <utility>
#include <thread>

#include "../detail/priority_queue.hpp"
#include "../detail/atomic_queue.hpp"
#include "../detail/basic_queue.hpp"
#include "filetime_clock.hpp"

namespace rod
{
	namespace _iocp
	{
		using unique_handle = _detail::unique_handle;
		using ntapi = _detail::ntapi;

		using time_point = typename filetime_clock::time_point;
		using clock = filetime_clock;

		enum flags_t { stop_possible = 1, stop_requested = 4, dispatched = 8 };
		enum event_id { wakeup, timeout };

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

		template<typename Op, typename Buff>
		struct io_cmd
		{
			void *hnd;
			Buff buff;
			std::size_t pos;
		};

		struct io_operation_base;
		struct timer_operation_base;

		class basic_sender;
		template<typename Rcv>
		struct basic_operation { class type; };

		class timer_sender;
		template<typename Rcv>
		struct timer_operation { class type; };

		template<typename Op, typename Buff>
		struct io_sender { class type; };
		template<typename Op, typename Buff, typename Rcv>
		struct io_operation { class type; };

		struct operation_base
		{
			using notify_func_t = void (*)(operation_base *);

			operation_base(operation_base &&) = delete;
			operation_base &operator=(operation_base &&) = delete;

			constexpr operation_base() noexcept = default;

			void notify() { std::exchange(notify_func, {})(this); }

			notify_func_t notify_func = {};
			operation_base *next = {};
			operation_base *prev = {};
		};
		struct timer_operation_base : operation_base
		{
			constexpr timer_operation_base(context *ctx, time_point tp, bool can_stop) noexcept : flags(can_stop ? flags_t::stop_possible : 0), tp(tp), ctx(ctx) {}

			[[nodiscard]] bool stop_possible() const noexcept { return flags.load(std::memory_order_relaxed) & flags_t::stop_possible; }

			timer_operation_base *timer_prev = {};
			timer_operation_base *timer_next = {};
			std::atomic<int> flags = {};
			time_point tp;
			context *ctx;
		};

		/* IO entries used to represent scheduled IO operations similar to how linux io_uring entry queues work.
		 * Entries are batched to allow for IO requests whose size overflows `ULONG` + potential support
		 * for vectorized IO later on. */
		struct io_entry
		{
			/* sizeof(io_entry) = 256/128 on 64/32-bit */
			static constexpr std::size_t max_size = 14;

			constexpr io_entry() noexcept : io_entry(nullptr) {}
			constexpr explicit io_entry(io_operation_base *parent) noexcept;

			bool get_state() const noexcept;

			io_entry *prev = {};
			io_entry *next = {};
			io_operation_base *parent;
			std::uint8_t started = {};
			std::uint8_t pending = {};
			std::uint8_t err_pos = {};
			bool parent_notified = {};
			ntapi::io_status_block batch[max_size] = {};
		};

		struct io_stop_operation : operation_base {};
		struct io_operation_base : operation_base
		{
			constexpr explicit io_operation_base(context *ctx) noexcept : ctx(ctx) {}

			ROD_API_PUBLIC void cancel_io(void *hnd);
			ROD_API_PUBLIC result<std::size_t, std::error_code> io_result() const noexcept;
			ROD_API_PUBLIC std::size_t start_io(io_cmd<async_read_some_at_t, std::span<std::byte>> cmd);
			ROD_API_PUBLIC std::size_t start_io(io_cmd<async_write_some_at_t, std::span<std::byte>> cmd);

			template<one_of<async_read_some_at_t, async_write_some_at_t> Op, typename Buff>
			std::size_t start_io(io_cmd<Op, Buff> cmd)
			{
				const auto bytes = rod::as_byte_buffer(const_cast<std::ranges::range_value_t<Buff> *>(std::data(cmd.buff)), std::size(cmd.buff));
				return start_io(io_cmd<Op, std::span<std::byte>>{cmd.hnd, bytes, cmd.pos});
			}

			context *ctx;
			io_entry *entry = {};
			std::size_t _bytes_done = {};
			std::atomic<int> _flags = {};
		};

		constexpr io_entry::io_entry(io_operation_base *parent) noexcept : parent(parent) { if (parent) parent->entry = this; }

		class context : operation_base
		{
			template<typename>
			friend struct basic_operation;
			template<typename>
			friend struct timer_operation;
			template<typename, typename, typename>
			friend struct io_operation;
			friend class file_handle;

		public:
			using time_point = _iocp::time_point;
			using clock = _iocp::clock;

		private:
			struct timer_cmp { constexpr bool operator()(const timer_operation_base &a, const timer_operation_base &b) const noexcept { return a.tp <= b.tp; }};
			using timer_queue_t = _detail::priority_queue<timer_operation_base, timer_cmp, &timer_operation_base::timer_prev, &timer_operation_base::timer_next>;

			using producer_queue_t = _detail::atomic_queue<operation_base, &operation_base::next>;
			using consumer_queue_t = _detail::basic_queue<operation_base, &operation_base::next>;

			using io_entry_pool_t = _detail::basic_queue<io_entry, &io_entry::next>;
			using io_entry_buff_t = std::vector<io_entry>;

		public:
			context(context &&) = delete;
			context &operator=(context &&) = delete;

			/** Initializes IO context with the default number of entries. */
			ROD_API_PUBLIC context();
			/** Initializes IO context with the specified number of entries. */
			ROD_API_PUBLIC context(std::size_t max_entries);
			ROD_API_PUBLIC ~context();

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

			/** Returns a scheduler used to schedule work to be executed on the context. */
			[[nodiscard]] constexpr scheduler get_scheduler() noexcept;

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

			ROD_API_PUBLIC void add_timer(timer_operation_base *node) noexcept;
			ROD_API_PUBLIC void del_timer(timer_operation_base *node) noexcept;

			void timeout_handler() noexcept;
			void set_timer(time_point tp);

			ROD_API_PUBLIC void release_io_entry(io_entry *entry) noexcept;
			ROD_API_PUBLIC bool acquire_io_entry(io_operation_base *node) noexcept;
			ROD_API_PUBLIC void schedule_waiting(io_operation_base *node) noexcept;
			ROD_API_PUBLIC void schedule_pending(io_operation_base *node) noexcept;

			void schedule_complete(std::pair<io_entry *, std::uint8_t> entry) noexcept;
			std::pair<io_entry *, std::uint8_t> find_entry(ULONG_PTR apc) noexcept;

			void acquire_pending_events() noexcept;
			bool acquire_producer_queue() noexcept;
			void acquire_elapsed_timers();

			std::error_code port_bind(void *hnd);
			void port_wait();

			/* TID of the current consumer thread. */
			std::atomic<std::thread::id> _consumer_tid = {};

			/* IOCP object handle & entry buffer. */
			_detail::unique_handle _iocp = {};
			io_entry_buff_t _io_entry_buff;
			io_entry_pool_t _io_entry_pool;

			/* Stop source associated with the context. */
			in_place_stop_source _stop_src;
			/* Queue of operation pending for acquisition by consumer thread. */
			producer_queue_t _producer_queue;
			/* Queue of operations pending for dispatch by consumer thread. */
			consumer_queue_t _consumer_queue;
			/* Queue of not-yet-started operations waiting for an IO entry to become available. */
			consumer_queue_t _waiting_queue;
			/* Queue of already started operations waiting for completion events to arrive. */
			consumer_queue_t _pending_queue;

			/* Win32 thread timer used to generate timer events. */
			unique_handle _thread_timer;
			/* Priority queue of pending timers. */
			timer_queue_t _timer_queue;

			time_point _next_timeout = {};
			bool _timer_started = false;
			bool _timer_pending = false;
			bool _stop_pending = false;
		};

		template<typename Rcv>
		class basic_operation<Rcv>::type : operation_base
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
			constexpr explicit type(context *ctx, time_point timeout, Rcv &&rcv) noexcept(std::is_nothrow_move_constructible_v<Rcv>) : timer_operation_base(ctx, timeout, get_stop_token(get_env(rcv)).stop_possible()), _rcv(std::forward<Rcv>(rcv)) {}

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
		class io_operation<Op, Buff, Rcv>::type : io_operation_base, io_stop_operation
		{
			using io_cmd_t = io_cmd<Op, Buff>;

			static void notify_start(operation_base *ptr) { static_cast<type *>(static_cast<io_operation_base *>(ptr))->start_consumer(); }
			static void notify_result(operation_base *ptr) { static_cast<type *>(static_cast<io_operation_base *>(ptr))->complete_result(); }
			static void notify_partial(operation_base *ptr) { static_cast<type *>(static_cast<io_operation_base *>(ptr))->complete_partial(); }
			static void notify_stopped(operation_base *ptr) { static_cast<type *>(static_cast<io_stop_operation *>(ptr))->complete_stopped(); }

		public:
			constexpr explicit type(context *ctx, Rcv &&rcv, io_cmd_t cmd) noexcept(std::is_nothrow_move_constructible_v<Rcv>) : io_operation_base(ctx), _rcv(std::forward<Rcv>(rcv)), _cmd(std::move(cmd)) {}

			friend void tag_invoke(start_t, type &op) noexcept { op.start(); }

		private:
			void start()
			{
				if (ctx->is_consumer_thread())
					start_consumer();
				else
				{
					io_operation_base::notify_func = notify_start;
					ctx->schedule_producer(static_cast<io_operation_base *>(this));
				}
			}
			void start_consumer() noexcept
			{
				if (_flags.load(std::memory_order_acquire) & flags_t::stop_requested)
					return; /* Stopped on a different thread. */

				/* Schedule a bathed read operation. */
				if (io_operation_base::entry || ctx->acquire_io_entry(this))
				{
					if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
						_stop_cb.init(get_env(_rcv), this);
					batch_consumer();
				}
				else
				{
					io_operation_base::notify_func = notify_start;
					ctx->schedule_waiting(this);
				}
			}
			void batch_consumer() noexcept
			{
				const auto batch = io_operation_base::start_io(_cmd);
				if (_bytes_done + batch < std::size(_cmd.buff) && entry->started == io_entry::max_size)
					io_operation_base::notify_func = notify_partial;
				else
					io_operation_base::notify_func = notify_result;
				ctx->schedule_pending(this);
			}

			void complete_result() noexcept
			{
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					_stop_cb.reset();
				if (!(_flags.fetch_or(flags_t::dispatched, std::memory_order_acq_rel) & flags_t::stop_requested))
					complete_result(io_result());
			}
			void complete_result(const result<std::size_t, std::error_code> &res) noexcept
			{
				ctx->release_io_entry(entry);
				if (res.has_value())
					[[likely]] set_value(std::move(_rcv), res.value());
				else if (const auto err = res.error_or({}); err.value() != ECANCELED)
					set_error(std::move(_rcv), err);
				else
					set_stopped(std::move(_rcv));
			}

			void complete_partial() noexcept
			{
				if (!(_flags.load(std::memory_order_acquire) & flags_t::stop_requested))
					complete_partial(io_result());
			}
			void complete_partial(const result<std::size_t, std::error_code> &res) noexcept
			{
				if (res.has_value())
				{
					_bytes_done += *res;
					batch_consumer();
				}
				else
				{
					if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
						_stop_cb.reset();
					_flags.fetch_or(flags_t::dispatched, std::memory_order_acq_rel);

					if (const auto err = res.error_or({}); err.value() != ECANCELED)
						set_error(std::move(_rcv), err);
					else
						set_stopped(std::move(_rcv));
				}
			}

			void request_stop()
			{
				if (!(_flags.fetch_or(flags_t::stop_requested, std::memory_order_acq_rel) & flags_t::dispatched))
				{
					io_operation_base::cancel_io(_cmd.hnd);
					io_stop_operation::notify_func = notify_stopped;
					ctx->schedule(static_cast<io_stop_operation *>(this));
				}
			}
			void complete_stopped()
			{
				if (io_operation_base::notify_func)
				{
					io_stop_operation::notify_func = notify_stopped;
					ctx->schedule_consumer(static_cast<io_stop_operation *>(this));
				}
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					set_stopped(std::move(_rcv));
				else
					std::terminate();
			}

			using stop_cb_t = stop_cb<env_of_t<Rcv>, &type::request_stop>;

			ROD_NO_UNIQUE_ADDRESS stop_cb_t _stop_cb;
			ROD_NO_UNIQUE_ADDRESS Rcv _rcv;
			io_cmd_t _cmd;
		};

		class basic_sender : public sender_base<set_value_t()>
		{
			template<typename Rcv>
			using operation_t = typename basic_operation<std::decay_t<Rcv>>::type;

		public:
			using sender_base::sender_base;

			template<decays_to<basic_sender> T, rod::receiver Rcv> requires receiver_of<Rcv, completion_signatures_of_t<basic_sender, env_of_t<Rcv>>>
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
			constexpr explicit timer_sender(context *ctx, time_point _tp) noexcept : sender_base(ctx), _tp(_tp) {}

			template<decays_to<timer_sender> T, rod::receiver Rcv> requires receiver_of<Rcv, completion_signatures_of_t<timer_sender, env_of_t<Rcv>>>
			friend constexpr operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<std::decay_t<Rcv>, Rcv>)
			{
				return operation_t<Rcv>{s._ctx, s._tp, std::forward<Rcv>(rcv)};
			}

		private:
			time_point _tp;
		};
		template<typename Op, typename Buff>
		class io_sender<Op, Buff>::type : public sender_base<set_value_t(std::size_t), set_error_t(std::error_code), set_stopped_t()>
		{
			template<typename Rcv>
			using operation_t = typename io_operation<Op, Buff, std::decay_t<Rcv>>::type;
			using io_cmd_t = io_cmd<Op, Buff>;

		public:
			template<typename... Args>
			constexpr explicit type(context *ctx, Args &&...args) noexcept : sender_base(ctx), _cmd(std::forward<Args>(args)...) {}

			template<decays_to<type> T, rod::receiver Rcv> requires receiver_of<Rcv, completion_signatures_of_t<type, env_of_t<Rcv>>>
			friend operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<std::decay_t<Rcv>, Rcv>)
			{
				return operation_t<Rcv>{s._ctx, std::forward<Rcv>(rcv), std::forward<T>(s)._cmd};
			}

		private:
			io_cmd_t _cmd;
		};

		/* IOCP-bound handle types. */
		class file_handle : public _file::system_handle
		{
			template<typename Op, typename Buff>
			using io_sender_t = typename io_sender<Op, Buff>::type;

		public:
			file_handle() = delete;

			constexpr file_handle(file_handle &&other) noexcept { swap(other); }
			constexpr file_handle &operator=(file_handle &&other) noexcept { return (swap(other), *this); }

			constexpr explicit file_handle(context *ctx) noexcept : _ctx(ctx) {}

			ROD_API_PUBLIC std::error_code open(const char *path, int mode, int prot) noexcept;
			ROD_API_PUBLIC std::error_code open(const wchar_t *path, int mode, int prot) noexcept;

			constexpr void swap(file_handle &other) noexcept
			{
				_file::system_handle::swap(other);
				std::swap(_ctx, other._ctx);
			}
			friend constexpr void swap(file_handle &a, file_handle &b) noexcept { a.swap(b); }

		public:
			template<reference_to<file_handle> Hnd, std::convertible_to<std::size_t> Pos, byte_buffer Buff>
			friend io_sender_t<async_read_some_at_t, Buff> tag_invoke(async_read_some_at_t, Hnd &&hnd, Pos pos, Buff &&buff) noexcept(std::is_nothrow_move_constructible_v<Buff>)
			{
				return io_sender_t<async_read_some_at_t, Buff>{hnd._ctx, hnd.native_handle(), std::forward<Buff>(buff), static_cast<std::size_t>(pos)};
			}
			template<reference_to<file_handle> Hnd, std::convertible_to<std::size_t> Pos, byte_buffer Buff>
			friend io_sender_t<async_write_some_at_t, Buff> tag_invoke(async_write_some_at_t, Hnd &&hnd, Pos pos, Buff &&buff) noexcept(std::is_nothrow_move_constructible_v<Buff>)
			{
				return io_sender_t<async_write_some_at_t, Buff>{hnd._ctx, hnd.native_handle(), std::forward<Buff>(buff), static_cast<std::size_t>(pos)};
			}

			template<reference_to<file_handle> Hnd, byte_buffer Buff>
			friend auto tag_invoke(async_read_some_t, Hnd &&hnd, Buff &&buff) { return async_read_some_at(std::forward<Hnd>(hnd), hnd.tell().value(), std::forward<Buff>(buff)); }
			template<reference_to<file_handle> Hnd, byte_buffer Buff>
			friend auto tag_invoke(async_write_some_t, Hnd &&hnd, Buff &&buff) { return async_write_some_at(std::forward<Hnd>(hnd), hnd.tell().value(), std::forward<Buff>(buff)); }

		private:
			/* Hide base open so the above overloads will work. */
			using _file::system_handle::open;

			context *_ctx = nullptr;
		};
		/* pipe_handle */
		/* wsa2_handle */

		class scheduler
		{
		public:
			constexpr explicit scheduler(context *ctx) noexcept : _ctx(ctx) {}

			friend constexpr auto tag_invoke(get_forward_progress_guarantee_t, const scheduler &) noexcept { return forward_progress_guarantee::weakly_parallel; }
			friend constexpr bool tag_invoke(execute_may_block_caller_t, const scheduler &) noexcept { return true; }

			template<decays_to<scheduler> T>
			friend constexpr auto tag_invoke(schedule_t, T &&s) noexcept { return basic_sender{s._ctx}; }
			template<decays_to<scheduler> T, decays_to<time_point> Tp>
			friend constexpr auto tag_invoke(schedule_at_t, T &&s, Tp &&tp) noexcept { return timer_sender{s._ctx, std::forward<Tp>(tp)}; }
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

	/** Windows-specific execution context implemented via IOCP. */
	using iocp_context = _iocp::context;

	static_assert(rod::scheduler<decltype(std::declval<iocp_context>().get_scheduler())>);
}
#endif
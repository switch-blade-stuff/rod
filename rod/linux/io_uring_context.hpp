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

#include "../posix/monotonic_clock.hpp"
#include "../posix/detail/file.hpp"
#include "../posix/detail/mmap.hpp"

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
			std::intmax_t result = {};
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

		template<typename Op, typename Buff>
		struct io_sender { class type; };
		template<typename Op, typename Buff, typename Rcv>
		struct io_operation { class type; };

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
			struct timer_cmp { constexpr bool operator()(const timer_operation_base &a, const timer_operation_base &b) const noexcept { return a.timeout <= b.timeout; }};
			using timer_queue_t = _detail::priority_queue<timer_operation_base, timer_cmp, &timer_operation_base::timer_prev, &timer_operation_base::timer_next>;
			using producer_queue_t = _detail::atomic_queue<operation_base, &operation_base::next>;
			using consumer_queue_t = _detail::basic_queue<operation_base, &operation_base::next>;

		public:
			context(context &&) = delete;
			context &operator=(context &&) = delete;

			/** Initializes the io_uring execution context with a default queue size.
			 * @throw std::system_error On failure to initialize descriptors or memory mappings. */
			ROD_API_PUBLIC context();
			/** Initializes the EPOLL execution context with the specified queue size.
			 * @param entries Number of entries in the io_uring queues.
			 * @throw std::system_error On failure to initialize descriptors or memory mappings. */
			ROD_API_PUBLIC explicit context(std::size_t entries);
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
			ROD_API_PUBLIC void schedule_waitlist(operation_base *node);

			bool submit_io_event(int op, void *data, int fd, void *addr, std::size_t n, std::size_t off) noexcept;
			bool submit_timer_event(time_point timeout) noexcept;
			bool submit_queue_event() noexcept;
			bool cancel_timer_event() noexcept;

			ROD_API_PUBLIC bool submit_io_event(operation_base *node, io_cmd<async_read_some_t, std::span<std::byte>> cmd) noexcept;
			ROD_API_PUBLIC bool submit_io_event(operation_base *node, io_cmd<async_write_some_t, std::span<std::byte>> cmd) noexcept;
			ROD_API_PUBLIC bool submit_io_event(operation_base *node, io_cmd<async_read_some_at_t, std::span<std::byte>> cmd) noexcept;
			ROD_API_PUBLIC bool submit_io_event(operation_base *node, io_cmd<async_write_some_at_t, std::span<std::byte>> cmd) noexcept;

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
				return submit_io_event(node, io_cmd<Op, std::span<std::byte>>{cmd.fd, cmd.off, bytes});
			}

			ROD_API_PUBLIC bool cancel_io_event(operation_base *node) noexcept;

			ROD_API_PUBLIC void add_timer(timer_operation_base *node) noexcept;
			ROD_API_PUBLIC void del_timer(timer_operation_base *node) noexcept;

			void acquire_consumer_queue();
			void acquire_elapsed_timers();
			void uring_enter();

			/* TID of the current consumer thread. */
			std::atomic<std::thread::id> _consumer_tid = {};

			/* Descriptors used for io_uring notifications. */
			_detail::unique_descriptor _uring_fd = {};
			_detail::unique_descriptor _event_fd = {};

			/* Memory mappings of io_uring queues. */
			_detail::mmap_handle _cq_mmap = {};
			_detail::mmap_handle _sq_mmap = {};
			_detail::mmap_handle _sqe_mmap = {};

			/* State of io_uring queues. */
			cq_state_t _cq = {};
			sq_state_t _sq = {};

			/* Stop source associated with the context. */
			in_place_stop_source _stop_src;
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
		class io_operation<Op, Rcv>::type : complete_base, stop_base
		{
			using io_cmd_t = io_cmd<Op, Buff>;

			static void notify_start(operation_base *ptr) { static_cast<type *>(static_cast<complete_base *>(ptr))->start_consumer(); }
			static void notify_value(operation_base *ptr) { static_cast<type *>(static_cast<complete_base *>(ptr))->complete_value(); }
			static void notify_stopped(operation_base *ptr) { static_cast<type *>(static_cast<stop_base *>(ptr))->complete_stopped(); }
			static void notify_request_stop(operation_base *ptr) { static_cast<type *>(static_cast<stop_base *>(ptr))->request_stop(); }

		public:
			constexpr explicit type(context *ctx, Rcv &&rcv, io_cmd_t cmd) noexcept(std::is_nothrow_move_constructible_v<Rcv>) : _rcv(std::forward<Rcv>(rcv)), _cmd(std::move(cmd)), _ctx(ctx) {}

			friend void tag_invoke(start_t, type &op) noexcept { op.start(); }

		private:
			void complete_value() noexcept
			{
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					_stop_cb.reset();
				if (_flags.fetch_or(flags_t::dispatched, std::memory_order_acq_rel) & flags_t::stop_requested)
					return;

				if (complete_base::result >= 0)
					[[likely]] set_value(std::move(_rcv), static_cast<std::size_t>(complete_base::result));
				else if (const auto err = int(-complete_base::result); err != ECANCELED)
					set_error(std::move(_rcv), std::error_code{err, std::system_category()});
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
			void start_consumer()
			{
				if (_flags.load(std::memory_order_acquire) & flags_t::stop_requested)
					return; /* Stopped on a different thread. */

				complete_base::notify_func = notify_value;
				if (!_ctx->submit_io_event(static_cast<complete_base *>(this), _cmd))
				{
					complete_base::notify_func = notify_start;
					_ctx->schedule_waitlist(static_cast<complete_base *>(this));
				}
			}

			void request_stop()
			{
				if (_flags.fetch_or(flags_t::stop_requested, std::memory_order_acq_rel) & flags_t::dispatched)
					return; /* Already completed on a different thread */

				stop_base::notify_func = notify_stopped;
				if (!_ctx->is_consumer_thread())
				{
					stop_base::notify_func = notify_request_stop;
					_ctx->schedule_producer(static_cast<stop_base *>(this));
				}
				else if (!_ctx->cancel_io_event(static_cast<stop_base *>(this)))
				{
					stop_base::notify_func = notify_request_stop;
					_ctx->schedule_waitlist(static_cast<stop_base *>(this));
				}
			}
			void complete_stopped() noexcept
			{
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
			template<decays_to_same<sender> T, typename Env>
			friend constexpr signs_t<Env> tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }

			template<decays_to_same<sender> T, rod::receiver Rcv> requires receiver_of<Rcv, signs_t<env_of_t<Rcv>>>
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
			template<decays_to_same<timer_sender> T, typename Env>
			friend constexpr signs_t<Env> tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }

			template<decays_to_same<timer_sender> T, rod::receiver Rcv> requires receiver_of<Rcv, signs_t<env_of_t<Rcv>>>
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
			constexpr explicit type(context *ctx, Buff2 &&buff, Args &&...args) noexcept : _cmd(std::forward<Args>(args)...), _ctx(ctx) {}

			friend env tag_invoke(get_env_t, const type &s) noexcept { return {s._ctx}; }
			template<decays_to_same<type> T, typename Env>
			friend signs_t tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }
			template<decays_to_same<type> T, receiver_of<signs_t> Rcv>
			friend operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<std::decay_t<Rcv>, Rcv>)
			{
				return operation_t<Rcv>{s._ctx, std::forward<Rcv>(rcv), std::forward<T>(s)._cmd};
			}

		private:
			io_cmd_t _cmd;
			context *_ctx;
		};

		/* io_uring scheduler handles. */
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
			template<reference_to_same<file_handle> Hnd, typename Buff>
			friend auto tag_invoke(async_read_some_t, Hnd &&hnd, Buff &&buff) noexcept { return io_sender_t<async_read_some_t, Buff>{hnd._ctx, hnd.native_handle(), std::forward<Buff>(buff)}; }
			template<reference_to_same<file_handle> Hnd, typename Buff>
			friend auto tag_invoke(async_write_some_t, Hnd &&hnd, Buff &&buff) noexcept { return io_sender_t<async_write_some_t, Buff>{hnd._ctx, hnd.native_handle(), std::forward<Buff>(buff)}; }
			template<reference_to_same<file_handle> Hnd, std::convertible_to<std::size_t> Pos, typename Buff>
			friend auto tag_invoke(async_read_some_at_t, Hnd &&hnd, Pos pos, Buff &&buff) noexcept { return io_sender_t<async_read_some_at_t, Buff>{hnd._ctx, hnd.native_handle(), static_cast<std::size_t>(pos), std::forward<Buff>(buff)}; }
			template<reference_to_same<file_handle> Hnd, std::convertible_to<std::size_t> Pos, typename Buff>
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

			template<decays_to_same<scheduler> T>
			friend constexpr auto tag_invoke(schedule_t, T &&s) noexcept { return sender{s._ctx}; }
			template<decays_to_same<scheduler> T, decays_to_same<time_point> Tp>
			friend constexpr auto tag_invoke(schedule_at_t, T &&s, Tp &&tp) noexcept { return timer_sender{std::forward<Tp>(tp), s._ctx}; }
			template<decays_to_same<scheduler> T, typename Dur>
			friend constexpr auto tag_invoke(schedule_after_t, T &&s, Dur &&dur) noexcept { return schedule_at(std::forward<T>(s), s.now() + dur); }

			template<decays_to_same<scheduler> T>
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

	/** Linux-specific execution context implemented via io_uring. */
	using io_uring_context = _io_uring::context;

	static_assert(rod::scheduler<decltype(std::declval<context>().get_scheduler())>);
}
#endif

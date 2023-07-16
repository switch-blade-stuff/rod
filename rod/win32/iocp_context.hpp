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
		using unique_handle = detail::unique_handle;
		using ntapi = detail::ntapi;

		using time_point = typename filetime_clock::time_point;
		using clock = filetime_clock;

		enum class event_type : ntapi::ulong_ptr { wakeup, timeout, };

		class context;
		class file_handle;

		struct env { context *_ctx; };

		template<typename Env, auto StopFunc>
		using stop_cb = detail::stop_cb_adaptor<Env, StopFunc>;

		struct operation_base;
		struct io_operation_base;
		struct timer_operation_base;

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
			timer_operation_base *timer_prev = {};
			timer_operation_base *timer_next = {};
			time_point timeout = {};
			context *ctx;

			std::atomic_flag stop_requested;
			std::atomic_flag dispatched;
			const bool stop_possible;
		};

		/* IO entries used to represent scheduled IO operations similar to how linux io_uring entry queues work.
		 * Entries are batched to allow for IO requests whose size overflows `ntapi::ulong` + potential support
		 * for vectorized IO later on. */
		struct io_entry_t
		{
			/* sizeof(io_entry_t) = 256/128 on 64/32-bit */
			static constexpr std::size_t max_size = 14;

			constexpr io_entry_t() noexcept : io_entry_t(nullptr) {}
			constexpr explicit io_entry_t(io_operation_base *parent) noexcept;

			int get_state() const noexcept;
			int get_error() const noexcept;

			io_entry_t *prev = {};
			io_entry_t *next = {};
			io_operation_base *parent;
			std::uint8_t started = {};
			std::uint8_t waiting = {};
			std::uint8_t err_pos = {};
			bool parent_notified = {};
			ntapi::io_status_block batch[max_size] = {};
		};

		struct io_operation_base : operation_base
		{
			constexpr io_operation_base(context *ctx, void *handle) noexcept : ctx(ctx), handle(handle) {}

			ROD_API_PUBLIC void cancel_io();
			ROD_API_PUBLIC result<std::size_t, std::error_code> io_result() const noexcept;
			ROD_API_PUBLIC result<std::size_t, std::error_code> batch_read(std::size_t pos, std::span<std::byte> buff) noexcept;
			ROD_API_PUBLIC result<std::size_t, std::error_code> batch_write(std::size_t pos, std::span<std::byte> buff) noexcept;

			context *ctx;
			void *handle;
			io_entry_t *entry = {};
		};

		constexpr io_entry_t::io_entry_t(io_operation_base *parent) noexcept : parent(parent) { parent->entry = this; }

		/* IOCP-bound handle types. */
		class file_handle : public _file::system_handle
		{
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

		public:
			template<decays_to<scheduler> T>
			friend result<_file::basic_file<file_handle>, std::error_code> tag_invoke(open_file_t, T &&sch, auto &&path, int mode, int prot) noexcept
			{
				auto file = _file::basic_file<file_handle>{sch._ctx};
				if (auto err = file.open(path, mode, prot); !err) [[likely]]
					return file;
				else
					return err;
			}

		private:
			context *_ctx;
		};

		class context
		{
			friend class file_handle;

		public:
			using time_point = _iocp::time_point;
			using clock = _iocp::clock;

		private:
			struct timer_cmp { constexpr bool operator()(const timer_operation_base &a, const timer_operation_base &b) const noexcept { return a.timeout <= b.timeout; }};
			using timer_queue_t = detail::priority_queue<timer_operation_base, timer_cmp, &timer_operation_base::timer_prev, &timer_operation_base::timer_next>;

			using producer_queue_t = detail::atomic_queue<operation_base, &operation_base::next>;
			using consumer_queue_t = detail::basic_queue<operation_base, &operation_base::next>;

			using io_entry_pool_t = detail::basic_queue<io_entry_t, &io_entry_t::next>;
			using io_entry_buff_t = std::vector<io_entry_t>;

		public:
			/** Initializes IO context with the default number of entries. */
			context() : context(32) {}
			/** Initializes IO context with the specified number of entries. */
			ROD_API_PUBLIC context(std::size_t max_entries);
			ROD_API_PUBLIC ~context();

			/** Blocks the current thread until `finish` is called and executes scheduled operations.
			 * @throw std::system_error On implementation-defined internal failures.
			 * @note Only one thread may call `run`. */
			ROD_API_PUBLIC void run();
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
			ROD_API_PUBLIC void finish();

			/** Returns copy of the stop source associated with the context. */
			[[nodiscard]] constexpr in_place_stop_source &get_stop_source() noexcept { return _stop_src; }
			/** Returns a stop token of the stop source associated with the context. */
			[[nodiscard]] constexpr in_place_stop_token get_stop_token() const noexcept { return _stop_src.get_token(); }
			/** Sends a stop request to the stop source associated with the context. */
			ROD_API_PUBLIC void request_stop();

		private:
			[[nodiscard]] ROD_API_PUBLIC bool is_consumer_thread() const noexcept;

			std::pair<io_entry_t *, std::size_t> find_entry(ntapi::io_status_block *iosb) noexcept
			{
				const auto off = reinterpret_cast<std::byte *>(iosb) - reinterpret_cast<std::byte *>(_io_entry_buff.data());
				const auto ptr = _io_entry_buff.data() + off / sizeof(io_entry_t);
				return {ptr, iosb - ptr->batch};
			}

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

			ROD_API_PUBLIC void release_io_entry(io_entry_t *entry) noexcept;
			ROD_API_PUBLIC bool acquire_io_entry(io_operation_base *node) noexcept;
			ROD_API_PUBLIC void schedule_waiting(io_operation_base *node) noexcept;
			ROD_API_PUBLIC void schedule_pending(io_operation_base *node) noexcept;
			void schedule_complete(std::pair<io_entry_t *, std::size_t> entry);

			void acquire_consumer_queue() noexcept;
			bool acquire_producer_queue() noexcept;
			void acquire_elapsed_timers();

			std::error_code port_bind(void *hnd);
			void port_wait();

			/* TID of the current consumer thread. */
			std::atomic<std::thread::id> _consumer_tid = {};

			/* IOCP object handle & entry buffer. */
			detail::unique_handle _iocp = {};
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
			bool _wait_pending = false;
			bool _stop_pending = false;
		};
	}

	/** Windows-specific execution context implemented via IOCP. */
	using iocp_context = _iocp::context;

	//static_assert(rod::scheduler<decltype(std::declval<iocp_context>().get_scheduler())>);
}
#endif
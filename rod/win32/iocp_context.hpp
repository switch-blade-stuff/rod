/*
 * Created by switch_blade on 2023-07-04.
 */

#pragma once

#include "../detail/adaptors/read_some.hpp"
#include "../detail/adaptors/write_some.hpp"

#ifdef _WIN32

#include <utility>
#include <thread>
#include <chrono>

#include "../detail/priority_queue.hpp"
#include "../detail/atomic_queue.hpp"
#include "../detail/basic_queue.hpp"
#include "detail/handle.hpp"

namespace rod
{
	namespace _iocp
	{
		using winapi = detail::winapi;

		class context;

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
		};

		/* sizeof(io_state_t) = 512/256 on 64/32-bit */
		inline constexpr std::size_t max_state_size = 30;

		struct io_state_t
		{
			constexpr explicit io_state_t(io_operation_base *parent) noexcept : parent(parent) {}

			io_state_t *prev = {};
			io_state_t *next = {};
			io_operation_base *parent;
			std::uint8_t started = {};
			std::uint8_t pending = {};
			bool parent_notified = {};
			winapi::io_status_block ops[max_state_size] = {};
		};
		struct io_pool_t
		{
			constexpr io_pool_t() noexcept = default;
			constexpr io_pool_t(io_pool_t &&other) noexcept { swap(other); }
			constexpr io_pool_t &operator=(io_pool_t &&other) noexcept { return (swap(other), *this); }
			constexpr ~io_pool_t() { delete[] data; }

			constexpr void swap(io_pool_t &other) noexcept
			{
				std::swap(size, other.size);
				std::swap(data, other.data);
			}

			std::size_t size = {};
			io_state_t *data = {};
		};

		struct io_operation_base : operation_base
		{
			constexpr io_operation_base(context *ctx, void *handle) noexcept : ctx(ctx), handle(handle) {}

			context *ctx;
			void *handle;
			io_state_t *state = {};
		};

		class context
		{
			/* TID of the current consumer thread. */
			std::atomic<std::thread::id> _consumer_tid = {};
			/* IOCP object handle & pool. */
			detail::unique_handle _iocp = {};
			io_pool_t _io_pool = {};
		};
		class scheduler;
	}

	/** Windows-specific execution context implemented via IOCP. */
	using iocp_context = _iocp::context;

	//static_assert(rod::scheduler<decltype(std::declval<iocp_context>().get_scheduler())>);
}
#endif
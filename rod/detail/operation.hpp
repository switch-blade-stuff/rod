/*
 * Created by switchblade on 2023-04-10.
 */

#pragma once

#include <coroutine>

#include "utility.hpp"

namespace rod
{
	namespace detail
	{
		struct start_t
		{
			template<typename O>
			constexpr decltype(auto) operator()(O &&o) noexcept requires tag_invocable<start_t, O>
			{
				return tag_invoke(*this, std::forward<O>(o));
			}
		};

		/* Operation state used to resume a coroutine. */
		template<typename P>
		class coroutine_operation
		{
		public:
			using promise_type = P;

		public:
			coroutine_operation() = delete;

			coroutine_operation(std::coroutine_handle<> handle) noexcept : m_handle(handle) {}
			coroutine_operation(coroutine_operation &&other) noexcept : m_handle(std::exchange(other.coro, {})) {}
			~coroutine_operation() { if (m_handle) m_handle.destroy(); }

			friend void tag_invoke(start_t, coroutine_operation &op) noexcept { op.coro.resume(); }

		private:
			std::coroutine_handle<> m_handle;
		};
	}

	using detail::start_t;

	/** Customization point object used to start work represented by an operation state. */
	inline constexpr auto start = start_t{};

	/** Concept used to define an operation state object type that can be used to start execution of work. */
	template<typename S>
	concept operation_state = queryable<S> && std::is_object_v<S> && requires(S &s) {{ start(s) } noexcept; };
}

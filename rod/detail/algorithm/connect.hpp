/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include "../concepts.hpp"

namespace rod
{
	inline namespace _connect
	{
		struct connect_t
		{
			template<typename S, typename R>
			struct _overload_hook
			{
				static constexpr bool allow_tag_invoke = receiver<R> && sender_in<S, env_of_t<R>> && receiver_of<R, completion_signatures_of_t<S, env_of_t<R>>> && tag_invocable<connect_t, S, R>;

				[[nodiscard]] constexpr decltype(auto) operator()(S &&s, R &&r) const noexcept(nothrow_tag_invocable<connect_t, S, R>) requires allow_tag_invoke
				{
					return tag_invoke(connect_t{}, std::forward<S>(s), std::forward<R>(r));
				}
			};

			template<typename S, typename R>
			[[nodiscard]] constexpr decltype(auto) operator()(S &&s, R &&r) const noexcept(std::is_nothrow_invocable_v<_overload_hook<S, R>, S, R>) requires std::invocable<_overload_hook<S, R>, S, R>
			{
				return _overload_hook<S, R>{}(std::forward<S>(s), std::forward<R>(r));
			}
		};
	}

	/** @brief Customization point object used to connect a sender with a receiver.
	 *
	 * Expression `connect(s, r)` for sub-expressions `s` and `r` where `r` satisfies the `receiver` concept is equivalent to one of the following:
	 * <ul>
	 * <li>If expression `tag_invoke(connect, s, r)` is well-formed, equivalent to the result of the expression (note: type of the expression must satisfy `operation_state`).</li>
	 * <li>Operation state task resulting from invocation of an exposition-only coroutine `connect-awaitable(s, r)` if `s` is awaitable using the `connect-awaitable-promise` promise type.</li>
	 * </ul> */
	inline constexpr auto connect = connect_t{};
	/** Alias for `decltype(connect(std::declval<S>(), std::declval<R>()))`. */
	template<typename S, typename R>
	using connect_result_t = decltype(connect(std::declval<S>(), std::declval<R>()));
}

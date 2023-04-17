/*
 * Created by switchblade on 2023-04-10.
 */

#pragma once

#include "utility.hpp"

namespace rod
{
	namespace detail { struct empty_env_t {}; }

	inline namespace _get_env
	{
		struct get_env_t
		{
		private:
			template<typename R>
			static constexpr bool is_noexcept = nothrow_tag_invocable<get_env_t, R>;
			template<typename R>
			static constexpr bool is_invocable = tag_invocable<get_env_t, R>;

			template<typename R>
			using result_t = tag_invoke_result_t<get_env_t, R>;

		public:
			template<typename R>
			[[nodiscard]] constexpr decltype(auto) operator()(R &&r) const noexcept(is_noexcept<R>) requires(is_invocable<R> && queryable<result_t<R>>)
			{
				return tag_invoke(*this, std::forward<R>(r));
			}
		};
	}

	/** Customization point object for which expression `get_env(r)` is equivalent to the result of expression `tag_invoke(get_env, r)` (note: type of the expression must satisfy `queryable`). */
	inline constexpr auto get_env = get_env_t{};
	/** Alias for `decltype(get_env(std::declval<T>()))` */
	template<typename T>
	using env_of_t = decltype(get_env(std::declval<T>()));

	namespace detail
	{
		template<typename T>
		concept has_env = requires { get_env(std::declval<T>()); };
		template<typename T>
		using deduce_env_of = std::conditional_t<has_env<T>, env_of_t<T>, empty_env_t>;
	}
}

/*
 * Created by switchblade on 2023-04-10.
 */

#pragma once

#include "../utility.hpp"

namespace rod
{
	inline namespace _get_env
	{
		struct empty_env {};
		struct get_env_t
		{
			template<typename T> requires tag_invocable<get_env_t, const T &>
			[[nodiscard]] constexpr decltype(auto) operator()(const T &value) const noexcept(nothrow_tag_invocable<get_env_t, const T &>)
			{
				return tag_invoke(*this, value);
			}
			template<typename T> requires(!tag_invocable<get_env_t, const T &>)
			[[nodiscard]] constexpr empty_env operator()(const T &) const noexcept { return {}; }
		};
	}

	/** Customization point object for which expression `get_env(value)` is equivalent to the result of expression `tag_invoke(get_env, value)`,
	 * or an empty environment placeholder if said expression is not valid. */
	inline constexpr auto get_env = get_env_t{};
	/** Alias for `decltype(get_env(std::declval<T>()))` */
	template<typename T>
	using env_of_t = decltype(get_env(std::declval<T>()));
}

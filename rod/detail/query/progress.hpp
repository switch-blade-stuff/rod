/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include "../utility.hpp"

namespace rod
{
	/** Enum defining forward progress guarantee constants. */
	enum class forward_progress_guarantee
	{
		concurrent,
		parallel,
		weakly_parallel
	};

	inline namespace _get_forward_progress_guarantee
	{
		struct get_forward_progress_guarantee_t
		{
			template<typename S, typename U = decltype(std::as_const(std::declval<S>()))>
			[[nodiscard]] constexpr forward_progress_guarantee operator()(S &&s) const noexcept
			{
				if constexpr (tag_invocable<get_forward_progress_guarantee_t, U>)
					return tag_invoke(*this, std::as_const(s));
				else
					return forward_progress_guarantee::weakly_parallel;
			}
		};
	}
	/** Customization point object used to obtain a forward progress guarantee from the passed scheduler. */
	inline constexpr auto get_forward_progress_guarantee = get_forward_progress_guarantee_t{};
}

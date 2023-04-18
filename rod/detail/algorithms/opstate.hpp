/*
 * Created by switchblade on 2023-04-10.
 */

#pragma once

#include "../utility.hpp"

namespace rod
{
	inline namespace _start
	{
		struct start_t
		{
			template<typename O> requires tag_invocable<start_t, O>
			constexpr decltype(auto) operator()(O &&op) const noexcept(nothrow_tag_invocable<start_t, O>)
			{
				return tag_invoke(*this, std::forward<O>(op));
			}
		};
	}

	/** Customization point object used to start work represented by an operation state.
	 * @param op Operation state used to start the scheduled work. */
	inline constexpr auto start = start_t{};

	/** Concept used to define an operation state object type that can be used to start execution of work. */
	template<typename S>
	concept operation_state = queryable<S> && std::is_object_v<S> && requires(S &s) {{ start(s) } noexcept; };
}

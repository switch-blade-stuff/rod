/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include "../concepts.hpp"

namespace rod
{
	inline namespace _schedule
	{
		struct schedule_t
		{
			template<typename S> requires tag_invocable<schedule_t, S> && sender<tag_invoke_result_t<schedule_t, S>>
			[[nodiscard]] constexpr decltype(auto) operator()(S &&s) const noexcept(nothrow_tag_invocable<schedule_t, S>)
			{
				return tag_invoke(*this, std::forward<S>(s));
			}
		};
	}

	/** Customization point object used to obtain a sender from the passed scheduler. */
	inline constexpr auto schedule = schedule_t{};
	/** Alias for the sender type of scheduler `S` obtained via a call to `schedule(S)`. */
	template<typename S>
	using schedule_result_t = decltype(schedule(std::declval<S>()));
}

/*
 * Created by switchblade on 2023-04-17.
 */

#pragma once

#include "schedule.hpp"
#include "just.hpp"

namespace rod
{
	inline namespace _transfer
	{
		struct transfer_t
		{
			template<sender S, scheduler Sch>
			[[nodiscard]] constexpr decltype(auto) operator()(S &&snd, Sch &&sch) const
			{
				if constexpr (requires { tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<S>(snd), std::forward<Sch>(sch)); })
					return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<S>(snd), std::forward<Sch>(sch));
				else if constexpr (tag_invocable<transfer_t, S, Sch>)
					return tag_invoke(*this, std::forward<S>(snd), std::forward<Sch>(sch));
				else
					return schedule_from(std::forward<Sch>(sch), std::forward<S>(snd));
			}
		};
	}

	/** Customization point object used to adapt a sender to use a different completion scheduler for the value & stop channels.
	 * @param snd Input sender to adapt to the specified completion scheduler.
	 * @param sch Scheduler that will be used for the value & stop channels.
	 * @return Sender completing on \a sch via the value & stop channels. */
	inline constexpr auto transfer = transfer_t{};

	inline namespace _transfer
	{
		struct transfer_just_t
		{
			template<scheduler S, typename... Vs> requires (tag_invocable<transfer_just_t, S, Vs...>)
			[[nodiscard]] constexpr decltype(auto) operator()(S &&sch, Vs &&...vals) const noexcept(nothrow_tag_invocable<transfer_just_t, S, Vs...>)
			{
				return tag_invoke(*this, std::forward<S>(sch), std::forward<Vs>(vals)...);
			}
			template<scheduler S, typename... Vs> requires (!tag_invocable<transfer_just_t, S, Vs...>)
			[[nodiscard]] constexpr decltype(auto) operator()(S &&sch, Vs &&...vals) const
			{
				return transfer(just(std::forward<Vs>(vals)...), std::forward<S>(sch));
			}
		};
	}

	/** Customization point object returning a sender that transfers a set of values to the specified scheduler through the value channel.
	 * @param sch Scheduler to schedule the value sender on.
	 * @param vals Values to send through the value channel.
	 * @return Sender that completes with `set_value(vals...)` on \a sch scheduler. */
	inline constexpr auto transfer_just = transfer_just_t{};
}

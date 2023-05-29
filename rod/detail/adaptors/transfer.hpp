/*
 * Created by switchblade on 2023-04-17.
 */

#pragma once

#include "schedule_from.hpp"
#include "closure.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	inline namespace _transfer
	{
		class transfer_t
		{
			template<typename Snd>
			using value_scheduler = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Sch>
			using back_adaptor = detail::back_adaptor<transfer_t, std::decay_t<Sch>>;

		public:
			template<sender Snd, scheduler Sch> requires detail::tag_invocable_with_completion_scheduler<transfer_t, set_value_t, Snd, Snd, Sch>
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd, Sch &&sch) const noexcept(nothrow_tag_invocable<transfer_t, value_scheduler<Snd>, Snd, Sch>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::forward<Sch>(sch));
			}
			template<sender Snd, scheduler Sch> requires(!detail::tag_invocable_with_completion_scheduler<transfer_t, set_value_t, Snd, Snd, Sch> && tag_invocable<transfer_t, Snd, Sch>)
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd, Sch &&sch) const noexcept(nothrow_tag_invocable<transfer_t, Snd, Sch>)
			{
				return tag_invoke(transfer_t{}, std::forward<Snd>(snd), std::forward<Sch>(sch));
			}
			template<sender Snd, scheduler Sch> requires(!detail::tag_invocable_with_completion_scheduler<transfer_t, set_value_t, Snd, Snd, Sch> && !tag_invocable<transfer_t, Snd, Sch>)
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd, Sch &&sch) const noexcept(detail::nothrow_callable<schedule_from_t, Snd, Sch>)
			{
				return schedule_from(std::forward<Sch>(sch), std::forward<Snd>(snd));
			}

			template<scheduler Sch>
			[[nodiscard]] constexpr back_adaptor<Sch> operator()(Sch &&sch) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Sch>, transfer_t, Sch>)
			{
				return {*this, std::forward<Sch>(sch)};
			}
		};
	}

	/** Customization point object used to adapt a sender to use a different completion scheduler for the value & stop channels.
	 * @param snd Input sender to adapt to the specified completion scheduler. If omitted, creates a pipe-able sender adaptor.
	 * @param sch Scheduler that will be used for the value & stop channels.
	 * @return Sender completing on \a sch via the value & stop channels. */
	inline constexpr auto transfer = transfer_t{};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

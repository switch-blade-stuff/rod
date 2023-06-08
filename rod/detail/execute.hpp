/*
 * Created by switchblade on 2023-06-07.
 */

#pragma once

#include "start_detached.hpp"
#include "adaptors/then.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	namespace _execute
	{
		struct execute_t
		{
			template<scheduler Sch, typename Fn> requires tag_invocable<execute_t, Sch, Fn> && detail::callable<Fn &> && std::move_constructible<Fn>
			constexpr void operator()(Sch &&sch, Fn &&fn) const noexcept(nothrow_tag_invocable<execute_t, Sch, Fn>) { tag_invoke(*this, std::forward<Sch>(sch), std::forward<Fn>(fn)); }
			template<scheduler Sch, typename Fn> requires(!tag_invocable<execute_t, Sch, Fn> && detail::callable<Fn &> && std::move_constructible<Fn>)
			constexpr void operator()(Sch &&sch, Fn &&fn) const { start_detached(then(schedule(std::forward<Sch>(sch)), std::forward<Fn>(fn))); }
		};
	}

	using _execute::execute_t;

	/** Customization point object used to schedule detached execution of a functor on a given scheduler.
	 * @param sch Scheduler to schedule execution on.
	 * @param fn Functor to execute on \a sch. */
	inline constexpr auto execute = execute_t{};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

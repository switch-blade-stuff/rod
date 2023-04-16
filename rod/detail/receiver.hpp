/*
 * Created by switchblade on 2023-04-09.
 */

#pragma once

#include "completion.hpp"
#include "exec_env.hpp"

namespace rod
{
	/** Concept used to define a generic receiver type. */
	template<typename R, typename U = std::remove_cvref_t<R>>
	concept receiver = requires(const U &r) {{ get_env(r) } -> queryable; } && std::move_constructible<U> && std::constructible_from<U, R>;

	/** Concept used to define a receiver type with a known set of completion signatures. */
	template<typename R, typename Cp>
	concept receiver_of = receiver<R> && requires { []<detail::valid_completion_for<R>... Sigs>(completion_signatures<Sigs...>) {}(Cp{}); };

	namespace detail
	{
	}
}
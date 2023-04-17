/*
 * Created by switchblade on 2023-04-09.
 */

#pragma once

#include "detail/concepts.hpp"

#include "detail/query/signatures.hpp"
#include "detail/query/allocator.hpp"
#include "detail/query/scheduler.hpp"
#include "detail/query/may_block.hpp"
#include "detail/query/progress.hpp"

#include "detail/algorithm/schedule.hpp"
#include "detail/algorithm/connect.hpp"
#include "detail/algorithm/opstate.hpp"
#include "detail/algorithm/read.hpp"

#include "detail/awaitable.hpp"
#include "stop_token.hpp"

namespace rod
{
	/** Concept used to define a scheduler type \a S that can be used to schedule work on it's associated execution context. */
	template<typename S>
	concept scheduler = queryable<S> && requires(S &&s, decltype(get_env(schedule(std::forward<S>(s)))) env)
	{
		{ schedule(std::forward<S>(s)) } -> sender;
		tag_invocable<get_completion_scheduler_t<set_value_t>, decltype(env)>;
		std::same_as<tag_invoke_result_t<get_completion_scheduler_t<set_value_t>, decltype(env)>, std::remove_cvref_t<S>>;
	} && std::equality_comparable<std::remove_cvref_t<S>> && std::copy_constructible<std::remove_cvref_t<S>>;
}

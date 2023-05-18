/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include "../factories/read.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	inline namespace _execute_may_block_caller
	{
		struct execute_may_block_caller_t
		{
			template<scheduler S> requires tag_invocable<execute_may_block_caller_t, const std::remove_cvref_t<S> &>
			[[nodiscard]] constexpr bool operator()(S &&s) const noexcept { return tag_invoke(*this, std::as_const(s)); }
			template<scheduler S>
			[[nodiscard]] constexpr auto operator()(S &&) const noexcept { return true; }
		};
	}

	/** Customization point object used to check if a call to `execute(s, f)` with scheduler `s` and an invocable `f` could block the current thread. */
	inline constexpr auto execute_may_block_caller = execute_may_block_caller_t{};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

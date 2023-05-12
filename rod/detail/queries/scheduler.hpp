/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include "../factories/read.hpp"

namespace rod
{
	inline namespace _get_scheduler
	{
		struct get_scheduler_t
		{
			[[nodiscard]] constexpr friend bool tag_invoke(forwarding_query_t, get_scheduler_t) noexcept { return true; }

			template<typename R> requires tag_invocable<get_scheduler_t, const std::remove_cvref_t<R> &>
			[[nodiscard]] constexpr decltype(auto) operator()(R &&r) const noexcept { return tag_invoke(*this, std::as_const(r)); }
			[[nodiscard]] constexpr decltype(auto) operator()() const noexcept { return read(*this); }
		};
	}

	/** Customization point object used to obtain a scheduler from from the passed object. */
	inline constexpr auto get_scheduler = get_scheduler_t{};

	inline namespace _get_delegatee_scheduler
	{
		struct get_delegatee_scheduler_t
		{
			[[nodiscard]] constexpr friend bool tag_invoke(forwarding_query_t, get_delegatee_scheduler_t) noexcept { return true; }

			template<typename R> requires tag_invocable<get_delegatee_scheduler_t, const std::remove_cvref_t<R> &>
			[[nodiscard]] constexpr decltype(auto) operator()(R &&r) const noexcept { return tag_invoke(*this, std::as_const(r)); }
			[[nodiscard]] constexpr decltype(auto) operator()() const noexcept { return read(*this); }
		};
	}

	/** Customization point object used to obtain a scheduler that can be used for the purpose of forward progress delegation. */
	inline constexpr auto get_delegatee_scheduler = get_delegatee_scheduler_t{};
}

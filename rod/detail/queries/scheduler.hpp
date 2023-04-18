/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include "../algorithms/read.hpp"

namespace rod
{
	inline namespace _get_scheduler
	{
		struct get_scheduler_t
		{
			[[nodiscard]] constexpr friend bool tag_invoke(forwarding_query_t, get_scheduler_t) noexcept { return true; }

			template<typename R, typename U = decltype(std::as_const(std::declval<R>()))>
			[[nodiscard]] constexpr decltype(auto) operator()(R &&r) const noexcept requires tag_invocable<get_scheduler_t, U>
			{
				return tag_invoke(*this, std::as_const(r));
			}
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

			template<typename R, typename U = decltype(std::as_const(std::declval<R>()))>
			[[nodiscard]] constexpr decltype(auto) operator()(R &&r) const noexcept requires tag_invocable<get_delegatee_scheduler_t, U>
			{
				return tag_invoke(*this, std::as_const(r));
			}
			[[nodiscard]] constexpr decltype(auto) operator()() const noexcept { return read(*this); }
		};
	}

	/** Customization point object used to obtain a scheduler that can be used for the purpose of forward progress delegation. */
	inline constexpr auto get_delegatee_scheduler = get_delegatee_scheduler_t{};

	inline namespace _get_completion_scheduler
	{
		template<typename T>
		struct get_completion_scheduler_t
		{
			template<typename S, typename U = decltype(std::as_const(std::declval<S>()))>
			[[nodiscard]] constexpr decltype(auto) operator()(S &&s) const noexcept requires tag_invocable<get_completion_scheduler_t, U>
			{
				return tag_invoke(*this, std::as_const(s));
			}
		};
	}

	/** Customization point object used to obtain the completion scheduler for channel \a T from the passed sender. */
	template<typename T>
	inline constexpr auto get_completion_scheduler = get_completion_scheduler_t<T>{};
}

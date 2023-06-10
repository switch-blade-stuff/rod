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

	namespace detail
	{
		template<typename S>
		concept has_scheduler = callable<get_scheduler_t, env_of_t<S>>;
		template<typename T, typename S, typename... Ts>
		concept tag_invocable_with_scheduler = has_scheduler<S> && tag_invocable<T, std::invoke_result_t<get_scheduler_t, env_of_t<S>>, Ts...>;
	}

	/** Customization point object used to obtain a scheduler from the passed object. */
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

	namespace detail
	{
		template<typename S>
		concept has_delegatee_scheduler = callable<get_delegatee_scheduler_t, env_of_t<S>>;
		template<typename T, typename S, typename... Ts>
		concept tag_invocable_with_delegatee_scheduler = has_delegatee_scheduler<S> && tag_invocable<T, std::invoke_result_t<get_delegatee_scheduler_t, env_of_t<S>>, Ts...>;
	}

	/** Customization point object used to obtain a scheduler that can be used for the purpose of forward progress delegation. */
	inline constexpr auto get_delegatee_scheduler = get_delegatee_scheduler_t{};
}

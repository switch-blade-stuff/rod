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

	namespace _detail
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

	namespace _detail
	{
		template<typename S>
		concept has_delegatee_scheduler = callable<get_delegatee_scheduler_t, env_of_t<S>>;
		template<typename T, typename S, typename... Ts>
		concept tag_invocable_with_delegatee_scheduler = has_delegatee_scheduler<S> && tag_invocable<T, std::invoke_result_t<get_delegatee_scheduler_t, env_of_t<S>>, Ts...>;
	}

	/** Customization point object used to obtain a scheduler that can be used for the purpose of forward progress delegation. */
	inline constexpr auto get_delegatee_scheduler = get_delegatee_scheduler_t{};

	inline namespace _get_completion_scheduler
	{
		template<typename T>
		struct get_completion_scheduler_t
		{
			[[nodiscard]] constexpr friend bool tag_invoke(forwarding_query_t, get_completion_scheduler_t) noexcept { return true; }

			template<typename Q> requires tag_invocable<get_completion_scheduler_t, const std::remove_cvref_t<Q> &>
			[[nodiscard]] constexpr decltype(auto) operator()(Q &&q) const noexcept { return tag_invoke(*this, std::as_const(q)); }
			[[nodiscard]] constexpr decltype(auto) operator()() const noexcept { return read(*this); }
		};
	}

	namespace _detail
	{
		template<typename S, typename C>
		concept has_completion_scheduler = callable<get_completion_scheduler_t<C>, env_of_t<S>>;
		template<typename T, typename C, typename S, typename... Ts>
		concept tag_invocable_with_completion_scheduler = has_completion_scheduler<S, C> && tag_invocable<T, std::invoke_result_t<get_completion_scheduler_t<C>, env_of_t<S>>, Ts...>;
	}

	/** Customization point object used to obtain the completion scheduler for channel \a T from the passed environment. */
	template<typename T>
	inline constexpr auto get_completion_scheduler = get_completion_scheduler_t<T>{};

	/** Concept used to define a scheduler type \a S that can be used to schedule work on it's associated execution context. */
	template<typename S>
	concept scheduler = queryable<S> && requires(S &&s, const get_completion_scheduler_t<set_value_t> tag)
	{
		{ schedule(std::forward<S>(s)) } -> sender;
		{ tag_invoke(tag, get_env(schedule(std::forward<S>(s)))) } -> std::same_as<std::remove_cvref_t<S>>;
	} && std::equality_comparable<std::remove_cvref_t<S>> && std::copy_constructible<std::remove_cvref_t<S>>;
}

/*
 * Created by switchblade on 2023-04-09.
 */

#pragma once

#include "detail/stop_token.hpp"
#include "detail/receiver.hpp"
#include "detail/sender.hpp"

namespace rod
{
	namespace detail
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

	using detail::get_completion_scheduler_t;

	/** Customization point object used to obtain the completion scheduler for signal \a T from the passed sender. */
	template<typename T>
	inline constexpr auto get_completion_scheduler = get_completion_scheduler_t<T>{};

	/** Concept used to check if type \a S can be used to schedule work on it's associated execution context. */
	template<typename S>
	concept scheduler = queryable<S> && requires(S &&s, const get_completion_scheduler_t<set_value_t> tag)
	{
		{ schedule(std::forward<S>(s)) } -> sender;
		{ tag_invoke(tag, get_env(schedule(std::forward<S>(s)))) } -> std::same_as<std::remove_cvref_t<S>>;
	} && std::equality_comparable<std::remove_cvref_t<S>> && std::copy_constructible<std::remove_cvref_t<S>>;

	namespace detail
	{
		struct execute_may_block_caller_t
		{
			template<scheduler S>
			[[nodiscard]] constexpr bool operator()(S &&s) noexcept
			{
				if constexpr (tag_invocable<execute_may_block_caller_t, const std::remove_cvref_t<S> &>)
					return tag_invoke(*this, std::as_const(s));
				else
					return true;
			}
		};
	}

	using detail::execute_may_block_caller_t;

	/** Customization point object used to check if a call to `execute(s, f)` with scheduler `s` and an invocable `f` could block the current thread. */
	inline constexpr auto execute_may_block_caller = execute_may_block_caller_t{};

	enum class forward_progress_guarantee
	{
		concurrent,
		parallel,
		weakly_parallel
	};

	namespace detail
	{
		class get_scheduler_t
		{
			template<typename T, typename U = decltype(std::as_const(std::declval<T>()))>
			static constexpr bool is_invocable = tag_invocable<get_scheduler_t, U>;
			template<typename T, typename U = decltype(std::as_const(std::declval<T>()))>
			using result_t = tag_invoke_result_t<get_scheduler_t, U>;

		public:
			[[nodiscard]] constexpr friend bool tag_invoke(forwarding_query_t, get_scheduler_t) noexcept { return true; }

			[[nodiscard]] constexpr auto operator()() const noexcept { return read(*this); }
			template<typename R> requires (is_invocable<R> && scheduler<result_t<R>>)
			[[nodiscard]] constexpr decltype(auto) operator()(R &&r) const noexcept
			{
				return tag_invoke(*this, std::as_const(r));
			}
		};
		class get_delegatee_scheduler_t
		{
			template<typename T, typename U = decltype(std::as_const(std::declval<T>()))>
			static constexpr bool is_invocable = tag_invocable<get_delegatee_scheduler_t, U>;
			template<typename T, typename U = decltype(std::as_const(std::declval<T>()))>
			using result_t = tag_invoke_result_t<get_delegatee_scheduler_t, U>;

		public:
			[[nodiscard]] constexpr friend bool tag_invoke(forwarding_query_t, get_delegatee_scheduler_t) noexcept { return true; }

			[[nodiscard]] constexpr auto operator()() const noexcept { return read(*this); }
			template<typename R> requires (is_invocable<R> && scheduler<result_t<R>>)
			[[nodiscard]] constexpr decltype(auto) operator()(R &&r) const noexcept
			{
				return tag_invoke(*this, std::as_const(r));
			}
		};
		struct get_forward_progress_guarantee_t
		{
			template<scheduler S, typename U = decltype(std::as_const(std::declval<S>()))>
			[[nodiscard]] constexpr forward_progress_guarantee operator()(S &&s) const noexcept
			{
				if constexpr (tag_invocable<get_forward_progress_guarantee_t, U>)
					return tag_invoke(*this, std::as_const(s));
				else
					return forward_progress_guarantee::weakly_parallel;
			}
		};
	}

	using detail::get_scheduler_t;
	using detail::get_delegatee_scheduler_t;
	using detail::get_forward_progress_guarantee_t;

	/** Customization point object used to obtain a scheduler from from the passed object. */
	inline constexpr auto get_scheduler = get_scheduler_t{};
	/** Customization point object used to obtain a scheduler that can be used for the purpose of forward progress delegation. */
	inline constexpr auto get_delegatee_scheduler = get_delegatee_scheduler_t{};
	/** Customization point object used to obtain a forward progress guarantee from the passed scheduler. */
	inline constexpr auto get_forward_progress_guarantee = get_forward_progress_guarantee_t{};
}

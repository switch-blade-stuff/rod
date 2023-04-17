/*
 * Created by switchblade on 2023-04-09.
 */

#pragma once

#include <system_error>

#include "environment.hpp"

namespace rod
{
	inline namespace _signals
	{
		struct set_value_t
		{
			template<typename R, typename... Vs>
			constexpr void operator()(R &&r, Vs &&...vs) noexcept(nothrow_tag_invocable<set_value_t, R, Vs...>) requires tag_invocable<set_value_t, R, Vs...>
			{
				tag_invoke(*this, std::forward<R>(r), std::forward<Vs>(vs)...);
			}
		};
		struct set_error_t
		{
			template<typename R, typename Err>
			constexpr void operator()(R &&r, Err &&err) noexcept(nothrow_tag_invocable<set_error_t, R, Err>) requires tag_invocable<set_error_t, R, Err>
			{
				tag_invoke(*this, std::forward<R>(r), std::forward<Err>(err));
			}
		};
		struct set_stopped_t
		{
			template<typename R>
			constexpr void operator()(R &&r) noexcept(nothrow_tag_invocable<set_stopped_t, R>) requires tag_invocable<set_stopped_t, R>
			{
				tag_invoke(*this, std::forward<R>(r));
			}
		};
	}

	inline constexpr auto set_value = set_value_t{};
	inline constexpr auto set_error = set_error_t{};
	inline constexpr auto set_stopped = set_stopped_t{};

	namespace detail
	{
		template<typename S, typename R, typename U = std::remove_cvref_t<R>>
		concept valid_completion_for = requires(S *s) { []<typename Tag, typename... Args>(Tag (*)(Args...)) requires nothrow_tag_invocable<Tag, U, Args...> {}(s); };

		template<typename>
		struct is_completion_signature : std::false_type {};
		template<typename... Vs>
		struct is_completion_signature<set_value_t(Vs...)> : std::true_type {};
		template<typename E>
		struct is_completion_signature<set_error_t(E)> : std::true_type {};
		template<>
		struct is_completion_signature<set_stopped_t()> : std::true_type {};

		template<typename F>
		concept completion_signature = is_completion_signature<F>::value;
	}

	/** Metaprogramming utility used to group multiple completion signatures as one type. */
	template<detail::completion_signature...>
	struct completion_signatures {};

	/** Concept used to define a generic receiver type. */
	template<typename R, typename U = std::remove_cvref_t<R>>
	concept receiver = requires(const U &r) {{ get_env(r) } -> queryable; } && std::move_constructible<U> && std::constructible_from<U, R>;
	/** Concept used to define a receiver type with a known set of completion signatures. */
	template<typename R, typename Cp>
	concept receiver_of = receiver<R> && requires { []<detail::valid_completion_for<R>... Sigs>(completion_signatures<Sigs...>) {}(Cp{}); };

	/** Concept used to define a generic sender type. */
	template<typename S, typename U = std::remove_cvref_t<S>>
	concept sender = requires(const U &s) {{ get_env(s) } -> queryable; } && std::move_constructible<U> && std::constructible_from<U, S>;
	/** Concept used to define a sender type who's environment specializes the `get_completion_signatures` customization point. */
	template<typename S, typename E = detail::empty_env_t>
	concept sender_in = sender<S> && requires(S &&s, E &&e) {{ get_completion_signatures(std::forward<S>(s), std::forward<E>(e)) } -> detail::instance_of<completion_signatures>; };
}
/*
 * Created by switchblade on 2023-04-09.
 */

#pragma once

#include <system_error>

#include "environment.hpp"

namespace rod
{
	inline namespace _channels
	{
		struct set_value_t
		{
			template<typename R, typename... Vs>
			constexpr void operator()(R &&rcv, Vs &&...vals) const noexcept(nothrow_tag_invocable<set_value_t, R, Vs...>) requires tag_invocable<set_value_t, R, Vs...>
			{
				tag_invoke(*this, std::forward<R>(rcv), std::forward<Vs>(vals)...);
			}
		};
		struct set_error_t
		{
			template<typename R, typename Err>
			constexpr void operator()(R &&rcv, Err &&err) const noexcept(nothrow_tag_invocable<set_error_t, R, Err>) requires tag_invocable<set_error_t, R, Err>
			{
				tag_invoke(*this, std::forward<R>(rcv), std::forward<Err>(err));
			}
		};
		struct set_stopped_t
		{
			template<typename R>
			constexpr void operator()(R &&rcv) const noexcept(nothrow_tag_invocable<set_stopped_t, R>) requires tag_invocable<set_stopped_t, R>
			{
				tag_invoke(*this, std::forward<R>(rcv));
			}
		};
	}

	/** Customization point object used to send a set of values through the value completion channel.
	 * @param rcv Receiver used to handle the value completion channel.
	 * @param vals Values sent through the value completion channel. */
	inline constexpr auto set_value = set_value_t{};
	/** Customization point object used to send an error through the error completion channel.
	 * @param rcv Receiver used to handle the error completion channel.
	 * @param err Error sent through the error completion channel. */
	inline constexpr auto set_error = set_error_t{};
	/** Customization point object used to send a stop signal through the stop completion channel.
	 * @param rcv Receiver used to handle the stop completion channel. */
	inline constexpr auto set_stopped = set_stopped_t{};

	namespace detail
	{
		template<typename R, typename F, typename... Args> requires std::invocable<F, Args...>
		constexpr void rcv_try_invoke(R &&rcv, F &&f, Args &&...args) noexcept
		{
			if constexpr (std::is_nothrow_invocable_v<F, Args...>)
				f(std::forward<Args>(args)...);
			else
			{
				try { f(std::forward<Args>(args)...); }
				catch (...) { set_error(std::forward<R>(rcv), std::current_exception()); }
			}
		}

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
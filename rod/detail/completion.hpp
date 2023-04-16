/*
 * Created by switchblade on 2023-04-10.
 */

#pragma once

#include <type_traits>
#include <exception>

#include "awaitable.hpp"

namespace rod
{
	inline namespace receivers
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
	template<detail::completion_signature... Fs>
	struct completion_signatures {};

	namespace detail
	{
		struct get_completion_signatures_t
		{
		private:
			template<typename S, typename E>
			[[nodiscard]] static constexpr bool is_noexcept() noexcept
			{
				if constexpr (tag_invocable<get_completion_signatures_t, S, E>)
					return nothrow_tag_invocable<get_completion_signatures_t, S, E>;
				else if constexpr (requires { typename std::remove_cvref_t<S>::completion_signatures; })
					return std::is_nothrow_constructible_v<typename std::remove_cvref_t<S>::completion_signatures>;
				else if constexpr (is_awaitable<S, env_promise<E>>)
					return true;
			}
			template<typename S, typename E>
			[[nodiscard]] static constexpr bool is_invocable() noexcept
			{
				return requires { typename std::remove_cvref_t<S>::completion_signatures; } ||
				       tag_invocable<get_completion_signatures_t, S, E> ||
				       is_awaitable<S, env_promise<E>>;
			}

		public:
			template<typename S, typename E>
			[[nodiscard]] constexpr decltype(auto) operator()(S &&s, E &&e) const noexcept(is_noexcept<S, E>()) requires(is_invocable<S, E>())
			{
				if constexpr (tag_invocable<get_completion_signatures_t, S, E>)
					return tag_invoke(*this, std::forward<S>(s), std::forward<E>(e));
				else if constexpr (requires { typename std::remove_cvref_t<S>::completion_signatures; })
					return typename std::remove_cvref_t<S>::completion_signatures{};
				else if constexpr (is_awaitable<S, env_promise<E>>)
				{
					using await_result = await_result_t<S, env_promise<E>>;
					if constexpr (!std::is_void_v<std::remove_cv_t<await_result>>)
						return completion_signatures<set_value_t(await_result), set_error_t(std::exception_ptr), set_stopped_t()>{};
					else
						return completion_signatures<set_value_t(), set_error_t(std::exception_ptr), set_stopped_t()>{};
				}
			}
		};
	}

	using detail::get_completion_signatures_t;

	/** Customization point object for which expression `get_completion_signatures(s, e)` is equivalent to one of the following:
	 * <ol>
	 * <li>If expression `tag_invoke(get_completion_signatures, s, e)` is well-formed, equivalent to the result of said expression.</li>
	 * <li>If expression `std::remove_cvref_t<S>::completion_signatures` names a well-formed type, equivalent to a value-initialized prvalue of said type.</li>
	 * <li>If expression `is-awaitable<S, env-promise<E>>` evaluates to `true` and `await-result-type<S, env-promise<E>>` is cv-qualified `void`, equivalent to
	 * `completion_signatures<set_value_t(), set_error_t(std::exception_ptr), set_stopped_t()>`.</li>
	 * <li>If expression `is-awaitable<S, env-promise<E>>` evaluates to `true`, equivalent to
	 * `completion_signatures<set_value_t(await-result-type<S, env-promise<E>>), set_error_t(std::exception_ptr), set_stopped_t()>`.</li>
	 * </ol>
	 * @note In all cases, result of `get_completion_signatures` must be an instance of `completion_signatures`. */
	inline constexpr auto get_completion_signatures = get_completion_signatures_t{};

	/** Alias for `decltype(get_completion_signatures(std::declval<S>(), std::declval<E>()))`. */
	template<typename S, typename E>
	using completion_signatures_of_t = decltype(get_completion_signatures(std::declval<S>(), std::declval<E>()));
}

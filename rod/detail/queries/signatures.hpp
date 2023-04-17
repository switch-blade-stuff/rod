/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include "../concepts.hpp"

namespace rod
{
	inline namespace _get_completion_signatures
	{
		struct get_completion_signatures_t
		{
			template<typename S, typename E>
			struct _overload_hook
			{
				[[nodiscard]] constexpr decltype(auto) operator()(S &&s, E &&e) const noexcept(nothrow_tag_invocable<get_completion_signatures_t, S, E>)
					requires tag_invocable<get_completion_signatures_t, S, E>
				{
					return tag_invoke(get_completion_signatures_t{}, std::forward<S>(s), std::forward<E>(e));
				}
				[[nodiscard]] constexpr decltype(auto) operator()(S &&, E &&) const noexcept(std::is_nothrow_constructible_v<typename std::remove_cvref_t<S>::completion_signatures>)
					requires (requires { typename std::remove_cvref_t<S>::completion_signatures; })
				{
					return typename std::remove_cvref_t<S>::completion_signatures{};
				}
			};

			template<typename S, typename E>
			[[nodiscard]] constexpr decltype(auto) operator()(S &&s, E &&e) const noexcept(std::is_nothrow_invocable_v<_overload_hook<S, E>, S, E>) requires std::invocable<_overload_hook<S, E>, S, E>
			{
				return _overload_hook<S, E>{}(std::forward<S>(s), std::forward<E>(e));
			}
		};
	}

	/** Customization point object used to query an object for it's advertised completion signatures either
	 * through an overload of `tag_invoke` or a member `completion_signatures` type.
	 * @note A separate overload exists for all awaitable types. */
	inline constexpr auto get_completion_signatures = get_completion_signatures_t{};
	/** Alias for `decltype(get_completion_signatures(std::declval<S>(), std::declval<E>()))`. */
	template<typename S, typename E>
	using completion_signatures_of_t = decltype(get_completion_signatures(std::declval<S>(), std::declval<E>()));

	namespace detail
	{
		template<typename, template<typename...> typename, typename, typename...>
		struct gather_signatures;
		template<typename Tag, template<typename...> typename T, template<typename...> typename V, typename... Ts, typename... Us, typename... Fs>
		struct gather_signatures<Tag, T, V<Ts...>, completion_signatures<Tag(Us...), Fs...>> : gather_signatures<Tag, T, V<Ts..., T<Us...>>, completion_signatures<Fs...>> {};
		template<typename Tag, template<typename...> typename T, template<typename...> typename V, typename... Ts, typename F, typename... Fs>
		struct gather_signatures<Tag, T, V<Ts...>, completion_signatures<F, Fs...>> : gather_signatures<Tag, T, V<Ts...>, completion_signatures<Fs...>> {};
		template<typename Tag, template<typename...> typename T, template<typename...> typename V, typename... Ts>
		struct gather_signatures<Tag, T, V<Ts...>, completion_signatures<>> { using type = V<Ts...>; };

		template<typename Tag, typename S, typename E, template<typename...> typename Tuple, template<typename...> typename Variant> requires sender_in<S, E>
		using gather_signatures_t = typename gather_signatures<Tag, Tuple, Variant<>, completion_signatures_of_t<S, E>>::type;
	}

	/** Concept used to check if sender type \a S can be connected to receiver \a R. */
	template<typename S, typename R>
	concept sender_to = sender_in<S, env_of_t<R>> && receiver_of<R, completion_signatures_of_t<S, env_of_t<R>>> && requires(S &&s, R &&r) { connect(std::forward<S>(s), std::forward<R>(r)); };
	/** Concept used to check if sender type \a S has advertises a completion signature returning for the `set_stopped_t` signal given an execution environment \a E. */
	template<typename S, typename E = detail::empty_env_t>
	concept sends_stopped = sender_in<S, E> && !std::same_as<detail::type_list_t<>, detail::gather_signatures_t<set_stopped_t, S, E, detail::type_list_t, detail::type_list_t>>;

	/** Given completion signatures `Ts` obtained via `completion_signatures_of_t<S, E>`, defines an alias for
	 * `Variant<Tuple<Args0...>, ..., Tuple<ArgsN...>>` where `ArgsN` is a template pack of the `N`th completion signature returning `set_value_t`. */
	template<typename S, typename E = detail::empty_env_t, template<typename...> typename Tuple = detail::decayed_tuple, template<typename...> typename Variant = detail::variant_or_empty> requires sender_in<S, E>
	using value_types_of_t = detail::gather_signatures_t<set_value_t, S, E, Tuple, Variant>;
	/** Given completion signatures `Ts` obtained via `completion_signatures_of_t<S, E>`, defines an alias for
	 * `Variant<Tuple<Args0...>, ..., Tuple<ArgsN...>>` where `ArgsN` is a template pack of the `N`th completion signature returning `set_error_t`. */
	template<typename S, typename E = detail::empty_env_t, template<typename...> typename Tuple = detail::decayed_tuple, template<typename...> typename Variant = detail::variant_or_empty> requires sender_in<S, E>
	using error_types_of_t = detail::gather_signatures_t<set_error_t, S, E, Tuple, Variant>;
}

/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include "../concepts.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	inline namespace _get_completion_signatures
	{
		struct get_completion_signatures_t
		{
			template<typename S, typename E> requires tag_invocable<get_completion_signatures_t, S, E>
			[[nodiscard]] constexpr decltype(auto) operator()(S &&snd, E &&env) const noexcept(nothrow_tag_invocable<get_completion_signatures_t, S, E>)
			{
				return tag_invoke(*this, std::forward<S>(snd), std::forward<E>(env));
			}
			template<typename S, typename E> requires(!tag_invocable<get_completion_signatures_t, S, E> && requires { typename std::remove_cvref_t<S>::completion_signatures; })
			[[nodiscard]] constexpr decltype(auto) operator()(S &&, E &&) const noexcept
			{
				return typename std::remove_cvref_t<S>::completion_signatures{};
			}

#ifdef ROD_HAS_COROUTINES
			template<typename S, typename E> requires(!tag_invocable<get_completion_signatures_t, S, E> && !requires { typename std::remove_cvref_t<S>::completion_signatures; })
			[[nodiscard]] constexpr decltype(auto) operator()(S &&snd, E &&env) const;
#endif
		};
	}

	/** Customization point object used to query a sender & execution environment objects for their advertised
	 * completion signatures either through an overload of `tag_invoke` or a member `completion_signatures` type. */
	inline constexpr auto get_completion_signatures = get_completion_signatures_t{};

	/** Alias for `decltype(get_completion_signatures(std::declval<S>(), std::declval<E>()))`. */
	template<typename S, typename E>
	using completion_signatures_of_t = decltype(get_completion_signatures(std::declval<S>(), std::declval<E>()));

	/** Concept used to define a sender type who's environment specializes the `get_completion_signatures` customization point. */
	template<typename S, typename E = empty_env>
	concept sender_in = sender<S> && requires(S &&s, E &&e) {{ get_completion_signatures(std::forward<S>(s), std::forward<E>(e)) } -> instance_of<completion_signatures>; };

	namespace detail
	{
		template<typename S, typename C>
		concept has_completion_scheduler = callable<get_completion_scheduler_t<C>, env_of_t<S>>;
		template<typename T, typename C, typename S, typename... Ts>
		concept tag_invocable_with_completion_scheduler = has_completion_scheduler<S, C> && tag_invocable<T, std::invoke_result_t<get_completion_scheduler_t<C>, env_of_t<S>>, Ts...>;

		template<typename>
		struct sender_of_helper;
		template<typename R, typename... Ts>
		struct sender_of_helper<R(Ts...)>
		{
			using tag = R;
			template<typename... Us>
			using as = R(Us...);
		};

		template<typename, template<typename...> typename, template<typename...> typename, typename, typename...>
		struct gather_signatures;
		template<typename Tag, template<typename...> typename T, template<typename...> typename V, typename... Ts, typename... Us, typename... Fs> requires(requires { typename apply_tuple_t<T, Us...>; })
		struct gather_signatures<Tag, T, V, type_list_t<Ts...>, completion_signatures<Tag(Us...), Fs...>> { using type = typename gather_signatures<Tag, T, V, type_list_t<Ts..., apply_tuple_t<T, Us...>>, completion_signatures<Fs...>>::type; };
		template<typename Tag, template<typename...> typename T, template<typename...> typename V, typename... Ts, typename F, typename... Fs>
		struct gather_signatures<Tag, T, V, type_list_t<Ts...>, completion_signatures<F, Fs...>> { using type = typename gather_signatures<Tag, T, V, type_list_t<Ts...>, completion_signatures<Fs...>>::type; };
		template<typename Tag, template<typename...> typename T, template<typename...> typename V, typename... Ts>
		struct gather_signatures<Tag, T, V, type_list_t<Ts...>, completion_signatures<>> { using type = apply_tuple_t<V, Ts...>; };

		template<typename Tag, typename S, typename E, template<typename...> typename Tuple, template<typename...> typename Variant>
		using gather_signatures_t = typename gather_signatures<Tag, Tuple, Variant, type_list_t<>, completion_signatures_of_t<S, E>>::type;

		template<typename Sig, typename... Ts>
		using test_signature = std::conjunction<matching_sig<Sig, Ts>...>;
	}

	/** Concept used to check if sender type \a S can be connected to receiver \a R. */
	template<typename S, typename R, typename E = env_of_t<R>>
	concept sender_to = sender_in<S, E> && receiver_of<R, completion_signatures_of_t<S, E>> && requires (S &&s, R &&r) { connect(std::forward<S>(s), std::forward<R>(r)); };

	/** Concept used to check if sender type \a S can complete with completion signatures \a Sig using environment \a E. */
	template<typename S, typename Sig, typename E = empty_env>
	concept sender_of = sender_in<S, E> && detail::gather_signatures_t<typename detail::sender_of_helper<Sig>::tag, S, E, detail::sender_of_helper<Sig>::template as, detail::bind_front<detail::test_signature, Sig>::template type>::value;

	/** Concept used to check if sender type \a S has advertises a completion signature returning for the stop channel given an execution environment \a E. */
	template<typename S, typename E = empty_env>
	concept sends_stopped = sender_in<S, E> && !std::same_as<type_list_t<>, detail::gather_signatures_t<set_stopped_t, S, E, type_list_t, type_list_t>>;

	/** Given completion signatures `Ts` obtained via `completion_signatures_of_t<S, E>`, defines an alias for
	 * `Variant<Tuple<Args0...>, ..., Tuple<ArgsN...>>` where `ArgsN` is a template pack of the `N`th completion signature of the value channel. */
	template<typename S, typename E = empty_env, template<typename...> typename Tuple = detail::decayed_tuple, template<typename...> typename Variant = detail::variant_or_empty> requires sender_in<S, E>
	using value_types_of_t = detail::gather_signatures_t<set_value_t, S, E, Tuple, Variant>;
	/** Given completion signatures `Ts` obtained via `completion_signatures_of_t<S, E>`, defines an alias for
	 * `Variant<Err0, ..., ErrN...>` where `ErrN` is the error type of the `N`th completion signature of the error channel. */
	template<typename S, typename E = empty_env, template<typename...> typename Variant = detail::variant_or_empty> requires sender_in<S, E>
	using error_types_of_t = detail::gather_signatures_t<set_error_t, S, E, std::type_identity_t, Variant>;

	namespace detail
	{
		template<typename>
		struct single_sender_value_type_impl;
		template<template<typename...> typename V, template<typename...> typename T, typename U>
		struct single_sender_value_type_impl<V<T<U>>> { using type = std::decay_t<U>; };
		template<template<typename...> typename V, template<typename...> typename T>
		struct single_sender_value_type_impl<V<T<>>> { using type = void; };
		template<template<typename...> typename V>
		struct single_sender_value_type_impl<V<>> { using type = void; };

		template<typename S, typename E>
		using single_sender_value_type = typename single_sender_value_type_impl<value_types_of_t<S, E>>::type;

		template<typename>
		struct is_single_sender_value : std::false_type {};
		template<template<typename...> typename T, typename U>
		struct is_single_sender_value<T<U>> : std::true_type {};
		template<typename... Ts>
		inline constexpr auto is_single_sender_value_v = is_single_sender_value<Ts...>::value;

		template<typename S, typename E>
		concept single_sender = sender_in<S, E> && is_single_sender_value_v<value_types_of_t<S, E>>;

		template<typename... Ts>
		using default_set_value = completion_signatures<set_value_t(Ts...)>;
		template<typename Err>
		using default_set_error = completion_signatures<set_error_t(Err)>;

		template<typename Snd, typename Env, typename AddSigs, template<typename...> typename SetVal, template<typename> typename SetErr, typename SetStop>
		struct impl_make_completion_signatures
		{
			template<typename... Ts>
			using error_list = type_list_t<SetErr<Ts>...>;

			using stop_types = type_list_t<std::conditional_t<sends_stopped<Snd, Env>, SetStop, completion_signatures<>>>;
			using value_types = value_types_of_t<Snd, Env, SetVal, type_list_t>;
			using error_types = error_types_of_t<Snd, Env, error_list>;

			using type = unique_tuple_t<apply_tuple_list_t<concat_tuples_t, concat_tuples_t<type_list_t<AddSigs>, value_types, error_types, stop_types>>>;
		};
	}

	/** Alias template used to adapt completion signatures of a sender. */
	template<typename Snd, typename Env = empty_env,
	         instance_of<completion_signatures> AddSigs = completion_signatures<>,
	         template<typename...> typename SetVal = detail::default_set_value,
	         template<typename> typename SetErr = detail::default_set_error,
	         instance_of<completion_signatures> SetStop = completion_signatures<set_stopped_t()>> requires sender_in<Snd, Env>
	using make_completion_signatures = typename detail::impl_make_completion_signatures<Snd, Env, AddSigs, SetVal, SetErr, SetStop>::type;
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

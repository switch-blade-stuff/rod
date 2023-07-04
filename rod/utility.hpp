/*
 * Created by switchblade on 2023-04-09.
 */

#pragma once

#include <system_error>
#include <exception>
#include <variant>

#include "tag.hpp"

namespace rod
{
	/** Concept used to check if type \a T is a movable value type. */
	template<typename T>
	concept movable_value = std::move_constructible<std::decay_t<T>> && std::constructible_from<std::decay_t<T>, T>;
	/** Concept used to check if type \a From is a reference of type \a To. */
	template<typename From, typename To>
	concept reference_to = std::same_as<std::remove_reference_t<From>, To>;
	/** Concept used to check if type \a From decays into type \a To. */
	template<typename From, typename To>
	concept decays_to = std::same_as<std::decay_t<From>, To>;
	/** Concept used to check if type \a T matches one of the types in \a Ts. */
	template<typename T, typename... Ts>
	concept one_of = (std::same_as<T, Ts> || ...);

	namespace detail
	{
		template<typename... Ts>
		using decayed_tuple = std::tuple<std::decay_t<Ts>...>;
		template<typename T>
		using decayed_ref = std::decay_t<T> &;

		template<typename T, typename... Args>
		concept callable = requires(T t, Args &&...args) { t(std::forward<Args>(args)...); };
		template<typename T, typename... Args>
		concept nothrow_callable = callable<T, Args...> && requires(T t, Args &&...args) { { t(std::forward<Args>(args)...) } noexcept; };

		template<typename T>
		using nothrow_decay_copyable = std::is_nothrow_constructible<std::decay_t<T>, T>;
		template<typename... Ts>
		using all_nothrow_decay_copyable = std::conjunction<nothrow_decay_copyable<Ts>...>;
		template<typename T>
		concept class_type = decays_to<T, T> && std::is_class_v<T>;

		template<typename, typename>
		struct matching_sig : std::false_type {};
		template<typename R0, typename... Args0, typename R1, typename... Args1>
		struct matching_sig<R0(Args0...), R1(Args1...)> : std::is_same<R0(Args0&&...), R0(Args1&&...)> {};

		template<typename...>
		inline constexpr bool always_true = true;

		template<bool>
		struct apply_tuple { template<template<typename...> typename T, typename... Ts> using type = T<Ts...>; };
		template<template<typename...> typename T, typename... Ts>
		using apply_tuple_t = typename apply_tuple<always_true<Ts...>>::template type<T, Ts...>;

		template<template<typename...> typename, typename>
		struct apply_tuple_list;
		template<template<typename...> typename T, template<typename...> typename L, typename... Ts>
		struct apply_tuple_list<T, L<Ts...>> { using type = apply_tuple_t<T, Ts...>; };
		template<template<typename...> typename T, typename L>
		using apply_tuple_list_t = typename apply_tuple_list<T, L>::type;

		template<typename...>
		struct concat_tuples;
		template<template<typename...> typename T, typename... Ts, typename... Us, typename... Vs>
		struct concat_tuples<T<Ts...>, T<Us...>, Vs...> : concat_tuples<apply_tuple_t<T, Ts..., Us...>, Vs...> {};
		template<template<typename...> typename T, typename... Ts>
		struct concat_tuples<T<Ts...>> { using type = apply_tuple_t<T, Ts...>; };
		template<typename... Ts>
		using concat_tuples_t = typename concat_tuples<Ts...>::type;

		template<typename F>
		struct eval_t
		{
			using type = std::invoke_result_t<F>;

			operator type() && noexcept(requires (F f) { { f() } noexcept; }) { return std::move(func)(); }
			type operator()() && noexcept(requires (F f) { { f() } noexcept; }) { return std::move(func)(); }

			ROD_NO_UNIQUE_ADDRESS F func;
		};
		template<typename F>
		eval_t(F) -> eval_t<F>;

		template<typename Err>
		[[nodiscard]] constexpr decltype(auto) as_except_ptr(Err &&err) noexcept
		{
			if constexpr (decays_to<Err, std::exception_ptr>)
				return std::forward<Err>(err);
			else if constexpr (decays_to<Err, std::error_code>)
				return std::make_exception_ptr(std::system_error(std::forward<Err>(err)));
			else
				return std::make_exception_ptr(std::forward<Err>(err));
		}

		template<template<typename...> typename T, typename... Ts>
		struct bind_front { template<typename... Us> using type = apply_tuple_t<T, Ts..., Us...>; };
		template<template<typename...> typename T, typename... Ts>
		struct bind_back { template<typename... Us> using type = apply_tuple_t<T, Us..., Ts...>; };

		template<typename From, typename To>
		struct copy_cvref_impl { using type = To; };

		template<typename From, typename To>
		struct copy_cvref_impl<const From, To> { using type = std::add_const_t<To>; };
		template<typename From, typename To>
		struct copy_cvref_impl<volatile From, To> { using type = std::add_volatile_t<To>; };
		template<typename From, typename To>
		struct copy_cvref_impl<const volatile From, To> { using type = std::add_cv_t<To>; };

		template<typename From, typename To>
		struct copy_cvref_impl<From &, To> { using type = std::add_lvalue_reference_t<typename copy_cvref_impl<From, To>::type>; };
		template<typename From, typename To>
		struct copy_cvref_impl<From &&, To> { using type = std::add_rvalue_reference_t<typename copy_cvref_impl<From, To>::type>; };
	}

	/** Metaprogramming utility used to copy reference, `const` & `volatile` qualifiers from \a From to \a To. */
	template<typename From, typename To>
	using copy_cvref = detail::copy_cvref_impl<From, To>;
	/** Alias for `typename rod::copy_cvref<From, To>::type`. */
	template<typename From, typename To>
	using copy_cvref_t = typename copy_cvref<From, To>::type;

	/** Metaprogramming utility used to copy `const` & `volatile` qualifiers from \a From to \a To. */
	template<typename From, typename To>
	using copy_cv = copy_cvref<std::remove_reference_t<From>, To>;
	/** Alias for `typename rod::copy_cv<From, To>::type`. */
	template<typename From, typename To>
	using copy_cv_t = typename copy_cv<From, To>::type;

	namespace detail
	{
		template<typename, template<typename...> typename>
		struct is_instance_of_impl : std::false_type {};
		template<template<typename...> typename T, typename... Ts>
		struct is_instance_of_impl<T<Ts...>, T> : std::true_type {};
	}

	/** Metaprogramming utility used to check if type \a U is an instance of template \a T. */
	template<typename U, template<typename...> typename T>
	using is_instance_of = detail::is_instance_of_impl<std::decay_t<U>, T>;
	/** Alias for `rod::is_instance_of<U, T>::value` */
	template<typename U, template<typename...> typename T>
	inline constexpr auto is_instance_of_v = is_instance_of<U, T>::value;
	/** Concept used to constrain type \a U to be an instance of template \a T. */
	template<typename U, template<typename...> typename T>
	concept instance_of = is_instance_of_v<U, T>;

	/** Utility type used to group type pack \a Ts. */
	template<typename... Ts>
	struct type_list_t {};
	/** Instance of `rod::type_list_t<Ts...>`. */
	template<typename... Ts>
	inline constexpr auto type_list = type_list_t<Ts...>{};

	namespace detail
	{
		template<typename, typename...>
		struct is_in_impl;
		template<typename U, typename T, typename... Ts>
		struct is_in_impl<U, T, Ts...> : is_in_impl<U, Ts...> {};
		template<typename U, typename... Ts>
		struct is_in_impl<U, U, Ts...> : std::true_type {};
		template<typename T>
		struct is_in_impl<T> : std::false_type {};

		template<typename, typename>
		struct is_in_tuple_impl;
		template<typename U, template<typename...> typename T, typename... Ts>
		struct is_in_tuple_impl<U, T<Ts...>> : is_in_impl<U, Ts...> {};
	}

	/** Metafunction used to check if type \a U is contained within type pack \a Ts. */
	template<typename U, typename... Ts>
	using is_in = detail::is_in_impl<U, Ts...>;
	/** Alias for `rod::is_in<U, Ts...>::value`. */
	template<typename U, typename... Ts>
	inline constexpr auto is_in_v = is_in<U, Ts...>::value;

	/** Metafunction used to check if type \a U is contained within tuple \a T. */
	template<typename U, typename T>
	using is_in_tuple = detail::is_in_tuple_impl<U, T>;
	/** Alias for `rod::is_in_tuple<U, T>::value`. */
	template<typename U, typename T>
	inline constexpr auto is_in_tuple_v = is_in_tuple<U, T>::value;

	namespace detail
	{
		template<typename>
		struct tuple_of;
		template<template <typename...> typename T, typename... Ts>
		struct tuple_of<T<Ts...>> { template <typename... Us> using type = apply_tuple_t<T, Us...>; };

		template<typename, typename>
		struct make_unique_list;
		template<template <typename...> typename T, typename... Ts, typename U, typename... Us> requires(is_in_impl<U, Ts...>::value)
		struct make_unique_list<T<Ts...>, T<U, Us...>> : make_unique_list<T<Ts...>, T<Us...>> {};
		template<template <typename...> typename T, typename... Ts, typename U, typename... Us> requires(!is_in_impl<U, Ts...>::value)
		struct make_unique_list<T<Ts...>, T<U, Us...>> : make_unique_list<T<U, Ts...>, T<Us...>> {};
		template<template <typename...> typename T, typename... Ts>
		struct make_unique_list<T<Ts...>, T<>> { using type = T<Ts...>; };
	}

	/** Metafunction used to filter duplicates from the passed tuple type. */
	template<typename T>
	using unique_tuple_t = typename detail::make_unique_list<typename detail::tuple_of<T>::template type<>, T>::type;

	namespace detail
	{
		template<typename...>
		struct empty_variant {};

		inline auto deduce_variant_or_empty(type_list_t<>) -> empty_variant<>;
		template<typename... Ts>
		inline auto deduce_variant_or_empty(type_list_t<Ts...>) -> std::variant<Ts...>;
		template<typename... Ts>
		using variant_or_empty = decltype(deduce_variant_or_empty(unique_tuple_t<type_list_t<std::decay_t<Ts>...>>{}));
	}
}

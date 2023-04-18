/*
 * Created by switchblade on 2023-04-09.
 */

#pragma once

#include <exception>
#include <variant>
#include <tuple>

#include "../tag.hpp"

namespace rod::detail
{
	template<typename Err>
	[[nodiscard]] constexpr decltype(auto) as_except_ptr(Err &&err) noexcept
	{
		if constexpr (std::same_as<std::decay_t<Err>, std::exception_ptr>)
			return std::forward<Err>(err);
		else if constexpr (std::same_as<std::decay_t<Err>, std::error_code>)
			return std::make_exception_ptr(std::system_error(std::forward<Err>(err)));
		else
			return std::make_exception_ptr(std::forward<Err>(err));
	}

	template<typename From, typename To>
	struct copy_cvref { using type = To; };

	template<typename From, typename To>
	struct copy_cvref<const From, To> { using type = std::add_const_t<To>; };
	template<typename From, typename To>
	struct copy_cvref<volatile From, To> { using type = std::add_volatile_t<To>; };
	template<typename From, typename To>
	struct copy_cvref<const volatile From, To> { using type = std::add_cv_t<To>; };

	template<typename From, typename To>
	struct copy_cvref<From &, To> { using type = std::add_lvalue_reference<copy_cvref<From, To>>; };
	template<typename From, typename To>
	struct copy_cvref<From &&, To> { using type = std::add_rvalue_reference<copy_cvref<From, To>>; };

	template<typename From, typename To>
	using copy_cvref_t = typename copy_cvref<From, To>::type;

	template<typename T>
	concept movable_value = std::move_constructible<std::decay_t<T>> && std::constructible_from<std::decay_t<T>, T>;

	template<typename T>
	concept ebo_candidate = std::conjunction<std::is_empty<T>, std::negation<std::is_final<T>>>::value;

	template<typename T>
	class ebo_helper
	{
	public:
		constexpr ebo_helper() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
		constexpr ebo_helper(const ebo_helper &other) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;
		constexpr ebo_helper(ebo_helper &&other) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
		constexpr ebo_helper &operator=(const ebo_helper &other) noexcept(std::is_nothrow_copy_assignable_v<T>) = default;
		constexpr ebo_helper &operator=(ebo_helper &&other) noexcept(std::is_nothrow_move_assignable_v<T>) = default;

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr ebo_helper(Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>) : m_value(std::forward<Args>(args)...) {}

		[[nodiscard]] constexpr T &value() noexcept { return m_value; }
		[[nodiscard]] constexpr const T &value() const noexcept { return m_value; }

		constexpr void swap(ebo_helper &other) noexcept(std::is_nothrow_swappable_v<T>)
		{
			using std::swap;
			swap(value(), other.value());
		}

	private:
		T m_value;
	};
	template<ebo_candidate T>
	class ebo_helper<T> : T
	{
	public:
		constexpr ebo_helper() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
		constexpr ebo_helper(const ebo_helper &other) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;
		constexpr ebo_helper(ebo_helper &&other) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
		constexpr ebo_helper &operator=(const ebo_helper &other) noexcept(std::is_nothrow_copy_assignable_v<T>) = default;
		constexpr ebo_helper &operator=(ebo_helper &&other) noexcept(std::is_nothrow_move_assignable_v<T>) = default;

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr ebo_helper(Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>) : T(std::forward<Args>(args)...) {}

		[[nodiscard]] constexpr T &value() noexcept { return static_cast<T &>(*this); }
		[[nodiscard]] constexpr const T &value() const noexcept { return static_cast<const T &>(*this); }

		constexpr void swap(ebo_helper &other) noexcept(std::is_nothrow_swappable_v<T>)
		{
			using std::swap;
			swap(value(), other.value());
		}
	};

	template<typename, template<typename...> typename>
	struct is_instance_of : std::false_type {};
	template<template<typename...> typename T, typename... Ts>
	struct is_instance_of<T<Ts...>, T> : std::true_type {};

	template<typename I, template<typename...> typename T>
	inline constexpr auto is_instance_of_v = is_instance_of<I, T>::value;
	template<typename I, template<typename...> typename T>
	concept instance_of = is_instance_of_v<I, T>;

	template<typename... Ts>
	struct type_list_t {};
	template<typename... Ts>
	inline constexpr auto type_list = type_list_t<Ts...>{};

	template<typename, typename...>
	struct is_in_impl;
	template<typename U, typename T, typename... Ts>
	struct is_in_impl<U, T, Ts...> : is_in_impl<U, Ts...> {};
	template<typename U, typename... Ts>
	struct is_in_impl<U, U, Ts...> : std::true_type {};
	template<typename T>
	struct is_in_impl<T> : std::false_type {};

	template<typename, typename>
	struct is_in_list_impl;
	template<typename T, template<typename...> typename L, typename... Ts>
	struct is_in_list_impl<T, L<Ts...>> : is_in_impl<T, Ts...> {};
	template<typename T, typename L>
	using is_in_list = is_in_list_impl<T, L>;

	template<typename, typename>
	struct make_unique_list;
	template<typename... Ts, typename U, typename... Us> requires(is_in_impl<U, Ts...>::value)
	struct make_unique_list<type_list_t<Ts...>, type_list_t<U, Us...>> : make_unique_list<type_list_t<Ts...>, type_list_t<Us...>> {};
	template<typename... Ts, typename U, typename... Us> requires (!is_in_impl<U, Ts...>::value)
	struct make_unique_list<type_list_t<Ts...>, type_list_t<U, Us...>> : make_unique_list<type_list_t<U, Ts...>, type_list_t<Us...>> {};
	template<typename... Ts>
	struct make_unique_list<type_list_t<Ts...>, type_list_t<>> { using type = type_list_t<Ts...>; };

	template<typename... Ts>
	using unique_list_t = typename make_unique_list<type_list_t<>, type_list_t<Ts...>>::type;
	template<typename... Ts>
	inline constexpr auto unique_list = unique_list_t<Ts...>{};

	template<typename... Ts>
	using decayed_tuple = std::tuple<std::decay_t<Ts>...>;

	struct empty_variant { empty_variant() = delete; };

	inline auto deduce_variant_or_empty(type_list_t<>) -> empty_variant;
	template<typename... Ts>
	inline auto deduce_variant_or_empty(type_list_t<Ts...>) -> std::variant<Ts...>;
	template<typename... Ts>
	using variant_or_empty = decltype(deduce_variant_or_empty(unique_list<std::decay_t<Ts>...>));

	template<template<typename...> typename T, typename... Ts>
	struct bind_front { template<typename... Us> using type = T<Ts..., Us...>; };
	template<template<typename...> typename T, typename... Ts>
	struct bind_back { template<typename... Us> using type = T<Us..., Ts...>; };

	template<typename, typename>
	struct push_front;
	template<template<typename...> typename T, typename... Ts, typename U>
	struct push_front<T<Ts...>, U> { using type = T<U, Ts...>; };
	template<typename T, typename U>
	using push_front_t = typename push_front<T, U>::type;

	template<typename, typename>
	struct push_back;
	template<template<typename...> typename T, typename... Ts, typename U>
	struct push_back<T<Ts...>, U> { using type = T<Ts..., U>; };
	template<typename T, typename U>
	using push_back_t = typename push_back<T, U>::type;

	template<template<typename...> typename, typename...>
	struct concat_on;
	template<template<typename...> typename V, template<typename...> typename T0, template<typename...> typename T1, typename... Ts, typename... Us, typename... Vs>
	struct concat_on<V, T0<Ts...>, T1<Us...>, Vs...> : concat_on<V, V<Ts..., Us...>, Vs...> {};
	template<template<typename...> typename V, template<typename...> typename T, typename... Ts>
	struct concat_on<V, T<Ts...>> { using type = V<Ts...>; };
	template<template<typename...> typename V, typename... Ts>
	using concat_on_t = typename bind_back<V, Ts...>::type;
}
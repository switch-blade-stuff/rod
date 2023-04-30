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
	namespace detail
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

			template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
			constexpr ebo_helper(Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>) : m_value(std::forward<Args>(args)...) {}

			constexpr ebo_helper &operator=(const ebo_helper &other) noexcept(std::is_nothrow_copy_assignable_v<T>) = default;
			constexpr ebo_helper &operator=(ebo_helper &&other) noexcept(std::is_nothrow_move_assignable_v<T>) = default;

			template<typename U = T> requires std::assignable_from<T, const U &>
			constexpr ebo_helper &operator=(const U &other) noexcept(std::is_nothrow_assignable_v<T, const U &>)
			{
				m_value = other;
				return *this;
			}
			template<typename U = T> requires std::assignable_from<T, U>
			constexpr ebo_helper &operator=(U &&other) noexcept(std::is_nothrow_assignable_v<T, U>)
			{
				m_value = std::forward<U>(other);
				return *this;
			}

			constexpr const ebo_helper &operator=(const ebo_helper &other) const noexcept(std::is_nothrow_assignable_v<const T &, const T &>)
			{
				m_value = other.m_value;
				return *this;
			}
			constexpr const ebo_helper &operator=(ebo_helper &&other) const noexcept(std::is_nothrow_assignable_v<const T &, T>)
			{
				m_value = std::move(other.m_value);
				return *this;
			}

			template<typename U = T> requires std::assignable_from<const T &, const U &>
			constexpr const ebo_helper &operator=(const U &other) const noexcept(std::is_nothrow_assignable_v<const T &, const U &>)
			{
				m_value = other;
				return *this;
			}
			template<typename U = T> requires std::assignable_from<const T &, U>
			constexpr const ebo_helper &operator=(U &&other) const noexcept(std::is_nothrow_assignable_v<const T &, U>)
			{
				m_value = std::forward<U>(other);
				return *this;
			}

			[[nodiscard]] constexpr T &value() & noexcept { return m_value; }
			[[nodiscard]] constexpr const T &value() const & noexcept { return m_value; }
			[[nodiscard]] constexpr T &&value() && noexcept { return std::move(m_value); }
			[[nodiscard]] constexpr const T &&value() const && noexcept { return std::move(m_value); }

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

			template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
			constexpr ebo_helper(Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>) : T(std::forward<Args>(args)...) {}

			constexpr ebo_helper &operator=(const ebo_helper &other) noexcept(std::is_nothrow_copy_assignable_v<T>) = default;
			constexpr ebo_helper &operator=(ebo_helper &&other) noexcept(std::is_nothrow_move_assignable_v<T>) = default;

			template<typename U = T> requires std::assignable_from<T, const U &>
			constexpr ebo_helper &operator=(const U &other) noexcept(std::is_nothrow_assignable_v<T, const U &>)
			{
				value() = other;
				return *this;
			}
			template<typename U = T> requires std::assignable_from<T, U>
			constexpr ebo_helper &operator=(U &&other) noexcept(std::is_nothrow_assignable_v<T, U>)
			{
				value() = std::forward<U>(other);
				return *this;
			}

			constexpr const ebo_helper &operator=(const ebo_helper &other) const noexcept(std::is_nothrow_assignable_v<const T &, const T &>)
			{
				value() = other.value();
				return *this;
			}
			constexpr const ebo_helper &operator=(ebo_helper &&other) const noexcept(std::is_nothrow_assignable_v<const T &, T>)
			{
				value() = std::move(other.value());
				return *this;
			}
			template<typename U = T> requires std::assignable_from<const T &, const U &>
			constexpr const ebo_helper &operator=(const U &other) const noexcept(std::is_nothrow_assignable_v<const T &, const U &>)
			{
				value() = other;
				return *this;
			}
			template<typename U = T> requires std::assignable_from<const T &, U>
			constexpr const ebo_helper &operator=(U &&other) const noexcept(std::is_nothrow_assignable_v<const T &, U>)
			{
				value() = std::forward<U>(other);
				return *this;
			}

			[[nodiscard]] constexpr T &value() & noexcept { return static_cast<T &>(*this); }
			[[nodiscard]] constexpr const T &value() const & noexcept { return static_cast<const T &>(*this); }
			[[nodiscard]] constexpr T &&value() && noexcept { return std::move(static_cast<T &>(*this)); }
			[[nodiscard]] constexpr const T &&value() const && noexcept { return std::move(static_cast<const T &>(*this)); }

			constexpr void swap(ebo_helper &other) noexcept(std::is_nothrow_swappable_v<T>)
			{
				using std::swap;
				swap(value(), other.value());
			}
		};

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
		template<template<typename...> typename V, template<typename...> typename T, typename... Ts,
				template<typename...> typename U, typename... Us, typename... Vs>
		struct concat_on<V, T<Ts...>, U<Us...>, Vs...> : concat_on<V, V<Ts..., Us...>, Vs...> {};
		template<template<typename...> typename V, template<typename...> typename T, typename... Ts>
		struct concat_on<V, T<Ts...>> { using type = V<Ts...>; };
		template<template<typename...> typename V>
		struct concat_on<V> { using type = V<>; };
		template<template<typename...> typename V, typename... Ts>
		using concat_on_t = typename concat_on<V, Ts...>::type;

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
	/** Alias for `typename copy_cvref<From, To>::type`. */
	template<typename From, typename To>
	using copy_cvref_t = typename copy_cvref<From, To>::type;

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
	/** Alias for `is_instance_of<U, T>::value` */
	template<typename U, template<typename...> typename T>
	inline constexpr auto is_instance_of_v = is_instance_of<U, T>::value;
	/** Concept used to constrain type \a U to be an instance of template \a T. */
	template<typename U, template<typename...> typename T>
	concept instance_of = is_instance_of_v<U, T>;

	/** Utility type used to group type pack \a Ts. */
	template<typename... Ts>
	struct type_list_t {};
	/** Instance of `type_list_t<Ts...>`. */
	template<typename... Ts>
	inline constexpr auto type_list = type_list_t<Ts...>{};

	namespace detail
	{
		template<template<typename...> typename T, typename U>
		struct apply_types { using type = T<U>; };
		template<template<typename...> typename T, typename... Ts>
		struct apply_types<T, type_list_t<Ts...>> { using type = T<Ts...>; };
		template<template<typename...> typename T, typename U>
		using apply_types_t = typename apply_types<T, U>::type;

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
	/** Alias for `is_in<U, Ts...>::value`. */
	template<typename U, typename... Ts>
	inline constexpr auto is_in_v = is_in<U, Ts...>::value;

	/** Metafunction used to check if type \a U is contained within tuple \a T. */
	template<typename U, typename T>
	using is_in_tuple = detail::is_in_tuple_impl<U, T>;
	/** Alias for `is_in_tuple<U, T>::value`. */
	template<typename U, typename T>
	inline constexpr auto is_in_tuple_v = is_in_tuple<U, T>::value;

	namespace detail
	{
		template<typename...>
		struct is_single_value_tuple : std::false_type {};
		template<template<typename...> typename V, typename T>
		struct is_single_value_tuple<V<T>> : std::true_type {};
		template<template<typename...> typename V>
		struct is_single_value_tuple<V<>> : std::true_type {};
		template<typename T>
		inline constexpr bool is_single_value_tuple_v = is_single_value_tuple<T>::value;

		template<typename>
		struct tuple_of;
		template<template <typename...> typename T, typename... Ts>
		struct tuple_of<T<Ts...>> { template <typename... Us> using type = T<Us...>; };

		template<typename, typename>
		struct make_unique_list;
		template<template <typename...> typename T, typename... Ts, typename U, typename... Us> requires(is_in_impl<U, Ts...>::value)
		struct make_unique_list<T<Ts...>, T<U, Us...>> : make_unique_list<T<Ts...>, T<Us...>> {};
		template<template <typename...> typename T, typename... Ts, typename U, typename... Us> requires (!is_in_impl<U, Ts...>::value)
		struct make_unique_list<T<Ts...>, T<U, Us...>> : make_unique_list<T<U, Ts...>, T<Us...>> {};
		template<template <typename...> typename T, typename... Ts>
		struct make_unique_list<T<Ts...>, T<>> { using type = T<Ts...>; };
	}

	/** Metafunction used to filter duplicates from the passed tuple type. */
	template<typename T>
	using unique_tuple_t = typename detail::make_unique_list<typename detail::tuple_of<T>::template type<>, T>::type;

	namespace detail
	{
		template<typename... Ts>
		using decayed_tuple = std::tuple<std::decay_t<Ts>...>;

		template<typename...>
		struct empty_variant { empty_variant() = delete; };

		inline auto deduce_variant_or_empty(type_list_t<>) -> empty_variant<>;
		template<typename... Ts>
		inline auto deduce_variant_or_empty(type_list_t<Ts...>) -> std::variant<Ts...>;
		template<typename... Ts>
		using variant_or_empty = decltype(deduce_variant_or_empty(unique_tuple_t<type_list_t<std::decay_t<Ts>...>>{}));
	}
}
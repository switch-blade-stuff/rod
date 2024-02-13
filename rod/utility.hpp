/*
 * Created by switchblade on 2023-04-09.
 */

#pragma once

#include <system_error>
#include <functional>
#include <exception>
#include <variant>
#include <memory>
#include <array>
#include <span>

#include <cstring>
#include <cstdint>
#include <cwchar>

#include "tag.hpp"

namespace rod
{
	/** Concept used to check if type \a T is a movable value type. */
	template<typename T>
	concept movable_value = std::move_constructible<std::decay_t<T>> && std::constructible_from<std::decay_t<T>, T>;

	/** Concept used to check if type \a T is a reference to a type derived from \a U. */
	template<typename T, typename U>
	concept reference_to_derived = std::derived_from<std::remove_reference_t<T>, U>;
	/** Concept used to check if type \a T decays to a type derived from \a U. */
	template<typename T, typename U>
	concept decays_to_derived = std::derived_from<std::decay_t<T>, U>;

	/** Concept used to check if type \a T is a reference of type \a U. */
	template<typename T, typename U>
	concept reference_to_same = std::same_as<std::remove_reference_t<T>, U>;
	/** Concept used to check if type \a From decays into type \a To. */
	template<typename From, typename To>
	concept decays_to_same = std::same_as<std::decay_t<From>, To>;

	/** Concept used to check if type \a T decays to type \a U or is derived from \a U. */
	template<typename T, typename U>
	concept decays_to_same_or_derived = decays_to_same<T, U> || decays_to_derived<T, U>;
	/** Concept used to check if type \a T is same as \a U or is derived from \a U. */
	template<typename T, typename U>
	concept is_same_or_derived = std::same_as<T, U> || std::derived_from<T, U>;

	/** Concept used to check if type \a T matches one of the types in \a Ts. */
	template<typename T, typename... Ts>
	concept one_of = (std::same_as<T, Ts> || ...);

	/** Concept used to check if tuple-like type \a T has an element at index \a N.
	 * @note Equivalent to C++23 exposition-only `is-tuple-element` concept. */
	template<typename T, std::size_t N>
	concept is_tuple_element = requires (T t)
	{
		typename std::tuple_element_t<N, T>;
		{ std::get<N>(t) } -> std::convertible_to<std::tuple_element_t<N, T> &>;
	};
	/** Concept used to check if type \a T is a tuple-like type (can be used with tuple algorithms and participate in structural binding).
	 * @note Equivalent to C++23 exposition-only `tuple-like` concept. */
	template<typename T>
	concept tuple_like = requires
	{
		typename std::tuple_size<T>::type;
		requires std::same_as<std::remove_cvref_t<decltype(std::tuple_size_v<T>)>, std::size_t>;
	} && []<std::size_t... I>(std::index_sequence<I...>) { return (is_tuple_element<T, I> && ...); }(std::make_index_sequence<std::tuple_size_v<T>>{});
	/** Concept used to check if type \a T is a pair-like type (models `tuple_like` and has only `2` elements).
	 * @note Equivalent to C++23 exposition-only `pair-like` concept. */
	template<typename T>
	concept pair_like = tuple_like<T> && std::tuple_size_v<T> == 2;

	namespace _detail
	{
		template<typename... Ts>
		using decayed_tuple = std::tuple<std::decay_t<Ts>...>;
		template<typename T>
		using decayed_ref = std::decay_t<T> &;

		template<typename T, typename... Args>
		concept callable = requires(T func, Args ...args) { func(std::forward<Args>(args)...); };
		template<typename T, typename... Args>
		concept nothrow_callable = callable<T, Args...> && requires(T func, Args ...args) { { func(std::forward<Args>(args)...) } noexcept; };

		template<typename R, typename T, typename... Args>
		concept callable_r = callable<T, Args...> && requires(T func, Args ...args) { { func(std::forward<Args>(args)...) } -> std::convertible_to<R>; };
		template<typename R, typename T, typename... Args>
		concept nothrow_callable_r = callable_r<R, T, Args...> && nothrow_callable<T, Args...>;

		template<typename T>
		using decay_copyable = std::is_constructible<std::decay_t<T>, T>;
		template<typename... Ts>
		using all_decay_copyable = std::conjunction<decay_copyable<Ts>...>;

		template<typename T>
		using nothrow_decay_copyable = std::is_nothrow_constructible<std::decay_t<T>, T>;
		template<typename... Ts>
		using all_nothrow_decay_copyable = std::conjunction<nothrow_decay_copyable<Ts>...>;

		template<typename T>
		concept class_type = decays_to_same<T, T> && std::is_class_v<T>;

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
	using copy_cvref = _detail::copy_cvref_impl<From, To>;
	/** Alias for `typename rod::copy_cvref&lt;From, To&gt;::type`. */
	template<typename From, typename To>
	using copy_cvref_t = typename copy_cvref<From, To>::type;

	/** Metaprogramming utility used to copy `const` & `volatile` qualifiers from \a From to \a To. */
	template<typename From, typename To>
	using copy_cv = copy_cvref<std::remove_reference_t<From>, To>;
	/** Alias for `typename rod::copy_cv&lt;From, To&gt;::type`. */
	template<typename From, typename To>
	using copy_cv_t = typename copy_cv<From, To>::type;

	namespace _detail
	{
		template<typename, template<typename...> typename>
		struct is_instance_of_impl : std::false_type {};
		template<template<typename...> typename T, typename... Ts>
		struct is_instance_of_impl<T<Ts...>, T> : std::true_type {};
	}

	/** Metaprogramming utility used to check if type \a U is an instance of template \a T. */
	template<typename U, template<typename...> typename T>
	using is_instance_of = _detail::is_instance_of_impl<std::decay_t<U>, T>;
	/** Alias for `rod::is_instance_of&lt;U, T&gt;::value` */
	template<typename U, template<typename...> typename T>
	inline constexpr auto is_instance_of_v = is_instance_of<U, T>::value;

	/** Concept used to constrain type \a U to be an instance of template \a T. */
	template<typename U, template<typename...> typename T>
	concept instance_of = is_instance_of_v<U, T>;
	/** Concept used to constrain type \a U to decay into an instance of template \a T. */
	template<typename U, template<typename...> typename T>
	concept decays_to_instance_of = instance_of<std::decay_t<U>, T>;

	/** Utility type used to group type pack \a Ts. */
	template<typename... Ts>
	struct type_list_t { static constexpr auto size = sizeof...(Ts); };
	/** Instance of `rod::type_list_t&lt;Ts...&gt;`. */
	template<typename... Ts>
	inline constexpr auto type_list = type_list_t<Ts...>{};

	namespace _detail
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
	using is_in = _detail::is_in_impl<U, Ts...>;
	/** Alias for `rod::is_in&lt;U, Ts...&gt;::value`. */
	template<typename U, typename... Ts>
	inline constexpr auto is_in_v = is_in<U, Ts...>::value;

	/** Metafunction used to check if type \a U is contained within tuple \a T. */
	template<typename U, typename T>
	using is_in_tuple = _detail::is_in_tuple_impl<U, T>;
	/** Alias for `rod::is_in_tuple&lt;U, T&gt;::value`. */
	template<typename U, typename T>
	inline constexpr auto is_in_tuple_v = is_in_tuple<U, T>::value;

	namespace _detail
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
	using unique_tuple_t = typename _detail::make_unique_list<typename _detail::tuple_of<T>::template type<>, T>::type;

	namespace _detail
	{
		template<typename T0, typename T1>
		inline constexpr bool test_member_swappable() noexcept
		{
			return requires(T0 &a, T1 &b) { a.swap(b); };
		}
		template<typename T0, typename T1>
		inline constexpr bool test_static_swappable() noexcept
		{
			using std::swap;
			return requires(T0 &a, T1 &b) { swap(a, b); };
		}

		template<typename T0, typename T1>
		using member_swappable = std::bool_constant<test_member_swappable<T0, T1>()>;
		template<typename T0, typename T1>
		using static_swappable = std::bool_constant<test_static_swappable<T0, T1>()>;

		template<typename T0, typename T1>
		inline constexpr bool test_nothrow_member_swappable() noexcept
		{
			return requires(T0 &a, T1 &b) { noexcept(a.swap(b)); };
		}
		template<typename T0, typename T1>
		inline constexpr bool test_nothrow_static_swappable() noexcept
		{
			using std::swap;
			return requires(T0 &a, T1 &b) { noexcept(swap(a, b)); };
		}

		template<typename T0, typename T1>
		using nothrow_member_swappable = std::bool_constant<test_nothrow_member_swappable<T0, T1>()>;
		template<typename T0, typename T1>
		using nothrow_static_swappable = std::bool_constant<test_nothrow_static_swappable<T0, T1>()>;

		template<typename T0, typename T1>
		struct adl_swappable : std::disjunction<member_swappable<std::remove_reference_t<T0>, std::remove_reference_t<T1>>, static_swappable<std::remove_reference_t<T0>, std::remove_reference_t<T1>>> {};
		template<typename T0, typename T1>
		struct nothrow_adl_swappable : std::disjunction<nothrow_member_swappable<std::remove_reference_t<T0>, std::remove_reference_t<T1>>, nothrow_static_swappable<std::remove_reference_t<T0>, std::remove_reference_t<T1>>> {};
	}

	/** Utility function used to select either a member overload of `swap` or an ADL overload with associated `std::swap`. */
	template<typename T0, typename T1> requires _detail::adl_swappable<T0, T1>::value
	inline constexpr void adl_swap(T0 &&a, T1 &&b) noexcept(_detail::nothrow_adl_swappable<T0, T1>::value)
	{
		using std::swap;
		if constexpr (!requires { std::forward<T0>(a).swap(std::forward<T1>(b)); })
			swap(std::forward<T0>(a), std::forward<T1>(b));
		else
			std::forward<T0>(a).swap(std::forward<T1>(b));
	}

	namespace _detail
	{
		template<typename T>
		class empty_base
		{
		public:
			constexpr empty_base() noexcept(std::is_nothrow_default_constructible_v<T>) requires std::constructible_from<T> = default;
			template<typename... Args> requires std::constructible_from<T, Args...>
			constexpr explicit empty_base(Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>) : _value(std::forward<Args>(args)...) {}
			template<typename U = T> requires(!decays_to_same<U, empty_base> && std::assignable_from<T, U>)
			constexpr empty_base &operator=(U &&value) noexcept(std::is_nothrow_assignable_v<T, U>) { return (_value = std::forward<U>(value), *this); }

			/** Returns reference to the underlying value. */
			[[nodiscard]] constexpr auto &value() & noexcept { return _value; }
			/** @copydoc value */
			[[nodiscard]] constexpr auto &value() const & noexcept { return _value; }
			/** @copydoc value */
			[[nodiscard]] constexpr auto &&value() && noexcept { return std::move(_value); }
			/** @copydoc value */
			[[nodiscard]] constexpr auto &&value() const && noexcept { return std::move(_value); }

			/** Returns pointer to the underlying value. */
			[[nodiscard]] constexpr auto get() noexcept requires(!std::is_reference_v<T>) { return std::addressof(value()); }
			/** @copydoc get */
			[[nodiscard]] constexpr auto get() const noexcept requires(!std::is_reference_v<T>) { return std::addressof(value()); }

			constexpr void swap(empty_base &other) noexcept(std::is_nothrow_swappable_v<T>) requires std::swappable<T> { adl_swap(_value, other._value); }

		private:
			T _value;
		};
		template<typename T> requires(std::is_object_v<T> && std::is_empty_v<T> && !std::is_final_v<T>)
		class empty_base<T> : T
		{
		public:
			constexpr empty_base() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
			template<typename... Args> requires std::constructible_from<T, Args...>
			constexpr explicit empty_base(Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>) : T(std::forward<Args>(args)...) {}
			template<typename U = T> requires(!decays_to_same<U, empty_base> && std::assignable_from<T, U>)
			constexpr empty_base &operator=(U &&value) noexcept(std::is_nothrow_assignable_v<T, U>) { return (*get() = std::forward<U>(value), *this); }

			/** Returns reference to the underlying value. */
			[[nodiscard]] constexpr auto &value() & noexcept { return static_cast<T &>(*this); }
			/** @copydoc value */
			[[nodiscard]] constexpr auto &value() const & noexcept { return static_cast<std::add_const_t<T> &>(*this); }
			/** @copydoc value */
			[[nodiscard]] constexpr auto &&value() && noexcept { return static_cast<T &&>(*this); }
			/** @copydoc value */
			[[nodiscard]] constexpr auto &&value() const && noexcept { return static_cast<std::add_const_t<T> &&>(*this); }

			/** Returns pointer to the underlying value. */
			[[nodiscard]] constexpr auto get() noexcept requires(!std::is_reference_v<T>) { return std::addressof(value()); }
			/** @copydoc get */
			[[nodiscard]] constexpr auto get() const noexcept requires(!std::is_reference_v<T>) { return std::addressof(value()); }

			constexpr void swap(empty_base &other) noexcept(std::is_nothrow_swappable_v<T>) requires std::swappable<T> { adl_swap(value(), other.value()); }
		};

		template<typename T>
		constexpr void swap(empty_base<T> &a, empty_base<T> &b) noexcept(std::is_nothrow_swappable_v<T>) requires std::swappable<T> { a.swap(b); }

		template<typename T>
		[[nodiscard]] constexpr auto operator<=>(const empty_base<T> &a, const empty_base<T> &b) noexcept(noexcept(a.value() <=> b.value())) requires(requires { a.value() <=> b.value(); }) { return a.value() <=> b.value(); }
		template<typename T>
		[[nodiscard]] constexpr bool operator==(const empty_base<T> &a, const empty_base<T> &b) noexcept(noexcept(a.value() == b.value())) requires(requires { a.value() == b.value(); }) { return a.value() == b.value(); }
		template<typename T>
		[[nodiscard]] constexpr bool operator!=(const empty_base<T> &a, const empty_base<T> &b) noexcept(noexcept(a.value() != b.value())) requires(requires { a.value() != b.value(); }) { return a.value() != b.value(); }
		template<typename T>
		[[nodiscard]] constexpr bool operator<=(const empty_base<T> &a, const empty_base<T> &b) noexcept(noexcept(a.value() <= b.value())) requires(requires { a.value() <= b.value(); }) { return a.value() <= b.value(); }
		template<typename T>
		[[nodiscard]] constexpr bool operator>=(const empty_base<T> &a, const empty_base<T> &b) noexcept(noexcept(a.value() >= b.value())) requires(requires { a.value() >= b.value(); }) { return a.value() >= b.value(); }
		template<typename T>
		[[nodiscard]] constexpr bool operator<(const empty_base<T> &a, const empty_base<T> &b) noexcept(noexcept(a.value() < b.value())) requires(requires { a.value() < b.value(); }) { return a.value() < b.value(); }
		template<typename T>
		[[nodiscard]] constexpr bool operator>(const empty_base<T> &a, const empty_base<T> &b) noexcept(noexcept(a.value() > b.value())) requires(requires { a.value() > b.value(); }) { return a.value() > b.value(); }
	}

	/** Utility type used to store a potentially-empty object using EBO that can be used as an alternative
	 * to the `[[no_unique_address]]` attribute for cases where it is not supported or has no effect. */
	template<typename T>
	using empty_base = _detail::empty_base<T>;

	namespace _detail
	{
		[[nodiscard]] inline static std::error_code current_error() noexcept
		{
			try { std::rethrow_exception(std::current_exception()); }
			catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
			catch (const std::system_error &e) { return e.code(); }
		}

		[[noreturn]] inline void throw_error_code(const std::error_code &err, const std::string &msg = {})
		{
			if (err != std::make_error_condition(std::errc::not_enough_memory))
				ROD_THROW(std::system_error(err, msg));
			else
				ROD_THROW(std::bad_alloc());
		}
		[[noreturn]] inline void throw_error_code(std::io_errc err, const std::string &msg = {})
		{
			throw_error_code(std::make_error_code(err), msg);
		}
		[[noreturn]] inline void throw_error_code(std::errc err, const std::string &msg = {})
		{
			throw_error_code(std::make_error_code(err), msg);
		}

		template<typename Err> requires(!requires (Err e) { e.throw_exception(); } && std::derived_from<std::decay_t<Err>, std::exception>)
		[[noreturn]] inline constexpr void throw_exception(Err &&error)
		{
			ROD_THROW(std::forward<Err>(error));
		}
		template<typename Err> requires(requires (Err e) { e.throw_exception(); })
		[[noreturn]] inline constexpr void throw_exception(Err &&error)
		{
			error.throw_exception();
		}
		template<typename Err> requires decays_to_same<Err, std::exception_ptr>
		[[noreturn]] inline constexpr void throw_exception(Err &&error)
		{
			std::rethrow_exception(std::forward<Err>(error));
		}
		template<typename Err> requires decays_to_same<Err, std::error_code>
		[[noreturn]] inline constexpr void throw_exception(Err &&error)
		{
			throw_error_code(error);
		}
	}

	/** Throws an exception from \a error either using an ADL or member overload or an implementation-defined method. */
	template<typename Err>
	[[noreturn]] inline constexpr void throw_exception(Err &&error) requires(requires { _detail::throw_exception(std::forward<Err>(error)); }) { _detail::throw_exception(std::forward<Err>(error)); }

	namespace _detail
	{
		template<typename...>
		struct empty_variant {};

		inline auto deduce_variant_or_empty(type_list_t<>) -> empty_variant<>;
		template<typename... Ts>
		inline auto deduce_variant_or_empty(type_list_t<Ts...>) -> std::variant<Ts...>;
		template<typename... Ts>
		using variant_or_empty = decltype(deduce_variant_or_empty(unique_tuple_t<type_list_t<std::decay_t<Ts>...>>{}));

		template<typename Err>
		[[nodiscard]] inline static std::exception_ptr to_except_ptr(Err &&error) noexcept
		{
			if constexpr (decays_to_same<Err, std::exception_ptr>)
				return std::forward<Err>(error);
			else if constexpr (std::derived_from<std::decay_t<Err>, std::exception>)
				return std::make_exception_ptr(std::forward<Err>(error));
			else if constexpr (decays_to_same<Err, std::error_code> || std::constructible_from<std::error_code, Err>)
				return std::make_exception_ptr(std::system_error(std::error_code(error)));
			else if constexpr (requires { throw_exception(std::forward<Err>(error)); })
				try { throw_exception(std::forward<Err>(error)); } catch (...) { return std::current_exception(); }
			else
				return std::make_exception_ptr(std::forward<Err>(error));
		}

		template<typename I, typename S, typename C = std::decay_t<std::iter_value_t<I>>>
		[[nodiscard]] constexpr std::size_t generic_strlen(I begin, S end) noexcept
		{
			std::size_t i = 0;
			while (begin++ != end && *begin != C{})
				i += 1;
			return i;
		}
		template<typename I, typename S, typename C = std::decay_t<std::iter_value_t<I>>>
		[[nodiscard]] constexpr std::size_t strlen(I begin, S end) noexcept
		{
#if defined(ROD_WIN32) || defined(_GNU_SOURCE) || _POSIX_C_SOURCE >= 200809L
			if constexpr (std::is_pointer_v<I>)
				if (!std::is_constant_evaluated())
				{
					if constexpr (std::same_as<C, char>)
						return strnlen(begin, static_cast<std::size_t>(end - begin));
					if constexpr (std::same_as<C, wchar_t>)
						return wcsnlen(begin, static_cast<std::size_t>(end - begin));
				}
#endif
			return generic_strlen(begin, end);
		}

		template<typename I, typename C = std::decay_t<std::iter_value_t<I>>>
		[[nodiscard]] constexpr std::size_t generic_strlen(I first) noexcept
		{
			std::size_t i = 0;
			while (*first++ != C{})
				i += 1;
			return i;
		}
		template<typename I, typename C = std::decay_t<std::iter_value_t<I>>>
		[[nodiscard]] constexpr std::size_t strlen(I first) noexcept
		{
			if constexpr (std::is_pointer_v<I>)
				if (!std::is_constant_evaluated())
				{
					if constexpr (std::same_as<C, char>)
						return std::strlen(first);
					if constexpr (std::same_as<C, wchar_t>)
						return std::wcslen(first);
				}
			return generic_strlen(first);
		}

		template<typename F>
		class defer_guard : empty_base<F>
		{
		public:
			defer_guard() = delete;
			defer_guard(const defer_guard &) = delete;

			constexpr defer_guard(defer_guard &&other) noexcept(std::is_nothrow_move_constructible_v<F>) : empty_base<F>(std::forward<empty_base<F>>(other)), _invoke(std::exchange(other._invoke, {})) {}
			constexpr defer_guard &operator=(defer_guard &&other) noexcept(std::is_nothrow_move_assignable_v<F>) { return (empty_base<F>::operator=(std::forward<empty_base<F>>(other)), std::swap(_invoke, other._invoke), *this); }

			template<typename F2>
			constexpr explicit defer_guard(F2 &&func) noexcept(std::is_nothrow_constructible_v<F, F2>) : empty_base<F>(std::forward<F2>(func)), _invoke(true) {}
			constexpr ~defer_guard() noexcept(nothrow_callable<F> && std::is_nothrow_destructible_v<F>) { if (std::exchange(_invoke, {})) std::invoke(empty_base<F>::value()); }

			constexpr void release() noexcept { _invoke = false; }

		private:
			bool _invoke;
		};

		template<typename F>
		struct eval_t : empty_base<F>
		{
			using type = std::invoke_result_t<F>;

			using empty_base<F>::empty_base;
			using empty_base<F>::operator=;

			operator type() && noexcept(requires (F f) { { f() } noexcept; }) { return std::move(empty_base<F>::value())(); }
			type operator()() && noexcept(requires (F f) { { f() } noexcept; }) { return std::move(empty_base<F>::value())(); }
		};
		template<typename F>
		eval_t(F) -> eval_t<F>;
	}

	/** Invokes functor \a func on destruction of the returned guard. */
	template<typename Func>
	[[nodiscard]] inline static auto defer_invoke(Func &&func) noexcept(std::is_nothrow_constructible_v<std::decay_t<Func>, Func>) { return _detail::defer_guard<std::decay_t<Func>>(std::forward<Func>(func)); }

	/** Utility function used to preform an implicit-only cast of \a value to type \a T. */
	template<typename T>
	inline constexpr T implicit_cast(std::type_identity_t<T> value) noexcept(std::is_nothrow_convertible_v<std::type_identity_t<T>, T>) { return value; }

	/** Utility function used to preform a copy of an input range into an output range via `const_cast`. */
	template<std::forward_iterator In, std::sentinel_for<In> S, std::forward_iterator Out, typename From = std::iter_value_t<In>, typename To = std::iter_value_t<Out>>
	inline constexpr Out const_cast_copy(In first, S last, Out out) noexcept(noexcept(++first) && noexcept(first != last) && noexcept(++out) && noexcept(*out = const_cast<To>(*first)))
	{
		for (; first != last; ++first, ++out)
			*out = const_cast<To>(*first);
		return out;
	}
	/** Utility function used to preform a copy of an input range into an output range via `static_cast`. */
	template<std::forward_iterator In, std::sentinel_for<In> S, std::forward_iterator Out, typename From = std::iter_value_t<In>, typename To = std::iter_value_t<Out>>
	inline constexpr Out static_cast_copy(In first, S last, Out out) noexcept(noexcept(++first) && noexcept(first != last) && noexcept(++out) && noexcept(*out = static_cast<To>(*first)))
	{
		for (; first != last; ++first, ++out)
			*out = static_cast<To>(*first);
		return out;
	}
	/** Utility function used to preform a copy of an input range into an output range via `implicit_cast`. */
	template<std::forward_iterator In, std::sentinel_for<In> S, std::forward_iterator Out, typename From = std::iter_value_t<In>, typename To = std::iter_value_t<Out>>
	inline constexpr Out implicit_cast_copy(In first, S last, Out out) noexcept(noexcept(++first) && noexcept(first != last) && noexcept(++out) && noexcept(*out = implicit_cast<To>(*first)))
	{
		for (; first != last; ++first, ++out)
			*out = implicit_cast<To>(*first);
		return out;
	}
	/** Utility function used to preform a copy of an input range into an output range via `reinterpret_cast`. */
	template<std::forward_iterator In, std::sentinel_for<In> S, std::forward_iterator Out, typename From = std::iter_value_t<In>, typename To = std::iter_value_t<Out>>
	inline static Out reinterpret_cast_copy(In first, S last, Out out) noexcept(noexcept(++first) && noexcept(first != last) && noexcept(++out) && noexcept(*out = reinterpret_cast<To>(*first)))
	{
		for (; first != last; ++first, ++out)
			*out = reinterpret_cast<To>(*first);
		return out;
	}

	/** Generates up to \a max bytes into \a dst buffer using a platform-specific cryptographic source such as `/dev/urandom` or `BCrypt`.
	 * @param dst Destination buffer to write the random-generated bytes into.
	 * @param max Maximum size of the destination buffer.
	 * @return Total amount of bytes written to \a dst, or `0` on failure to generate random numbers. */
	ROD_API_PUBLIC std::size_t system_random(void *buff, std::size_t max) noexcept;

	/** Returns the minimum user-available page size of the current system. */
	[[nodiscard]] ROD_API_PUBLIC std::size_t get_page_size() noexcept;
	/** Returns a span of current system's page sizes. If \a avail is set to `true`, returns only the page sizes available to the user. */
	[[nodiscard]] ROD_API_PUBLIC std::span<const std::size_t> get_page_sizes(bool avail = true) noexcept;

	namespace _detail
	{
		struct free_deleter { void operator()(auto *p) const noexcept { std::free(p); } };
	}

	/** Specialization of `std::unique_ptr` for use with `malloc` & `free`. */
	template<typename T>
	using malloc_ptr = std::unique_ptr<T, _detail::free_deleter>;

	/** Factory function used to allocate a `malloc_ptr` for a single default-initialized object of type \a T.
	 * @return `malloc_ptr&lt;T&gt;` pointing to the allocated object, or an empty `malloc_ptr` on allocation failure. */
	template<typename T> requires std::same_as<std::remove_extent_t<T>, T>
	[[nodiscard]] inline static malloc_ptr<T> make_malloc_ptr_for_overwrite() noexcept { return malloc_ptr<T>(static_cast<T *>(std::malloc(sizeof(T)))); }
	/** Factory function used to allocate a `malloc_ptr` for an array of default-initialized objects of type \a T.
	 * @param size Size of the allocated array.
	 * @return `malloc_ptr&lt;T&gt;` pointing to the allocated array, or an empty `malloc_ptr` on allocation failure. */
	template<typename T, typename U = std::remove_extent_t<T>> requires(!std::same_as<U, T>)
	[[nodiscard]] inline static malloc_ptr<T> make_malloc_ptr_for_overwrite(std::size_t size) noexcept { return malloc_ptr<T>(static_cast<U *>(std::malloc(sizeof(U) * size))); }

	/** Factory function used to allocate a `malloc_ptr` for a single object of type \a T constructed from arguments \a args.
	 * @param args Arguments passed to the constructor of \a T.
	 * @return `malloc_ptr&lt;T&gt;` pointing to the allocated object, or an empty `malloc_ptr` on allocation failure. */
	template<typename T, typename... Args> requires std::same_as<std::remove_extent_t<T>, T> && std::constructible_from<T, Args...>
	[[nodiscard]] inline static malloc_ptr<T> make_malloc_ptr(Args &&...args) noexcept
	{
		auto res = make_malloc_ptr_for_overwrite<T>();
		if (res.get() != nullptr) [[likely]]
			new (res.get()) T(std::forward<Args>(args)...);
		return res;
	}
	/** Factory function used to allocate a `malloc_ptr` for an array of default-constructed objects of type \a T.
	 * @param size Size of the allocated array.
	 * @return `malloc_ptr&lt;T&gt;` pointing to the allocated array, or an empty `malloc_ptr` on allocation failure. */
	template<typename T, typename U = std::remove_extent_t<T>> requires(!std::same_as<U, T> && std::constructible_from<U>)
	[[nodiscard]] inline static malloc_ptr<T> make_malloc_ptr(std::size_t size) noexcept
	{
		auto res = make_malloc_ptr_for_overwrite<T>(size);
		if (res.get() != nullptr) [[likely]]
		{
			for (std::size_t i = 0; i < size; ++i)
				new(res.get() + i) U();
		}
		return res;
	}
}

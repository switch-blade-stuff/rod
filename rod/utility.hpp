/*
 * Created by switchblade on 2023-04-09.
 */

#pragma once

#include <system_error>
#include <exception>
#include <variant>
#include <array>

#include <cstring>
#include <cwchar>

#include "tag.hpp"

namespace rod
{
	/** Concept used to check if type \a T is a movable value type. */
	template<typename T>
	concept movable_value = std::move_constructible<std::decay_t<T>> && std::constructible_from<std::decay_t<T>, T>;

	/** Concept used to check if type \a T is a reference to a type derived from \a U. */
	template<typename T, typename U>
	concept derived_reference = std::derived_from<std::remove_reference_t<T>, U>;
	/** Concept used to check if type \a T decays to a type derived from \a U. */
	template<typename T, typename U>
	concept decays_to_derived = std::derived_from<std::decay_t<T>, U>;

	/** Concept used to check if type \a T is a reference of type \a U. */
	template<typename T, typename U>
	concept reference_to = std::same_as<std::remove_reference_t<T>, U>;
	/** Concept used to check if type \a From decays into type \a To. */
	template<typename From, typename To>
	concept decays_to = std::same_as<std::decay_t<From>, To>;

	/** Concept used to check if type \a T matches one of the types in \a Ts. */
	template<typename T, typename... Ts>
	concept one_of = (std::same_as<T, Ts> || ...);

	namespace _detail
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
	struct type_list_t {};
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
		struct adl_swappable : std::false_type {};
		template<typename T0, typename T1>
		struct nothrow_adl_swappable : std::false_type {};

		template<typename T0, typename T1>
		inline constexpr bool test_member_swappable(T0 &&a, T1 &&b) noexcept
		{
			return requires { std::forward<T0>(a).swap(std::forward<T1>(b)); };
		}
		template<typename T0, typename T1>
		inline constexpr bool test_static_swappable(T0 &&a, T1 &&b) noexcept
		{
			using std::swap;
			return requires { swap(std::forward<T0>(a), std::forward<T1>(b)); };
		}

		template<typename T0, typename T1>
		using member_swappable = std::bool_constant<test_member_swappable(std::declval<T0>(), std::declval<T1>())>;
		template<typename T0, typename T1>
		using static_swappable = std::bool_constant<test_static_swappable(std::declval<T0>(), std::declval<T1>())>;

		template<typename T0, typename T1> requires member_swappable<T0, T1>::value
		struct adl_swappable<T0, T1> : std::true_type {};
		template<typename T0, typename T1> requires static_swappable<T0, T1>::value
		struct adl_swappable<T0, T1> : std::true_type {};

		template<typename T0, typename T1>
		inline constexpr bool test_nothrow_member_swappable(T0 &&a, T1 &&b) noexcept
		{
			return noexcept(std::forward<T0>(a).swap(std::forward<T1>(b)));
		}
		template<typename T0, typename T1>
		inline constexpr bool test_nothrow_static_swappable(T0 &&a, T1 &&b) noexcept
		{
			using std::swap;
			return noexcept(swap(std::forward<T0>(a), std::forward<T1>(b)));
		}

		template<typename T0, typename T1>
		using nothrow_member_swappable = std::bool_constant<test_nothrow_member_swappable(std::declval<T0>(), std::declval<T1>())>;
		template<typename T0, typename T1>
		using nothrow_static_swappable = std::bool_constant<test_nothrow_static_swappable(std::declval<T0>(), std::declval<T1>())>;

		template<typename T0, typename T1> requires member_swappable<T0, T1>::value
		struct nothrow_adl_swappable<T0, T1> : nothrow_member_swappable<T0, T1> {};
		template<typename T0, typename T1> requires static_swappable<T0, T1>::value
		struct nothrow_adl_swappable<T0, T1> : nothrow_static_swappable<T0, T1> {};
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

			constexpr empty_base(const empty_base &other) noexcept(std::is_nothrow_copy_constructible_v<T>) requires std::copy_constructible<T> : _value(other._value) {}
			constexpr empty_base &operator=(const empty_base &other) noexcept(std::is_nothrow_copy_assignable_v<T>) requires std::assignable_from<T, const T &> { return (_value = other._value, *this); }

			constexpr empty_base(empty_base &&other) noexcept(std::is_nothrow_move_constructible_v<T>) requires std::move_constructible<T> : _value(std::move(other._value)) {}
			constexpr empty_base &operator=(empty_base &&other) noexcept(std::is_nothrow_move_assignable_v<T>) requires std::assignable_from<T, T &&> { return (_value = std::move(other._value), *this); }

			template<typename... Args> requires std::constructible_from<T, Args...>
			constexpr explicit empty_base(Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>) : _value(std::forward<Args>(args)...) {}
			template<typename U = T> requires(!decays_to<U, empty_base> && std::assignable_from<T, U>)
			constexpr empty_base &operator=(U &&value) noexcept(std::is_nothrow_assignable_v<T, U>) { return (_value = std::forward<U>(value), *this); }

			/** Returns pointer to the contained value. */
			[[nodiscard]] constexpr T *get() noexcept { return std::addressof(_value); }
			/** @copydoc get */
			[[nodiscard]] constexpr const T *get() const noexcept { return std::addressof(_value); }

			/** Returns reference to the contained value. */
			[[nodiscard]] constexpr auto &value() & noexcept { return _value; }
			/** @copydoc value */
			[[nodiscard]] constexpr auto &value() const & noexcept { return _value; }
			/** @copydoc value */
			[[nodiscard]] constexpr auto &&value() && noexcept { return std::move(_value); }
			/** @copydoc value */
			[[nodiscard]] constexpr auto &&value() const && noexcept { return std::move(_value); }

			constexpr void swap(empty_base &other) noexcept(std::is_nothrow_swappable_v<T>) requires std::swappable<T> { adl_swap(_value, other._value); }

		private:
			T _value;
		};
		template<typename T> requires(std::is_object_v<T> && std::is_empty_v<T> && !std::is_final_v<T>)
		class empty_base<T> : T
		{
		public:
			constexpr empty_base() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
			constexpr empty_base(const empty_base &) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;
			constexpr empty_base &operator=(const empty_base &) noexcept(std::is_nothrow_copy_assignable_v<T>) = default;
			constexpr empty_base(empty_base &&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
			constexpr empty_base &operator=(empty_base &&) noexcept(std::is_nothrow_move_assignable_v<T>) = default;

			template<typename... Args> requires std::constructible_from<T, Args...>
			constexpr explicit empty_base(Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>) : T(std::forward<Args>(args)...) {}
			template<typename U = T> requires(!decays_to<U, empty_base> && std::assignable_from<T, U>)
			constexpr empty_base &operator=(U &&value) noexcept(std::is_nothrow_assignable_v<T, U>) { return (*get() = std::forward<U>(value), *this); }

			/** Returns pointer to the contained value. */
			[[nodiscard]] constexpr T *get() noexcept { return std::addressof(static_cast<T &>(*this)); }
			/** @copydoc get */
			[[nodiscard]] constexpr const T *get() const noexcept { return std::addressof(static_cast<const T &>(*this)); }

			/** Returns reference to the contained value. */
			[[nodiscard]] constexpr auto &value() & noexcept { return static_cast<T &>(*this); }
			/** @copydoc value */
			[[nodiscard]] constexpr auto &value() const & noexcept { return static_cast<const T &>(*this); }
			/** @copydoc value */
			[[nodiscard]] constexpr auto &&value() && noexcept { return static_cast<T &&>(*this); }
			/** @copydoc value */
			[[nodiscard]] constexpr auto &&value() const && noexcept { return static_cast<const T &&>(*this); }

			constexpr void swap(empty_base &other) noexcept(std::is_nothrow_swappable_v<T>) requires std::swappable<T> { adl_swap(*get(), other.*get()); }
		};

		template<typename T>
		constexpr void swap(empty_base<T> &a, empty_base<T> &b) noexcept(std::is_nothrow_swappable_v<T>) requires std::swappable<T> { a.swap(b); }

		template<typename T>
		[[nodiscard]] constexpr auto operator<=>(const empty_base<T> &a, const empty_base<T> &b) noexcept(noexcept(*a.get() <=> *b.get())) requires(requires { *a.get() <=> *b.get(); }) { return *a.get() <=> *b.get(); }
		template<typename T>
		[[nodiscard]] constexpr bool operator==(const empty_base<T> &a, const empty_base<T> &b) noexcept(noexcept(*a.get() == *b.get())) requires(requires { *a.get() == *b.get(); }) { return *a.get() == *b.get(); }
		template<typename T>
		[[nodiscard]] constexpr bool operator!=(const empty_base<T> &a, const empty_base<T> &b) noexcept(noexcept(*a.get() != *b.get())) requires(requires { *a.get() != *b.get(); }) { return *a.get() != *b.get(); }
		template<typename T>
		[[nodiscard]] constexpr bool operator<=(const empty_base<T> &a, const empty_base<T> &b) noexcept(noexcept(*a.get() <= *b.get())) requires(requires { *a.get() <= *b.get(); }) { return *a.get() <= *b.get(); }
		template<typename T>
		[[nodiscard]] constexpr bool operator>=(const empty_base<T> &a, const empty_base<T> &b) noexcept(noexcept(*a.get() >= *b.get())) requires(requires { *a.get() >= *b.get(); }) { return *a.get() >= *b.get(); }
		template<typename T>
		[[nodiscard]] constexpr bool operator<(const empty_base<T> &a, const empty_base<T> &b) noexcept(noexcept(*a.get() < *b.get())) requires(requires { *a.get() < *b.get(); }) { return *a.get() < *b.get(); }
		template<typename T>
		[[nodiscard]] constexpr bool operator>(const empty_base<T> &a, const empty_base<T> &b) noexcept(noexcept(*a.get() > *b.get())) requires(requires { *a.get() > *b.get(); }) { return *a.get() > *b.get(); }
	}

	/** Utility type used to store a potentially-empty object using EBO that can be used as an alternative
	 * to the `[[no_unique_address]]` attribute for cases where it is not supported or has no effect. */
	template<typename T>
	using empty_base = _detail::empty_base<T>;

	namespace _detail
	{
		template<typename...>
		struct empty_variant {};

		inline auto deduce_variant_or_empty(type_list_t<>) -> empty_variant<>;
		template<typename... Ts>
		inline auto deduce_variant_or_empty(type_list_t<Ts...>) -> std::variant<Ts...>;
		template<typename... Ts>
		using variant_or_empty = decltype(deduce_variant_or_empty(unique_tuple_t<type_list_t<std::decay_t<Ts>...>>{}));
	}

	/** Utility function used to throw `std::system_error` if the passed error evaluated to `true`.
	 * @param[in] err Error to assert the value of. */
	static void assert_error_code(std::error_code err) { if (err) throw std::system_error(err); }
	/** @copydoc assert_error_code
	 * @param[in] msg Message passed to `std::system_error`. */
	static void assert_error_code(std::error_code err, const char *msg) { if (err) throw std::system_error(err, msg); }
	/** @copydoc assert_error_code */
	static void assert_error_code(std::error_code err, const std::string &msg) { if (err) throw std::system_error(err, msg); }

	namespace _detail
	{
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

		template<typename I, typename S, typename C = std::decay_t<std::iter_value_t<I>>>
		[[nodiscard]] constexpr std::size_t generic_strlen(I first, S last) noexcept
		{
			std::size_t i = 0;
			while (first++ != last && *first != C{})
				i += 1;
			return i;
		}
		template<typename I, typename S, typename C = std::decay_t<std::iter_value_t<I>>>
		[[nodiscard]] constexpr std::size_t strlen(I first, S last) noexcept
		{
#if defined(ROD_WIN32) || defined(_GNU_SOURCE) || _POSIX_C_SOURCE >= 200809L
			if constexpr (std::is_pointer_v<I>)
				if (!std::is_constant_evaluated())
				{
					if constexpr (std::same_as<C, char>)
						return strnlen(first, static_cast<std::size_t>(last - first));
					if constexpr (std::same_as<C, wchar_t>)
						return wcsnlen(first, static_cast<std::size_t>(last - first));
				}
#endif
			return generic_strlen(first, last);
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
		class defer_invoker : empty_base<F>
		{
		public:
			defer_invoker() = delete;
			defer_invoker(const defer_invoker &) = delete;
			defer_invoker &operator=(const defer_invoker &) = delete;
			defer_invoker(defer_invoker &&) = delete;
			defer_invoker &operator=(defer_invoker &&) = delete;

			template<typename F2>
			constexpr explicit defer_invoker(F2 &&func) noexcept(std::is_nothrow_constructible_v<F, F2>) : empty_base<F>(std::forward<F2>(func)) {}
			constexpr ~defer_invoker() noexcept(nothrow_callable<F>) { std::invoke(empty_base<F>::value()); }
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

	/** Invokes functor \a func on destruction of the returned handle. */
	template<typename Func>
	[[nodiscard]] static auto defer_invoke(Func &&func) noexcept(std::is_nothrow_constructible_v<_detail::defer_invoker<std::decay_t<Func>>, Func>)
	{
		return _detail::defer_invoker<std::decay_t<Func>>{std::forward<Func>(func)};
	}

	/** Utility function used to preform a copy of an input range into an output range via `const_cast`. */
	template<std::forward_iterator In, std::sentinel_for<In> S, std::forward_iterator Out, typename From = std::iter_value_t<In>, typename To = std::iter_value_t<Out>>
	inline constexpr Out const_cast_copy(In first, S last, Out out) noexcept(noexcept(++first) && noexcept(first != last) && noexcept(++out) && noexcept(*out = const_cast<To>(*first)))
	{
		using to_type = std::iter_value_t<Out>;
		for (; first != last; ++first, ++out)
			*out = const_cast<To>(*first);
		return out;
	}
	/** Utility function used to preform a copy of an input range into an output range via `static_cast`. */
	template<std::forward_iterator In, std::sentinel_for<In> S, std::forward_iterator Out, typename From = std::iter_value_t<In>, typename To = std::iter_value_t<Out>>
	inline constexpr Out static_cast_copy(In first, S last, Out out) noexcept(noexcept(++first) && noexcept(first != last) && noexcept(++out) && noexcept(*out = static_cast<To>(*first)))
	{
		using to_type = std::iter_value_t<Out>;
		for (; first != last; ++first, ++out)
			*out = static_cast<To>(*first);
		return out;
	}
	/** Utility function used to preform a copy of an input range into an output range via `reinterpret_cast`. */
	template<std::forward_iterator In, std::sentinel_for<In> S, std::forward_iterator Out, typename From = std::iter_value_t<In>, typename To = std::iter_value_t<Out>>
	inline static Out reinterpret_cast_copy(In first, S last, Out out) noexcept(noexcept(++first) && noexcept(first != last) && noexcept(++out) && noexcept(*out = reinterpret_cast<To>(*first)))
	{
		using to_type = std::iter_value_t<Out>;
		for (; first != last; ++first, ++out)
			*out = reinterpret_cast<To>(*first);
		return out;
	}

	/** Utility function used to mark a portion of code as unreachable. */
	[[noreturn]] inline void unreachable() noexcept
	{
#if defined(__GNUC__)
		__builtin_unreachable();
#elif defined(_MSC_VER)
		__assume(false);
#elif !defined(NDEBUG)
		std::terminate();
#endif
	}
}

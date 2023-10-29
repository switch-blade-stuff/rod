/*
 * Created by switchblade on 2023-10-28.
 */

#pragma once

#include "delegate.hpp"

namespace rod
{
	namespace _detail
	{
		template<typename, typename...>
		struct select_property_result;

		template<typename Func, typename... Args> requires std::is_invocable_v<Func, Args...>
		struct select_property_result<Func, Args...> { using type = std::invoke_result_t<Func, Args...>; };
		template<typename Func, typename... Args>
		struct select_property_result { using type = void; };
	}

	/** Proxy object used to bind getter and setter functors with optional arguments. */
	template<typename GetFunc, typename SetFunc, typename... BindArgs>
	class property : std::tuple<BindArgs...>, empty_base<GetFunc>, empty_base<SetFunc>
	{
		template<typename, typename, typename...>
		friend class property;

		using args_base = std::tuple<BindArgs...>;
		using get_base = empty_base<GetFunc>;
		using set_base = empty_base<SetFunc>;

		template<typename P, typename... Ts>
		using is_get_invocable = std::is_invocable<copy_cvref_t<P, GetFunc>, copy_cvref_t<P, BindArgs>..., Ts...>;
		template<typename P, typename... Ts>
		using is_set_invocable = std::is_invocable<copy_cvref_t<P, SetFunc>, copy_cvref_t<P, BindArgs>..., Ts...>;

		template<typename P, typename... Ts>
		using is_nothrow_get_invocable = std::is_nothrow_invocable<copy_cvref_t<P, GetFunc>, copy_cvref_t<P, BindArgs>..., Ts...>;
		template<typename P, typename... Ts>
		using is_nothrow_set_invocable = std::is_nothrow_invocable<copy_cvref_t<P, SetFunc>, copy_cvref_t<P, BindArgs>..., Ts...>;

		template<typename P, typename... Ts>
		using get_result = typename _detail::select_property_result<copy_cvref_t<P, GetFunc>, copy_cvref_t<P, BindArgs>..., Ts...>::type;
		template<typename P, typename... Ts>
		using set_result = typename _detail::select_property_result<copy_cvref_t<P, SetFunc>, copy_cvref_t<P, BindArgs>..., Ts...>::type;

		template<typename GetFunc2, typename SetFunc2, typename... BindArgs2>
		using is_constructible = std::conjunction<std::is_constructible<GetFunc, GetFunc2>, std::is_constructible<SetFunc, SetFunc2>, std::is_constructible<BindArgs, BindArgs2>...>;
		template<typename Other>
		using is_constructible_from = is_constructible<copy_cvref_t<Other, GetFunc>, copy_cvref_t<Other, SetFunc>, copy_cvref_t<Other, BindArgs>...>;

		template<typename GetFunc2, typename SetFunc2, typename... BindArgs2>
		using is_nothrow_constructible = std::conjunction<std::is_nothrow_constructible<GetFunc, GetFunc2>, std::is_nothrow_constructible<SetFunc, SetFunc2>, std::is_nothrow_constructible<BindArgs, BindArgs2>...>;
		template<typename Other>
		using is_nothrow_constructible_from = is_nothrow_constructible<copy_cvref_t<Other, GetFunc>, copy_cvref_t<Other, SetFunc>, copy_cvref_t<Other, BindArgs>...>;

		template<typename GetFunc2, typename SetFunc2, typename... BindArgs2>
		using is_swappable_with = std::conjunction<std::is_swappable_with<GetFunc, GetFunc2>, std::is_swappable_with<SetFunc, SetFunc2>, std::is_swappable_with<BindArgs, BindArgs2>...>;
		template<typename GetFunc2, typename SetFunc2, typename... BindArgs2>
		using is_nothrow_swappable_with = std::conjunction<std::is_nothrow_swappable_with<GetFunc, GetFunc2>, std::is_nothrow_swappable_with<SetFunc, SetFunc2>, std::is_nothrow_swappable_with<BindArgs, BindArgs2>...>;

	public:
		constexpr property() noexcept(std::is_nothrow_default_constructible_v<GetFunc> && std::is_nothrow_default_constructible_v<SetFunc> && std::is_nothrow_default_constructible_v<args_base>) = default;
		constexpr property(property &&) noexcept(std::is_nothrow_move_constructible_v<GetFunc> && std::is_nothrow_move_constructible_v<SetFunc> && std::is_nothrow_move_constructible_v<args_base>) = default;
		constexpr property(const property &) noexcept(std::is_nothrow_copy_constructible_v<GetFunc> && std::is_nothrow_copy_constructible_v<SetFunc> && std::is_nothrow_copy_constructible_v<args_base>) = default;

		constexpr property &operator=(property &&) noexcept(std::is_nothrow_move_assignable_v<GetFunc> && std::is_nothrow_move_assignable_v<SetFunc> && std::is_nothrow_move_assignable_v<args_base>) = default;
		constexpr property &operator=(const property &) noexcept(std::is_nothrow_copy_assignable_v<GetFunc> && std::is_nothrow_copy_assignable_v<SetFunc> && std::is_nothrow_copy_assignable_v<args_base>) = default;

		/** Initialzes property from functors \a get_func and \a set_func, and bound arguments \a args. */
		template<typename GetFunc2, typename SetFunc2, typename... BindArgs2> requires is_constructible<GetFunc2, SetFunc2, BindArgs2...>::value
		constexpr property(GetFunc2 &&get_func, SetFunc2 &&set_func, BindArgs2 &&...args) noexcept(is_nothrow_constructible<GetFunc2, SetFunc2, BindArgs2...>::value)
				: args_base(std::forward<BindArgs2>(args)...), get_base(std::forward<GetFunc2>(get_func)), set_base(std::forward<SetFunc2>(set_func)) {}

		/** Initialzes property from the functors and arguments of \a other. */
		template<typename GetFunc2, typename SetFunc2, typename... BindArgs2> requires(!decays_to_same<property<GetFunc2, SetFunc2, BindArgs2...>, property> && is_constructible<GetFunc2, SetFunc2, BindArgs2...>::value)
		constexpr property(property<GetFunc2, SetFunc2, BindArgs2...> &&other) noexcept(is_nothrow_constructible<GetFunc2, SetFunc2, BindArgs2...>::value)
				: args_base(std::forward<std::tuple<BindArgs2...> &&>(other)), get_base(std::forward<empty_base<GetFunc2> &&>(other).value()), set_base(std::forward<empty_base<SetFunc2> &&>(other).value()) {}
		/** Initialzes property from the functors and arguments of \a other. */
		template<typename GetFunc2, typename SetFunc2, typename... BindArgs2> requires(!decays_to_same<property<GetFunc2, SetFunc2, BindArgs2...>, property> && is_constructible<const GetFunc2 &, const SetFunc2 &, const BindArgs2 &...>::value)
		constexpr property(const property<GetFunc2, SetFunc2, BindArgs2...> &other) noexcept(is_nothrow_constructible<const GetFunc2 &, const SetFunc2 &, const BindArgs2 &...>::value)
				: args_base(std::forward<const std::tuple<BindArgs2...> &>(other)), get_base(std::forward<const empty_base<GetFunc2> &>(other).value()), set_base(std::forward<const empty_base<SetFunc2> &>(other).value()) {}

		/** Invokes the get functor with the bound arguments and \a args.
		 * @param args Arguments passed to the get functor after the bound arguments list.
		 * @return Value returned by the get functor. */
		template<typename... Args> requires is_get_invocable<property &, Args...>::value
		constexpr auto get(Args &&...args) noexcept(is_nothrow_get_invocable<property &, Args...>::value) -> get_result<property &, Args...>
		{
			return std::apply([&]<typename... Args2>(Args2 &&...args2) { return std::invoke(get_base::value(), std::forward<Args2>(args2)..., std::forward<Args>(args)...); }, static_cast<args_base &>(*this));
		}
		/** @copydoc get */
		template<typename... Args> requires is_get_invocable<const property &, Args...>::value
		constexpr auto get(Args &&...args) const noexcept(is_nothrow_get_invocable<const property &, Args...>::value) -> get_result<const property &, Args...>
		{
			return std::apply([&]<typename... Args2>(Args2 &&...args2) { return std::invoke(get_base::value(), std::forward<Args2>(args2)..., std::forward<Args>(args)...); }, static_cast<const args_base &>(*this));
		}

#if 1
		/** Invokes the get functor with the bound arguments.
		 * @return Value returned by the get functor. */
		template<typename T = get_result<property &>> requires(!decays_to_same<T, property> && is_get_invocable<property &, T>::value)
		constexpr T operator()() noexcept(is_nothrow_get_invocable<property &, T>::value) { return get(); }
		/** @copydoc operator() */
		template<typename T = get_result<const property &>> requires(!decays_to_same<T, property> && is_get_invocable<const property &>::value)
		constexpr T operator()() const noexcept(is_nothrow_get_invocable<const property &, T>::value) { return get(); }
#else
		/** Invokes the get functor with the bound arguments and converts it's result to \a T.
		 * @return Value returned by the get functor. */
		template<typename T = get_result<property &>> requires(!decays_to_instance_of<T, property> && is_get_invocable<property &>::value)
		constexpr explicit(!std::is_convertible_v<get_result<property &>, T>) operator T() noexcept(is_nothrow_get_invocable<property &>::value) { return static_cast<T>(get()); }
		/** @copydoc operator T */
		template<typename T = get_result<const property &>> requires(!decays_to_instance_of<T, property> && is_get_invocable<const property &>::value)
		constexpr explicit(!std::is_convertible_v<get_result<const property &>, T>) operator T() const noexcept(is_nothrow_get_invocable<const property &>::value) { return static_cast<T>(get()); }
#endif

		/** Invokes the set functor with the bound arguments and \a args.
		 * @param args Arguments passed to the set functor after the bound arguments list.
		 * @return Value returned by the set functor. */
		template<typename... Args> requires is_set_invocable<property &, Args...>::value
		constexpr auto set(Args &&...args) noexcept(is_nothrow_set_invocable<property &, Args...>::value) -> set_result<property &, Args...>
		{
			return std::apply([&]<typename... Args2>(Args2 &&...args2) { return std::invoke(set_base::value(), std::forward<Args2>(args2)..., std::forward<Args>(args)...); }, static_cast<args_base &>(*this));
		}
		/** @copydoc set */
		template<typename... Args> requires is_set_invocable<const property &, Args...>::value
		constexpr auto set(Args &&...args) const noexcept(is_nothrow_set_invocable<const property &, Args...>::value) -> set_result<const property &, Args...>
		{
			return std::apply([&]<typename... Args2>(Args2 &&...args2) { return std::invoke(set_base::value(), std::forward<Args2>(args2)..., std::forward<Args>(args)...); }, static_cast<const args_base &>(*this));
		}

#if 1
		/** Invokes the set functor with \a value.
		 * @param value Argument passed to the set functor after the bound arguments list.
		 * @return Value returned by the set functor. */
		 template<typename T, typename... Ts> requires(!decays_to_same<T, property> && is_set_invocable<property &, T>::value)
		constexpr auto operator()(T &&value) noexcept(is_nothrow_set_invocable<property &, T>::value) -> set_result<property &, T> { return set(std::forward<T>(value)); }
		/** @copydoc operator() */
		template<typename T> requires(!decays_to_same<T, property> && is_set_invocable<const property &, T>::value)
		constexpr auto operator()(T &&value) const noexcept(is_nothrow_set_invocable<const property &, T>::value) -> set_result<const property &, T> { return set(std::forward<T>(value)); }
#else
		/** Invokes the set functor with \a value.
		 * @param value Argument passed to the set functor after the bound arguments list.
		 * @return Value returned by the set functor. */
		template<typename T> requires(!decays_to_same<T, property> && is_set_invocable<property &, T>::value)
		constexpr auto operator=(T &&value) noexcept(is_nothrow_set_invocable<property &, T>::value) -> set_result<property &, T> { return set(std::forward<T>(value)); }
#endif

		/** Creates an inner property as a copy of `this` by binding \a args. */
		template<typename... Args> requires is_constructible_from<property &>::value && _detail::all_decay_copyable<Args...>::value
		constexpr property<GetFunc, SetFunc, BindArgs..., std::decay_t<Args>...> bind(Args &&...args) & noexcept(is_nothrow_constructible_from<property &>::value && _detail::all_nothrow_decay_copyable<Args...>::value)
		{
			return std::apply([&]<typename... Args2>(Args2 &&...args2) -> property<GetFunc, SetFunc, BindArgs..., std::decay_t<Args>...>
			{
				return {std::forward<get_base &>(*this).value(), std::forward<set_base &>(*this).value(), std::forward<Args2>(args2)..., std::forward<Args>(args)...};
			}, std::forward<args_base &>(*this));
		}
		/** @copydoc bind */
		template<typename... Args> requires is_constructible_from<const property &>::value && _detail::all_decay_copyable<Args...>::value
		constexpr property<GetFunc, SetFunc, BindArgs..., std::decay_t<Args>...> bind(Args &&...args) const & noexcept(is_nothrow_constructible_from<const property &>::value && _detail::all_nothrow_decay_copyable<Args...>::value)
		{
			return std::apply([&]<typename... Args2>(Args2 &&...args2) -> property<GetFunc, SetFunc, BindArgs..., std::decay_t<Args>...>
			{
				return {std::forward<const get_base &>(*this).value(), std::forward<const set_base &>(*this).value(), std::forward<Args2>(args2)..., std::forward<Args>(args)...};
			}, std::forward<const args_base &>(*this));
		}
		/** @copydoc bind */
		template<typename... Args> requires is_constructible_from<property &&>::value && _detail::all_decay_copyable<Args...>::value
		constexpr property<GetFunc, SetFunc, BindArgs..., std::decay_t<Args>...> bind(Args &&...args) && noexcept(is_nothrow_constructible_from<property &&>::value && _detail::all_nothrow_decay_copyable<Args...>::value)
		{
			return std::apply([&]<typename... Args2>(Args2 &&...args2) -> property<GetFunc, SetFunc, BindArgs..., std::decay_t<Args>...>
			{
				return {std::forward<get_base &&>(*this).value(), std::forward<set_base &&>(*this).value(), std::forward<Args2>(args2)..., std::forward<Args>(args)...};
			}, std::forward<args_base &&>(*this));
		}
		/** @copydoc bind */
		template<typename... Args> requires is_constructible_from<const property &&>::value && _detail::all_decay_copyable<Args...>::value
		constexpr property<GetFunc, SetFunc, BindArgs..., std::decay_t<Args>...> bind(Args &&...args) const && noexcept(is_nothrow_constructible_from<const property &&>::value && _detail::all_nothrow_decay_copyable<Args...>::value)
		{
			return std::apply([&]<typename... Args2>(Args2 &&...args2) -> property<GetFunc, SetFunc, BindArgs..., std::decay_t<Args>...>
			{
				return {std::forward<const get_base &&>(*this).value(), std::forward<const set_base &&>(*this).value(), std::forward<Args2>(args2)..., std::forward<Args>(args)...};
			}, std::forward<const args_base &&>(*this));
		}

		template<typename GetFunc2, typename SetFunc2, typename... BindArgs2> requires is_swappable_with<GetFunc2, SetFunc2, BindArgs2...>::value
		constexpr void swap(property<GetFunc2, SetFunc2, BindArgs2...> &other) noexcept(is_nothrow_swappable_with<GetFunc2, SetFunc2, BindArgs2...>::value)
		{
			args_base::swap(other);
			get_base::swap(other);
			set_base::swap(other);
		}
		template<typename GetFunc1, typename SetFunc1, typename... BindArgs1, typename GetFunc2, typename SetFunc2, typename... BindArgs2>
		friend constexpr void swap(property<GetFunc1, SetFunc1, BindArgs1...> &a, property<GetFunc2, SetFunc2, BindArgs2...> &b) noexcept(noexcept(a.swap(b))) requires(requires { a.swap(b); }) { a.swap(b); }
	};

	template<typename GetFunc, typename SetFunc, typename... BindArgs>
	property(GetFunc &&, SetFunc &&, BindArgs &&...) -> property<std::decay_t<GetFunc>, std::decay_t<SetFunc>, std::decay_t<BindArgs>...>;
}

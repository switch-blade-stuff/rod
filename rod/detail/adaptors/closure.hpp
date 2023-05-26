/*
 * Created by switchblade on 2023-04-18.
 */

#pragma once

#include "../../packed_pair.hpp"
#include "../concepts.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	namespace detail
	{
		template<typename>
		struct sender_adaptor_closure {};

		template<typename A>
		concept is_sender_adaptor = std::derived_from<std::decay_t<A>, sender_adaptor_closure<std::decay_t<A>>> && std::move_constructible<std::decay_t<A>> && std::constructible_from<std::decay_t<A>, A>;
		template<typename A, typename S>
		concept is_sender_adaptor_for = is_sender_adaptor<A> && callable<A, std::decay_t<S>> && sender<std::invoke_result_t<A, std::decay_t<S>>>;

		template<sender S, is_sender_adaptor_for<S> A>
		constexpr std::invoke_result_t<A, S> operator|(S &&snd, A &&adp) noexcept(nothrow_callable<A, S>) { return std::forward<A>(adp)(std::forward<S>(snd)); }

		template<typename T0, typename T1>
		class bind_compose : public sender_adaptor_closure<bind_compose<T0, T1>>, packed_pair<T0, T1>
		{
			using pair_base = packed_pair<T0, T1>;

			template<typename S>
			constexpr static auto nothrow_move_invoke = nothrow_callable<T0, S> && nothrow_callable<T1, std::invoke_result_t<T0, S>>;
			template<typename S>
			constexpr static auto nothrow_const_invoke = nothrow_callable<const T0 &, S> && nothrow_callable<const T1 &, std::invoke_result_t<const T0 &, S>>;

			using pair_base::first;
			using pair_base::second;

		public:
			using pair_base::packed_pair;
			using pair_base::operator=;
			using pair_base::swap;

			template<sender S> requires detail::callable<T0, S> && detail::callable<T1, std::invoke_result_t<T0, S>>
			constexpr std::invoke_result_t<T1, std::invoke_result_t<T0, S>> operator()(S &&snd) && noexcept(nothrow_move_invoke<S>)
			{
				return std::move(second)(std::move(first)(std::forward<S>(snd)));
			}
			template<sender S> requires detail::callable<const T0 &, S> && detail::callable<const T1 &, std::invoke_result_t<const T0 &, S>>
			constexpr std::invoke_result_t<const T1 &, std::invoke_result_t<const T0 &, S>> operator()(S &&snd) const & noexcept(nothrow_const_invoke<S>)
			{
				return second(first(std::forward<S>(snd)));
			}
		};
		template<typename F, typename... Args>
		class back_adaptor : public sender_adaptor_closure<back_adaptor<F, Args...>>, packed_pair<F, std::tuple<Args...>>
		{
			using pair_base = packed_pair<F, std::tuple<Args...>>;

			using pair_base::first;
			using pair_base::second;

		public:
			using pair_base::pair_base;
			using pair_base::operator=;
			using pair_base::swap;

			template<typename T, typename... Ts>
			constexpr back_adaptor(T &&f, std::tuple<Ts &&...> args) noexcept(std::is_nothrow_constructible_v<pair_base, T, std::tuple<Ts && ...>>) : pair_base(std::forward<T>(f), std::move(args)) {}

			template<typename S> requires callable<F, S, Args...>
			constexpr decltype(auto) operator()(S &&snd) && noexcept(nothrow_callable<F, S, Args...>)
			{
				return std::apply([&snd, this](Args &...as) -> decltype(auto) { return std::move(first)(std::forward<S>(snd), static_cast<Args &&>(as)...); }, second);
			}
			template<typename S> requires callable<const F &, S, const Args &...>
			constexpr decltype(auto) operator()(S &&snd) const & noexcept(nothrow_callable<const F &, S, const Args &...>)
			{
				return std::apply([&snd, this](const Args &...as) -> decltype(auto) { return first(std::forward<S>(snd), as...); }, second);
			}
		};

		template<is_sender_adaptor A0, is_sender_adaptor A1>
		constexpr bind_compose<std::decay_t<A0>, std::decay_t<A1>> operator|(A0 &&a, A1 &&b) noexcept(std::is_nothrow_constructible_v<bind_compose<std::decay_t<A0>, std::decay_t<A1>>, A0, A1>)
		{
			return {std::forward<A0>(a), std::forward<A1>(b)};
		}
	}

	using detail::sender_adaptor_closure;
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

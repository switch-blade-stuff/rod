/*
 * Created by switchblade on 2023-04-18.
 */

#pragma once

#include "../queries/completion.hpp"
#include "closure.hpp"

namespace rod
{
	namespace _into_variant
	{
		template<typename S, typename E> requires sender_in<S, E>
		using into_variant_type = value_types_of_t<S, E>;

		template<typename S, typename E>
		struct deduce_nothrow
		{
			template<typename... Args> requires std::constructible_from<detail::decayed_tuple<Args...>, Args...>
			using type = std::bool_constant<noexcept(into_variant_type<S, E>(detail::decayed_tuple<Args...>(std::declval<Args>()...)))>;
		};
		template<typename S, typename E>
		struct deduce_set_value
		{
			template<typename...>
			using type = set_value_t(into_variant_type<S, E>);
		};
		template<typename S, typename E>
		struct deduce_set_error
		{
			using has_throwing = is_in_tuple<std::true_type, value_types_of_t<S, E, deduce_nothrow, type_list_t>>;
			using type = std::conditional_t<has_throwing::value, completion_signatures<set_error_t(std::exception_ptr)>, completion_signatures<>>;
		};

		template<typename R, typename V>
		struct receiver { struct type; };
		template<typename S>
		struct sender { struct type; };

		template<typename R, typename V>
		struct receiver<R, V>::type
		{
			friend constexpr decltype(auto) tag_invoke(get_env_t, const type &r) noexcept(detail::nothrow_callable<get_env_t, const R &>) { return get_env(r._rcv); }

			template<typename... Vs> requires std::constructible_from<V, std::tuple<Vs &&...>>
			friend constexpr void tag_invoke(set_value_t, type &&r, Vs &&...vs) noexcept
			{
				try { set_value(std::move(r._rcv), V{std::tuple<Vs &&...>{std::forward<Vs>(vs)...}}); }
				catch (...) { set_error(std::move(r._rcv), std::current_exception()); }
			}
			template<typename Err>
			friend constexpr void tag_invoke(set_error_t, type &&r, Err &&err) noexcept { set_error(std::move(r._rcv), std::forward<Err>(err)); }
			friend constexpr void tag_invoke(set_stopped_t, type &&r) noexcept { set_stopped(std::move(r._rcv)); }

			[[ROD_NO_UNIQUE_ADDRESS]] R _rcv;
		};

		template<typename S>
		struct sender<S>::type
		{
			using is_sender = std::true_type;

			template<typename R>
			using _receiver_t = typename receiver<std::decay_t<R>, into_variant_type<S, std::decay_t<env_of_t<R>>>>::type;
			template<typename E>
			using _signs = make_completion_signatures<S, E, typename deduce_set_error<S, E>::type, deduce_set_value<S, E>::template type>;

			friend constexpr decltype(auto) tag_invoke(get_env_t, const type &s) noexcept(detail::nothrow_callable<get_env_t, const S &>) { return get_env(s._snd); }
			template<typename E>
			friend constexpr _signs<std::decay_t<E>> tag_invoke(get_completion_signatures_t, type &&, E &&) noexcept { return {}; }

			template<detail::decays_to<type> T, rod::receiver R> requires sender_to<S, _receiver_t<R>>
			friend constexpr decltype(auto) tag_invoke(connect_t, T &&s, R &&rcv) noexcept(detail::nothrow_callable<connect_t, S, _receiver_t<R>>)
			{
				return connect(std::move(s._snd), _receiver_t<R>{std::forward<R>(rcv)});
			}

			[[ROD_NO_UNIQUE_ADDRESS]] S _snd;
		};

		class into_variant_t
		{
			using back_adaptor = detail::back_adaptor<into_variant_t>;
			template<typename S>
			using sender_t = typename sender<std::decay_t<S>>::type;

		public:
			template<rod::sender S>
			[[nodiscard]] constexpr sender_t<S> operator()(S &&snd) const noexcept(std::is_nothrow_constructible_v<sender_t<S>, S>)
			{
				return sender_t<S>{std::forward<S>(snd)};
			}
			[[nodiscard]] constexpr back_adaptor operator()() const noexcept { return {}; }
		};
	}

	using _into_variant::into_variant_t;

	/** Sender adaptor used to convert value completion signatures of another sender into a variant of tuples.
	 * @param snd Sender to adapt completion signatures of. If omitted, creates a pipe-able sender adaptor.
	 * @return Sender completing via `set_value(std::variant<Ts0, ..., TsN>)` where `TsN` is a tuple made from the `N`th value completion signature of \a snd. */
	inline constexpr auto into_variant = into_variant_t{};

}

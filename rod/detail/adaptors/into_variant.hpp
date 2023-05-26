/*
 * Created by switchblade on 2023-04-18.
 */

#pragma once

#include "../queries/completion.hpp"
#include "../concepts.hpp"
#include "closure.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	namespace _into_variant
	{
		template<typename S, typename E> requires sender_in<S, E>
		using into_variant_type = value_types_of_t<S, E>;

		template<typename R, typename V>
		struct receiver { struct type; };
		template<typename S>
		struct sender { struct type; };

		template<typename R, typename V>
		struct receiver<R, V>::type
		{
			using is_receiver = std::true_type;

			friend constexpr env_of_t<R> tag_invoke(get_env_t, const type &r) noexcept(detail::nothrow_callable<get_env_t, const R &>) { return get_env(r._rcv); }

			template<typename... Vs> requires std::constructible_from<V, std::tuple<Vs &&...>>
			friend constexpr void tag_invoke(set_value_t, type &&r, Vs &&...vs) noexcept
			{
				try { set_value(std::move(r._rcv), V{std::tuple<Vs &&...>{std::forward<Vs>(vs)...}}); }
				catch (...) { set_error(std::move(r._rcv), std::current_exception()); }
			}
			template<typename Err>
			friend constexpr void tag_invoke(set_error_t, type &&r, Err &&err) noexcept
			{
				static_assert(detail::callable<set_error_t, R, Err>);
				set_error(std::move(r._rcv), std::forward<Err>(err));
			}
			friend constexpr void tag_invoke(set_stopped_t, type &&r) noexcept
			{
				static_assert(detail::callable<set_stopped_t, R>);
				set_stopped(std::move(r._rcv));
			}

			[[ROD_NO_UNIQUE_ADDRESS]] R _rcv;
		};

		template<typename S>
		struct sender<S>::type
		{
			using is_sender = std::true_type;

			template<typename R>
			using _receiver_t = typename receiver<R, into_variant_type<S, env_of_t<R>>>::type;

			template<typename E, typename...>
			using _value_signs_t = set_value_t(into_variant_type<S, E>);
			template<typename E>
			using _error_signs_t = std::conditional_t<value_types_of_t<S, E, detail::all_nothrow_decay_copyable, std::conjunction>::value, completion_signatures<>, completion_signatures<set_error_t(std::exception_ptr)>>;
			template<typename E>
			using _signs_t = make_completion_signatures<S, E, _error_signs_t<E>, detail::bind_front<_value_signs_t, E>::template type>;

			friend constexpr env_of_t<S> tag_invoke(get_env_t, const type &s) noexcept(detail::nothrow_callable<get_env_t, const S &>) { return get_env(s._snd); }
			template<typename E>
			friend constexpr _signs_t<std::decay_t<E>> tag_invoke(get_completion_signatures_t, type &&, E) noexcept { return {}; }

			template<detail::decays_to<type> T, rod::receiver Rcv> requires sender_to<S, _receiver_t<Rcv>>
			friend constexpr decltype(auto) tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(detail::nothrow_callable<connect_t, S, _receiver_t<Rcv>>)
			{
				return connect(std::move(s._snd), _receiver_t<Rcv>{std::move(rcv)});
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
	 * @return Sender who's value completion type is `std::variant<Ts0, ..., TsN>` where `TsN` is a tuple made from the `N`th value completion signature of \a snd. */
	inline constexpr auto into_variant = into_variant_t{};

}
ROD_TOPLEVEL_NAMESPACE_CLOSE

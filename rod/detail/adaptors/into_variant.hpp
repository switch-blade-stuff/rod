/*
 * Created by switchblade on 2023-04-18.
 */

#pragma once

#include "../queries/completion.hpp"
#include "../receiver_adaptor.hpp"
#include "closure.hpp"

namespace rod
{
	namespace _into_variant
	{
		template<typename S, typename E> requires sender_in<S, E>
		using into_variant_type = value_types_of_t<S, E>;

		template<typename, typename>
		struct receiver { class type; };
		template<typename>
		struct sender { class type; };

		template<typename Rcv, typename V>
		class receiver<Rcv, V>::type : public receiver_adaptor<type, Rcv>
		{
			friend receiver_adaptor<type, Rcv>;

		public:
			constexpr type(Rcv &&rcv) noexcept(std::is_nothrow_move_constructible_v<Rcv>) : receiver_adaptor<type, Rcv>(std::forward<Rcv>(rcv)) {}

		private:
			template<typename... Args> requires(!detail::all_nothrow_decay_copyable<Args...>::value && std::constructible_from<V, std::tuple<Args &&...>>)
			constexpr void set_value(Args &&...args) noexcept
			{
				try { set_value(std::move(receiver_adaptor<type, Rcv>::base()), V{std::tuple<Args &&...>{std::forward<Args>(args)...}}); }
				catch (...) { set_error(std::move(receiver_adaptor<type, Rcv>::base()), std::current_exception()); }
			}
			template<typename... Args> requires detail::all_nothrow_decay_copyable<Args...>::value && std::constructible_from<V, std::tuple<Args &&...>>
			constexpr void set_value(Args &&...args) noexcept
			{
				set_value(std::move(receiver_adaptor<type, Rcv>::base()), V{std::tuple<Args &&...>{std::forward<Args>(args)...}});
			}
		};

		template<typename Snd>
		class sender<Snd>::type
		{
		public:
			using is_sender = std::true_type;

		private:
			template<typename Rcv>
			using receiver_t = typename receiver<Rcv, into_variant_type<Snd, env_of_t<Rcv>>>::type;

			template<typename E, typename...>
			using value_signs_t = set_value_t(into_variant_type<Snd, E>);
			template<typename E>
			using error_signs_t = std::conditional_t<value_types_of_t<Snd, E, detail::all_nothrow_decay_copyable, std::conjunction>::value, completion_signatures<>, completion_signatures<set_error_t(std::exception_ptr)>>;
			template<typename E>
			using signs_t = make_completion_signatures<Snd, E, error_signs_t<E>, detail::bind_front<value_signs_t, E>::template type>;

		public:
			template<typename Snd2>
			constexpr type(Snd2 &&snd) noexcept(std::is_nothrow_constructible_v<Snd, Snd2>) : _snd(std::forward<Snd2>(snd)) {}

		public:
			friend constexpr env_of_t<Snd> tag_invoke(get_env_t, const type &s) noexcept(detail::nothrow_callable<get_env_t, const Snd &>) { return get_env(s._snd); }
			template<typename E>
			friend constexpr signs_t<std::decay_t<E>> tag_invoke(get_completion_signatures_t, type &&, E) noexcept { return {}; }

			template<decays_to<type> T, rod::receiver Rcv> requires sender_to<Snd, receiver_t<Rcv>>
			friend constexpr decltype(auto) tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(detail::nothrow_callable<connect_t, Snd, receiver_t<Rcv>> && std::is_nothrow_move_constructible_v<Rcv>)
			{
				return connect(std::move(s._snd), receiver_t<Rcv>{std::move(rcv)});
			}

		private:
			ROD_NO_UNIQUE_ADDRESS Snd _snd;
		};

		class into_variant_t
		{
			using back_adaptor = detail::back_adaptor<into_variant_t>;
			template<typename Snd>
			using sender_t = typename sender<std::decay_t<Snd>>::type;

		public:
			template<rod::sender Snd>
			[[nodiscard]] constexpr sender_t<Snd> operator()(Snd &&snd) const noexcept(std::is_nothrow_constructible_v<sender_t<Snd>, Snd>) { return sender_t<Snd>{std::forward<Snd>(snd)}; }
			[[nodiscard]] constexpr back_adaptor operator()() const noexcept { return {}; }
		};
	}

	using _into_variant::into_variant_t;

	/** Sender adaptor used to convert value completion signatures of another sender into a variant of tuples.
	 * @param snd Sender to adapt completion signatures of. If omitted, creates a pipe-able sender adaptor.
	 * @return Sender who's value completion type is `std::variant&lt;Ts0, ..., TsN&gt;` where `TsN` is a tuple made from the `N`th value completion signature of \a snd. */
	inline constexpr auto into_variant = into_variant_t{};

}

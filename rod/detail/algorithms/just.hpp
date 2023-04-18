/*
 * Created by switchblade on 2023-04-17.
 */

#pragma once

#include "../queries/signatures.hpp"
#include "connect.hpp"
#include "opstate.hpp"

namespace rod
{
	namespace _just
	{
		struct just_t;
		struct just_error_t;
		struct just_stopped_t;

		template<typename, typename, detail::movable_value...>
		struct operation { class type; };
		template<typename, detail::movable_value...>
		struct sender { class type; };

		template<typename R, typename C, detail::movable_value... Ts>
		class operation<R, C, Ts...>::type : std::tuple<Ts...>, detail::ebo_helper<R>
		{
			friend class sender<C, Ts...>::type;

			template<typename T0, typename T1>
			constexpr type(T0 &&v, T1 &&r ) : std::tuple<Ts...> (std::forward<T0>(v)), detail::ebo_helper<R>(std::forward<T1>(r)) {}

		public:
			friend constexpr void tag_invoke(start_t, type &op) noexcept { std::apply([&op](Ts &...vals) { C{}(std::move(op.receiver()), std::move(vals)...); }, op.values()); }

		private:
			[[nodiscard]] constexpr auto &receiver() noexcept { return detail::ebo_helper<R>::value(); }
			[[nodiscard]] constexpr auto &values() noexcept { return *static_cast<std::tuple<Ts...> *>(this); }
		};
		template<typename C, detail::movable_value... Ts>
		class sender<C, Ts...>::type : std::tuple<Ts...>
		{
			friend struct just_t;
			friend struct just_error_t;
			friend struct just_stopped_t;

			template<typename R>
			using operation_t = typename operation<std::decay_t<R>, C, Ts...>::type;

		public:
			using completion_signatures = completion_signatures<C(Ts...)>;

		private:
			template<typename... Vs>
			constexpr type(Vs &&...vals) : std::tuple<Ts...>{ std::forward<Vs>(vals)... } {}

		public:
			template<receiver_of<completion_signatures> R> requires (std::copy_constructible<Ts> && ...)
			friend constexpr operation_t<R> tag_invoke(connect_t, const type &s, R &&r) { return {s.vs_, std::forward<R>(r)}; }
			template<receiver_of<completion_signatures> R>
			friend constexpr operation_t<R> tag_invoke(connect_t, type &&s, R &&r) { return {std::move(s.vs_), std::forward<R>(r)}; }

			friend constexpr detail::empty_env_t tag_invoke(get_env_t, const type &) noexcept { return {}; }
		};
	}

	using _just::just_t;
	using _just::just_error_t;
	using _just::just_stopped_t;

	struct _just::just_t
	{
		template<typename... Vs>
		[[nodiscard]] constexpr typename sender<set_value_t, std::decay_t<Vs>...>::type operator()(Vs &&...vals) const { return {std::forward<Vs>(vals)...}; }
	};

	/** Returns a sender that passes a set of values through the value channel.
	 * @param vals Values to be sent through the value channel.
	 * @return Sender that completes via `set_value(vals...)`. */
	inline constexpr auto just = just_t{};

	struct _just::just_error_t
	{
		template<typename Err>
		[[nodiscard]] constexpr typename sender<set_error_t, std::decay_t<Err>>::type operator()(Err &&err) const { return {std::forward<Err>(err)}; }
	};

	/** Returns a sender a sender that passes an error through the error channel.
	 * @param err Error to be sent through the error channel.
	 * @return Sender that completes via `set_error(err)`. */
	inline constexpr auto just_error = just_error_t{};

	struct _just::just_stopped_t { [[nodiscard]] constexpr typename sender<set_stopped_t>::type operator()() const { return {}; }};

	/** Customization point object returning a sender that completes immediately through the stopped.
	 * @return Sender that completes via `set_stopped()`. */
	inline constexpr auto just_stopped = just_stopped_t{};
}
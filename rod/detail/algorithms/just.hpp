/*
 * Created by switchblade on 2023-04-17.
 */

#pragma once

#include "../queries/signatures.hpp"
#include "connect.hpp"
#include "opstate.hpp"

namespace rod
{
	inline namespace _just
	{
		struct just_t;
		struct just_error_t;
		struct just_stopped_t;
	}

	namespace detail
	{
		template<typename Signal, movable_value... Ts>
		class just_sender : std::tuple<Ts...>
		{
			friend struct _just::just_t;
			friend struct _just::just_error_t;
			friend struct _just::just_stopped_t;

		public:
			using completion_signatures = completion_signatures<Signal(Ts...)>;

		private:
			template<typename R>
			class operation : std::tuple<Ts...>, ebo_helper<R>
			{
				friend class just_sender;

				template<typename T0, typename T1>
				constexpr operation(T0 &&v, T1 &&r) : std::tuple<Ts...>(std::forward<T0>(v)), ebo_helper<R>(std::forward<T1>(r)) {}

			public:
				friend constexpr void tag_invoke(start_t, operation &op) noexcept { std::apply([&op](Ts &...vals) { Signal{}(std::move(op.receiver()), std::move(vals)...); }, op.values()); }

			private:
				[[nodiscard]] constexpr auto &receiver() noexcept { return ebo_helper<R>::value(); }
				[[nodiscard]] constexpr auto &values() noexcept { return *static_cast<std::tuple<Ts...> *>(this); }
			};

			template<typename... Vs>
			constexpr just_sender(Vs &&...vals) : std::tuple<Ts...>{std::forward<Vs>(vals)...} {}

		public:
			template<receiver_of<completion_signatures> R> requires (std::copy_constructible<Ts> && ...)
			friend constexpr operation<std::decay_t<R>> tag_invoke(connect_t, const just_sender &s, R &&r)
			{
				return {s.vs_, std::forward<R>(r)};
			}
			template<receiver_of<completion_signatures> R>
			friend constexpr operation<std::decay_t<R>> tag_invoke(connect_t, just_sender &&s, R &&r)
			{
				return {std::move(s.vs_), std::forward<R>(r)};
			}

			friend constexpr empty_env_t tag_invoke(get_env_t, const just_sender &) noexcept { return {}; }
		};
	}

	struct _just::just_t
	{
		template<typename... Vs>
		[[nodiscard]] constexpr detail::just_sender<set_value_t, std::decay_t<Vs>...> operator()(Vs &&...vs) const { return {std::forward<Vs>(vs)...}; }
	};

	/** Customization point object returning a sender that passes a set of values through the value channel. */
	inline constexpr auto just = just_t{};

	struct _just::just_error_t
	{
		template<typename Err>
		[[nodiscard]] constexpr detail::just_sender<set_error_t, std::decay_t<Err>> operator()(Err &&err) const { return {std::forward<Err>(err)}; }
	};

	/** Customization point object returning a sender that passes an error through the error channel. */
	inline constexpr auto just_error = just_error_t{};

	struct _just::just_stopped_t { [[nodiscard]] constexpr detail::just_sender<set_stopped_t> operator()() const { return {}; }};

	/** Customization point object returning a sender that completes immediately through the stopped. */
	inline constexpr auto just_stopped = just_stopped_t{};
}
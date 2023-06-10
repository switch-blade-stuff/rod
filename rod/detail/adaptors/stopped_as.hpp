/*
 * Created by switchblade on 2023-05-17.
 */

#pragma once

#include <optional>

#include "../factories/just.hpp"
#include "../concepts.hpp"
#include "closure.hpp"
#include "let.hpp"

namespace rod
{
	namespace _stopped_as
	{
		template<typename, typename>
		struct operation { struct type; };
		template<typename, typename>
		struct receiver { struct type; };
		template<typename>
		struct sender { struct type; };

		template<typename Snd, typename Rcv>
		struct receiver<Snd, Rcv>::type
		{
			using is_receiver = std::true_type;
			using _operation_t = typename operation<Snd, Rcv>::type;
			using _value_t = std::decay_t<detail::single_sender_value_type<Snd, env_of_t<Rcv>>>;

			template<decays_to<type> T, typename V>
			friend constexpr void tag_invoke(set_value_t, T &&r, V &&value) noexcept { r._set_value(std::forward<V>(value)); }
			template<decays_to<type> T, typename Err>
			friend constexpr void tag_invoke(set_error_t, T &&r, Err &&err) noexcept { r._set_error(std::forward<Err>(err)); }
			template<decays_to<type> T>
			friend constexpr void tag_invoke(set_stopped_t, T &&r) noexcept { r._set_stopped(); }

			template<typename V>
			constexpr void _set_value(V &&value) noexcept;
			template<typename Err>
			constexpr void _set_error(Err &&err) noexcept;
			constexpr void _set_stopped() noexcept;

			_operation_t *_op = {};
		};

		template<typename Snd, typename Rcv>
		struct operation<Snd, Rcv>::type
		{
			using _receiver_t = typename receiver<Snd, Rcv>::type;

			type(type &&) = delete;
			type &operator=(type &&) = delete;

			constexpr type(Snd &&snd, Rcv &&rcv) : _rcv(std::forward<Rcv>(rcv)), _state(connect(std::forward<Snd>(snd), _receiver_t{this})) {}

			friend void tag_invoke(start_t, type &op) noexcept { start(op._state); }

			ROD_NO_UNIQUE_ADDRESS Rcv _rcv;
			connect_result_t<Snd, _receiver_t> _state;
		};

		template<typename Snd, typename Rcv>
		template<typename V>
		constexpr void receiver<Snd, Rcv>::type::_set_value(V &&value) noexcept
		{
			static_assert(std::constructible_from<_value_t, V>);
			static_assert(detail::callable<set_value_t, Rcv, std::optional<_value_t>>);

			const auto do_set_value = [&]() { set_value(std::move(_op->_rcv), std::optional<_value_t>{std::forward<V>(value)}); };
			if constexpr (!std::is_nothrow_constructible_v<std::optional<_value_t>, V>)
				try { do_set_value(); } catch (...) { set_error(std::move(_op->_rcv), std::current_exception()); }
			else
				do_set_value();
		}
		template<typename Snd, typename Rcv>
		template<typename Err>
		constexpr void receiver<Snd, Rcv>::type::_set_error(Err &&err) noexcept
		{
			static_assert(detail::callable<set_error_t, Rcv, Err>);
			set_error(std::move(_op->_rcv), std::forward<Err>(err));
		}
		template<typename Snd, typename Rcv>
		constexpr void receiver<Snd, Rcv>::type::_set_stopped() noexcept
		{
			static_assert(detail::callable<set_value_t, Rcv, std::optional<_value_t>>);
			set_value(std::move(_op->_rcv), std::optional<_value_t>{std::nullopt});
		}

		template<typename Snd>
		struct sender<Snd>::type
		{
			using is_sender = std::true_type;

			template<typename T, typename Rcv>
			using _operation_t = typename operation<copy_cvref_t<T, Snd>, Rcv>::type;
			template<typename T, typename Rcv>
			using _receiver_t = typename receiver<copy_cvref_t<T, Snd>, Rcv>::type;

			template<typename T>
			using _error_signs_t = completion_signatures<set_error_t(T)>;
			template<typename... Ts>
			using _value_signs_t = completion_signatures<set_value_t(std::optional<std::decay_t<Ts>>...)>;
			template<typename T, typename Env>
			using _signs_t = make_completion_signatures<copy_cvref_t<T, Snd>, Env, completion_signatures<set_error_t(std::exception_ptr)>, _value_signs_t, _error_signs_t, completion_signatures<>>;

			friend constexpr env_of_t<const Snd &> tag_invoke(get_env_t, const type &s) noexcept(detail::nothrow_callable<get_env_t, const Snd &>) { return get_env(s._snd); }
			template<decays_to<type> T, typename Env>
			friend constexpr _signs_t<T, Env> tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }

			template<decays_to<type> T, typename Rcv>
			friend constexpr _operation_t<T, Rcv> tag_invoke(connect_t, T &&s, Rcv rcv)
			{
				static_assert(detail::single_sender<copy_cvref_t<T, Snd>, env_of_t<Rcv>>);
				return {std::forward<T>(s)._snd, std::move(rcv)};
			}

			ROD_NO_UNIQUE_ADDRESS Snd _snd;
		};

		class stopped_as_optional_t
		{
			template<typename Snd>
			using sender_t = typename sender<Snd>::type;

		public:
			template<rod::sender Snd>
			[[nodiscard]] constexpr sender_t<Snd> operator()(Snd &&snd) const noexcept(std::is_nothrow_constructible_v<sender_t<Snd>, Snd>) { return sender_t<Snd>{std::forward<Snd>(snd)}; }
			[[nodiscard]] constexpr detail::back_adaptor<stopped_as_optional_t> operator()() const noexcept { return {}; }
		};
		class stopped_as_error_t
		{
			template<typename Err>
			using back_adaptor = detail::back_adaptor<stopped_as_error_t, Err>;

		public:
			template<rod::sender Snd, movable_value Err>
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd, Err err) const
			{
				return snd | let_stopped([e = std::move(err)]() mutable noexcept(std::is_nothrow_move_constructible_v<Err>)
				{
					return just_error(std::move(e));
				});
			}
			template<movable_value Err>
			[[nodiscard]] constexpr back_adaptor<Err> operator()(Err &&err) const noexcept(std::is_nothrow_move_constructible_v<Err>)
			{
				return back_adaptor<Err>{*this, std::move(err)};
			}
		};
	}

	using _stopped_as::stopped_as_optional_t;

	/** @brief Sender adaptor used to adapt value and stop completion channels of an input sender into an optional value.
	 *
	 * If the input sender completes via the value channel, the optional is initialized with the resulting value and passed through the value channel.
	 * Otherwise, if the input sender completes via the stop channel, the optional is initialized with `std::nullopt` and passed through the value channel.
	 * @param snd Sender who's value and stop channels to adapt into an optional. If omitted, creates a pipe-able sender adaptor.
	 * @return Sender who's value completion type is `std::optional<V>`, where `V` is the value completion type of \a snd.
	 * @note Resulting sender never completes via the stop channel. */
	inline constexpr auto stopped_as_optional = stopped_as_optional_t{};

	using _stopped_as::stopped_as_error_t;

	/** Sender adaptor used to adapt the stop completion channel of an input sender into an error completion.
	 * @param snd Sender who's stop channel to adapt into an error. If omitted, creates a pipe-able sender adaptor.
	 * @param err Error to send through the error channel when
	 * @return Sender that transforms stop completion channel into the specified error.
	 * @note Resulting sender never completes via the stop channel. */
	inline constexpr auto stopped_as_error = stopped_as_error_t{};
}

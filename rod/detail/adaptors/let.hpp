/*
 * Created by switchblade on 2023-05-12.
 */

#pragma once

#include <tuple>

#include "../queries/completion.hpp"
#include "../concepts.hpp"
#include "closure.hpp"

namespace rod
{
	namespace _let
	{
		template<typename C, typename E, typename F, typename S>
		struct filter_signs
		{
			using type = completion_signatures<S>;
		};
		template<typename C, typename E, typename F, typename... Args>
		struct filter_signs<C, E, F, C(Args...)>
		{
			using type = make_completion_signatures<
			        std::invoke_result_t<F, detail::decayed_ref<Args>...>, E,
			        completion_signatures<set_error_t(std::exception_ptr)>>;
		};

		template<typename C, typename E, typename F, typename S>
		using filter_signs_t = typename filter_signs<C, E, F, S>::type;

		template<typename, typename, typename>
		struct deduce_state;
		template<typename R, typename F, template<typename...> typename T, typename... Args>
		struct deduce_state<R, F, T<Args...>>
		{
			using type = connect_result_t<std::invoke_result_t<F, detail::decayed_ref<Args>...>, R>;
		};

		template<typename R, typename F, typename T>
		using deduce_state_t = typename deduce_state<R, F, T>::type;

		template<typename, typename, typename, typename...>
		struct operation_base { struct type; };
		template<typename, typename, typename, typename>
		struct operation { struct type; };
		template<typename, typename, typename, typename...>
		struct receiver { struct type; };
		template<typename, typename, typename>
		struct sender { struct type; };

		template<typename C, typename R, typename F, typename... Ts>
		struct operation_base<C, R, F, Ts...>::type
		{
			using _state_t = std::variant<std::monostate, deduce_state_t<R, F, Ts>...>;
			using _result_t = std::variant<std::monostate, Ts...>;

			[[ROD_NO_UNIQUE_ADDRESS]] R _rcv;
			[[ROD_NO_UNIQUE_ADDRESS]] F _fn;
			_state_t _state = std::monostate{};
			_result_t _res = std::monostate{};
		};

		template<typename C, typename R, typename F, typename... Ts>
		struct receiver<C, R, F, Ts...>::type
		{
			using is_receiver = std::true_type;

			using _operation_base_t = typename operation_base<C, R, F, Ts...>::type;

			friend constexpr decltype(auto) tag_invoke(get_env_t, const type &r) noexcept(detail::nothrow_callable<get_env_t, const R &>) { return get_env(r._op->_rcv); }

			template<detail::completion_channel T, typename... Args> requires std::same_as<T, C> && std::invocable<F, detail::decayed_ref<Args>...> && sender_to<std::invoke_result_t<F, detail::decayed_ref<Args>...>, R>
			friend constexpr void tag_invoke(T, type &&r, Args &&...args) noexcept try
			{
				using tuple_t = detail::decayed_tuple<Args...>;
				using state_t = deduce_state_t<R, F, tuple_t>;

				auto &args_tuple = r._op->_res.template emplace<tuple_t>(std::forward<Args>(args)...);
				start(r._op->_state.template emplace<state_t>(detail::implicit_eval{[&]()
				{
					return connect(std::apply(std::move(r._op->_fn), args_tuple), std::move(r._op->_rcv));
				}}));
			}
			catch (...) { set_error(std::move(r._op->_rcv), std::current_exception()); }

			template<detail::completion_channel T, typename... Args> requires(!std::same_as<T, C> && detail::callable<T, R, Args...>)
			friend constexpr void tag_invoke(T c, type &&r, Args &&...args) noexcept { c(std::move(r._op->_rcv), std::forward<Args>(args)...); }

			_operation_base_t *_op;
		};

		template<typename C, typename S, typename R, typename F>
		using bind_receiver = typename detail::gather_signatures_t<C, S, env_of_t<R>, detail::decayed_tuple, detail::bind_front<receiver, C, R, F>::template type>::type;
		template<typename C, typename S, typename R, typename F>
		using bind_operation = typename bind_receiver<C, S, R, F>::_operation_base_t;

		template<typename C, typename S, typename R, typename F>
		struct operation<C, S, R, F>::type : bind_operation<C, S, R, F>
		{
			using _op_base_t = bind_operation<C, S, R, F>;
			using _receiver_t = bind_receiver<C, S, R, F>;

			template<typename R2>
			constexpr type(S &&snd, R2 &&rcv, F fn) : _op_base_t{std::forward<R2>(rcv), std::move(fn)}, _state(connect(std::move(snd), _receiver_t{this})) {}

			friend constexpr void tag_invoke(start_t, type &op) noexcept { start(op._state); }

			[[ROD_NO_UNIQUE_ADDRESS]] connect_result_t<S, _receiver_t> _state;
		};

		template<typename C, typename S, typename F>
		struct sender<C, S, F>::type
		{
			using is_sender = std::true_type;

			template<typename T, typename R>
			using _operation_t = typename operation<C, copy_cvref_t<T, S>, R, F>::type;
			template<typename T, typename R>
			using _receiver_t = bind_receiver<C, copy_cvref_t<T, S>, R, F>;

			template<typename T, typename E>
			using _signs = unique_tuple_t<detail::apply_tuple_t<detail::concat_tuples_t,
								detail::apply_tuple_list_t<detail::bind_front<filter_signs_t, C, E, F>::template type,
								completion_signatures_of_t<copy_cvref_t<T, S>, E>>>>;

			friend constexpr decltype(auto) tag_invoke(get_env_t, const type &s) noexcept(detail::nothrow_callable<get_env_t, const S &>) { return get_env(s._snd); }
			template <detail::decays_to<type> T, typename E>
			friend constexpr _signs<T, E> tag_invoke(get_completion_signatures_t, T &&, E &&) noexcept { return {}; }

			template<detail::decays_to<type> T, rod::receiver R> requires sender_to<copy_cvref_t<T, S>, _receiver_t<T, R>>
			friend constexpr _operation_t<T, R> tag_invoke(connect_t, T &&s, R rcv)
			{
				return _operation_t<T, R>{std::forward<T>(s)._snd, std::move(rcv), std::forward<T>(s)._fn};
			}

			[[ROD_NO_UNIQUE_ADDRESS]] S _snd;
			[[ROD_NO_UNIQUE_ADDRESS]] F _fn;
		};

		template<typename C>
		class let_channel
		{
			template<typename S>
			using value_completion = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<S>())));
			template<typename S, typename F>
			using sender_t = typename sender<C, std::decay_t<S>, std::decay_t<F>>::type;
			template<typename F>
			using back_adaptor = detail::back_adaptor<let_channel, std::decay_t<F>>;

		public:
			template<rod::sender Snd, detail::movable_value F> requires detail::tag_invocable_with_completion_scheduler<let_channel, set_value_t, Snd, F>
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd, F &&fn) const noexcept(nothrow_tag_invocable<let_channel, value_completion<Snd>, Snd, F>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::forward<F>(fn));
			}
			template<rod::sender Snd, detail::movable_value F> requires(!detail::tag_invocable_with_completion_scheduler<let_channel, set_value_t, Snd, F> && tag_invocable<let_channel, Snd, F>)
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd, F &&fn) const noexcept(nothrow_tag_invocable<let_channel, Snd, F>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<F>(fn));
			}
			template<rod::sender Snd, detail::movable_value F> requires(!detail::tag_invocable_with_completion_scheduler<let_channel, set_value_t, Snd, F> && !tag_invocable<let_channel, Snd, F>)
			[[nodiscard]] constexpr sender_t<Snd, F> operator()(Snd &&snd, F &&fn) const noexcept(std::is_nothrow_constructible_v<sender_t<Snd, F>, Snd, F>)
			{
				return sender_t<Snd, F>{std::forward<Snd>(snd), std::forward<F>(fn)};
			}

			template<detail::movable_value F>
			[[nodiscard]] constexpr back_adaptor<F> operator()(F &&fn) const noexcept(std::is_nothrow_constructible_v<back_adaptor<F>, let_channel, F>)
			{
				return back_adaptor<F>{*this, std::forward<F>(fn)};
			}
		};

		using let_value_t = let_channel<set_value_t>;
		using let_error_t = let_channel<set_error_t>;
		using let_stopped_t = let_channel<set_stopped_t>;
	}

	using _let::let_value_t;

	/** Customization point object used to transform a sender’s value completion channel into a new child operation.
	 * @param snd Sender who's value channel to transform into a new operation. If omitted, creates a pipe-able sender adaptor.
	 * @param fn Function invoked with results of the value channel of \a snd and returning a new child sender.
	 * @return Sender who's value completion channel starts a new operation using the sender returned by \a fn. */
	inline constexpr auto let_value = let_value_t{};

	using _let::let_error_t;

	/** Customization point object used to transform a sender’s error completion channel into a new child operation.
	 * @param snd Sender who's error channel to transform into a new operation. If omitted, creates a pipe-able sender adaptor.
	 * @param fn Function invoked with results of the error channel of \a snd and returning a new child sender.
	 * @return Sender who's error completion channel starts a new operation using the sender returned by \a fn. */
	inline constexpr auto let_error = let_error_t{};

	using _let::let_stopped_t;

	/** Customization point object used to transform a sender’s stop completion channel into a new child operation.
	 * @param snd Sender who's stop channel to transform into a new operation. If omitted, creates a pipe-able sender adaptor.
	 * @param fn Function returning a new child sender.
	 * @return Sender who's stop completion channel starts a new operation using the sender returned by \a fn. */
	inline constexpr auto let_stopped = let_stopped_t{};
}

/*
 * Created by switchblade on 2023-05-12.
 */

#pragma once

#include <tuple>

#include "../queries/completion.hpp"
#include "../async_base.hpp"
#include "closure.hpp"

namespace rod
{
	namespace _let
	{
		template<typename>
		class let_channel;

		template<typename C, typename Env, typename Fn, typename Snd>
		struct filter_signs { using type = completion_signatures<Snd>; };
		template<typename C, typename Env, typename Fn, typename... Args>
		struct filter_signs<C, Env, Fn, C(Args...)> { using type = make_completion_signatures<std::invoke_result_t<Fn, _detail::decayed_ref<Args>...>, Env, completion_signatures<set_error_t(std::exception_ptr)>>; };

		template<typename C, typename Env, typename Fn, typename Snd>
		using filter_signs_t = typename filter_signs<C, Env, Fn, Snd>::type;

		template<typename, typename, typename>
		struct deduce_state;
		template<typename Rcv, typename Fn, template<typename...> typename T, typename... Args>
		struct deduce_state<Rcv, Fn, T<Args...>> { using type = connect_result_t<std::invoke_result_t<Fn, _detail::decayed_ref<Args>...>, Rcv>; };

		template<typename Rcv, typename Fn, typename T>
		using deduce_state_t = typename deduce_state<Rcv, Fn, T>::type;

		template<typename, typename, typename, typename...>
		struct operation_base { struct type; };
		template<typename, typename, typename, typename>
		struct operation { class type; };
		template<typename, typename, typename, typename...>
		struct receiver { class type; };
		template<typename, typename, typename>
		struct sender { class type; };

		template<typename C, typename Rcv, typename Fn, typename... Ts>
		struct operation_base<C, Rcv, Fn, Ts...>::type : empty_base<Rcv>, empty_base<Fn>
		{
			using rcv_base = empty_base<Rcv>;
			using func_base = empty_base<Fn>;

			using state_t = std::variant<std::monostate, deduce_state_t<Rcv, Fn, Ts>...>;
			using result_t = std::variant<std::monostate, Ts...>;

			template<typename Rcv2, typename Fn2>
			constexpr explicit type(Rcv2 &&rcv, Fn2 &&fn) noexcept(std::is_nothrow_constructible_v<Rcv, Rcv2> && std::is_nothrow_constructible_v<Fn, Fn2>) : rcv_base(std::forward<Rcv2>(rcv)), func_base(std::forward<Fn2>(fn)) {}

			[[nodiscard]] constexpr decltype(auto) rcv() noexcept { return rcv_base::value(); }
			[[nodiscard]] constexpr decltype(auto) fn() noexcept { return func_base::value(); }

			state_t _state = std::monostate{};
			result_t _res = std::monostate{};
		};

		template<typename C, typename Rcv, typename Fn, typename... Ts>
		class receiver<C, Rcv, Fn, Ts...>::type
		{
			template<typename, typename, typename, typename>
			friend struct operation;

		public:
			using is_receiver = std::true_type;

		private:
			using operation_base = typename operation_base<C, Rcv, Fn, Ts...>::type;

		public:
			constexpr explicit type(operation_base *op) noexcept : _op(op) {}

			friend constexpr env_of_t<Rcv> tag_invoke(get_env_t, const type &r) noexcept(_detail::nothrow_callable<get_env_t, const Rcv &>) { return get_env(r._op->rcv()); }

			template<_detail::completion_channel T, typename... Args> requires(!std::same_as<T, C> && _detail::callable<T, Rcv, Args...>)
			friend void tag_invoke(T, type &&r, Args &&...args) noexcept { r.complete_forward(T{}, std::forward<Args>(args)...); }
			template<_detail::completion_channel T, typename... Args> requires decays_to<T, C> && std::invocable<Fn, Args...>
			friend void tag_invoke(T, type &&r, Args &&...args) noexcept { r.complete_selected(std::forward<Args>(args)...); }

		private:
			template<typename T, typename... Args>
			constexpr void complete_forward(T, Args &&...args) noexcept { T{}(std::move(_op->rcv()), std::forward<Args>(args)...); }
			template<typename... Args>
			constexpr void complete_selected(Args &&...args) noexcept
			{
				static_assert(sender_to<std::invoke_result_t<Fn, Args...>, Rcv>, "Sender returned by the functor of `rod::let` must be connectable with it's downstream receiver");
				using state_t = deduce_state_t<Rcv, Fn, _detail::decayed_tuple<Args...>>;

				const auto connect = [&]()
				{
					auto &tuple = _op->_res.template emplace<_detail::decayed_tuple<Args...>>(std::forward<Args>(args)...);
					return rod::connect(std::apply(std::move(_op->fn()), tuple), std::move(_op->rcv()));
				};
				try { start(_op->_state.template emplace<state_t>(_detail::eval_t{connect})); }
				catch (...) { set_error(std::move(_op->rcv()), std::current_exception()); }
			}

			operation_base *_op = {};
		};

		template<typename C, typename Rcv, typename Fn, typename... Ts>
		using unique_receiver = _detail::apply_tuple_list_t<_detail::bind_front<receiver, C, Rcv, Fn>::template type, unique_tuple_t<type_list_t<Ts...>>>;
		template<typename C, typename Snd, typename Rcv, typename Fn>
		using bind_receiver = typename _detail::gather_signatures_t<C, Snd, env_of_t<Rcv>, _detail::decayed_tuple, _detail::bind_front<unique_receiver, C, Rcv, Fn>::template type>::type;

		template<typename C, typename Snd, typename Rcv, typename Fn>
		class operation<C, Snd, Rcv, Fn>::type : bind_receiver<C, Snd, Rcv, Fn>::operation_base
		{
		private:
			using operation_base = typename bind_receiver<C, Snd, Rcv, Fn>::operation_base;
			using receiver_t = bind_receiver<C, Snd, Rcv, Fn>;
			using state_t = connect_result_t<Snd, receiver_t>;

		public:
			type(type &&) = delete;
			type &operator=(type &&) = delete;

			template<typename Snd2>
			constexpr explicit type(Snd2 &&snd, Rcv &&rcv, Fn fn) : operation_base(std::forward<Rcv>(rcv), std::move(fn)), _state(connect(std::forward<Snd>(snd), receiver_t(this))) {}

			friend constexpr void tag_invoke(start_t, type &op) noexcept { start(op._state); }

		private:
			state_t _state;
		};

		template<typename C, typename Snd, typename Fn>
		class sender<C, Snd, Fn>::type : empty_base<Snd>, empty_base<Fn>
		{
			using snd_base = empty_base<Snd>;
			using func_base = empty_base<Fn>;

		public:
			using is_sender = std::true_type;

		private:
			template<typename T, typename Rcv>
			using operation_t = typename operation<C, copy_cvref_t<T, Snd>, Rcv, Fn>::type;
			template<typename T, typename Rcv>
			using receiver_t = bind_receiver<C, copy_cvref_t<T, Snd>, Rcv, Fn>;

			template<typename Env, typename... Ts>
			using apply_filter = unique_tuple_t<_detail::concat_tuples_t<filter_signs_t<C, Env, Fn, Ts>...>>;
			template<typename T, typename Env>
			using signs_t = _detail::apply_tuple_list_t<_detail::bind_front<apply_filter, Env>::template type, completion_signatures_of_t<copy_cvref_t<T, Snd>, Env>>;

		public:
			template<typename Snd2, typename Fn2>
			constexpr explicit type(Snd2 &&snd, Fn2 &&fn) noexcept(std::is_nothrow_constructible_v<Snd, Snd2> && std::is_nothrow_constructible_v<Snd, Fn2>) : snd_base(std::forward<Snd2>(snd)), func_base(std::forward<Fn2>(fn)) {}

			friend constexpr env_of_t<Snd> tag_invoke(get_env_t, const type &s) noexcept(_detail::nothrow_callable<get_env_t, const Snd &>) { return get_env(s.snd_base::value()); }
			template <decays_to<type> T, typename Env>
			friend constexpr signs_t<T, Env> tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }

			template<decays_to<type> T, rod::receiver Rcv> requires sender_to<copy_cvref_t<T, Snd>, receiver_t<T, Rcv>>
			friend constexpr operation_t<T, Rcv> tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(std::is_nothrow_constructible_v<operation_t<T, Rcv>, copy_cvref_t<T, Snd>, Rcv, copy_cvref_t<T, Fn>>)
			{
				return operation_t<T, Rcv>{std::forward<T>(s).snd_base::value(), std::move(rcv), std::forward<T>(s).func_base::value()};
			}
		};

		template<typename C>
		class let_channel
		{
			template<typename Snd>
			using value_scheduler = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Snd, typename Fn>
			using sender_t = typename sender<C, std::decay_t<Snd>, std::decay_t<Fn>>::type;
			template<typename Fn>
			using back_adaptor = _detail::back_adaptor<let_channel, std::decay_t<Fn>>;

		public:
			template<rod::sender Snd, movable_value Fn> requires _detail::tag_invocable_with_completion_scheduler<let_channel, set_value_t, Snd, Snd, Fn>
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd, Fn &&fn) const noexcept(nothrow_tag_invocable<let_channel, value_scheduler<Snd>, Snd, Fn>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::forward<Fn>(fn));
			}
			template<rod::sender Snd, movable_value Fn> requires(!_detail::tag_invocable_with_completion_scheduler<let_channel, set_value_t, Snd, Snd, Fn> && tag_invocable<let_channel, Snd, Fn>)
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd, Fn &&fn) const noexcept(nothrow_tag_invocable<let_channel, Snd, Fn>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<Fn>(fn));
			}
			template<rod::sender Snd, movable_value Fn> requires(!_detail::tag_invocable_with_completion_scheduler<let_channel, set_value_t, Snd, Snd, Fn> && !tag_invocable<let_channel, Snd, Fn>)
			[[nodiscard]] constexpr sender_t<Snd, Fn> operator()(Snd &&snd, Fn &&fn) const noexcept(std::is_nothrow_constructible_v<sender_t<Snd, Fn>, Snd, Fn>)
			{
				return sender_t<Snd, Fn>{std::forward<Snd>(snd), std::forward<Fn>(fn)};
			}

			template<movable_value Fn>
			[[nodiscard]] constexpr back_adaptor<Fn> operator()(Fn &&fn) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Fn>, let_channel, Fn>)
			{
				return back_adaptor<Fn>{*this, std::forward<Fn>(fn)};
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


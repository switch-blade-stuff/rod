/*
 * Created by switchblade on 2023-04-17.
 */

#pragma once

#include <functional>

#include "../queries/completion.hpp"
#include "../concepts.hpp"
#include "closure.hpp"

namespace rod
{
	namespace _then
	{
		template<typename>
		class upon_channel;

		template<typename, typename, typename>
		struct deduce_signs;
		template<typename Snd, typename Fn>
		struct deduce_signs<void, Snd, Fn>
		{
			template<typename T, typename E>
			using has_throwing = std::negation<value_types_of_t<copy_cvref_t<T, Snd>, E, _detail::bind_front<std::is_nothrow_invocable, Fn>::template type, std::conjunction>>;
			template<typename T, typename E>
			using error_signs = std::conditional_t<has_throwing<T, E>::value, completion_signatures<set_error_t(std::exception_ptr)>, completion_signatures<>>;
		};
		template<typename Snd, typename Fn>
		struct deduce_signs<set_value_t, Snd, Fn>
		{
			template<typename... Ts>
			using value_signs = completion_signatures<_detail::make_signature_t<set_value_t, std::invoke_result_t<Fn, Ts...>>>;
			template<typename T, typename E>
			using error_signs = typename deduce_signs<void, Snd, Fn>::template error_signs<T, E>;

			template<typename T, typename E>
			using type = make_completion_signatures<copy_cvref_t<T, Snd>, E, error_signs<T, E>, value_signs>;
		};
		template<typename Snd, typename Fn>
		struct deduce_signs<set_error_t, Snd, Fn>
		{
			template<typename T>
			using value_signs = completion_signatures<_detail::make_signature_t<set_value_t, std::invoke_result_t<Fn, T>>>;
			template<typename T, typename E>
			using error_signs = typename deduce_signs<void, Snd, Fn>::template error_signs<T, E>;

			template<typename T, typename E>
			using type = make_completion_signatures<copy_cvref_t<T, Snd>, E, error_signs<T, E>, _detail::default_set_value, value_signs>;
		};
		template<typename Snd, typename Fn>
		struct deduce_signs<set_stopped_t, Snd, Fn>
		{
			using value_signs = completion_signatures<_detail::make_signature_t<set_value_t, std::invoke_result_t<Fn>>>;
			template<typename T, typename E>
			using error_signs = typename deduce_signs<void, Snd, Fn>::template error_signs<T, E>;

			template<typename T, typename E>
			using type = make_completion_signatures<copy_cvref_t<T, Snd>, E, error_signs<T, E>, _detail::default_set_value, _detail::default_set_error, value_signs>;
		};

		template<typename Rcv, typename Fn>
		struct operation_base : empty_base<Rcv>, empty_base<Fn>
		{
			template<typename Rcv2, typename Fn2>
			constexpr operation_base(Rcv2 &&rcv, Fn2 &&fn) noexcept(std::is_nothrow_constructible_v<Rcv, Rcv2> && std::is_nothrow_constructible_v<Fn, Fn2>) : empty_base<Rcv>(std::forward<Rcv2>(rcv)), empty_base<Fn>(std::forward<Fn2>(fn)) {}

			[[nodiscard]] constexpr decltype(auto) rcv() noexcept { return empty_base<Rcv>::value(); }
			[[nodiscard]] constexpr decltype(auto) func() noexcept { return empty_base<Fn>::value(); }
			[[nodiscard]] constexpr decltype(auto) rcv() const noexcept { return empty_base<Rcv>::value(); }
			[[nodiscard]] constexpr decltype(auto) func() const noexcept { return empty_base<Fn>::value(); }
		};

		template<typename, typename, typename, typename>
		struct operation { class type; };
		template<typename, typename, typename>
		struct receiver { class type; };
		template<typename, typename, typename>
		struct sender { class type; };

		template<typename C, typename Snd, typename Rcv, typename Fn>
		class operation<C, Snd, Rcv, Fn>::type : operation_base<Rcv, Fn>
		{
			using receiver_t = typename receiver<C, Rcv, Fn>::type;
			using state_t = connect_result_t<Snd, receiver_t>;

		public:
			type(type &&) = delete;
			type &operator=(type &&) = delete;

			template<typename Snd2, typename Rcv2, typename Fn2>
			constexpr explicit type(Snd2 &&snd, Rcv2 &&rcv, Fn2 &&fn) noexcept(std::is_nothrow_constructible_v<operation_base<Rcv, Fn>, Rcv2, Fn2> && _detail::nothrow_callable<connect_t, Snd2, receiver_t>)
					: operation_base<Rcv, Fn>(std::forward<Rcv2>(rcv), std::forward<Fn2>(fn)), _state(connect(std::forward<Snd2>(snd), receiver_t(this))) {}

			friend constexpr void tag_invoke(start_t, type &op) noexcept { start(op._state); }

		private:
			state_t _state;
		};

		template<typename C, typename Rcv, typename Fn>
		class receiver<C, Rcv, Fn>::type
		{
		public:
			using is_receiver = std::true_type;

		public:
			constexpr explicit type(operation_base<Rcv, Fn> *op) noexcept : _op(op) {}

			friend constexpr env_of_t<Rcv> tag_invoke(get_env_t, const type &r) noexcept(nothrow_tag_invocable<get_env_t, const Rcv &>) { return get_env(r._op->rcv()); }

			template<_detail::completion_channel T, typename... Args> requires(!std::same_as<T, C> && _detail::callable<T, Rcv, Args...>)
			friend constexpr void tag_invoke(T, type &&r, Args &&...args) noexcept { r.complete_forward(T{}, std::forward<Args>(args)...); }
			template<_detail::completion_channel T, typename... Args> requires std::same_as<T, C> && std::invocable<Fn, Args...>
			friend constexpr void tag_invoke(T, type &&r, Args &&...args) noexcept { r.complete_selected(T{}, std::forward<Args>(args)...); }

		private:
			template<typename T, typename... Args>
			constexpr void complete_forward(T, Args &&...args) noexcept { T{}(std::move(_op->rcv()), std::forward<Args>(args)...); }
			template<typename T, typename... Args>
			constexpr void complete_selected(T, Args &&...args) noexcept
			{
				const auto do_invoke = [&]()
				{
					if constexpr (std::same_as<std::invoke_result_t<Fn, Args...>, void>)
						(std::invoke(std::move(_op->func()), std::forward<Args>(args)...), set_value(std::move(_op->rcv())));
					else
						set_value(std::move(_op->rcv()), std::invoke(std::move(_op->func()), std::forward<Args>(args)...));
				};

				if constexpr (!std::is_nothrow_invocable_v<Fn, Args...>)
					try { do_invoke(); } catch (...) { set_error(std::move(_op->rcv()), std::current_exception()); }
				else
					do_invoke();
			}

			operation_base<Rcv, Fn> *_op;
		};

		template<typename C, typename Snd, typename Fn>
		class sender<C, Snd, Fn>::type : empty_base<Snd>, empty_base<Fn>
		{
			using assert_invocable_function = _detail::gather_signatures_t<C, Snd, empty_env, _detail::bind_front<std::is_invocable, Fn>::template type, std::conjunction>;
			static_assert(assert_invocable_function::value, "Functor passed to rod::then must be invocable with completion results of the upstream sender");

		public:
			using is_sender = std::true_type;

		private:
			template<typename... Ts>
			using input_tuple = C(Ts...);
			template<typename... Ts>
			using input_variant = completion_signatures<Ts...>;
			template<typename T, typename E>
			using input_signs_t = _detail::gather_signatures_t<C, copy_cvref_t<T, Snd>, E, input_tuple, input_variant>;

			template<typename T, typename Rcv>
			using operation_t = typename operation<C, copy_cvref_t<T, Snd>, Rcv, Fn>::type;
			template<typename Rcv>
			using receiver_t = typename receiver<C, Rcv, Fn>::type;

			template<typename T, typename E>
			using signs_t = typename deduce_signs<C, Snd, Fn>::template type<T, E>;

			using snd_base = empty_base<Snd>;
			using func_base = empty_base<Fn>;

		public:
			template<typename Snd2, typename Fn2>
			constexpr explicit type(Snd2 &&snd, Fn2 &&fn) noexcept(std::is_nothrow_constructible_v<Snd, Snd2> && std::is_nothrow_constructible_v<Fn, Fn2>) : snd_base(std::forward<Snd2>(snd)), func_base(std::forward<Fn2>(fn)) {}

			friend constexpr env_of_t<Snd> tag_invoke(get_env_t, const type &s) noexcept(nothrow_tag_invocable<get_env_t, const Snd &>) { return get_env(s.snd_base::value()); }
			template<decays_to<type> T, typename E>
			friend constexpr signs_t<T, E> tag_invoke(get_completion_signatures_t, T &&, E) noexcept { return {}; }

			template<decays_to<type> T, rod::receiver Rcv> requires receiver_of<receiver_t<Rcv>, input_signs_t<T, env_of_t<Rcv>>>
			friend constexpr operation_t<T, Rcv> tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(std::is_nothrow_constructible_v<operation_t<T, Rcv>, copy_cvref_t<T, Snd>, Rcv, copy_cvref_t<T, Fn>>)
			{
				return operation_t<T, Rcv>{std::forward<T>(s).snd_base::value(), std::move(rcv), std::forward<T>(s).func_base::value()};
			}
		};

		template<typename C>
		class upon_channel
		{
			template<typename Snd>
			using completion_scheduler = decltype(get_completion_scheduler<C>(get_env(std::declval<Snd>())));
			template<typename Fn>
			using back_adaptor = _detail::back_adaptor<upon_channel<C>, std::decay_t<Fn>>;
			template<typename Snd, typename Fn>
			using sender_t = typename sender<C, Snd, std::decay_t<Fn>>::type;

		public:
			template<rod::sender Snd, movable_value Fn> requires _detail::tag_invocable_with_completion_scheduler<upon_channel<C>, C, Snd, Snd, Fn>
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd, Fn &&fn) const noexcept(nothrow_tag_invocable<upon_channel<C>, completion_scheduler<Snd>, Snd, Fn>)
			{
				return tag_invoke(*this, get_completion_scheduler<C>(get_env(snd)), std::forward<Snd>(snd), std::forward<Fn>(fn));
			}
			template<rod::sender Snd, movable_value Fn> requires(!_detail::tag_invocable_with_completion_scheduler<upon_channel<C>, C, Snd, Snd, Fn> && tag_invocable<upon_channel<C>, Snd, Fn>)
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd, Fn &&fn) const noexcept(nothrow_tag_invocable<upon_channel<C>, Snd, Fn>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<Fn>(fn));
			}
			template<rod::sender Snd, movable_value Fn> requires(!_detail::tag_invocable_with_completion_scheduler<upon_channel<C>, C, Snd, Snd, Fn> && !tag_invocable<upon_channel<C>, Snd, Fn>)
			[[nodiscard]] constexpr sender_t<Snd, Fn> operator()(Snd &&snd, Fn &&fn) const noexcept(std::is_nothrow_constructible_v<sender_t<Snd, Fn>, Snd, Fn>)
			{
				return sender_t<Snd, Fn>{std::forward<Snd>(snd), std::forward<Fn>(fn)};
			}

			template<movable_value Fn>
			[[nodiscard]] constexpr back_adaptor<Fn> operator()(Fn &&fn) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Fn>, upon_channel<C>, Fn>)
			{
				return {*this, std::forward<Fn>(fn)};
			}
		};

		using upon_value_t = upon_channel<set_value_t>;
		using upon_error_t = upon_channel<set_error_t>;
		using upon_stopped_t = upon_channel<set_stopped_t>;
	}

	using _then::upon_value_t;
	using then_t = upon_value_t;

	/** Customization point object used to adapt a sender to execute a functor with the values passed through the value channel.
	 * @param snd Sender who's value channel to adapt for execution of \a fn. If omitted, creates a pipe-able sender adaptor.
	 * @param fn Function invoked with value results of \a snd.
	 * @return Sender completing via the value channel with results of \a fn invoked with value results of \a snd. */
	inline constexpr auto then = then_t{};
	/** @copydoc then */
	inline constexpr auto upon_value = then;

	using _then::upon_error_t;

	/** Customization point object used to adapt a sender to execute a functor with the error passed through the error channel.
	 * @param snd Sender who's error channel to adapt for execution of \a fn. If omitted, creates a pipe-able sender adaptor.
	 * @param fn Function invoked with error result of \a snd.
	 * @return Sender completing via the value channel with results of \a fn invoked with error result of \a snd. */
	inline constexpr auto upon_error = upon_error_t{};

	using _then::upon_stopped_t;

	/** Customization point object used to adapt a sender to execute a functor when a stop signal is received.
	 * @param snd Sender who's stop channel to adapt for execution of \a fn. If omitted, creates a pipe-able sender adaptor.
	 * @param fn Function invoked when \a snd is stopped.
	 * @return Sender completing via the value channel with results of \a fn. */
	inline constexpr auto upon_stopped = upon_stopped_t{};
}

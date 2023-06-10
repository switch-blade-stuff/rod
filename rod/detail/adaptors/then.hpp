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
		template<typename, typename, typename>
		struct deduce_signs;
		template<typename S, typename F>
		struct deduce_signs<void, S, F>
		{
			template<typename T, typename E>
			using has_throwing = std::negation<value_types_of_t<copy_cvref_t<T, S>, E, detail::bind_front<std::is_nothrow_invocable, F>::template type, std::conjunction>>;
			template<typename T, typename E>
			using error_signs = std::conditional_t<has_throwing<T, E>::value, completion_signatures<set_error_t(std::exception_ptr)>, completion_signatures<>>;
		};
		template<typename S, typename F>
		struct deduce_signs<set_value_t, S, F>
		{
			template<typename... Ts>
			using value_signs = completion_signatures<detail::make_signature_t<set_value_t, std::invoke_result_t<F, Ts...>>>;
			template<typename T, typename E>
			using error_signs = typename deduce_signs<void, S, F>::template error_signs<T, E>;

			template<typename T, typename E>
			using type = make_completion_signatures<copy_cvref_t<T, S>, E, error_signs<T, E>, value_signs>;
		};
		template<typename S, typename F>
		struct deduce_signs<set_error_t, S, F>
		{
			template<typename T>
			using value_signs = completion_signatures<detail::make_signature_t<set_value_t, std::invoke_result_t<F, T>>>;
			template<typename T, typename E>
			using error_signs = typename deduce_signs<void, S, F>::template error_signs<T, E>;

			template<typename T, typename E>
			using type = make_completion_signatures<copy_cvref_t<T, S>, E, error_signs<T, E>, detail::default_set_value, value_signs>;
		};
		template<typename S, typename F>
		struct deduce_signs<set_stopped_t, S, F>
		{
			using value_signs = completion_signatures<detail::make_signature_t<set_value_t, std::invoke_result_t<F>>>;
			template<typename T, typename E>
			using error_signs = typename deduce_signs<void, S, F>::template error_signs<T, E>;

			template<typename T, typename E>
			using type = make_completion_signatures<copy_cvref_t<T, S>, E, error_signs<T, E>, detail::default_set_value, detail::default_set_error, value_signs>;
		};

		template<typename, typename>
		struct storage { struct type; };

		template<typename R, typename F>
		struct storage<R, F>::type
		{
			ROD_NO_UNIQUE_ADDRESS R _rcv;
			ROD_NO_UNIQUE_ADDRESS F _fn;
		};

		template<typename, typename, typename, typename>
		struct operation { struct type; };
		template<typename, typename, typename>
		struct receiver { struct type; };
		template<typename, typename, typename>
		struct sender { struct type; };

		template<typename C, typename S, typename R, typename F>
		struct operation<C, S, R, F>::type
		{
			using _receiver_t = typename receiver<C, R, F>::type;
			using _storage_t = typename storage<R, F>::type;

			using _state_t = connect_result_t<S, _receiver_t>;

			type(type &&) = delete;
			type &operator=(type &&) = delete;

			constexpr type(S &&snd, R rcv, F fn) : _storage{std::move(rcv), std::move(fn)}, _state(connect(std::forward<S>(snd), _receiver_t{&_storage})) {}

			friend constexpr void tag_invoke(start_t, type &op) noexcept { start(op._state); }

			ROD_NO_UNIQUE_ADDRESS _storage_t _storage;
			_state_t _state;
		};

		template<typename C, typename R, typename F>
		struct receiver<C, R, F>::type
		{
			using is_receiver = std::true_type;
			using _storage_t = typename storage<R, F>::type;

			friend constexpr env_of_t<R> tag_invoke(get_env_t, const type &r) noexcept(nothrow_tag_invocable<get_env_t, const R &>) { return get_env(r._data->_rcv); }

			template<detail::completion_channel T, typename... Args> requires std::same_as<T, C> && std::invocable<F, Args...>
			friend constexpr void tag_invoke(T, type &&r, Args &&...args) noexcept
			{
				if constexpr (!std::is_nothrow_invocable_v<F, Args...>)
					try { r._complete(std::forward<Args>(args)...); } catch (...) { set_error(std::move(r._data->_rcv), std::current_exception()); }
				else
					r._complete(std::forward<Args>(args)...);;
			}
			template<detail::completion_channel T, typename... Args> requires(!std::same_as<T, C> && detail::callable<T, R, Args...>)
			friend constexpr void tag_invoke(T, type &&r, Args &&...args) noexcept { T{}(std::move(r._data->_rcv), std::forward<Args>(args)...); }

			template<typename... Args>
			constexpr void _complete(Args &&...args) noexcept(std::is_nothrow_invocable_v<F, Args...>)
			{
				if constexpr (std::same_as<std::invoke_result_t<F, Args...>, void>)
					(std::invoke(std::move(_data->_fn), std::forward<Args>(args)...), set_value(std::move(_data->_rcv)));
				else
					set_value(std::move(_data->_rcv), std::invoke(std::move(_data->_fn), std::forward<Args>(args)...));
			}

			_storage_t *_data;
		};

		template<typename C, typename S, typename F>
		struct sender<C, S, F>::type
		{
			using is_sender = std::true_type;

			using _assert_invocable_function = detail::gather_signatures_t<C, S, empty_env, detail::bind_front<std::is_invocable, F>::template type, std::conjunction>;
			static_assert(_assert_invocable_function::value, "Functor passed to rod::then must be invocable with completion results of the upstream sender");

			template<typename... Ts>
			using _input_tuple = C(Ts...);
			template<typename... Ts>
			using _input_variant = completion_signatures<Ts...>;
			template<typename T, typename E>
			using _input_signs_t = detail::gather_signatures_t<C, copy_cvref_t<T, S>, E, _input_tuple, _input_variant>;

			template<typename T, typename E>
			using _signs_t = typename deduce_signs<C, S, F>::template type<T, E>;
			template<typename T, typename R>
			using _operation_t = typename operation<C, copy_cvref_t<T, S>, R, F>::type;
			template<typename R>
			using _receiver_t = typename receiver<C, R, F>::type;

			friend constexpr env_of_t<S> tag_invoke(get_env_t, const type &s) noexcept(nothrow_tag_invocable<get_env_t, const S &>) { return get_env(s._snd); }

			template<decays_to<type> T, typename E>
			friend constexpr _signs_t<T, E> tag_invoke(get_completion_signatures_t, T &&, E) noexcept { return {}; }
			template<decays_to<type> T, rod::receiver Rcv>
			friend constexpr _operation_t<T, Rcv> tag_invoke(connect_t, T &&s, Rcv r)/* noexcept(std::is_nothrow_constructible_v<_operation_t<T, Rcv>, copy_cvref_t<T, S>, Rcv, copy_cvref_t<T, F>>)*/
			{
				static_assert(receiver_of<_receiver_t<Rcv>, _input_signs_t<T, env_of_t<Rcv>>>);
				return _operation_t<T, Rcv>(std::forward<T>(s)._snd, std::move(r), std::forward<T>(s)._fn);
			}

			ROD_NO_UNIQUE_ADDRESS S _snd;
			ROD_NO_UNIQUE_ADDRESS F _fn;
		};

		template<typename C>
		class upon_channel
		{
			template<typename S>
			using completion_scheduler = decltype(get_completion_scheduler<C>(get_env(std::declval<S>())));
			template<typename F>
			using back_adaptor = detail::back_adaptor<upon_channel<C>, std::decay_t<F>>;
			template<typename S, typename F>
			using sender_t = typename sender<C, S, std::decay_t<F>>::type;

		public:
			template<rod::sender S, movable_value F> requires detail::tag_invocable_with_completion_scheduler<upon_channel<C>, C, S, S, F>
			[[nodiscard]] constexpr rod::sender auto operator()(S &&snd, F &&fn) const noexcept(nothrow_tag_invocable<upon_channel<C>, completion_scheduler<S>, S, F>)
			{
				return tag_invoke(*this, get_completion_scheduler<C>(get_env(snd)), std::forward<S>(snd), std::forward<F>(fn));
			}
			template<rod::sender S, movable_value F> requires(!detail::tag_invocable_with_completion_scheduler<upon_channel<C>, C, S, S, F> && tag_invocable<upon_channel<C>, S, F>)
			[[nodiscard]] constexpr rod::sender auto operator()(S &&snd, F &&fn) const noexcept(nothrow_tag_invocable<upon_channel<C>, S, F>)
			{
				return tag_invoke(*this, std::forward<S>(snd), std::forward<F>(fn));
			}
			template<rod::sender S, movable_value F> requires(!detail::tag_invocable_with_completion_scheduler<upon_channel<C>, C, S, S, F> && !tag_invocable<upon_channel<C>, S, F>)
			[[nodiscard]] constexpr sender_t<S, F> operator()(S &&snd, F &&fn) const noexcept(std::is_nothrow_constructible_v<sender_t<S, F>, S, F>)
			{
				return sender_t<S, F>{std::forward<S>(snd), std::forward<F>(fn)};
			}

			template<movable_value F>
			[[nodiscard]] constexpr back_adaptor<F> operator()(F &&fn) const noexcept(std::is_nothrow_constructible_v<back_adaptor<F>, upon_channel<C>, F>)
			{
				return {*this, std::forward<F>(fn)};
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

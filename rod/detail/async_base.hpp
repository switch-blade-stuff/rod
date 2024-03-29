/*
 * Created by switchblade on 2023-04-09.
 */

#pragma once

#include "../utility.hpp"
#include "../result.hpp"

namespace rod
{
	inline namespace _get_env
	{
		struct empty_env {};
		struct get_env_t
		{
			template<typename T> requires tag_invocable<get_env_t, const T &>
			[[nodiscard]] constexpr decltype(auto) operator()(const T &value) const noexcept(nothrow_tag_invocable<get_env_t, const T &>) { return tag_invoke(*this, value); }
			template<typename T> requires(!tag_invocable<get_env_t, const T &>)
			[[nodiscard]] constexpr empty_env operator()(const T &) const noexcept { return {}; }
		};
	}

	/** Customization point object for which expression `get_env(value)` is equivalent to the result of expression `tag_invoke(get_env, value)`,
	 * or an empty environment placeholder if said expression is not valid. */
	inline constexpr auto get_env = get_env_t{};
	/** Alias for `decltype(get_env(std::declval&lt;T&gt;()))` */
	template<typename T>
	using env_of_t = decltype(get_env(std::declval<T>()));

	namespace _channels
	{
		struct completion_channel_base {};

		template<typename T>
		concept completion_receiver = !(std::is_lvalue_reference_v<T> || std::is_const_v<std::remove_reference_t<T>>);

		struct set_value_t : completion_channel_base
		{
			template<completion_receiver R, typename... Vs> requires tag_invocable<set_value_t, R, Vs...>
			constexpr void operator()(R &&rcv, Vs &&...vals) const noexcept { tag_invoke(*this, std::forward<R>(rcv), std::forward<Vs>(vals)...); }
		};
		struct set_error_t : completion_channel_base
		{
			template<completion_receiver R, typename Err> requires tag_invocable<set_error_t, R, Err>
			constexpr void operator()(R &&rcv, Err &&err) const noexcept { tag_invoke(*this, std::forward<R>(rcv), std::forward<Err>(err)); }
		};
		struct set_stopped_t : completion_channel_base
		{
			template<completion_receiver R> requires tag_invocable<set_stopped_t, R>
			constexpr void operator()(R &&rcv) const noexcept { tag_invoke(*this, std::forward<R>(rcv)); }
		};
	}

	using _channels::set_value_t;
	using _channels::set_error_t;
	using _channels::set_stopped_t;

	/** Customization point object used to send a set of values through the value completion channel.
	 * @param rcv Receiver used to handle the value completion channel.
	 * @param vals Values sent through the value completion channel. */
	inline constexpr auto set_value = set_value_t{};
	/** Customization point object used to send an error through the error completion channel.
	 * @param rcv Receiver used to handle the error completion channel.
	 * @param err Error sent through the error completion channel. */
	inline constexpr auto set_error = set_error_t{};
	/** Customization point object used to send a stop signal through the stop completion channel.
	 * @param rcv Receiver used to handle the stop completion channel. */
	inline constexpr auto set_stopped = set_stopped_t{};

	namespace _detail
	{
		template<typename T, typename... Ts>
		struct make_signature { using type = T(Ts...); };
		template<typename T, std::same_as<void>... Ts>
		struct make_signature<T, Ts...> { using type = T(); };
		template<typename T, typename... Ts>
		using make_signature_t = typename make_signature<T, Ts...>::type;

		template<typename R, typename F, typename... Args> requires callable<F, Args...>
		constexpr void rcv_try_invoke(R &&rcv, F &&f, Args &&...args) noexcept
		{
			if constexpr (nothrow_callable<F, Args...>)
				f(std::forward<Args>(args)...);
			else
			{
				try { f(std::forward<Args>(args)...); }
				catch (...) { set_error(std::forward<R>(rcv), std::current_exception()); }
			}
		}

		template<typename S, typename R, typename U = std::remove_cvref_t<R>>
		concept valid_completion_for = requires(S *s) { []<typename Tag, typename... Args>(Tag (*)(Args...)) requires nothrow_tag_invocable<Tag, U, Args...> {}(s); };

		template<typename>
		struct is_completion_signature : std::false_type {};
		template<typename... Vs>
		struct is_completion_signature<set_value_t(Vs...)> : std::true_type {};
		template<typename E>
		struct is_completion_signature<set_error_t(E)> : std::true_type {};
		template<>
		struct is_completion_signature<set_stopped_t()> : std::true_type {};

		template<typename T>
		inline constexpr bool is_completion_signature_v = is_completion_signature<T>::value;

		template<typename T>
		concept completion_channel = std::derived_from<T, _channels::completion_channel_base>;
		template<typename T>
		concept completion_signature = is_completion_signature_v<T>;
	}

	/** Metaprogramming utility used to group multiple completion signatures as one type. */
	template<_detail::completion_signature...>
	struct completion_signatures {};

	/** Type trait used to check if type \a R defines a member `is_receiver` type. */
	template<typename R>
	inline constexpr bool enable_receiver = requires { typename R::is_receiver; };

	/** Concept used to define a generic receiver type. */
	template<typename R, typename U = std::remove_cvref_t<R>>
	concept receiver = enable_receiver<U> && requires(const U &r) {{ get_env(r) } -> queryable; } && std::move_constructible<U> && std::constructible_from<U, R>;

	/** Concept used to define a receiver type with a known set of completion signatures. */
	template<typename R, typename Cp>
	concept receiver_of = receiver<R> && requires { []<typename... Sigs>(completion_signatures<Sigs...>) requires(_detail::valid_completion_for<Sigs, R> && ...) {}(Cp{}); };

	/** Type trait used to check if type \a S defines a member `is_sender` type. */
	template<typename S>
	inline constexpr bool enable_sender = requires { typename S::is_sender; };

	/** Concept used to define a generic sender type. */
	template<typename S, typename U = std::remove_cvref_t<S>>
	concept sender = enable_sender<U> && requires(const U &s) {{ get_env(s) } -> queryable; } && std::move_constructible<U> && std::constructible_from<U, S>;

	inline namespace _start
	{
		struct start_t
		{
			template<typename O> requires(tag_invocable<start_t, O> && !std::is_rvalue_reference_v<O>)
			constexpr decltype(auto) operator()(O &&op) const noexcept { return tag_invoke(*this, std::forward<O>(op)); }
		};
	}

	/** Customization point object used to start work represented by an operation state.
	 * @param op Operation state used to start the scheduled work. */
	inline constexpr auto start = start_t{};

	/** Concept used to define an operation state object type that can be used to start execution of work. */
	template<typename S>
	concept operation_state = queryable<S> && std::is_object_v<S> && requires(S &s) { start(s); };

	inline namespace _connect
	{
		struct connect_t
		{
			template<sender S, receiver R> requires tag_invocable<connect_t, S, R>
			[[nodiscard]] constexpr operation_state decltype(auto) operator()(S &&snd, R &&rcv) const noexcept(nothrow_tag_invocable<connect_t, S, R>)
			{
				return tag_invoke(*this, std::forward<S>(snd), std::forward<R>(rcv));
			}

#ifdef ROD_HAS_COROUTINES
			template<sender S, receiver R> requires(!tag_invocable<connect_t, S, R>)
			[[nodiscard]] constexpr operation_state decltype(auto) operator()(S &&snd, R &&rcv) const;
#endif
		};
	}

	/** Customization point object used to connect a sender with a receiver.
	 * @param snd Sender or an awaitable to connect with \a rcv.
	 * @param rcv Receiver to connect \a snd to.
	 * @return Operation state used to represent the sender-receiver connection. */
	inline constexpr auto connect = connect_t{};

	/** Alias for `decltype(connect(std::declval&lt;S&gt;(), std::declval&lt;R&gt;()))`. */
	template<typename S, typename R>
	using connect_result_t = decltype(connect(std::declval<S>(), std::declval<R>()));

	inline namespace _schedule
	{
		struct schedule_t
		{
			template<typename S> requires tag_invocable<schedule_t, S>
			[[nodiscard]] constexpr rod::sender auto operator()(S &&sch) const noexcept(nothrow_tag_invocable<schedule_t, S>)
			{
				return tag_invoke(*this, std::forward<S>(sch));
			}
		};
	}

	/** Customization point object used to obtain a sender from the passed scheduler.
	 * @param sch Scheduler to create a sender from.
	 * @return Sender created from \a sch. */
	inline constexpr auto schedule = schedule_t{};
	/** Alias for the sender type of scheduler `S` obtained via a call to `schedule(S)`. */
	template<typename S>
	using schedule_result_t = decltype(schedule(std::declval<S>()));

	inline namespace _schedule
	{
		struct schedule_after_t
		{
			template<typename S, typename Dur> requires tag_invocable<schedule_after_t, S, Dur>
			[[nodiscard]] constexpr rod::sender auto operator()(S &&sch, Dur &&dur) const noexcept(nothrow_tag_invocable<schedule_after_t, S, Dur>)
			{
				return tag_invoke(*this, std::forward<S>(sch), std::forward<Dur>(dur));
			}
		};
	}

	/** Customization point object used to obtain a sender from the passed scheduler that completes after the specified duration.
	 * @param sch Scheduler to create a sender from.
	 * @param dur Duration specifying the difference from now until completion of the scheduler.
	 * @return Sender created from \a sch. */
	inline constexpr auto schedule_after = schedule_after_t{};
	/** Alias for the sender type of scheduler `S` obtained via a call to `schedule_after(S, Dur)`. */
	template<typename S, typename Dur>
	using schedule_after_result_t = decltype(schedule_after(std::declval<S>(), std::declval<Dur>()));

	inline namespace _schedule
	{
		struct schedule_at_t
		{
			template<typename S, typename Tp> requires tag_invocable<schedule_at_t, S, Tp>
			[[nodiscard]] constexpr rod::sender auto operator()(S &&sch, Tp &&tp) const noexcept(nothrow_tag_invocable<schedule_at_t, S, Tp>)
			{
				return tag_invoke(*this, std::forward<S>(sch), std::forward<Tp>(tp));
			}
		};
	}

	/** Customization point object used to obtain a sender from the passed scheduler that completes after the specified time point.
	 * @param sch Scheduler to create a sender from.
	 * @param tp Time point to schedule the sender at.
	 * @return Sender created from \a sch. */
	inline constexpr auto schedule_at = schedule_at_t{};
	/** Alias for the sender type of scheduler `S` obtained via a call to `schedule_at(S, TP)`. */
	template<typename S, typename Tp>
	using schedule_at_result_t = decltype(schedule_at(std::declval<S>(), std::declval<Tp>()));
}

/*
 * Created by switchblade on 2023-04-09.
 */

#pragma once

#include <system_error>

#include "operation.hpp"
#include "receiver.hpp"

namespace rod
{
	/** Concept used to define a generic sender type. */
	template<typename S, typename U = std::remove_cvref_t<S>>
	concept sender = requires (const U &s) { { get_env(s) } -> queryable; } && std::move_constructible<U> && std::constructible_from<U, S>;

	/** Concept used to define a sender type who's environment specializes the `get_completion_signatures` customization point. */
	template<typename S, typename E = detail::empty_env_t>
	concept sender_in = sender<S> && requires (S &&s, E &&e) { { get_completion_signatures(std::forward<S>(s), std::forward<E>(e)) } -> detail::instance_of<completion_signatures>; };

	namespace detail
	{
		struct connect_awaitable_t
		{
			template<typename R>
			class promise;

			template<typename R>
			using operation = coroutine_operation<promise<R>>;
			template<typename R>
			class promise : initial_promise
			{
			public:
				friend auto tag_invoke(get_env_t, const promise &p) -> env_of_t<R> { return get_env(p.m_rcv); }

			public:
				promise(auto &, R &r) noexcept : m_rcv(r) {}

				[[nodiscard]] operation<R> get_return_object() noexcept {return {std::coroutine_handle<promise>::from_promise(*this)};}
				[[nodiscard]] std::coroutine_handle<> unhandled_stopped() noexcept { return (set_stopped(std::move(m_rcv)), std::noop_coroutine()); }

				template<typename A>
				decltype(auto) await_transform(A &&a) noexcept { return std::forward<A>(a); }
				template<typename A> requires tag_invocable<as_awaitable_t, A, promise &>
				decltype(auto) await_transform(A &&a) noexcept(nothrow_tag_invocable<as_awaitable_t, A, promise &>)
				{
					return tag_invoke(as_awaitable, std::forward<A>(a), *this);
				}

			private:
				R &m_rcv;
			};

			template<typename A, typename R, typename Res = await_result_t<A, promise<R>>>
			using vals_list_t = std::conditional_t<std::is_void_v<Res>, type_list_t<>, type_list_t<Res>>;

			template<typename>
			struct deduce_sigs;
			template<typename... Vs>
			struct deduce_sigs<type_list_t<Vs...>> { using type = completion_signatures<set_value_t(Vs...), set_error_t(std::exception_ptr), set_stopped_t()>; };
			template<typename Vals>
			using deduce_sigs_t = typename deduce_sigs<Vals>::type;

			template<typename A, typename R>
			[[nodiscard]] operation<R> operator()(A a, R r) requires is_awaitable<A, promise<R>> && receiver_of<R, deduce_sigs_t<vals_list_t<A, R>>>
			{
				std::exception_ptr ep;
				try
				{
					if constexpr (std::is_void_v<await_result_t<A, promise<R>>>)
						co_await (co_await std::move(a), suspend_invoke(set_value, std::move(r)));
					else
						co_await suspend_invoke(set_value, std::move(r), co_await std::move(a));
				}
				catch(...) { ep = std::current_exception(); }

				co_await suspend_invoke(set_error, std::move(r), std::move(ep));
			}
		};

		struct connect_t
		{
		private:
			template<typename S, typename R>
			constexpr static bool allow_tag_invoke = sender_in<S, env_of_t<R>> && receiver_of<R, completion_signatures_of_t<S, env_of_t<R>>> && tag_invocable<connect_t, S, R>;

		public:
			template<typename S, receiver R>
			[[nodiscard]] constexpr decltype(auto) operator()(S &&s, R &&r) const noexcept(nothrow_tag_invocable<connect_t, S, R>) requires allow_tag_invoke<S, R> { return tag_invoke(*this, std::forward<S>(s), std::forward<R>(r)); }
			template<typename S, receiver R>
			[[nodiscard]] constexpr decltype(auto) operator()(S &&s, R &&r) const noexcept(std::is_nothrow_invocable_v<connect_awaitable_t, S, R>) requires(!allow_tag_invoke<S, R>) { return connect_awaitable_t{}(s, r); }
		};
	}

	using detail::connect_t;

	/** @brief Customization point object used to connect a sender with a receiver.
	 *
	 * Expression `connect(s, r)` for sub-expressions `s` and `r` where `r` satisfies the `receiver` concept is equivalent to one of the following:
	 * <ul>
	 * <li>If expression `tag_invoke(connect, s, r)` is well-formed, equivalent to the result of the expression (note: type of the expression must satisfy `operation_state`).</li>
	 * <li>Operation state task resulting from invocation of an exposition-only coroutine `connect-awaitable(s, r)` if `s` is awaitable using the `connect-awaitable-promise` promise type.</li>
	 * </ul> */
	inline constexpr auto connect = connect_t{};
	/** Alias for `decltype(connect(std::declval<S>(), std::declval<R>()))`. */
	template<typename S, typename R>
	using connect_result_t = decltype(connect(std::declval<S>(), std::declval<R>()));

	/** Concept used to define a sender type `S` that can be connected to receiver `R`. */
	template<typename S, typename R>
	concept sender_to = sender_in<S, env_of_t<R>> && receiver_of<R, completion_signatures_of_t<S, env_of_t<R>>> && requires (S &&s, R &&r) { connect(std::forward<S>(s), std::forward<R>(r)); };

	namespace detail
	{
		template<typename, template<typename...> typename, typename, typename...>
		struct gather_signatures;
		template<typename Tag, template<typename...> typename T, template<typename...> typename V, typename... Ts, typename... Us, typename... Fs>
		struct gather_signatures<Tag, T, V<Ts...>, completion_signatures<Tag(Us...), Fs...>> : gather_signatures<Tag, T, V<Ts..., T<Us...>>, completion_signatures<Fs...>> {};
		template<typename Tag, template<typename...> typename T, template<typename...> typename V, typename... Ts, typename F, typename... Fs>
		struct gather_signatures<Tag, T, V<Ts...>, completion_signatures<F, Fs...>> : gather_signatures<Tag, T, V<Ts...>, completion_signatures<Fs...>> {};
		template<typename Tag, template<typename...> typename T, template<typename...> typename V, typename... Ts>
		struct gather_signatures<Tag, T, V<Ts...>, completion_signatures<>> { using type = V<Ts...>; };

		template<typename Tag, typename S, typename E, template<typename...> typename Tuple, template<typename...> typename Variant> requires sender_in<S, E>
		using gather_signatures_t = typename gather_signatures<Tag, Tuple, Variant<>, completion_signatures_of_t<S, E>>::type;
	}

	/** Metaprogramming utility used to check if sender type \a S has a completion signature returning `set_stopped_t` given an execution environment \a E. */
	template<typename S, typename E = detail::empty_env_t>
	concept sends_stopped = sender_in<S, E> && !std::same_as<detail::type_list_t<>, detail::gather_signatures_t<set_stopped_t, S, E, detail::type_list_t, detail::type_list_t>>;

	/** Given completion signatures `Ts` obtained via `completion_signatures_of_t<S, E>`, defines an alias for
	 * `Variant<Tuple<Args0...>, ..., Tuple<ArgsN...>>` where `ArgsN` is a template pack of the `N`th completion signature returning `set_value_t`. */
	template<typename S, typename E = detail::empty_env_t, template<typename...> typename Tuple = detail::decayed_tuple, template<typename...> typename Variant = detail::variant_or_empty> requires sender_in<S, E>
	using value_types_of_t = detail::gather_signatures_t<set_value_t, S, E, Tuple, Variant>;
	/** Given completion signatures `Ts` obtained via `completion_signatures_of_t<S, E>`, defines an alias for
	 * `Variant<Tuple<Args0...>, ..., Tuple<ArgsN...>>` where `ArgsN` is a template pack of the `N`th completion signature returning `set_error_t`. */
	template<typename S, typename E = detail::empty_env_t, template<typename...> typename Tuple = detail::decayed_tuple, template<typename...> typename Variant = detail::variant_or_empty> requires sender_in<S, E>
	using error_types_of_t = detail::gather_signatures_t<set_error_t, S, E, Tuple, Variant>;

	namespace detail
	{
		template<typename>
		struct deduce_single_sender_value_type;
		template<template<typename...> typename V, template<typename...> typename T, typename U>
		struct deduce_single_sender_value_type<V<T<U>>> { using type = U; };
		template<template<typename...> typename V, template<typename...> typename T>
		struct deduce_single_sender_value_type<V<T<>>> { using type = void; };
		template<template<typename...> typename V>
		struct deduce_single_sender_value_type<V<>> { using type = void; };

		template<typename S, typename E>
		using single_sender_value_type = typename deduce_single_sender_value_type<value_types_of_t<S, E>>::type;
		
		template<typename Err>
		[[nodiscard]] constexpr decltype(auto) as_except_ptr(Err &&err) noexcept
		{
			if constexpr (std::same_as<std::decay_t<Err>, std::exception_ptr>)
				return std::forward<Err>(err);
			else if constexpr (std::same_as<std::decay_t<Err>, std::error_code>)
				return std::make_exception_ptr(std::system_error(std::forward<Err>(err)));
			else
				return std::make_exception_ptr(std::forward<Err>(err));
		}

		template<typename S, typename P>
		class sender_awaitable
		{
			static_assert(has_env<P>, "Awaitable promise type must provide a specialization of the `get_env` query object");

			struct unit {};
			using value_t = single_sender_value_type<S, deduce_env_of<P>>;
			using result_t = std::conditional_t<std::is_void_v<value_t>, unit, value_t>;

			struct receiver
			{
				[[nodiscard]] friend constexpr decltype(auto) tag_invoke(get_env_t, const receiver &cr) noexcept
				{
					return get_env(std::as_const(cr.continuation.promise()));
				}
				template<typename T, typename Env, typename... Args>
				friend constexpr decltype(auto) tag_invoke(T tag, Env &&env, Args &&...args) noexcept requires(std::same_as<std::decay_t<Env>, std::decay_t<env_of_t<receiver>>>)
				{
					return tag(std::forward<Env>(env), std::forward<Args>(args)...);
				}

				template<typename... Vs>
				friend constexpr void tag_invoke(set_value_t, receiver &&r, Vs &&...vs) noexcept requires std::constructible_from<result_t, Vs...>
				{
					try { r.result_ptr->template emplace<1>(std::forward<Vs>(vs)...); }
					catch(...) { r.result_ptr->emplace<2>(std::current_exception()); }
					r.continuation.resume();
				}
				template<typename Err>
				friend constexpr void tag_invoke(set_error_t, receiver &&r, Err &&err) noexcept
				{
					r.result_ptr->emplace<2>(as_except_ptr(std::forward<Err>(err)));
					r.continuation.resume();
				}
				friend constexpr void tag_invoke(set_stopped_t, receiver &&r) noexcept
				{
					static_cast<std::coroutine_handle<>>(r.continuation.promise().unhandled_stopped()).resume();
				}

				std::variant<std::monostate, result_t, std::exception_ptr> *result_ptr;
				std::coroutine_handle<P> continuation;
			};

		public:
			constexpr sender_awaitable(S &&s, P &p) : m_state(connect(std::forward<S>(s), receiver{&m_result, std::coroutine_handle<P>::from_promise(p)})) {}

			constexpr bool await_ready() const noexcept { return false; }
			void await_suspend(std::coroutine_handle<P>) noexcept { start(m_state); }

			constexpr value_t await_resume()
			{
				if (m_result.index() == 2) [[unlikely]]
					rethrow_exception(std::get<2>(m_result));
				else if constexpr (!std::is_void_v<value_t>)
					return std::forward<value_t>(std::get<1>(m_result));
			}

		private:
			std::variant<std::monostate, result_t, std::exception_ptr> m_result = {};
			connect_result_t<S, receiver> m_state;
		};

		template<typename S, typename E>
		concept single_sender = sender_in<S, E> && requires { typename single_sender_value_type<S, E>; };
		template<typename S, typename P>
		concept awaitable_sender = single_sender<S, deduce_env_of<P>> && sender_to<S, typename sender_awaitable<S, P>::receiver> && requires (P &p)
		{
			{ p.unhandled_stopped() } -> std::convertible_to<std::coroutine_handle<>>;
		};

		template<typename T, typename P>
		constexpr bool as_awaitable_t::test_nothrow_invoke() noexcept
		{
			if constexpr (tag_invocable<as_awaitable_t, T, P &>)
				return nothrow_tag_invocable<as_awaitable_t, T, P &>;
			else if constexpr (is_awaitable<T, P>)
				return std::is_nothrow_constructible_v<sender_awaitable<T, P>, T, std::coroutine_handle<P>>;
			else
				return true;
		}
		template<typename T, typename P>
		constexpr decltype(auto) as_awaitable_t::operator()(T &&t, P &p) const noexcept(test_nothrow_invoke<T, P>())
		{
			if constexpr (tag_invocable<as_awaitable_t, T, P &>)
			{
				using result_t = tag_invoke_result_t<as_awaitable_t, T, P &>;
				static_assert(is_awaitable<result_t, P>);
				return tag_invoke(*this, std::forward<T>(t), p);
			}
			else if constexpr (!is_awaitable<T> && awaitable_sender<T, P>)
				return sender_awaitable<T, P>{std::forward<T>(t), std::coroutine_handle<P>::from_promise(p)};
			else
				return std::forward<T>(t);
		}
	}

	namespace detail
	{
		class read_t
		{
			template<typename>
			class sender;

			template<typename T, typename R>
			class operation : ebo_helper<R>
			{
				friend class sender<T>;

			public:
				using ebo_helper<R>::ebo_helper;

				friend constexpr void tag_invoke(start_t, operation &op) noexcept
				{
					auto &rcv = op.receiver();
					try { set_value(std::move(rcv), T{}(get_env(rcv))); }
					catch(...) { set_error(std::move(rcv), std::current_exception()); }
				}

			private:
				[[nodiscard]] constexpr auto &receiver() noexcept { ebo_helper<T>::value(); }
			};

			template<typename T>
			class sender
			{
			public:
				template<receiver R>
				[[nodiscard]] constexpr friend operation<T, std::decay_t<R>> tag_invoke(connect_t, sender, R &&r) { return {std::forward<R>(r)}; }

				template<typename Env> requires tag_invocable<T, Env>
				friend auto tag_invoke(get_completion_signatures_t, sender, Env) -> completion_signatures<set_value_t(tag_invoke_result_t<T, Env>), set_error_t(std::exception_ptr)>;
				template<typename Env> requires nothrow_tag_invocable<T, Env>
				friend auto tag_invoke(get_completion_signatures_t, sender, Env) -> completion_signatures<set_value_t(tag_invoke_result_t<T, Env>)>;

				friend constexpr empty_env_t tag_invoke(get_env_t, const sender &) noexcept { return {}; }
			};

		public:
			template<typename T>
			[[nodiscard]] constexpr sender<T> operator()(T) const noexcept { return {}; }
		};
	}

	using detail::read_t;

	/** Customization point object used to create a sender that returns a value from the associated environment through the value channel. */
	inline constexpr auto read = read_t{};

	namespace detail
	{
		struct schedule_t
		{
			template<typename S> requires tag_invocable<schedule_t, S> && sender<tag_invoke_result_t<schedule_t, S>>
			[[nodiscard]] constexpr decltype(auto) operator()(S &&s) const noexcept(nothrow_tag_invocable<schedule_t, S>)
			{
				return tag_invoke(*this, std::forward<S>(s));
			}
		};
	}

	using detail::schedule_t;

	/** Customization point object used to obtain a sender from the passed scheduler. */
	inline constexpr auto schedule = schedule_t{};

	/** Alias for the sender type of scheduler `S` obtained via a call to `schedule(S)`. */
	template<typename S>
	using schedule_result_t = decltype(schedule(std::declval<S>()));
}
/*
 * Created by switchblade on 2023-04-09.
 */

#pragma once

#include <coroutine>

#include "algorithm/connect.hpp"
#include "algorithm/opstate.hpp"
#include "query/signatures.hpp"

namespace rod
{
	namespace detail
	{
		struct awaitable_prom;
		template<typename P = awaitable_prom>
		struct awaitable_coro : std::coroutine_handle<P> { using promise_type = P; };
		struct awaitable_prom
		{
			void return_void() {}
			void unhandled_exception() {}
			std::suspend_never final_suspend() noexcept { return {}; }
			std::suspend_never initial_suspend() noexcept { return {}; }
			awaitable_coro<> get_return_object() { return {awaitable_coro<>::from_promise(*this)}; }
		};

		template<typename P, typename T>
		inline awaitable_coro<P> is_awaitable_test(T &&t) { co_await t; }

		template<typename P>
		concept has_await_transform = requires(P p) { p.await_transform(); };
		template<typename T, typename P = awaitable_prom>
		concept is_awaitable = !has_await_transform<P> && requires(T t) { is_awaitable_test<P>(t); };

		template<typename P, typename E>
		using promise_awaitable_t = std::conditional_t<has_await_transform<P>, decltype(std::declval<P>().await_transform(std::declval<E>())), E>;

		template<typename T>
		inline decltype(auto) deduce_awaiter(T &&awaitable)
		{
			if constexpr (!requires { awaitable.operator co_await (); })
				return operator co_await(static_cast<T &&>(awaitable));
			else
				return awaitable.operator co_await();
		}
		template<typename T>
		using deduce_awaiter_t = decltype(deduce_awaiter(std::declval<T>()));

		template<typename T, typename P = awaitable_prom>
		using await_result_t = decltype(std::declval<deduce_awaiter_t<promise_awaitable_t<P, T>>>().await_resume());

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
	}

	inline namespace _as_awaitable
	{
		class as_awaitable_t
		{
			template<typename S, typename P>
			class awaitable
			{
				static_assert(detail::has_env<P>, "Awaitable promise type must provide a specialization of the `get_env` query object");

				struct unit {};
				using value_t = detail::single_sender_value_type<S, detail::deduce_env_of<P>>;
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
						catch (...) { r.result_ptr->emplace<2>(std::current_exception()); }
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
				constexpr awaitable(S &&s, P &p) : m_state(connect(std::forward<S>(s), receiver{&m_result, std::coroutine_handle<P>::from_promise(p)})) {}

				constexpr bool await_ready() const noexcept { return false; }
				void await_suspend(std::coroutine_handle<P>) noexcept { start(m_state); }

				constexpr value_t await_resume()
				{
					if (m_result.index() == 2)
						std::rethrow_exception(std::get<2>(m_result));
					else if constexpr (!std::is_void_v<value_t>)
						return std::forward<value_t>(std::get<1>(m_result));
				}

			private:
				std::variant<std::monostate, result_t, std::exception_ptr> m_result = {};
				connect_result_t<S, receiver> m_state;
			};

			template<typename S, typename E>
			constexpr static bool single_sender = sender_in<S, E> && requires { typename detail::single_sender_value_type<S, E>; };
			template<typename S, typename P>
			constexpr static bool awaitable_sender = single_sender<S, detail::deduce_env_of<P>> && sender_to<S, typename awaitable<S, P>::receiver> && requires(P &p)
			{
				{ p.unhandled_stopped() } -> std::convertible_to<std::coroutine_handle<>>;
			};

			template<typename T, typename P>
			[[nodiscard]] static constexpr bool test_nothrow_invoke() noexcept
			{
				if constexpr (tag_invocable<as_awaitable_t, T, P &>)
					return nothrow_tag_invocable<as_awaitable_t, T, P &>;
				else if constexpr (detail::is_awaitable<T, P>)
					return std::is_nothrow_constructible_v<awaitable<T, P>, T, std::coroutine_handle<P>>;
				else
					return true;
			}

		public:
			template<typename T, typename P>
			[[nodiscard]] constexpr decltype(auto) operator()(T &&t, P &p) const noexcept(test_nothrow_invoke<T, P>())
			{
				if constexpr (tag_invocable<as_awaitable_t, T, P &>)
				{
					using result_t = tag_invoke_result_t<as_awaitable_t, T, P &>;
					static_assert(detail::is_awaitable<result_t, P>);
					return tag_invoke(*this, std::forward<T>(t), p);
				}
				else if constexpr (!detail::is_awaitable<T> && awaitable_sender<T, P>)
					return awaitable<T, P>{std::forward<T>(t), std::coroutine_handle<P>::from_promise(p)};
				else
					return std::forward<T>(t);
			}
		};
	}

	/** Utility used to transform an object into a form awaitable within a coroutine. */
	inline constexpr auto as_awaitable = as_awaitable_t{};

	namespace detail
	{
		template<typename P>
		class awaitable_operation
		{
		public:
			using promise_type = P;

		public:
			awaitable_operation() = delete;

			awaitable_operation(std::coroutine_handle<> handle) noexcept : m_handle(handle) {}
			awaitable_operation(awaitable_operation &&other) noexcept : m_handle(std::exchange(other.coro, {})) {}
			~awaitable_operation() { if (m_handle) m_handle.destroy(); }

			friend void tag_invoke(start_t, awaitable_operation &op) noexcept { op.coro.resume(); }

		private:
			std::coroutine_handle<> m_handle;
		};

		template<typename = void>
		class awaitable_promise;
		template<>
		class awaitable_promise<void>
		{
			std::suspend_always initial_suspend() noexcept { return {}; }

			[[noreturn]] std::suspend_always final_suspend() noexcept { std::terminate(); }
			[[noreturn]] void unhandled_exception() noexcept { std::terminate(); }
			[[noreturn]] void return_void() noexcept { std::terminate(); }
		};
		template<typename R>
		class awaitable_promise : awaitable_promise<>
		{
		public:
			friend auto tag_invoke(get_env_t, const awaitable_promise &p) -> env_of_t<R> { return get_env(p.m_rcv); }

		public:
			awaitable_promise(auto &, R &r) noexcept : m_rcv(r) {}

			[[nodiscard]] awaitable_operation<R> get_return_object() noexcept { return {std::coroutine_handle<awaitable_promise>::from_promise(*this)}; }
			[[nodiscard]] std::coroutine_handle<> unhandled_stopped() noexcept { return (set_stopped(std::move(m_rcv)), std::noop_coroutine()); }

			template<typename A>
			decltype(auto) await_transform(A &&a) noexcept { return std::forward<A>(a); }
			template<typename A>
			requires tag_invocable<as_awaitable_t, A, awaitable_promise &>
			decltype(auto) await_transform(A &&a) noexcept(nothrow_tag_invocable<as_awaitable_t, A, awaitable_promise &>)
			{
				return tag_invoke(as_awaitable, std::forward<A>(a), *this);
			}

		private:
			R &m_rcv;
		};

		template<typename>
		struct deduce_awaitable_sigs;
		template<typename... Vs>
		struct deduce_awaitable_sigs<detail::type_list_t<Vs...>> { using type = completion_signatures<set_value_t(Vs...), set_error_t(std::exception_ptr), set_stopped_t()>; };
		template<typename A, typename R, typename Res = detail::await_result_t<A, awaitable_promise<R>>>
		using awaitable_sigs_t = typename deduce_awaitable_sigs<std::conditional_t<std::is_void_v<Res>, detail::type_list_t<>, detail::type_list_t<Res>>>::type;

		template<typename T>
		struct env_promise
		{
			friend constexpr const T &tag_invoke(get_env_t, const env_promise &p);

			template<typename E>
			constexpr decltype(auto) await_transform(E &&e)
			{
				if constexpr (tag_invocable<as_awaitable_t, E, env_promise &>)
					return tag_invoke(as_awaitable, std::forward<E>(e), *this);
				else
					return std::forward<E>(e);
			}
		};
	}

	/* `get_completion_signatures` overload for awaitable types. */
	template<typename S, typename E> requires detail::is_awaitable<S, detail::env_promise<E>>
	struct get_completion_signatures_t::_overload_hook<S, E>
	{
		using result_t = detail::await_result_t<S, detail::env_promise<E>>;

		[[nodiscard]] constexpr auto operator()(S &&, E &&) noexcept
		{
			if constexpr (!std::is_void_v<std::remove_cv_t<result_t>>)
				return completion_signatures<set_value_t(result_t), set_error_t(std::exception_ptr), set_stopped_t()>{};
			else
				return completion_signatures<set_value_t(), set_error_t(std::exception_ptr), set_stopped_t()>{};
		}
	};
	/* `connect` overload for awaitable types. */
	template<typename A, typename R> requires detail::is_awaitable<A, detail::awaitable_promise<R>> && receiver_of<R, detail::awaitable_sigs_t<A, R>>
	struct connect_t::_overload_hook<A, R>
	{
		[[nodiscard]] detail::awaitable_operation<R> operator()(auto a, auto r)
		{
			constexpr auto invoke = []<typename F, typename... Args>(F f, Args &&...args) noexcept
			{
				struct awaiter : detail::ebo_helper<F>
				{
					using detail::ebo_helper<F>::ebo_helper;

					static constexpr bool await_ready() noexcept { return false; }
					void await_suspend(std::coroutine_handle<>) noexcept { detail::ebo_helper<F>::value()(); }
					[[noreturn]] void await_resume() noexcept { std::terminate(); }
				};
				return awaiter{[&, f]() noexcept { f(std::forward<Args>(args)...); }};
			};

			std::exception_ptr ep;
			try
			{
				if constexpr (std::is_void_v<detail::await_result_t<A, detail::awaitable_promise<R>>>)
					co_await (co_await std::move(a), invoke(set_value, std::move(r)));
				else
					co_await invoke(set_value, std::move(r), co_await std::move(a));
			}
			catch (...) { ep = std::current_exception(); }
			co_await invoke(set_error, std::move(r), std::move(ep));
		}
	};
}

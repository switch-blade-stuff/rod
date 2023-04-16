/*
 * Created by switchblade on 2023-04-09.
 */

#pragma once

#include <coroutine>
#include <exception>

#include "exec_env.hpp"

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
			if constexpr (!requires { awaitable.operator co_await(); })
				return operator co_await(static_cast<T &&>(awaitable));
			else
				return awaitable.operator co_await();
		}
		template<typename T>
		using deduce_awaiter_t = decltype(deduce_awaiter(std::declval<T>()));

		template<typename T, typename P = awaitable_prom>
		using await_result_t = decltype(std::declval<deduce_awaiter_t<promise_awaitable_t<P, T>>>().await_resume());

		/* Promise type that only supports initial_invoke. */
		struct initial_promise
		{
			std::suspend_always initial_suspend() noexcept { return {}; }

			[[noreturn]] std::suspend_always final_suspend() noexcept { std::terminate(); }
			[[noreturn]] void unhandled_exception() noexcept { std::terminate(); }
			[[noreturn]] void return_void() noexcept { std::terminate(); }
		};

		/* Suspend-only awaiter type. */
		template<typename F>
		struct suspend_awaiter : ebo_helper<F>
		{
			using ebo_helper<F>::ebo_helper;

			static constexpr bool await_ready() noexcept { return false; }

			void await_suspend(std::coroutine_handle<>) noexcept { ebo_helper<F>::value()(); }
			[[noreturn]] void await_resume() noexcept { std::terminate(); }
		};
		template<typename F>
		suspend_awaiter(F &&f) -> suspend_awaiter<std::decay_t<F>>;

		/* Suspend-execution of `f(args...)` using a suspend-only awaiter. */
		template<typename F, typename... Args>
		[[nodiscard]] constexpr static auto suspend_invoke(F f, Args &&...args) noexcept { return suspend_awaiter{[&, f]() noexcept { f(std::forward<Args>(args)...); }}; }

		struct as_awaitable_t
		{
		private:
			template<typename T, typename P>
			[[nodiscard]] static constexpr bool test_nothrow_invoke() noexcept;

		public:
			template<typename T, typename P>
			[[nodiscard]] constexpr decltype(auto) operator()(T &&t, P &p) const noexcept(test_nothrow_invoke<T, P>());
		};

		template<typename T>
		template<typename E>
		constexpr decltype(auto) env_promise<T>::await_transform(E &&e)
		{
			if constexpr (requires { tag_invoke(as_awaitable_t{}, std::forward<E>(e), *this); })
				return tag_invoke(as_awaitable_t{}, std::forward<E>(e), *this);
			else
				return std::forward<E>(e);
		}
	}

	using detail::as_awaitable_t;

	/** Utility used to transform an invocable object into a form awaitable within a coroutine. */
	inline constexpr auto as_awaitable = as_awaitable_t{};
}

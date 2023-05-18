/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include "detail/config.hpp"

#ifdef ROD_HAS_COROUTINES

#include "scheduling.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	namespace _task
	{
		template<template<typename> typename Task, typename Base, typename T>
		class promise : public Base
		{
		public:
			using result_type = T;

		private:
			using state_t = std::optional<T>;

			using lvalue_result = std::add_lvalue_reference_t<T>;
			using rvalue_result = std::conditional_t<std::is_arithmetic_v<T> || std::is_pointer_v<T>, T, std::add_rvalue_reference_t<T>>;

		public:
			[[nodiscard]] inline Task<result_type> get_return_object() noexcept;

			template<typename V> requires std::constructible_from<T, V>
			constexpr void return_value(V &&value) noexcept(std::is_nothrow_constructible_v<T, V>) { m_state.emplace(std::forward<V>(value)); }

			[[nodiscard]] lvalue_result result() & { return (Base::rethrow_exception(), *m_state); }
			[[nodiscard]] rvalue_result result() && { return (Base::rethrow_exception(), std::move(*m_state)); }

			template<typename U>
			[[nodiscard]] decltype(auto) await_transform(U &&value)
			{
				return as_awaitable(std::forward<U>(value), *this);
			}

		private:
			state_t m_state;
		};
		template<template<typename> typename Task, typename Base, typename T>
		class promise<Task, Base, T &> : public Base
		{
		public:
			using result_type = T &;

		public:
			[[nodiscard]] Task<result_type> get_return_object() noexcept;

			constexpr void return_value(T &ref) noexcept { m_result = &ref; }
			result_type result() { return (Base::rethrow_exception(), *m_result); }

			template<typename U>
			[[nodiscard]] decltype(auto) await_transform(U &&value)
			{
				return as_awaitable(std::forward<U>(value), *this);
			}

		private:
			T *m_result = nullptr;
		};
		template<template<typename> typename Task, typename Base>
		class promise<Task, Base, void> : public Base
		{
		public:
			using result_type = void;

		public:
			[[nodiscard]] Task<result_type> get_return_object() noexcept;

			constexpr void return_void() noexcept {}
			result_type result() { Base::rethrow_exception(); }

			template<typename U>
			[[nodiscard]] decltype(auto) await_transform(U &&value)
			{
				return as_awaitable(std::forward<U>(value), *this);
			}
		};

		template<typename T>
		class task
		{
			class promise_base : public with_awaitable_senders<promise_base>
			{
				using sender_base = with_awaitable_senders<promise_base>;

				struct final_awaiter
				{
					constexpr void await_resume() noexcept {}
					constexpr bool await_ready() noexcept { return false; }

#ifdef ROD_HAS_INLINE_RESUME
					template<typename P>
					auto await_suspend(std::coroutine_handle<P> h) noexcept { return h.promise().continuation(); }
#else
					template<typename P>
					__declspec(noinline) void await_suspend(std::coroutine_handle<P> h) noexcept
					{
						if (auto &p = h.promise(); p.m_has_cont.test_and_set())
							h.promise().continuation().resume();
					}
#endif
				};

			public:
				constexpr std::suspend_always initial_suspend() noexcept { return {}; }
				constexpr final_awaiter final_suspend() noexcept { return {}; }

				void unhandled_exception() noexcept { m_err = std::current_exception(); }
				void rethrow_exception() { if (m_err) std::rethrow_exception(std::move(m_err)); }

				template<typename P>
				auto next(std::coroutine_handle<P> next) noexcept
				{
					sender_base::set_continuation(next);
#ifndef ROD_HAS_INLINE_RESUME
					return !m_has_cont.test_and_set();
#endif
				}

			private:
				std::exception_ptr m_err = {};

#ifndef ROD_HAS_INLINE_RESUME
				std::atomic_flag m_has_cont = {};
#endif
			};

			class awaitable
			{
				friend class task;

				using promise_type = promise<task, promise_base, T>;
				using result_type = typename promise_type::result_type;

			private:
				awaitable(std::coroutine_handle<promise_type> h) noexcept : m_handle(h) {}

			public:
				bool await_ready() const noexcept { return !m_handle || m_handle.done(); }
				result_type await_resume() { return m_handle.promise().result(); }

				template<typename P>
				auto await_suspend(std::coroutine_handle<P> next) noexcept
				{
#ifndef ROD_HAS_INLINE_RESUME
					return (m_handle.resume(), m_handle.promise().next(next));
#else
					return (m_handle.promise().next(next), m_handle);
#endif
				}

			private:
				std::coroutine_handle<promise_type> m_handle = {};
			};

		public:
			using promise_type = typename awaitable::promise_type;

		public:
			task(const task &) = delete;
			task &operator=(const task &) = delete;

			explicit task(std::coroutine_handle<promise_type> h) noexcept : m_handle(h) {}

			task(task &&other) noexcept : m_handle(std::exchange(other.m_handle, {})) {}
			task &operator=(task &&other) noexcept { return (swap(other), *this); }
			~task() { if (m_handle) m_handle.destroy(); }

			/** Checks if the task has completed execution. */
			[[nodiscard]] bool ready() const noexcept { return !m_handle || m_handle.done(); }

			awaitable operator co_await() const & noexcept { return {m_handle}; }
			awaitable operator co_await() const && noexcept { return {m_handle}; }

			void swap(task &other) noexcept { std::swap(m_handle, other.m_handle); }
			friend void swap(task &a, task &b) noexcept { a.swap(b); }

		private:
			std::coroutine_handle<promise_type> m_handle = {};
		};

		template<typename T>
		class shared_task
		{
			struct task_node
			{
				using stop_func = std::coroutine_handle<> (*)(void *) noexcept;

				template<typename P>
				explicit task_node(std::coroutine_handle<P> h, task_node *next = {}) noexcept : m_cont(h), m_next(next) { bind(h); }

				[[nodiscard]] std::coroutine_handle<> unhandled_stopped() noexcept { return m_stop_func(m_cont.address()); }

				template<typename P>
				void bind(std::coroutine_handle<P> h) noexcept
				{
					if constexpr (requires { h.unhandled_stopped(); })
						m_stop_func = [](void *p) noexcept { return std::coroutine_handle<P>::from_address(p).promise().unhandled_stopped(); };
					else
						m_stop_func = {};
				}

				std::coroutine_handle<> m_cont = {};
				stop_func m_stop_func = {};
				task_node *m_next = {};
			};

			class promise_base
			{
				struct final_awaiter
				{
					constexpr void await_resume() noexcept {}
					constexpr bool await_ready() noexcept { return false; }

#ifdef ROD_HAS_INLINE_RESUME
					template<typename P>
					inline void await_suspend(std::coroutine_handle<P> h) noexcept;
#else
					template<typename P>
					__declspec(noinline) void await_suspend(std::coroutine_handle<P> h) noexcept;
#endif
				};

			public:
				constexpr std::suspend_always initial_suspend() noexcept { return {}; }
				constexpr final_awaiter final_suspend() noexcept { return {}; }

				void unhandled_exception() noexcept { m_err = std::current_exception(); }
				void rethrow_exception() { if (m_err) std::rethrow_exception(std::move(m_err)); }

				[[nodiscard]] inline std::coroutine_handle<> unhandled_stopped() noexcept;

				bool ready() const noexcept { return m_queue.load(std::memory_order_acquire) == this; }
				bool release() noexcept { return m_refs.fetch_sub(1, std::memory_order_acq_rel) == 1; }
				void acquire() noexcept { m_refs.fetch_add(1, std::memory_order_relaxed); }

				bool try_push(std::coroutine_handle<> h, task_node &node) noexcept
				{
					/* Eagerly start the coroutine to prevent recursive stack overflow. */
					auto old = m_queue.load(std::memory_order_acquire);
					if (old == &m_queue && m_queue.compare_exchange_strong(old, nullptr, std::memory_order_acq_rel, std::memory_order_acquire))
					{
						h.resume();
						old = m_queue.load(std::memory_order_acquire);
					}

					while (old != this)
					{
						node.m_next = static_cast<task_node *>(old);
						if (m_queue.compare_exchange_weak(old, &node, std::memory_order_acq_rel, std::memory_order_acquire))
							return true;
					}
					return false;
				}

			private:
				template<std::invocable<task_node *> F>
				void for_each(F &&func) noexcept
				{
					if (const auto queue = m_queue.exchange(this, std::memory_order_acq_rel); queue)
					{
						auto node = static_cast<task_node *>(queue);
						for (;;)
						{
							const auto next = node->m_next;
							if (next == nullptr) break;

							func(node);
							node = next;
						}
						/* Enable tailcall optimization. */
						func(node);
					}
				}

				std::atomic<std::size_t> m_refs = 1;
				std::atomic<void *> m_queue = &m_queue;
				std::exception_ptr m_err;
			};

			class awaitable
			{
				friend class shared_task;

				using promise_type = promise<shared_task, promise_base, T>;
				using result_type = typename promise_type::result_type;

			private:
				awaitable(std::coroutine_handle<promise_type> h) noexcept : m_handle(h) {}

			public:
				bool await_ready() const noexcept { return !m_handle || m_handle.done(); }
				result_type await_resume() { return m_handle.promise().result(); }

				template<typename P>
				auto await_suspend(std::coroutine_handle<P> next) { return ((m_node.m_cont = next), m_handle.promise().try_push(m_handle, m_node)); }

			private:
				std::coroutine_handle<promise_type> m_handle = {};
				task_node m_node = {};
			};

		public:
			using promise_type = typename awaitable::promise_type;

		public:
			shared_task() = delete;

			explicit shared_task(std::coroutine_handle<promise_type> h) : m_handle(h) {}

			shared_task(shared_task &&other) noexcept : m_handle(std::exchange(other.m_handle, {})) {}
			shared_task &operator=(shared_task &&other) noexcept
			{
				if (this != &other) swap(other);
				return *this;
			}

			shared_task(const shared_task &other) noexcept : m_handle(other.m_handle)
			{
				if (m_handle) m_handle.promise().acquire();
			}
			shared_task& operator=(const shared_task &other) noexcept
			{
				if (m_handle != other.m_handle)
				{
					destroy();
					if ((m_handle = other.m_handle))
						m_handle.promise().acquire();
				}

				return *this;
			}

			~shared_task() { destroy(); }

			/** Checks if the task has completed execution. */
			[[nodiscard]] bool ready() const noexcept { return !m_handle || m_handle.promise().ready(); }

			awaitable operator co_await() const & noexcept { return {m_handle}; }
			awaitable operator co_await() const && noexcept { return {m_handle}; }

			void swap(shared_task &other) noexcept { std::swap(m_handle, other.m_handle); }
			friend void swap(shared_task &a, shared_task &b) noexcept { a.swap(b); }

			[[nodiscard]] friend bool operator==(const shared_task <T> &a, const shared_task<T> &b) noexcept { return a.m_handle == b.m_handle; }

		private:
			void destroy() noexcept { if (m_handle && m_handle.promise().release()) m_handle.destroy(); }

			std::coroutine_handle<promise_type> m_handle = {};
		};

		template<typename T>
		std::coroutine_handle<> shared_task<T>::promise_base::unhandled_stopped() noexcept
		{
			for_each([](task_node *node) { node->unhandled_stopped(); });
			return std::noop_coroutine();
		}

		template<typename T>
		template<typename P>
		void shared_task<T>::promise_base::final_awaiter::await_suspend(std::coroutine_handle<P> h) noexcept
		{
			h.promise().for_each([](task_node *node) { node->m_cont.resume(); });
		}

		template<template<typename> typename Task, typename Base, typename T>
		Task<T> promise<Task, Base, T>::get_return_object() noexcept { return Task<T>{std::coroutine_handle<promise>::from_promise(*this)}; }
		template<template<typename> typename Task, typename Base, typename T>
		Task<T &> promise<Task, Base, T &>::get_return_object() noexcept { return Task<T &>{std::coroutine_handle<promise>::from_promise(*this)}; }
		template<template<typename> typename Task, typename Base>
		Task<void> promise<Task, Base, void>::get_return_object() noexcept { return Task<void>{std::coroutine_handle<promise>::from_promise(*this)}; }
	}

	/** Lazily-evaluated coroutine returning \a T as the promised result type. */
	template<typename T = void>
	using task = _task::task<T>;
	/** Task that can be atomically shared and awaited by multiple coroutines. */
	template<typename T = void>
	using shared_task = _task::shared_task<T>;
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include "detail/config.hpp"

#ifdef ROD_HAS_COROUTINES

#include "scheduling.hpp"

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
			constexpr void return_value(V &&value) noexcept(std::is_nothrow_constructible_v<T, V>) { _state.emplace(std::forward<V>(value)); }

			[[nodiscard]] lvalue_result result() & { return (Base::rethrow_exception(), *_state); }
			[[nodiscard]] rvalue_result result() && { return (Base::rethrow_exception(), std::move(*_state)); }

			template<typename U>
			[[nodiscard]] decltype(auto) await_transform(U &&value)
			{
				return as_awaitable(std::forward<U>(value), *this);
			}

		private:
			state_t _state;
		};
		template<template<typename> typename Task, typename Base, typename T>
		class promise<Task, Base, T &> : public Base
		{
		public:
			using result_type = T &;

		public:
			[[nodiscard]] inline Task<result_type> get_return_object() noexcept;

			constexpr void return_value(T &ref) noexcept { _result = &ref; }
			result_type result() { return (Base::rethrow_exception(), *_result); }

			template<typename U>
			[[nodiscard]] decltype(auto) await_transform(U &&value)
			{
				return as_awaitable(std::forward<U>(value), *this);
			}

		private:
			T *_result = nullptr;
		};
		template<template<typename> typename Task, typename Base>
		class promise<Task, Base, void> : public Base
		{
		public:
			using result_type = void;

		public:
			[[nodiscard]] inline Task<result_type> get_return_object() noexcept;

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
			template<typename, typename...>
			friend struct std::coroutine_traits;

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
						if (auto &p = h.promise(); p._has_cont.test_and_set())
							h.promise().continuation().resume();
					}
#endif
				};

			public:
				constexpr std::suspend_always initial_suspend() noexcept { return {}; }
				constexpr final_awaiter final_suspend() noexcept { return {}; }

				void unhandled_exception() noexcept { _err = std::current_exception(); }
				void rethrow_exception() { if (_err) std::rethrow_exception(std::move(_err)); }

				template<typename P>
				auto next(std::coroutine_handle<P> next) noexcept
				{
					sender_base::set_continuation(next);
#ifndef ROD_HAS_INLINE_RESUME
					return !_has_cont.test_and_set();
#endif
				}

			private:
				std::exception_ptr _err = {};

#ifndef ROD_HAS_INLINE_RESUME
				std::atomic_flag _has_cont = {};
#endif
			};

			class awaitable
			{
				friend class task;

				using promise_type = promise<task, promise_base, T>;
				using result_type = typename promise_type::result_type;

			private:
				constexpr awaitable(std::coroutine_handle<promise_type> h) noexcept : _handle(h) {}

			public:
				bool await_ready() const noexcept { return !_handle || _handle.done(); }
				result_type await_resume() { return _handle.promise().result(); }

				template<typename P>
				auto await_suspend(std::coroutine_handle<P> next) noexcept
				{
#ifndef ROD_HAS_INLINE_RESUME
					return (_handle.resume(), _handle.promise().next(next));
#else
					return (_handle.promise().next(next), _handle);
#endif
				}

			private:
				std::coroutine_handle<promise_type> _handle = {};
			};

		public:
			using promise_type = typename awaitable::promise_type;

		public:
			task(const task &) = delete;
			task &operator=(const task &) = delete;

			task(task &&other) noexcept : _handle(std::exchange(other._handle, {})) {}
			task &operator=(task &&other) noexcept { return (swap(other), *this); }

			template<std::derived_from<promise_type> P>
			constexpr explicit task(std::coroutine_handle<P> h) noexcept : _handle(std::coroutine_handle<promise_type>::from_address(h.address())) {}

			~task() { if (_handle) _handle.destroy(); }

			/** Checks if the task has completed execution. */
			[[nodiscard]] bool ready() const noexcept { return !_handle || _handle.done(); }

			constexpr awaitable operator co_await() const & noexcept { return {_handle}; }
			constexpr awaitable operator co_await() const && noexcept { return {_handle}; }

			constexpr void swap(task &other) noexcept { std::swap(_handle, other._handle); }
			friend constexpr void swap(task &a, task &b) noexcept { a.swap(b); }

		private:
			std::coroutine_handle<promise_type> _handle = {};
		};

		template<typename T>
		class shared_task
		{
			template<typename, typename...>
			friend struct std::coroutine_traits;

			struct task_node
			{
				using stop_func = std::coroutine_handle<> (*)(void *) noexcept;

				template<typename P>
				explicit task_node(std::coroutine_handle<P> h, task_node *next = {}) noexcept : _cont(h), _next(next) { bind(h); }

				[[nodiscard]] std::coroutine_handle<> unhandled_stopped() noexcept { return _stop_func(_cont.address()); }

				template<typename P>
				void bind(std::coroutine_handle<P> h) noexcept
				{
					if constexpr (requires { h.unhandled_stopped(); })
						_stop_func = [](void *p) noexcept { return std::coroutine_handle<P>::from_address(p).promise().unhandled_stopped(); };
					else
						_stop_func = {};
				}

				std::coroutine_handle<> _cont = {};
				stop_func _stop_func = {};
				task_node *_next = {};
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

				void unhandled_exception() noexcept { _err = std::current_exception(); }
				void rethrow_exception() { if (_err) std::rethrow_exception(std::move(_err)); }

				[[nodiscard]] inline std::coroutine_handle<> unhandled_stopped() noexcept;

				bool ready() const noexcept { return _queue.load(std::memory_order_acquire) == this; }
				bool release() noexcept { return _refs.fetch_sub(1, std::memory_order_acq_rel) == 1; }
				void acquire() noexcept { _refs.fetch_add(1, std::memory_order_relaxed); }

				bool try_push(std::coroutine_handle<> h, task_node &node) noexcept
				{
					/* Eagerly start the coroutine to prevent recursive stack overflow. */
					auto old = _queue.load(std::memory_order_acquire);
					if (old == &_queue && _queue.compare_exchange_strong(old, nullptr, std::memory_order_acq_rel, std::memory_order_acquire))
					{
						h.resume();
						old = _queue.load(std::memory_order_acquire);
					}

					while (old != this)
					{
						node._next = static_cast<task_node *>(old);
						if (_queue.compare_exchange_weak(old, &node, std::memory_order_acq_rel, std::memory_order_acquire))
							return true;
					}
					return false;
				}

			private:
				template<std::invocable<task_node *> F>
				void for_each(F &&func) noexcept
				{
					if (const auto queue = _queue.exchange(this, std::memory_order_acq_rel); queue)
					{
						auto node = static_cast<task_node *>(queue);
						for (;;)
						{
							const auto next = node->_next;
							if (next == nullptr) break;

							func(node);
							node = next;
						}
						/* Enable tailcall optimization. */
						func(node);
					}
				}

				std::atomic<std::size_t> _refs = 1;
				std::atomic<void *> _queue = &_queue;
				std::exception_ptr _err;
			};

			class awaitable
			{
				friend class shared_task;

				using promise_type = promise<shared_task, promise_base, T>;
				using result_type = typename promise_type::result_type;

			private:
				constexpr awaitable(std::coroutine_handle<promise_type> h) noexcept : _handle(h) {}

			public:
				bool await_ready() const noexcept { return !_handle || _handle.done(); }
				result_type await_resume() { return _handle.promise().result(); }

				template<typename P>
				auto await_suspend(std::coroutine_handle<P> next) { return ((_node._cont = next), _handle.promise().try_push(_handle, _node)); }

			private:
				std::coroutine_handle<promise_type> _handle = {};
				task_node _node = {};
			};

		public:
			using promise_type = typename awaitable::promise_type;

		public:
			shared_task() = delete;

			shared_task(shared_task &&other) noexcept : _handle(std::exchange(other._handle, {})) {}
			shared_task &operator=(shared_task &&other) noexcept
			{
				if (this != &other) swap(other);
				return *this;
			}

			shared_task(const shared_task &other) noexcept : _handle(other._handle)
			{
				if (_handle) _handle.promise().acquire();
			}
			shared_task& operator=(const shared_task &other) noexcept
			{
				if (_handle != other._handle)
				{
					destroy();
					if ((_handle = other._handle))
						_handle.promise().acquire();
				}

				return *this;
			}

			template<std::derived_from<promise_type> P>
			constexpr explicit shared_task(std::coroutine_handle<P> h) noexcept : _handle(std::coroutine_handle<promise_type>::from_address(h.address())) {}

			~shared_task() { destroy(); }

			/** Checks if the task has completed execution. */
			[[nodiscard]] bool ready() const noexcept { return !_handle || _handle.promise().ready(); }

			constexpr awaitable operator co_await() const & noexcept { return {_handle}; }
			constexpr awaitable operator co_await() const && noexcept { return {_handle}; }

			[[nodiscard]] constexpr bool operator==(const shared_task &other) const noexcept { return _handle == other._handle; }

			constexpr void swap(shared_task &other) noexcept { std::swap(_handle, other._handle); }
			friend constexpr void swap(shared_task &a, shared_task &b) noexcept { a.swap(b); }

		private:
			void destroy() noexcept { if (_handle && _handle.promise().release()) _handle.destroy(); }

			std::coroutine_handle<promise_type> _handle = {};
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
			h.promise().for_each([](task_node *node) { node->_cont.resume(); });
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

template<typename T, typename... Args>
struct std::coroutine_traits<rod::task<T>, Args...> { using promise_type = typename rod::task<T>::promise_type; };
template<typename T, typename Alloc, typename... Args>
struct std::coroutine_traits<rod::task<T>, std::allocator_arg_t, Alloc, Args...> { using promise_type = rod::detail::promise_with_allocator<typename rod::task<T>::promise_type, std::decay_t<Alloc>>; };
template<typename T, typename U, typename Alloc, typename... Args>
struct std::coroutine_traits<rod::task<T>, U, std::allocator_arg_t, Alloc, Args...> { using promise_type = rod::detail::promise_with_allocator<typename rod::task<T>::promise_type, std::decay_t<Alloc>>; };

template<typename T, typename... Args>
struct std::coroutine_traits<rod::shared_task<T>, Args...> { using promise_type = typename rod::shared_task<T>::promise_type; };
template<typename T, typename Alloc, typename... Args>
struct std::coroutine_traits<rod::shared_task<T>, std::allocator_arg_t, Alloc, Args...> { using promise_type = rod::detail::promise_with_allocator<typename rod::shared_task<T>::promise_type, std::decay_t<Alloc>>; };
template<typename T, typename U, typename Alloc, typename... Args>
struct std::coroutine_traits<rod::shared_task<T>, U, std::allocator_arg_t, Alloc, Args...> { using promise_type = rod::detail::promise_with_allocator<typename rod::shared_task<T>::promise_type, std::decay_t<Alloc>>; };

#endif

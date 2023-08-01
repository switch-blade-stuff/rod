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
		template<typename T>
		class promise_storage
		{
			using lvalue_result = std::add_lvalue_reference_t<T>;
			using rvalue_result = std::conditional_t<std::is_arithmetic_v<T> || std::is_pointer_v<T>, T, std::add_rvalue_reference_t<T>>;

		public:
#ifdef NDEBUG
			[[nodiscard]] lvalue_result result() & { return *_result; }
			[[nodiscard]] rvalue_result result() && { return *std::move(_result); }
#else
			[[nodiscard]] lvalue_result result() & { return _result.value(); }
			[[nodiscard]] rvalue_result result() && { return std::move(_result).value(); }
#endif

			template<typename V> requires std::constructible_from<T, V>
			constexpr void return_value(V &&value) noexcept(std::is_nothrow_constructible_v<T, V>) { _result.emplace(std::forward<V>(value)); }

		private:
			std::optional<T> _result;
		};
		template<typename T>
		class promise_storage<T &>
		{
		public:
			constexpr T &result() noexcept { return *_result; }
			constexpr void return_value(T &ref) noexcept { _result = &ref; }

		private:
			T *_result = nullptr;
		};
		template<>
		class promise_storage<void>
		{
		public:
			constexpr void result() noexcept {}
			constexpr void return_void() noexcept {}
		};

		template<template<typename> typename Task, typename Base, typename T, typename Alloc>
		class promise : public Base, public with_allocator_promise<promise<Task, Base, T, Alloc>, Alloc>
		{
		public:
			using result_type = T;

		public:
			using with_allocator_promise<promise<Task, Base, T, Alloc>, Alloc>::operator new;
			using with_allocator_promise<promise<Task, Base, T, Alloc>, Alloc>::operator delete;

			[[nodiscard]] inline Task<result_type> get_return_object() noexcept;
			template<typename U>
			[[nodiscard]] decltype(auto) await_transform(U &&value) { return as_awaitable(std::forward<U>(value), *this); }
		};

		template<typename T>
		class task
		{
			class promise_base : public with_awaitable_senders<promise_base>, public promise_storage<T>
			{
				using senders_base = with_awaitable_senders<promise_base>;
				using storage_base = promise_storage<T>;

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
				void rethrow_exception() { if (_err) std::rethrow_exception(_err); }

				[[nodiscard]] decltype(auto) result() & { return (rethrow_exception(), storage_base::result()); }
				[[nodiscard]] decltype(auto) result() && { return (rethrow_exception(), storage_base::result()); }

				template<typename P>
				auto next(std::coroutine_handle<P> next) noexcept
				{
					senders_base::set_continuation(next);
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

		public:
			using promise_type = promise<task, promise_base, T, std::allocator<void>>;
			template<typename Alloc>
			using allocator_promise_type = promise<task, promise_base, T, Alloc>;

		private:
			class awaitable
			{
			public:
				constexpr explicit awaitable(std::coroutine_handle<promise_base> h) noexcept : _handle(h) {}

				bool await_ready() const noexcept { return !_handle || _handle.done(); }
				decltype(auto) await_resume() { return _handle.promise().result(); }
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
				std::coroutine_handle<promise_base> _handle = {};
			};

		public:
			task(const task &) = delete;
			task &operator=(const task &) = delete;

			task(task &&other) noexcept : task(other.release()) {}
			task &operator=(task &&other) noexcept { return (swap(other), *this); }

			template<std::derived_from<promise_base> P>
			constexpr explicit task(std::coroutine_handle<P> h) noexcept : _handle(std::coroutine_handle<promise_base>::from_address(h.address())) {}
			constexpr explicit task(std::coroutine_handle<void> h) noexcept : _handle(std::coroutine_handle<promise_base>::from_address(h.address())) {}

			~task() { if (_handle) _handle.destroy(); }

			/** Checks if the task has completed execution. */
			[[nodiscard]] bool ready() const noexcept { return !_handle || _handle.done(); }

			constexpr awaitable operator co_await() const & noexcept { return awaitable{_handle}; }
			constexpr awaitable operator co_await() const && noexcept { return awaitable{_handle}; }

			/** Releases the underlying coroutine handle. */
			std::coroutine_handle<> release() { return std::exchange(_handle, std::coroutine_handle<promise_base>{}); }

			constexpr void swap(task &other) noexcept { std::swap(_handle, other._handle); }
			friend constexpr void swap(task &a, task &b) noexcept { a.swap(b); }

		private:
			std::coroutine_handle<promise_base> _handle = {};
		};

		template<typename T>
		class shared_task
		{
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

			class promise_base : public promise_storage<T>
			{
				using storage_base = promise_storage<T>;

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
				void rethrow_exception() { if (_err) std::rethrow_exception(_err); }

				[[nodiscard]] inline std::coroutine_handle<> unhandled_stopped() noexcept;

				[[nodiscard]] decltype(auto) result() & { return (rethrow_exception(), storage_base::result()); }
				[[nodiscard]] decltype(auto) result() && { return (rethrow_exception(), storage_base::result()); }

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

		public:
			using promise_type = promise<shared_task, promise_base, T, std::allocator<void>>;
			template<typename Alloc>
			using allocator_promise_type = promise<shared_task, promise_base, T, Alloc>;

		private:
			class awaitable
			{
			public:
				constexpr explicit awaitable(std::coroutine_handle<promise_base> h) noexcept : _handle(h) {}

				bool await_ready() const noexcept { return !_handle || _handle.done(); }
				decltype(auto) await_resume() { return _handle.promise().result(); }
				template<typename P>
				auto await_suspend(std::coroutine_handle<P> next)
				{
					_node._cont = next;
					return _handle.promise().try_push(_handle, _node);
				}

			private:
				std::coroutine_handle<promise_base> _handle = {};
				task_node _node = {};
			};

		public:
			shared_task() = delete;

			shared_task(shared_task &&other) noexcept : shared_task(other.release()) {}
			shared_task &operator=(shared_task &&other) noexcept
			{
				if (this != &other) swap(other);
				return *this;
			}

			shared_task(const shared_task &other) noexcept : _handle(other._handle)
			{
				if (_handle) _handle.promise().acquire();
			}
			shared_task &operator=(const shared_task &other) noexcept
			{
				if (_handle != other._handle)
				{
					destroy();
					if ((_handle = other._handle))
						_handle.promise().acquire();
				}

				return *this;
			}

			template<std::derived_from<promise_base> P>
			constexpr explicit shared_task(std::coroutine_handle<P> h) noexcept : _handle(std::coroutine_handle<promise_base>::from_address(h.address())) {}
			constexpr explicit shared_task(std::coroutine_handle<void> h) noexcept : _handle(std::coroutine_handle<promise_base>::from_address(h.address())) {}

			~shared_task() { destroy(); }

			/** Checks if the task has completed execution. */
			[[nodiscard]] bool ready() const noexcept { return !_handle || _handle.promise().ready(); }

			constexpr awaitable operator co_await() const & noexcept { return awaitable{_handle}; }
			constexpr awaitable operator co_await() const && noexcept { return awaitable{_handle}; }

			/** Releases the underlying coroutine handle. */
			std::coroutine_handle<> release() { return std::exchange(_handle, std::coroutine_handle<promise_base>{}); }

			[[nodiscard]] friend constexpr bool operator==(const shared_task &, const shared_task &) noexcept = default;

			constexpr void swap(shared_task &other) noexcept { std::swap(_handle, other._handle); }
			friend constexpr void swap(shared_task &a, shared_task &b) noexcept { a.swap(b); }

		private:
			void destroy() noexcept { if (_handle && _handle.promise().release()) _handle.destroy(); }

			std::coroutine_handle<promise_base> _handle = {};
		};

		template<typename T>
		std::coroutine_handle<> shared_task<T>::promise_base::unhandled_stopped() noexcept { return (for_each([](task_node *node) { node->unhandled_stopped(); }), std::noop_coroutine()); }
		template<typename T>
		template<typename P>
		void shared_task<T>::promise_base::final_awaiter::await_suspend(std::coroutine_handle<P> h) noexcept { h.promise().for_each([](task_node *node) { node->_cont.resume(); }); }

		template<template<typename> typename Task, typename Base, typename T, typename Alloc>
		Task<T> promise<Task, Base, T, Alloc>::get_return_object() noexcept { return Task<T>{std::coroutine_handle<promise>::from_promise(*this)}; }
	}

	/** Lazily-evaluated coroutine returning \a T as the promised result type. */
	template<typename T = void>
	using task = _task::task<T>;
	/** Task that can be atomically shared and awaited by multiple coroutines. */
	template<typename T = void>
	using shared_task = _task::shared_task<T>;
}
#endif

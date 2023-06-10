/*
 * Created by switchblade on 2023-04-30.
 */

#pragma once

#include "detail/config.hpp"

#ifdef ROD_HAS_COROUTINES

#include <coroutine>
#include <utility>
#include <atomic>
#include <mutex>

namespace rod
{
	class async_mutex;
	class async_lock_guard;

	/** Coroutine mutex used to suspend a coroutine until the lock is acquired. */
	class async_mutex
	{
		class awaiter
		{
			friend class async_mutex;

		public:
			explicit awaiter(async_mutex &mtx) noexcept : _mtx(mtx) {}

			constexpr void await_resume() const noexcept {}
			constexpr bool await_ready() const noexcept { return false; }
			bool await_suspend(std::coroutine_handle<> cont) noexcept
			{
				_cont = cont;
				for (auto state = _mtx._state.load(std::memory_order_acquire);;)
				{
					if (!state)
					{
						if (_mtx._state.compare_exchange_weak(
								state, &_mtx._state,
								std::memory_order_acquire,
								std::memory_order_relaxed))
							return false;
						continue;
					}

					_next = static_cast<awaiter *>(state);
					if (_mtx._state.compare_exchange_weak(
							state, this,
							std::memory_order_release,
							std::memory_order_relaxed))
						return true;
				}
			}

		private:
			std::coroutine_handle<> _cont = {};
			awaiter *_next = {};

		protected:
			async_mutex &_mtx;
		};
		class guard_awaiter : public awaiter
		{
		public:
			using awaiter::awaiter;

			[[nodiscard]] inline async_lock_guard await_resume() const noexcept;
		};

	public:
		async_mutex(const async_mutex &) = delete;
		async_mutex &operator=(const async_mutex &) = delete;
		async_mutex(async_mutex &&) = delete;
		async_mutex &operator=(async_mutex &&) = delete;

		constexpr async_mutex() noexcept = default;

		/** Asynchronously locks the mutex and returns an awaiter suspended until the lock is acquired. */
		[[nodiscard]] awaiter async_lock() noexcept { return awaiter{*this}; }
		/** @brief Asynchronously locks the mutex and returns a lock guard awaiter suspended until the lock is acquired.
		 *
		 * Expression `co_await mtx.async_scoped_lock()` evaluates to an RAII lock guard
		 * for the locked mutex, where `mtx` is an instance of `async_mutex`. */
		[[nodiscard]] guard_awaiter async_scoped_lock() noexcept { return guard_awaiter{*this}; }

		/** Attempts to lock the mutex.
		 * @return `true` is locked successfully, `false` if the mutex is already locked. */
		bool try_lock() noexcept
		{
			void *old = nullptr;
			return _state.compare_exchange_strong(old, &_state, std::memory_order_acquire, std::memory_order_relaxed);
		}
		/** Unlocks the mutex and resumes the next waiting coroutine inside this call. */
		void unlock()
		{
			auto front = _queue;
			if (!front)
			{
				void *state = &_state;
				if (_state.compare_exchange_strong(
						state, nullptr,
						std::memory_order_release,
						std::memory_order_relaxed))
					return;

				state = _state.exchange(&_state, std::memory_order_acquire);
				for (auto node = static_cast<awaiter *>(state);;)
				{
					const auto next = node->_next;
					node->_next = std::exchange(front, node);
					if (!(node = next))
						break;
				}
			}

			/* Un-link & resume the blocked coroutine. */
			_queue = front->_next;
			front->_cont.resume();
		}

	private:
		/* _state == &_state - locked with no waiters.
		 * _state == nullptr - not locked. */
		std::atomic<void *> _state = {};
		awaiter *_queue = {};
	};

	/** RAII lock guard for the `async_mutex` mutex type. */
	class async_lock_guard
	{
	public:
		async_lock_guard(const async_lock_guard &) = delete;
		async_lock_guard &operator=(const async_lock_guard &) = delete;

		async_lock_guard(async_lock_guard &&other) noexcept : _mtx(std::exchange(other._mtx, {})) {}
		async_lock_guard(async_mutex &mtx, std::adopt_lock_t) noexcept : _mtx(&mtx) {}
		~async_lock_guard() { if (_mtx) _mtx->unlock(); }

	private:
		async_mutex *_mtx;
	};

	async_lock_guard async_mutex::guard_awaiter::await_resume() const noexcept
	{
		return {_mtx, std::adopt_lock};
	}
}
#endif

/*
 * Created by switchblade on 2023-05-16.
 */

#pragma once

#include <concepts>
#include <utility>
#include <atomic>

#include "config.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	/* Lightweight shared reference counter implementation for cases where shared_ptr is overkill. */

	struct shared_base
	{
		auto acquire() noexcept { return (_refs.fetch_add(1, std::memory_order_relaxed), this); }
		bool release() noexcept { return _refs.fetch_sub(1, std::memory_order_acq_rel) == 1; }

		std::atomic<std::size_t> _refs = 1;
	};

	template<typename T>
	class shared_handle
	{
	public:
		constexpr shared_handle() noexcept = default;
		constexpr shared_handle(T *ptr) noexcept : _ptr(ptr) {}

		shared_handle(const shared_handle &other) noexcept : _ptr(other.acquire()) {}
		shared_handle &operator=(const shared_handle &other) noexcept
		{
			if (this != &other)
			{
				release();
				_ptr = other.acquire();
			}
			return *this;
		}

		constexpr shared_handle(shared_handle &&other) noexcept { std::swap(_ptr, other._ptr); }
		constexpr shared_handle &operator=(shared_handle &&other) noexcept { return (std::swap(_ptr, other._ptr), *this); }

		~shared_handle() { release(); }

		void reset()
		{
			release();
			_ptr = {};
		}

		[[nodiscard]] constexpr T *get() const noexcept { return _ptr; }
		[[nodiscard]] constexpr T *operator->() const noexcept { return get(); }
		[[nodiscard]] constexpr T &operator*() const noexcept { return *get(); }
		[[nodiscard]] constexpr operator bool() const noexcept { return get(); }

	private:
		auto acquire() const noexcept { return _ptr ? static_cast<T *>(static_cast<shared_base *>(_ptr)->acquire()) : _ptr; }
		void release() { if (_ptr && static_cast<shared_base *>(_ptr)->release()) delete _ptr; }

		T *_ptr = {};
	};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

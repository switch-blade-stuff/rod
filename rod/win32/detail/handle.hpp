/*
 * Created by switchblade on 2023-05-20.
 */

#pragma once

#include "ntapi.hpp"

#ifdef ROD_WIN32

#include <system_error>
#include <utility>

namespace rod::detail
{
	class basic_handle
	{
		static void *invalid_handle() noexcept { return reinterpret_cast<void *>(static_cast<std::uintptr_t>(-1)); }

	public:
		constexpr basic_handle() noexcept = default;
		constexpr explicit basic_handle(void *handle) noexcept : _handle(handle) {}

		ROD_API_PUBLIC std::error_code close() noexcept;
		ROD_API_PUBLIC std::error_code wait() const noexcept;

		void *release() noexcept { return std::exchange(_handle, invalid_handle()); }
		void *release(void *hnd) noexcept { return std::exchange(_handle, hnd); }

		[[nodiscard]] bool is_open() const noexcept { return _handle != invalid_handle(); }
		[[nodiscard]] void *native_handle() const noexcept { return _handle; }

		constexpr void swap(basic_handle &other) noexcept { std::swap(_handle, other._handle); }
		friend constexpr void swap(basic_handle &a, basic_handle &b) noexcept { a.swap(b); }

	private:
		void *_handle = invalid_handle();
	};

	class unique_handle : basic_handle
	{
	public:
		unique_handle(const unique_handle &) = delete;
		unique_handle &operator=(const unique_handle &) = delete;

		constexpr unique_handle() noexcept = default;
		constexpr unique_handle(unique_handle &&other) noexcept { swap(other); }
		constexpr unique_handle &operator=(unique_handle &&other) noexcept { return (swap(other), *this); }

		constexpr explicit unique_handle(void *handle) noexcept : basic_handle(handle) {}

		~unique_handle() { if (is_open()) close(); }

		using basic_handle::wait;
		using basic_handle::close;
		using basic_handle::is_open;
		using basic_handle::release;
		using basic_handle::native_handle;

		constexpr void swap(unique_handle &other) noexcept { basic_handle::swap(other); }
		friend constexpr void swap(unique_handle &a, unique_handle &b) noexcept { a.swap(b); }
	};
}
#endif

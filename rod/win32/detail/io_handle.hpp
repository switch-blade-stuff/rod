/*
 * Created by switchblade on 2023-05-20.
 */

#pragma once

#include "../../detail/config.hpp"

#ifdef _WIN32

#define NOMINMAX
#include <windows.h>

#include <system_error>
#include <utility>

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	class basic_io_handle
	{
	public:
		constexpr basic_io_handle() noexcept = default;
		constexpr explicit basic_io_handle(void *handle) noexcept : m_handle(handle) {}

		constexpr void *release() noexcept { return std::exchange(m_handle, nullptr); }

		ROD_PUBLIC std::error_code close() noexcept;

		[[nodiscard]] bool is_open() const noexcept { return m_handle != INVALID_HANDLE_VALUE; }
		[[nodiscard]] void *native_handle() const noexcept { return m_handle; }

		constexpr void swap(basic_io_handle &other) noexcept { std::swap(m_handle, other.m_handle); }
		friend constexpr void swap(basic_io_handle &a, basic_io_handle &b) noexcept { a.swap(b); }

	private:
		void *m_handle = INVALID_HANDLE_VALUE;
	};

	class unique_io_handle : basic_io_handle
	{
	public:
		unique_io_handle(const unique_io_handle &) = delete;
		unique_io_handle &operator=(const unique_io_handle &) = delete;

		constexpr unique_io_handle() noexcept = default;
		constexpr unique_io_handle(unique_io_handle &&other) noexcept { swap(other); }
		constexpr unique_io_handle &operator=(unique_io_handle &&other) noexcept { return (swap(other), *this); }

		constexpr explicit unique_io_handle(void *handle) noexcept : basic_io_handle(handle) {}

		ROD_PUBLIC ~unique_io_handle();

		using basic_io_handle::close;
		using basic_io_handle::release;

		using basic_io_handle::is_open;
		using basic_io_handle::native_handle;

		constexpr void swap(unique_io_handle &other) noexcept { basic_io_handle::swap(other); }
		friend constexpr void swap(unique_io_handle &a, unique_io_handle &b) noexcept { a.swap(b); }
	};
}
#endif

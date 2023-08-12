/*
 * Created by switchblade on 2023-05-20.
 */

#pragma once

#include "../../detail/config.hpp"

#ifdef ROD_WIN32

#include "../../result.hpp"
#include "ntapi.hpp"

namespace rod::_win32
{
	struct io_handle
	{
		using native_handle_type = void *;

		io_handle(const io_handle &) noexcept = delete;
		io_handle &operator=(const io_handle &) noexcept = delete;

		io_handle() noexcept : value(-1) {}
		io_handle(io_handle &&other) noexcept : value(std::exchange(other.value, -1)) {}
		io_handle &operator=(io_handle &&other) noexcept { return (std::swap(value, other.value), *this); }

		explicit io_handle(std::intptr_t value) noexcept : value(value) {}
		explicit io_handle(native_handle_type hnd) noexcept : handle(hnd) {}

		~io_handle() { if (is_open()) close().value(); }

		[[nodiscard]] ROD_API_PUBLIC result<io_handle> clone() const noexcept;
		[[nodiscard]] ROD_API_PUBLIC result<std::wstring> path() const noexcept;

		[[nodiscard]] bool is_open() const noexcept { return value != io_handle().value; }
		[[nodiscard]] native_handle_type native_handle() const noexcept { return handle; }

		ROD_API_PUBLIC result<> close() noexcept;
		native_handle_type release() noexcept { return std::exchange(handle, io_handle().handle); }
		native_handle_type release(native_handle_type hnd) noexcept { return std::exchange(handle, hnd); }

		void swap(io_handle &other) noexcept { std::swap(value, other.value); }

		union
		{
			native_handle_type handle;
			std::intptr_t value;
		};
	};
}
#endif

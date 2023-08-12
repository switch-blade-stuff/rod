/*
 * Created by switch_blade on 2023-08-09.
 */

#pragma once

#include "../../detail/path_view.hpp"
#include "handle.hpp"

#ifdef ROD_WIN32

namespace rod::_win32
{
	class path_handle : io_handle
	{
	public:
		using native_handle_type = typename io_handle::native_handle_type;

		static ROD_API_PUBLIC result<path_handle> open(fs::path_view path) noexcept;
		static ROD_API_PUBLIC result<path_handle> open(const path_handle &base, fs::path_view path) noexcept;

	public:
		path_handle() noexcept = default;
		path_handle(path_handle &&) noexcept = default;
		path_handle &operator=(path_handle &&) noexcept = default;

		explicit path_handle(std::intptr_t value) noexcept : io_handle(value) {}
		explicit path_handle(native_handle_type hnd) noexcept : io_handle(hnd) {}

		void swap(path_handle &other) noexcept { io_handle::swap(other); }

		using io_handle::path;
		using io_handle::clone;
		using io_handle::close;
		using io_handle::release;
		using io_handle::is_open;
		using io_handle::native_handle;
	};

	[[nodiscard]] bool operator==(const path_handle &a, const path_handle &b) noexcept { return a.native_handle() == b.native_handle(); }
	[[nodiscard]] bool operator!=(const path_handle &a, const path_handle &b) noexcept { return a.native_handle() != b.native_handle(); }
}

#endif

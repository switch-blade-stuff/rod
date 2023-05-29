/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#ifdef _MSC_VER

#include <cstdio>

#include "../../detail/file_fwd.hpp"
#include "io_handle.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	class native_file : unique_io_handle
	{
	public:
		using native_handle_type = void *;

		static ROD_PUBLIC native_file open(const char *path, int mode, std::error_code &err) noexcept;
		static ROD_PUBLIC native_file open(const wchar_t *path, int mode, std::error_code &err) noexcept;
		static ROD_PUBLIC native_file reopen(native_handle_type fd, int mode, std::error_code &err) noexcept;

		constexpr native_file() = default;

		constexpr explicit native_file(native_handle_type fd) noexcept : unique_io_handle(fd) {}
		constexpr explicit native_file(unique_io_handle &&fd) noexcept : unique_io_handle(std::move(fd)) {}

		using unique_io_handle::tell;
		using unique_io_handle::seek;
		using unique_io_handle::close;

		std::error_code flush() noexcept { return unique_io_handle::sync(); }

		ROD_PUBLIC std::size_t sync_read(void *dst, std::size_t n, std::error_code &err) noexcept;
		ROD_PUBLIC std::size_t sync_write(const void *src, std::size_t n, std::error_code &err) noexcept;
		ROD_PUBLIC std::size_t sync_read_at(void *dst, std::size_t n, std::ptrdiff_t off, std::error_code &err) noexcept;
		ROD_PUBLIC std::size_t sync_write_at(const void *src, std::size_t n, std::ptrdiff_t off, std::error_code &err) noexcept;

		using unique_io_handle::is_open;
		using unique_io_handle::native_handle;

		constexpr void swap(native_file &other) noexcept { unique_io_handle::swap(other); }
		friend constexpr void swap(native_file &a, native_file &b) noexcept { a.swap(b); }
	};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

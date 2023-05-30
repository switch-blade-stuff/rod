/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#ifdef _WIN32

#include <cstdio>

#include "../../detail/file_fwd.hpp"
#include "io_handle.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	class system_file : unique_io_handle
	{
	public:
		using native_handle_type = void *;

		static ROD_PUBLIC system_file open(const char *path, int mode, std::error_code &err) noexcept;
		static ROD_PUBLIC system_file open(const wchar_t *path, int mode, std::error_code &err) noexcept;
		static ROD_PUBLIC system_file reopen(native_handle_type fd, int mode, std::error_code &err) noexcept;

	public:
		constexpr system_file() = default;

		constexpr explicit system_file(native_handle_type fd) noexcept : unique_io_handle(fd) {}
		constexpr explicit system_file(unique_io_handle &&fd) noexcept : unique_io_handle(std::move(fd)) {}

		using unique_io_handle::close;
		using unique_io_handle::is_open;
		using unique_io_handle::native_handle;

		ROD_PUBLIC std::size_t tell(std::error_code &err) const noexcept;
		ROD_PUBLIC std::size_t seek(std::ptrdiff_t off, int dir, std::error_code &err) noexcept;

		ROD_PUBLIC std::error_code flush() noexcept;
		ROD_PUBLIC std::size_t sync_read(void *dst, std::size_t n, std::error_code &err) noexcept;
		ROD_PUBLIC std::size_t sync_write(const void *src, std::size_t n, std::error_code &err) noexcept;
		ROD_PUBLIC std::size_t sync_read_at(void *dst, std::size_t n, std::ptrdiff_t off, std::error_code &err) noexcept;
		ROD_PUBLIC std::size_t sync_write_at(const void *src, std::size_t n, std::ptrdiff_t off, std::error_code &err) noexcept;

		constexpr void swap(system_file &other) noexcept { unique_io_handle::swap(other); }
		friend constexpr void swap(system_file &a, system_file &b) noexcept { a.swap(b); }
	};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

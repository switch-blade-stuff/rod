/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#include <cstdio>

#include "../../detail/file_fwd.hpp"
#include "descriptor.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	class native_file : unique_descriptor
	{
	public:
		using native_handle_type = int;

		static ROD_PUBLIC native_file open(const char *path, int mode, std::error_code &err) noexcept;
		static ROD_PUBLIC native_file open(const wchar_t *path, int mode, std::error_code &err) noexcept;
		static ROD_PUBLIC native_file reopen(native_handle_type fd, int mode, std::error_code &err) noexcept;

		constexpr native_file() = default;

		constexpr explicit native_file(native_handle_type fd) noexcept : unique_descriptor(fd) {}
		constexpr explicit native_file(unique_descriptor &&fd) noexcept : unique_descriptor(std::move(fd)) {}

		using unique_descriptor::close;
		using unique_descriptor::is_open;
		using unique_descriptor::native_handle;

		ROD_PUBLIC std::ptrdiff_t tell(std::error_code &err) const noexcept;
		ROD_PUBLIC std::ptrdiff_t seek(std::ptrdiff_t off, int dir, std::error_code &err) noexcept;

		ROD_PUBLIC std::error_code flush() noexcept;
		ROD_PUBLIC std::size_t sync_read(void *dst, std::size_t n, std::error_code &err) noexcept;
		ROD_PUBLIC std::size_t sync_write(const void *src, std::size_t n, std::error_code &err) noexcept;
		ROD_PUBLIC std::size_t sync_read_at(void *dst, std::size_t n, std::ptrdiff_t off, std::error_code &err) noexcept;
		ROD_PUBLIC std::size_t sync_write_at(const void *src, std::size_t n, std::ptrdiff_t off, std::error_code &err) noexcept;

		constexpr void swap(native_file &other) noexcept { unique_descriptor::swap(other); }
		friend constexpr void swap(native_file &a, native_file &b) noexcept { a.swap(b); }
	};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

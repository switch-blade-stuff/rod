/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#ifdef _WIN32

#include <cstdio>

#include "io_handle.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	class system_file : unique_io_handle
	{
	public:
		using native_handle_type = void *;

		enum fileprot : int
		{
			user_exec = 0b000'000'001,
			user_read = 0b000'000'010,
			user_write = 0b000'000'100,
			group_exec = 0b000'001'000,
			group_read = 0b000'010'000,
			group_write = 0b000'100'000,
			other_exec = 0b001'000'000,
			other_read = 0b010'000'000,
			other_write = 0b100'000'000,
			_default = user_read | user_write | group_read | group_write | other_read | other_write,
		};
		enum openmode : int
		{
			in = 0b0000'0001,
			out = 0b0000'0010,
			binary = 0b0000'0100,
			direct = 0b0000'1000,
			ate = 0b0001'0000,
			app = 0b0010'0000,
			trunc = 0b0100'0000,
			noreplace = 0b1000'0000,
		};
		enum seekdir : int
		{
			beg = SEEK_SET,
			cur = SEEK_CUR,
			end = SEEK_END,
		};

		static ROD_PUBLIC system_file reopen(native_handle_type fd, int mode, std::error_code &err) noexcept;
		static ROD_PUBLIC system_file open(const char *path, int mode, int prot, std::error_code &err) noexcept;
		static ROD_PUBLIC system_file open(const wchar_t *path, int mode, int prot, std::error_code &err) noexcept;

		static system_file open(const char *path, int mode, std::error_code &err) noexcept { return open(path, mode, fileprot::_default, err); }
		static system_file open(const wchar_t *path, int mode, std::error_code &err) noexcept { return open(path, mode, fileprot::_default, err); }

	public:
		constexpr system_file() = default;

		constexpr explicit system_file(native_handle_type hnd) noexcept : unique_io_handle(hnd) {}
		constexpr explicit system_file(unique_io_handle &&hnd) noexcept : unique_io_handle(std::move(hnd)) {}

		using unique_io_handle::close;
		using unique_io_handle::release;
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

/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#ifdef _WIN32

#include <filesystem>
#include <cstdio>

#include "../../result.hpp"
#include "io_handle.hpp"

namespace rod::detail
{
	class system_file : unique_io_handle
	{
	public:
		using native_handle_type = void *;

		enum openprot : int
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
			nocreate = 0b0'1000'0000,
			noreplace = 0b1'0000'0000,
		};
		enum seekdir : int
		{
			beg = SEEK_SET,
			cur = SEEK_CUR,
			end = SEEK_END,
		};

		static ROD_API_PUBLIC result<system_file, std::error_code> reopen(native_handle_type hnd, int mode) noexcept;
		static ROD_API_PUBLIC result<system_file, std::error_code> open(const char *path, int mode, int prot) noexcept;
		static ROD_API_PUBLIC result<system_file, std::error_code> open(const wchar_t *path, int mode, int prot) noexcept;

		static result<system_file, std::error_code> open(const char *path, int mode) noexcept { return open(path, mode, openprot::_default); }
		static result<system_file, std::error_code> open(const wchar_t *path, int mode) noexcept { return open(path, mode, openprot::_default); }

	public:
		constexpr system_file() = default;

		constexpr explicit system_file(native_handle_type hnd) noexcept : unique_io_handle(hnd) {}
		constexpr explicit system_file(unique_io_handle &&hnd) noexcept : unique_io_handle(std::move(hnd)) {}

		constexpr explicit system_file(native_handle_type hnd, std::size_t off) noexcept : unique_io_handle(hnd), _offset(off) {}
		constexpr explicit system_file(unique_io_handle &&hnd, std::size_t off) noexcept : unique_io_handle(std::move(hnd)), _offset(off) {}

		using unique_io_handle::close;
		using unique_io_handle::is_open;
		using unique_io_handle::native_handle;

		void *release() noexcept
		{
			_offset = std::numeric_limits<std::size_t>::max();
			return unique_io_handle::release();
		}
		void *release(void *hnd) noexcept
		{
			_offset = std::numeric_limits<std::size_t>::max();
			return unique_io_handle::release(hnd);
		}

		ROD_API_PUBLIC result<std::size_t, std::error_code> size() const noexcept;
		ROD_API_PUBLIC result<std::size_t, std::error_code> tell() const noexcept;
		ROD_API_PUBLIC result<std::filesystem::path, std::error_code> path() const;

		ROD_API_PUBLIC result<std::size_t, std::error_code> resize(std::size_t n) noexcept;
		ROD_API_PUBLIC result<std::size_t, std::error_code> seek(std::ptrdiff_t off, int dir) noexcept;

		ROD_API_PUBLIC std::error_code sync() noexcept;
		std::error_code flush() noexcept { return sync(); }

		ROD_API_PUBLIC result<std::size_t, std::error_code> sync_read(void *dst, std::size_t n) noexcept;
		ROD_API_PUBLIC result<std::size_t, std::error_code> sync_write(const void *src, std::size_t n) noexcept;
		ROD_API_PUBLIC result<std::size_t, std::error_code> sync_read_at(void *dst, std::size_t n, std::size_t off) noexcept;
		ROD_API_PUBLIC result<std::size_t, std::error_code> sync_write_at(const void *src, std::size_t n, std::size_t off) noexcept;

		constexpr void swap(system_file &other) noexcept
		{
			unique_io_handle::swap(other);
			std::swap(_offset, other._offset);
		}
		friend constexpr void swap(system_file &a, system_file &b) noexcept { a.swap(b); }

	private:
		std::size_t _offset = std::numeric_limits<std::size_t>::max();
	};
}
#endif

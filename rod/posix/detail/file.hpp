/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#ifdef __unix__

#include <sys/stat.h>
#include <filesystem>
#include <cstdio>

#include "../../result.hpp"
#include "descriptor.hpp"

namespace rod::detail
{
	class system_file : unique_descriptor
	{
	public:
		using native_handle_type = int;

		enum openprot : int
		{
			user_exec = S_IXUSR,
			user_read = S_IRUSR,
			user_write = S_IWUSR,
			group_exec = S_IXGRP,
			group_read = S_IRGRP,
			group_write = S_IWGRP,
			other_exec = S_IXOTH,
			other_read = S_IROTH,
			other_write = S_IWOTH,
			_default = user_read | user_write | group_read | group_write | other_read | other_write,
		};
		enum openmode : int
		{
			in = 0b0001,
			out = 0b0010,
			binary = 0b0100,
			direct = 0b1000,
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

		static ROD_API_PUBLIC result<system_file, std::error_code> reopen(native_handle_type fd, int mode) noexcept;
		static ROD_API_PUBLIC result<system_file, std::error_code> open(const char *path, int mode, int prot) noexcept;
		static ROD_API_PUBLIC result<system_file, std::error_code> open(const wchar_t *path, int mode, int prot) noexcept;

		static result<system_file, std::error_code> open(const char *path, int mode) noexcept { return open(path, mode, openprot::_default); }
		static result<system_file, std::error_code> open(const wchar_t *path, int mode) noexcept { return open(path, mode, openprot::_default); }

	public:
		constexpr system_file() = default;

		constexpr explicit system_file(native_handle_type fd) noexcept : unique_descriptor(fd) {}
		constexpr explicit system_file(unique_descriptor &&fd) noexcept : unique_descriptor(std::move(fd)) {}

		using unique_descriptor::close;
		using unique_descriptor::release;
		using unique_descriptor::is_open;
		using unique_descriptor::native_handle;

		ROD_API_PUBLIC result<std::size_t, std::error_code> tell() const noexcept;
		ROD_API_PUBLIC result<std::size_t, std::error_code> seek(std::ptrdiff_t off, int dir) noexcept;
		auto setpos(std::size_t pos) noexcept { return seek(static_cast<std::ptrdiff_t>(pos), beg); }

		ROD_API_PUBLIC std::error_code sync() noexcept;
		std::error_code flush() noexcept { return sync(); }

		ROD_API_PUBLIC result<std::size_t, std::error_code> sync_read(void *dst, std::size_t n) noexcept;
		ROD_API_PUBLIC result<std::size_t, std::error_code> sync_write(const void *src, std::size_t n) noexcept;
		ROD_API_PUBLIC result<std::size_t, std::error_code> sync_read_at(void *dst, std::size_t n, std::size_t off) noexcept;
		ROD_API_PUBLIC result<std::size_t, std::error_code> sync_write_at(const void *src, std::size_t n, std::size_t off) noexcept;

		constexpr void swap(system_file &other) noexcept { unique_descriptor::swap(other); }
		friend constexpr void swap(system_file &a, system_file &b) noexcept { a.swap(b); }
	};
}
#endif

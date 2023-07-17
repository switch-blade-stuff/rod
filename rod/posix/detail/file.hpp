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

		static constexpr auto nonblock = file_base::openmode_max + 1;

	public:
		constexpr system_file() = default;

		constexpr explicit system_file(native_handle_type fd) noexcept : unique_descriptor(fd) {}
		constexpr explicit system_file(unique_descriptor &&fd) noexcept : unique_descriptor(std::move(fd)) {}

		using unique_descriptor::close;
		using unique_descriptor::release;
		using unique_descriptor::is_open;
		using unique_descriptor::native_handle;

		ROD_API_PUBLIC std::error_code open(const char *path, int mode, int prot) noexcept;
		ROD_API_PUBLIC std::error_code open(const wchar_t *path, int mode, int prot) noexcept;

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

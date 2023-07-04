/*
 * Created by switch_blade on 2023-07-03.
 */

#pragma once

#ifdef _WIN32

#include "../../result.hpp"
#include "handle.hpp"
#include "mmap.hpp"

namespace rod::detail
{
	class system_shm : unique_handle
	{
	public:
		using native_handle_type = void *;

		enum openmode : int
		{
			readonly = 0b001,
			nocreate = 0b010,
			noreplace = 0b100,
		};

		static ROD_API_PUBLIC result<system_shm, std::error_code> open(const char *name, std::size_t size, int mode) noexcept;
		static ROD_API_PUBLIC result<system_shm, std::error_code> open(const wchar_t *name, std::size_t size, int mode) noexcept;

	public:
		system_shm() = delete;

		constexpr system_shm(system_shm &&other) noexcept : unique_handle(std::move(other)) {}
		constexpr system_shm &operator=(system_shm &&other) noexcept { return (swap(other), *this); }

		constexpr explicit system_shm(native_handle_type hnd) noexcept : unique_handle(hnd) {}
		constexpr explicit system_shm(unique_handle &&hnd) noexcept : unique_handle(std::move(hnd)) {}

		using unique_handle::native_handle;

		ROD_API_PUBLIC result<system_mmap, std::error_code> map(std::size_t off, std::size_t n, int mode) const noexcept;
		result<system_mmap, std::error_code> map(std::size_t off, std::size_t n) const noexcept { return map(off, n, system_mmap::read | system_mmap::write); }

		constexpr void swap(system_shm &other) noexcept { unique_handle::swap(other); }
		friend constexpr void swap(system_shm &a, system_shm &b) noexcept { a.swap(b); }
	};
}
#endif
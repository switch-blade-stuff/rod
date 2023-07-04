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
	class system_shm
	{
	public:
		using native_handle_type = void *;

		enum openmode : int
		{
			readonly = 0b001,
			nocreate = 0b010,
		};

		static ROD_API_PUBLIC result<system_shm, std::error_code> open(const char *name, std::size_t size, int mode) noexcept;
		static ROD_API_PUBLIC result<system_shm, std::error_code> open(const wchar_t *name, std::size_t size, int mode) noexcept;
		static result<system_shm, std::error_code> open(const wchar_t *shm_name, const wchar_t *mtx_name, std::size_t size, int mode) noexcept;

	public:
		system_shm() = delete;

		constexpr system_shm(system_shm &&other) noexcept { swap(other); }
		constexpr system_shm &operator=(system_shm &&other) noexcept { return (swap(other), *this); }

		constexpr explicit system_shm(native_handle_type shm, native_handle_type mtx) noexcept : _shm(shm), _mtx(mtx) {}

		ROD_API_PUBLIC bool try_lock() noexcept;
		ROD_API_PUBLIC std::error_code lock() noexcept;
		ROD_API_PUBLIC std::error_code unlock() noexcept;

		ROD_API_PUBLIC result<system_mmap, std::error_code> map(std::size_t off, std::size_t n, int mode) const noexcept;
		result<system_mmap, std::error_code> map(std::size_t off, std::size_t n) const noexcept { return map(off, n, system_mmap::read | system_mmap::write); }

		native_handle_type native_handle() const noexcept { return _shm.native_handle(); }

		constexpr void swap(system_shm &other) noexcept
		{
			_shm.swap(other._shm);
			_mtx.swap(other._mtx);
		}
		friend constexpr void swap(system_shm &a, system_shm &b) noexcept { a.swap(b); }

	private:
		unique_handle _shm = {};
		unique_handle _mtx = {};
	};
}
#endif
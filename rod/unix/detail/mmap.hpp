/*
 * Created by switchblade on 2023-05-30.
 */

#pragma once

#ifdef __unix__

#include "../../result.hpp"

#include <fcntl.h>
#include <utility>

namespace rod::detail
{
	class mmap_handle
	{
	public:
		static result<std::size_t, std::error_code> get_pagesize() noexcept;
		static result<std::size_t, std::error_code> pagesize_off(std::size_t x) noexcept
		{
			if (auto res = get_pagesize(); res.has_value()) [[likely]]
				return x - static_cast<std::size_t>(x % *res);
			else
				return res;
		}

	public:
		mmap_handle(const mmap_handle &) = delete;
		mmap_handle &operator=(const mmap_handle &) = delete;

		constexpr mmap_handle() noexcept = default;
		constexpr mmap_handle(mmap_handle &&other) noexcept { swap(other); }
		constexpr mmap_handle &operator=(mmap_handle &&other) noexcept { return (swap(other), *this); }

		constexpr mmap_handle(void *data, std::size_t size) noexcept : _data(static_cast<std::byte *>(data)), _size(size) {}

		ROD_API_PUBLIC ~mmap_handle();
		ROD_API_PUBLIC std::error_code unmap() noexcept;

		constexpr std::pair<void *, std::size_t> release() noexcept { return std::pair{std::exchange(_data, nullptr), std::exchange(_size, 0)}; }
		constexpr std::pair<void *, std::size_t> release(void *data, std::size_t size) noexcept { return std::pair{std::exchange(_data, static_cast<std::byte *>(data)), std::exchange(_size, size)}; }

		[[nodiscard]] constexpr bool empty() const noexcept { return !_data; }
		[[nodiscard]] constexpr std::byte *data() const noexcept { return _data; }
		[[nodiscard]] constexpr std::size_t size() const noexcept { return _size; }

		constexpr void swap(mmap_handle &other) noexcept
		{
			std::swap(_data, other._data);
			std::swap(_size, other._size);
		}
		friend constexpr void swap(mmap_handle &a, mmap_handle &b) noexcept { a.swap(b); }

	private:
		std::byte *_data = {};
		std::size_t _size = {};
	};

	class system_mmap
	{
	public:
		using native_handle_type = void *;

		enum mapmode : int
		{
			copy = 0b0001,
			read = 0b0010,
			write = 0b0100,
			expand = 0b1000,
		};

	public:
		system_mmap(const system_mmap &) = delete;
		system_mmap &operator=(const system_mmap &) = delete;

		constexpr system_mmap() noexcept = default;
		constexpr system_mmap(system_mmap &&other) noexcept { swap(other); }
		constexpr system_mmap &operator=(system_mmap &&other) noexcept { return (swap(other), *this); }

		constexpr explicit system_mmap(void *data, std::size_t base, std::size_t size) noexcept : _mmap(data, size), _base(base) {}

		[[nodiscard]] constexpr std::byte *data() const noexcept { return _mmap.data() + _base; }
		[[nodiscard]] constexpr void *native_data() const noexcept { return _mmap.data(); }

		[[nodiscard]] constexpr std::size_t size() const noexcept { return _mmap.size() - _base; }
		[[nodiscard]] constexpr std::size_t native_size() const noexcept { return _mmap.size(); }

		[[nodiscard]] constexpr bool empty() const noexcept { return _mmap.empty(); }
		[[nodiscard]] constexpr native_handle_type native_handle() const noexcept { return native_data(); }

		constexpr void swap(system_mmap &other) noexcept
		{
			using std::swap;
			swap(_mmap, other._mmap);
			swap(_base, other._base);
		}
		friend constexpr void swap(system_mmap &a, system_mmap &b) noexcept { a.swap(b); }

	private:
		mmap_handle _mmap = {};
		std::size_t _base = {};
	};
}
#endif

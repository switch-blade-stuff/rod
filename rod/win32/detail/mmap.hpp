/*
 * Created by switch_blade on 2023-07-03.
 */

#pragma once

#ifdef _WIN32

#include <system_error>
#include <utility>

#include "../../result.hpp"

namespace rod::detail
{
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

		static std::size_t get_pagesize() noexcept;
		static std::size_t pagesize_off(std::size_t x) noexcept { return x - static_cast<std::size_t>(x % get_pagesize()); }

	public:
		system_mmap(const system_mmap &) = delete;
		system_mmap &operator=(const system_mmap &) = delete;

		constexpr system_mmap() noexcept = default;
		constexpr system_mmap(system_mmap &&other) noexcept { swap(other); }
		constexpr system_mmap &operator=(system_mmap &&other) noexcept { return (swap(other), *this); }

		constexpr explicit system_mmap(void *data, std::size_t base, std::size_t size) noexcept : _data(static_cast<std::byte *>(data)), _base(base), _size(size) {}

		ROD_API_PUBLIC ~system_mmap();

		[[nodiscard]] constexpr std::byte *data() const noexcept { return _data + _base; }
		[[nodiscard]] constexpr void *native_data() const noexcept { return _data; }

		[[nodiscard]] constexpr std::size_t size() const noexcept { return _size - _base; }
		[[nodiscard]] constexpr std::size_t native_size() const noexcept { return _size; }

		[[nodiscard]] constexpr bool empty() const noexcept { return !_data; }
		[[nodiscard]] constexpr native_handle_type native_handle() const noexcept { return native_data(); }

		constexpr void swap(system_mmap &other) noexcept
		{
			std::swap(_data, other._data);
			std::swap(_base, other._base);
			std::swap(_size, other._size);
		}
		friend constexpr void swap(system_mmap &a, system_mmap &b) noexcept { a.swap(b); }

	private:
		std::byte *_data = {};
		std::size_t _base = {};
		std::size_t _size = {};
	};
}
#endif

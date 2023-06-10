/*
 * Created by switchblade on 2023-05-30.
 */

#pragma once

#ifdef __unix__

#include "../../detail/config.hpp"

#include <system_error>
#include <fcntl.h>
#include <utility>

namespace rod::detail
{
	class system_mmap
	{
	public:
		enum mapmode : int
		{
			copy = 0b000'001,
			exec = 0b000'010,
			read = 0b000'100,
			write = 0b001'000,
			expand = 0b010'000,
			_default = read | write,
		};

		static ROD_PUBLIC system_mmap map(int fd, std::size_t off, std::size_t size, int mode, std::error_code &err) noexcept;
		static system_mmap map(std::size_t size, int mode, std::error_code &err) noexcept { return map(-1, 0, size, mode, err); }

	public:
		system_mmap(const system_mmap &) = delete;
		system_mmap &operator=(const system_mmap &) = delete;

		constexpr system_mmap() noexcept = default;
		constexpr system_mmap(system_mmap &&other) noexcept { swap(other); }
		constexpr system_mmap &operator=(system_mmap &&other) noexcept { return (swap(other), *this); }

		constexpr system_mmap(void *data, std::size_t size) noexcept : system_mmap(data, 0, size) {}
		constexpr system_mmap(void *data, std::size_t base, std::size_t size) noexcept : _data(static_cast<std::byte *>(data)), _base(base), _size(size) {}

		ROD_PUBLIC ~system_mmap();
		ROD_PUBLIC std::error_code unmap() noexcept;

		constexpr std::pair<void *, std::size_t> release() noexcept { return release(nullptr, 0); }
		constexpr std::pair<void *, std::size_t> release(void *data, std::size_t size) noexcept { return release(data, 0, size); }
		constexpr std::pair<void *, std::size_t> release(void *data, std::size_t base, std::size_t size) noexcept { return release(static_cast<std::byte *>(data), base, size); }

		constexpr std::pair<void *, std::size_t> release(std::byte *data, std::size_t base, std::size_t size) noexcept
		{
			_base = base;
			std::swap(_data, data);
			std::swap(_size, size);
			return std::pair{data, size};
		}

		[[nodiscard]] constexpr bool empty() const noexcept { return !_data; }
		[[nodiscard]] constexpr std::byte *data() const noexcept { return _data + _base; }
		[[nodiscard]] constexpr std::size_t size() const noexcept { return _size - _base; }

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

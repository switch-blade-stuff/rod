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

	class system_mapping
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

		static ROD_API_PUBLIC result<system_mapping, std::error_code> map(int fd, std::size_t off, std::size_t size, int mode) noexcept;
		static result<system_mapping, std::error_code> map(std::size_t size, int mode) noexcept { return map(-1, 0, size, mode); }

	private:
		constexpr system_mapping(void *data, std::size_t base, std::size_t size) noexcept : _mmap(data, size), _base(base) {}

	public:
		system_mapping(const system_mapping &) = delete;
		system_mapping &operator=(const system_mapping &) = delete;

		constexpr system_mapping() noexcept = default;
		constexpr system_mapping(system_mapping &&other) noexcept { swap(other); }
		constexpr system_mapping &operator=(system_mapping &&other) noexcept { return (swap(other), *this); }

		[[nodiscard]] constexpr bool empty() const noexcept { return _mmap.empty(); }
		[[nodiscard]] constexpr std::byte *data() const noexcept { return _mmap.data() + _base; }
		[[nodiscard]] constexpr std::size_t size() const noexcept { return _mmap.size() - _base; }

		constexpr void swap(system_mapping &other) noexcept
		{
			using std::swap;
			swap(_mmap, other._mmap);
			swap(_base, other._base);
		}
		friend constexpr void swap(system_mapping &a, system_mapping &b) noexcept { a.swap(b); }

	private:
		mmap_handle _mmap = {};
		std::size_t _base = {};
	};
}
#endif

/*
 * Created by switchblade on 2023-05-30.
 */

#pragma once

#ifdef ROD_POSIX

#include "../../result.hpp"

#include <fcntl.h>
#include <utility>

namespace rod::_detail
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
}
#endif

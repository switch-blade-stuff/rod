/*
 * Created by switchblade on 2023-05-20.
 */

#pragma once

#ifdef __unix__

#include "../../result.hpp"

#include <system_error>
#include <utility>

namespace rod::detail
{
	class basic_descriptor
	{
	public:
		constexpr basic_descriptor() noexcept = default;
		constexpr explicit basic_descriptor(int fd) noexcept : _fd(fd) {}

		constexpr int release(int fd = -1) noexcept { return std::exchange(_fd, fd); }

		ROD_API_PUBLIC std::error_code close() noexcept;

		ROD_API_PUBLIC result<bool, std::error_code> poll_read(int timeout) noexcept;
		ROD_API_PUBLIC result<bool, std::error_code> poll_write(int timeout) noexcept;
		ROD_API_PUBLIC result<bool, std::error_code> poll_error(int timeout) noexcept;

		ROD_API_PUBLIC result<std::size_t, std::error_code> read(void *dst, std::size_t n) noexcept;
		ROD_API_PUBLIC result<std::size_t, std::error_code> write(const void *src, std::size_t n) noexcept;
		ROD_API_PUBLIC result<std::size_t, std::error_code> read_at(void *dst, std::size_t n, std::size_t off) noexcept;
		ROD_API_PUBLIC result<std::size_t, std::error_code> write_at(const void *src, std::size_t n, std::size_t off) noexcept;

		[[nodiscard]] constexpr bool is_open() const noexcept { return _fd > -1; }
		[[nodiscard]] constexpr int native_handle() const noexcept { return _fd; }

		constexpr void swap(basic_descriptor &other) noexcept { std::swap(_fd, other._fd); }
		friend constexpr void swap(basic_descriptor &a, basic_descriptor &b) noexcept { a.swap(b); }

	private:
		int _fd = -1;
	};

	class unique_descriptor : basic_descriptor
	{
	public:
		unique_descriptor(const unique_descriptor &) = delete;
		unique_descriptor &operator=(const unique_descriptor &) = delete;

		constexpr unique_descriptor() noexcept = default;
		constexpr unique_descriptor(unique_descriptor &&other) noexcept { swap(other); }
		constexpr unique_descriptor &operator=(unique_descriptor &&other) noexcept { return (swap(other), *this); }

		constexpr explicit unique_descriptor(int fd) noexcept : basic_descriptor(fd) {}

		ROD_API_PUBLIC ~unique_descriptor();

		using basic_descriptor::close;
		using basic_descriptor::release;

		using basic_descriptor::poll_read;
		using basic_descriptor::poll_write;
		using basic_descriptor::poll_error;

		using basic_descriptor::read;
		using basic_descriptor::write;
		using basic_descriptor::read_at;
		using basic_descriptor::write_at;

		using basic_descriptor::is_open;
		using basic_descriptor::native_handle;

		constexpr void swap(unique_descriptor &other) noexcept { basic_descriptor::swap(other); }
		friend constexpr void swap(unique_descriptor &a, unique_descriptor &b) noexcept { a.swap(b); }
	};
}
#endif

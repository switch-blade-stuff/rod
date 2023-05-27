/*
 * Created by switchblade on 2023-05-20.
 */

#pragma once

#include "../../detail/config.hpp"

#ifdef __unix__

#include <system_error>
#include <utility>

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	class basic_descriptor
	{
	public:
		constexpr basic_descriptor() noexcept = default;
		constexpr basic_descriptor(basic_descriptor &&other) noexcept { swap(other); }
		constexpr basic_descriptor &operator=(basic_descriptor &&other) noexcept { return (swap(other), *this); }

		constexpr explicit basic_descriptor(int fd) noexcept : m_fd(fd) {}

		constexpr int release() noexcept { return std::exchange(m_fd, -1); }

		ROD_PUBLIC std::error_code close() noexcept;

		ROD_PUBLIC std::error_code sync() noexcept;
		ROD_PUBLIC bool poll_read(int timeout, std::error_code &err) noexcept;
		ROD_PUBLIC bool poll_write(int timeout, std::error_code &err) noexcept;
		ROD_PUBLIC bool poll_error(int timeout, std::error_code &err) noexcept;

		ROD_PUBLIC std::size_t read(void *dst, std::size_t n, std::error_code &err) noexcept;
		ROD_PUBLIC std::size_t write(const void *src, std::size_t n, std::error_code &err) noexcept;
		ROD_PUBLIC std::size_t read_at(void *dst, std::size_t n, std::ptrdiff_t off, std::error_code &err) noexcept;
		ROD_PUBLIC std::size_t write_at(const void *src, std::size_t n, std::ptrdiff_t off, std::error_code &err) noexcept;

		ROD_PUBLIC std::ptrdiff_t seek(std::ptrdiff_t off, int dir, std::error_code &err) noexcept;
		ROD_PUBLIC std::ptrdiff_t tell(std::error_code &err) const noexcept;

		[[nodiscard]] constexpr bool is_open() const noexcept { return m_fd > -1; }
		[[nodiscard]] constexpr int native_handle() const noexcept { return m_fd; }

		constexpr void swap(basic_descriptor &other) noexcept { std::swap(m_fd, other.m_fd); }
		friend constexpr void swap(basic_descriptor &a, basic_descriptor &b) noexcept { a.swap(b); }

	private:
		int m_fd = -1;
	};

	class unique_descriptor : basic_descriptor
	{
	public:
		unique_descriptor(const unique_descriptor &) = delete;
		unique_descriptor &operator=(const unique_descriptor &) = delete;

		constexpr unique_descriptor() noexcept = default;
		constexpr unique_descriptor(unique_descriptor &&other) noexcept : basic_descriptor(std::move(other)) {}
		constexpr unique_descriptor &operator=(unique_descriptor &&other) noexcept { return (basic_descriptor::operator=(std::move(other)), *this); }

		constexpr explicit unique_descriptor(int fd) noexcept : basic_descriptor(fd) {}

		ROD_PUBLIC ~unique_descriptor();

		using basic_descriptor::close;
		using basic_descriptor::release;

		using basic_descriptor::sync;
		using basic_descriptor::poll_read;
		using basic_descriptor::poll_write;
		using basic_descriptor::poll_error;

		using basic_descriptor::tell;
		using basic_descriptor::seek;
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

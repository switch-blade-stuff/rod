/*
 * Created by switchblade on 2023-05-20.
 */

#pragma once

#ifdef __unix__

#include <system_error>
#include <aio.h>

#include "../config.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	/* RAII handle to a file descriptor. */
	class descriptor_handle
	{
	public:
		descriptor_handle(const descriptor_handle &) = delete;
		descriptor_handle &operator=(const descriptor_handle &) = delete;

		constexpr descriptor_handle() noexcept = default;
		constexpr descriptor_handle(descriptor_handle &&other) noexcept { swap(other); }
		constexpr descriptor_handle &operator=(descriptor_handle &&other) noexcept { return (swap(other), *this); }

		constexpr explicit descriptor_handle(int fd) noexcept : m_fd(fd) {}

		ROD_PUBLIC ~descriptor_handle();

		ROD_PUBLIC std::error_code close() noexcept;

		ROD_PUBLIC std::size_t read(void *dst, std::size_t n, std::error_code &err) noexcept;
		ROD_PUBLIC std::size_t write(const void *src, std::size_t n, std::error_code &err) noexcept;

		ROD_PUBLIC std::size_t seek(std::ptrdiff_t off, int dir, std::error_code &err) noexcept;
		ROD_PUBLIC std::size_t tell(std::error_code &err) const noexcept;

		[[nodiscard]] constexpr bool is_open() const noexcept { return m_fd > -1; }
		[[nodiscard]] constexpr int native_handle() const noexcept { return m_fd; }

		constexpr void swap(descriptor_handle &other) noexcept { std::swap(m_fd, other.m_fd); }
		friend constexpr void swap(descriptor_handle &a, descriptor_handle &b) noexcept { a.swap(b); }

	private:
		int m_fd = -1;
	};
}
#endif

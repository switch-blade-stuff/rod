/*
 * Created by switchblade on 2023-05-20.
 */

#pragma once

#include "../../detail/config.hpp"

#ifdef _MSC_VER

#include <system_error>
#include <utility>

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	class basic_io_handle
	{
	public:
		constexpr basic_io_handle() noexcept = default;
		constexpr explicit basic_io_handle(void *handle) noexcept : m_handle(handle) {}

		constexpr void *release() noexcept { return std::exchange(m_handle, nullptr); }

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

		[[nodiscard]] constexpr bool is_open() const noexcept { return m_handle != nullptr; }
		[[nodiscard]] constexpr void *native_handle() const noexcept { return m_handle; }

		constexpr void swap(basic_io_handle &other) noexcept { std::swap(m_handle, other.m_handle); }
		friend constexpr void swap(basic_io_handle &a, basic_io_handle &b) noexcept { a.swap(b); }

	private:
		void *m_handle = nullptr;
	};

	class unique_io_handle : basic_io_handle
	{
	public:
		unique_io_handle(const unique_io_handle &) = delete;
		unique_io_handle &operator=(const unique_io_handle &) = delete;

		constexpr unique_io_handle() noexcept = default;
		constexpr unique_io_handle(unique_io_handle &&other) noexcept { swap(other); }
		constexpr unique_io_handle &operator=(unique_io_handle &&other) noexcept { return (swap(other), *this); }

		constexpr explicit unique_io_handle(void *handle) noexcept : basic_io_handle(handle) {}

		ROD_PUBLIC ~unique_io_handle();

		using basic_io_handle::close;
		using basic_io_handle::release;

		using basic_io_handle::sync;
		using basic_io_handle::poll_read;
		using basic_io_handle::poll_write;
		using basic_io_handle::poll_error;

		using basic_io_handle::tell;
		using basic_io_handle::seek;
		using basic_io_handle::read;
		using basic_io_handle::write;
		using basic_io_handle::read_at;
		using basic_io_handle::write_at;

		using basic_io_handle::is_open;
		using basic_io_handle::native_handle;

		constexpr void swap(unique_io_handle &other) noexcept { basic_io_handle::swap(other); }
		friend constexpr void swap(unique_io_handle &a, unique_io_handle &b) noexcept { a.swap(b); }
	};
}
#endif

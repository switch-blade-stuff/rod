/*
 * Created by switchblade on 2023-05-30.
 */

#pragma once

#ifdef __unix__

#include "../../detail/config.hpp"

#include <system_error>
#include <sys/mman.h>
#include <utility>

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	class system_mmap
	{
	public:
		enum mapprot : int
		{
			exec = PROT_EXEC,
			read = PROT_READ,
			write = PROT_WRITE,
			_default = read | write,
		};
		enum mapmode : int
		{
			copy = MAP_PRIVATE,
			shared = MAP_SHARED,
			_anon = MAP_ANONYMOUS,
		};

		static ROD_PUBLIC system_mmap map(void *addr, std::size_t n, int fd, std::size_t off, int mode, int prot, std::error_code &err) noexcept;
		static system_mmap map(void *addr, std::size_t n, int fd, std::size_t off, int mode, std::error_code &err) noexcept { return map(addr, n, fd, off, mode, mapprot::_default, err); }

		static system_mmap map(void *addr, std::size_t n, int mode, std::error_code &err) noexcept { return map(addr, n, mode, mapprot::_default, err); }
		static system_mmap map(void *addr, std::size_t n, int mode, int prot, std::error_code &err) noexcept { return map(addr, n, -1, 0, mode | mapmode::_anon, prot, err); }

	public:
		system_mmap(const system_mmap &) = delete;
		system_mmap &operator=(const system_mmap &) = delete;

		constexpr system_mmap() noexcept = default;
		constexpr system_mmap(system_mmap &&other) noexcept { swap(other); }
		constexpr system_mmap &operator=(system_mmap &&other) noexcept { return (swap(other), *this); }

		constexpr system_mmap(std::byte *data, std::size_t size) noexcept : system_mmap(data, size, 0) {}
		constexpr system_mmap(void *data, std::size_t size) noexcept : system_mmap(static_cast<std::byte *>(data), size) {}

		constexpr system_mmap(std::byte *base, std::size_t size, std::size_t off) noexcept : m_data(base + off), m_base(base), m_size(size) {}
		constexpr system_mmap(void *base, std::size_t size, std::size_t off) noexcept : system_mmap(static_cast<std::byte *>(base), size, off) {}

		ROD_PUBLIC ~system_mmap();

		ROD_PUBLIC std::error_code unmap() noexcept;
		ROD_PUBLIC std::error_code remap(std::size_t new_size) noexcept;

		constexpr std::pair<void *, std::size_t> release() noexcept { return {std::exchange(m_base, {}), m_size}; }
		constexpr std::pair<void *, std::size_t> release(void *base, std::size_t size, std::size_t off = 0) noexcept { return release(static_cast<std::byte *>(base), size, off); }
		constexpr std::pair<void *, std::size_t> release(std::byte *base, std::size_t size, std::size_t off = 0) noexcept { return (m_data = base + off, std::pair{std::exchange(m_base, base), std::exchange(m_size, size)}); }

		[[nodiscard]] constexpr bool empty() const noexcept { return !m_base; }
		[[nodiscard]] constexpr operator bool() const noexcept { return !empty(); }

		[[nodiscard]] constexpr void *data() const noexcept { return m_data; }
		[[nodiscard]] constexpr std::size_t size() const noexcept { return m_size - (m_data - m_base); }

		[[nodiscard]] constexpr void *base_data() const noexcept { return m_base; }
		[[nodiscard]] constexpr std::size_t base_size() const noexcept { return m_size; }

		constexpr void swap(system_mmap &other) noexcept
		{
			std::swap(m_data, other.m_data);
			std::swap(m_base, other.m_base);
			std::swap(m_size, other.m_size);
		}
		friend constexpr void swap(system_mmap &a, system_mmap &b) noexcept { a.swap(b); }

	private:
		std::byte *m_data = {};
		std::byte *m_base = {};
		std::size_t m_size = {};
	};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

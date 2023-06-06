/*
 * Created by switchblade on 2023-05-30.
 */

#pragma once

#ifdef __unix__

#include "../../detail/config.hpp"

#include <system_error>
#include <fcntl.h>
#include <utility>

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	class mmap_handle
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

		static ROD_PUBLIC mmap_handle map(int fd, std::size_t off, std::size_t size, int mode, std::error_code &err) noexcept;
		static mmap_handle map(std::size_t size, int mode, std::error_code &err) noexcept { return mmap(-1, 0, size, mode, err); }

	public:
		mmap_handle(const mmap_handle &) = delete;
		mmap_handle &operator=(const mmap_handle &) = delete;

		constexpr mmap_handle() noexcept = default;
		constexpr mmap_handle(mmap_handle &&other) noexcept { swap(other); }
		constexpr mmap_handle &operator=(mmap_handle &&other) noexcept { return (swap(other), *this); }

		constexpr mmap_handle(void *data, std::size_t size) noexcept : mmap_handle(data, 0, size) {}
		constexpr mmap_handle(void *data, std::size_t base, std::size_t size) noexcept : m_data(static_cast<std::byte *>(data)), m_base(base), m_size(size) {}

		ROD_PUBLIC ~mmap_handle();
		ROD_PUBLIC std::error_code unmap() noexcept;

		constexpr auto release() noexcept { return release(nullptr, 0); }
		constexpr auto release(void *data, std::size_t size) noexcept { return release(data, 0, size); }
		constexpr auto release(void *data, std::size_t base, std::size_t size) noexcept
		{
			m_base = base;
			std::swap(m_data, data);
			std::swap(m_size, size);
			return std::pair{data, size};
		}

		[[nodiscard]] constexpr std::byte *data() const noexcept { return m_data + m_base; }
		[[nodiscard]] constexpr std::size_t size() const noexcept { return m_size - m_base; }

		constexpr void swap(mmap_handle &other) noexcept
		{
			std::swap(m_data, other.m_data);
			std::swap(m_base, other.m_base);
			std::swap(m_size, other.m_size);
		}
		friend constexpr void swap(mmap_handle &a, mmap_handle &b) noexcept { a.swap(b); }

	private:
		std::byte *m_data = {};
		std::size_t m_base = {};
		std::size_t m_size = {};
	};

	class shared_mmap : mmap_handle
	{
	public:
		using native_handle_type = int;
		using mmap_handle::mapmode;

		enum mapprot : int
		{
			user_exec = S_IXUSR,
			user_read = S_IRUSR,
			user_write = S_IWUSR,
			group_exec = S_IXGRP,
			group_read = S_IRGRP,
			group_write = S_IWGRP,
			other_exec = S_IXOTH,
			other_read = S_IROTH,
			other_write = S_IWOTH,
			_default = user_read | user_write | group_read | group_write | other_read | other_write,
		};

		static ROD_PUBLIC shared_mmap open(const char *name, std::size_t off, std::size_t size, int mode, int prot, std::error_code &err) noexcept;
		static ROD_PUBLIC shared_mmap open(const wchar_t *name, std::size_t off, std::size_t size, int mode, int prot, std::error_code &err) noexcept;
		static shared_mmap open(const char *name, std::size_t off, std::size_t size, int mode, std::error_code &err) noexcept { return open(name, off, size, mode, mapprot::_default, err); }
		static shared_mmap open(const wchar_t *name, std::size_t off, std::size_t size, int mode, std::error_code &err) noexcept { return open(name, off, size, mode, mapprot::_default, err); }

	public:
		shared_mmap(const shared_mmap &) = delete;
		shared_mmap &operator=(const shared_mmap &) = delete;

		constexpr shared_mmap() noexcept = default;
		constexpr shared_mmap(shared_mmap &&other) noexcept { swap(other); }
		constexpr shared_mmap &operator=(shared_mmap &&other) noexcept { return (swap(other), *this); }

		constexpr shared_mmap(native_handle_type fd, std::size_t off, void *data, std::size_t size) noexcept : mmap_handle(data, size), m_fd(fd), m_off(off) {}

		ROD_PUBLIC ~shared_mmap();
		ROD_PUBLIC std::error_code close() noexcept;

		using mmap_handle::data;
		using mmap_handle::size;

		[[nodiscard]] constexpr bool is_open() const noexcept { return m_fd < 0; }
		[[nodiscard]] constexpr native_handle_type native_handle() const noexcept { return m_fd; }

		constexpr void swap(shared_mmap &other) noexcept
		{
			std::swap(m_off, other.m_off);
			std::swap(m_fd, other.m_fd);
			mmap_handle::swap(other);
		}
		friend constexpr void swap(shared_mmap &a, shared_mmap &b) noexcept { a.swap(b); }

	private:
		std::size_t m_off = {};
		int m_fd = -1;
	};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

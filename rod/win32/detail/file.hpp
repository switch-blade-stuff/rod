/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#include "../../detail/basic_file.hpp"

#ifdef ROD_WIN32

#include "handle.hpp"

namespace rod::_file
{
	using detail::unique_handle;
	using detail::ntapi;

	class system_handle : unique_handle
	{
	public:
		using native_handle_type = void *;

		static constexpr auto overlapped = file_base::openmode_max + 1;

	private:
		static constexpr auto npos = std::numeric_limits<std::size_t>::max();

	public:
		constexpr system_handle() noexcept = default;
		constexpr system_handle(system_handle &&other) noexcept { swap(other); }
		constexpr system_handle &operator=(system_handle &&other) noexcept { return (swap(other), *this); }

		constexpr explicit system_handle(native_handle_type hnd) noexcept : system_handle(hnd, npos) {}
		constexpr explicit system_handle(native_handle_type hnd, std::size_t pos) noexcept : unique_handle(hnd), _pos(pos) {}

		using unique_handle::close;
		using unique_handle::is_open;
		using unique_handle::native_handle;

		ROD_API_PUBLIC std::error_code open(const char *path, int mode, int prot) noexcept;
		ROD_API_PUBLIC std::error_code open(const wchar_t *path, int mode, int prot) noexcept;

		ROD_API_PUBLIC std::error_code sync() noexcept;
		std::error_code flush() noexcept { return sync(); }

		[[nodiscard]] ROD_API_PUBLIC result<std::size_t, std::error_code> size() const noexcept;
		[[nodiscard]] ROD_API_PUBLIC result<std::size_t, std::error_code> tell() const noexcept;
		[[nodiscard]] ROD_API_PUBLIC result<std::filesystem::path, std::error_code> path() const;

		ROD_API_PUBLIC result<std::size_t, std::error_code> resize(std::size_t n) noexcept;
		ROD_API_PUBLIC result<std::size_t, std::error_code> seek(std::ptrdiff_t off, int dir) noexcept;

		constexpr auto setpos(std::size_t pos) noexcept { return _pos = pos; }
		void *release() noexcept { return (_pos = npos, unique_handle::release()); }
		void *release(void *hnd) noexcept { return (_pos = npos, unique_handle::release(hnd)); }

		constexpr void swap(system_handle &other) noexcept
		{
			unique_handle::swap(other);
			std::swap(_pos, other._pos);
		}
		friend constexpr void swap(system_handle &a, system_handle &b) noexcept { a.swap(b); }

	private:
		[[nodiscard]] result<ntapi::ulong, std::error_code> read_chunk(void *buff, ntapi::ulong size, std::size_t pos) noexcept;
		[[nodiscard]] result<ntapi::ulong, std::error_code> write_chunk(const void *buff, ntapi::ulong size, std::size_t pos) noexcept;

		[[nodiscard]] ROD_API_PUBLIC result<std::size_t, std::error_code> read_some(std::span<std::byte> buff) noexcept;
		[[nodiscard]] ROD_API_PUBLIC result<std::size_t, std::error_code> write_some(std::span<const std::byte> buff) noexcept;
		[[nodiscard]] ROD_API_PUBLIC result<std::size_t, std::error_code> read_some_at(std::size_t pos, std::span<std::byte> buff) noexcept;
		[[nodiscard]] ROD_API_PUBLIC result<std::size_t, std::error_code> write_some_at(std::size_t pos, std::span<const std::byte> buff) noexcept;

	public:
		template<derived_reference<system_handle> F, byte_buffer Buff>
		friend auto tag_invoke(read_some_t, F &&f, Buff &&buff) noexcept(noexcept(rod::as_byte_buffer(buff))) { return f.read_some(rod::as_byte_buffer(buff)); }
		template<derived_reference<system_handle> F, byte_buffer Buff>
		friend auto tag_invoke(write_some_t, F &&f, Buff &&buff) noexcept(noexcept(rod::as_byte_buffer(buff))) { return f.write_some(rod::as_byte_buffer(buff)); }
		template<derived_reference<system_handle> F, std::convertible_to<std::size_t> Pos, byte_buffer Buff>
		friend auto tag_invoke(read_some_at_t, F &&f, Pos pos, Buff &&buff) noexcept(noexcept(rod::as_byte_buffer(buff))) { return f.read_some_at(static_cast<std::size_t>(pos), rod::as_byte_buffer(buff)); }
		template<derived_reference<system_handle> F, std::convertible_to<std::size_t> Pos, byte_buffer Buff>
		friend auto tag_invoke(write_some_at_t, F &&f, Pos pos, Buff &&buff) noexcept(noexcept(rod::as_byte_buffer(buff))) { return f.write_some_at(static_cast<std::size_t>(pos), rod::as_byte_buffer(buff)); }

	private:
		std::size_t _pos = npos;
	};
}
#endif

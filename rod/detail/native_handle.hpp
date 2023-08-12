/*
 * Created by switch_blade on 2023-08-07.
 */

#pragma once

#include "../utility.hpp"

namespace rod::_detail
{
	/* Opaque object representing a native system handle (ex. descriptor or HANDLE) + usage flags. */
	struct native_handle
	{
		enum handle_usage : std::uint8_t
		{
			file = 1,
			path = 2,
			link = 3,
			fifo = 4,
			sock = 5,
			mmap = 6,
			proc = 7,
			directory = 8,
			user_data = 9,
		};
		enum access_flags : std::uint8_t
		{
			async_io = 0x01,
			aligned_io = 0x02,
			allow_read = 0x04,
			allow_write = 0x08,
			append_only = 0x10,
			stream_only = 0x20,
		};
		enum caching_flags : std::uint8_t
		{
			force_sync = 1,
			cache_meta = 2,
			cache_read = 4,
			cache_write = 8,
		};

	private:
#ifdef ROD_WIN32
		using win_socket = std::intptr_t;
		using win_handle = void *;
#endif

	public:
		constexpr native_handle() noexcept = default;
		constexpr native_handle(const native_handle &) noexcept = default;
		constexpr native_handle &operator=(const native_handle &) noexcept = default;

		constexpr native_handle(native_handle &&other) noexcept
		{
			_usage = std::exchange(other._usage, {});
			_caching = std::exchange(other._caching, {});
			_access = std::exchange(other._access, {});
			_flags = std::exchange(other._flags, {});
			_value = std::exchange(other._value, {});
		}
		constexpr native_handle &operator=(native_handle &&other) noexcept
		{
			std::swap(_usage, other._usage);
			std::swap(_caching, other._caching);
			std::swap(_access, other._access);
			std::swap(_flags, other._flags);
			std::swap(_value, other._value);
			return *this;
		}

		[[nodiscard]] constexpr bool is_open() const noexcept { return _value != -1; }
		[[nodiscard]] constexpr explicit operator bool() const noexcept { return is_open(); }

		[[nodiscard]] constexpr bool operator==(const native_handle &other) const noexcept { return _value == other._value; }

	public:
		handle_usage _usage = {};
		caching_flags _caching = {};
		access_flags _access = {};
		std::uint32_t _flags = {};

		union
		{
			std::intptr_t _value = -1;
#ifdef ROD_WIN32
			win_socket _socket;
			win_handle _handle;
#endif
#ifdef ROD_POSIX
			int _descriptor;
#endif
			void *_user_ptr;
		};
	};
}

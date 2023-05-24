/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#include <concepts>
#include <ranges>
#include <span>

#include "../scheduling.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	/** Concept used to check if type \a T is a buffer type (contiguous range of `std::byte` or integral values) that can participate in read & write operations. */
	template<typename T>
	concept io_buffer = std::ranges::contiguous_range<T> && (std::same_as<std::ranges::range_value_t<T>, std::byte> || std::integral<std::ranges::range_value_t<T>>);

	/** Creates a mutable span of bytes created from the specified raw pointer and size. */
	[[nodiscard]] inline std::span<std::byte> make_byte_buffer(void *ptr, std::size_t n) noexcept
	{
		const auto bytes = static_cast<std::byte *>(ptr);
		return {bytes, bytes + n};
	}
	/** Creates an immutable span of bytes created from the specified raw pointer and size. */
	[[nodiscard]] inline std::span<const std::byte> make_byte_buffer(const void *ptr, std::size_t n) noexcept
	{
		const auto bytes = static_cast<const std::byte *>(ptr);
		return {bytes, bytes + n};
	}

	/** Returns a mutable span of bytes created from the specified pointer to integral and size. */
	template<std::integral T>
	[[nodiscard]] inline std::span<std::byte> make_byte_buffer(T *ptr, std::size_t n) noexcept
	{
		return make_byte_buffer(static_cast<void *>(ptr), n * sizeof(T));
	}
	/** Returns an immutable span of bytes created from the specified pointer to integral and size. */
	template<std::integral T>
	[[nodiscard]] inline std::span<const std::byte> make_byte_buffer(const T *ptr, std::size_t n) noexcept
	{
		return make_byte_buffer(static_cast<const void *>(ptr), n * sizeof(T));
	}
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

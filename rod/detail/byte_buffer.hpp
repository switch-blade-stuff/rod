/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#include <concepts>
#include <ranges>
#include <span>

#include "../scheduling.hpp"

namespace rod
{
	/** Concept used to check if type \a T is a contiguous buffer of bytes (`std::byte`, `std::int8_t` or `std::uint8_t`). */
	template<typename T>
	concept byte_buffer = std::ranges::contiguous_range<T> && one_of<std::remove_cv_t<std::ranges::range_value_t<T>>, std::byte, std::uint8_t, std::int8_t, char, char8_t>;

	/** Creates a span of bytes created from the specified raw pointer and size. */
	[[nodiscard]] inline std::span<std::byte> as_byte_buffer(void *ptr, std::size_t n) noexcept
	{
		const auto bytes = static_cast<std::byte *>(ptr);
		return {bytes, bytes + n};
	}
	/** @copydoc as_byte_buffer */
	[[nodiscard]] inline std::span<const std::byte> as_byte_buffer(const void *ptr, std::size_t n) noexcept
	{
		const auto bytes = static_cast<const std::byte *>(ptr);
		return {bytes, bytes + n};
	}

	/** Returns a span of bytes created from the specified pointer to integral and size. */
	template<std::integral T>
	[[nodiscard]] inline decltype(auto) as_byte_buffer(T *ptr, std::size_t n) noexcept
	{
		return as_byte_buffer(static_cast<copy_cv_t<T, void> *>(ptr), n * sizeof(T));
	}
	/** Returns a span of bytes created from the elements pointed to by the specified contiguous iterators. */
	template<std::contiguous_iterator I, std::sentinel_for<I> S> requires std::integral<std::iter_value_t<I>>
	[[nodiscard]] inline decltype(auto) as_byte_buffer(I first, S last) noexcept(noexcept(std::distance(first, last)))
	{
		return as_byte_buffer(std::to_address(first), static_cast<std::size_t>(std::distance(first, last)));
	}

	/** Forwards \a data. */
	template<typename R> requires one_of<std::decay_t<R>, std::span<std::byte>, std::span<const std::byte>>
	[[nodiscard]] inline decltype(auto) as_byte_buffer(R &&data) noexcept { return std::forward<R>(data); }
	/** Returns a span of bytes created from the elements of the specified contiguous range. */
	template<typename R> requires(!one_of<std::decay_t<R>, std::span<std::byte>, std::span<const std::byte>> && std::ranges::contiguous_range<R> && std::integral<std::ranges::range_value_t<R>>)
	[[nodiscard]] inline decltype(auto) as_byte_buffer(R &&data) noexcept(noexcept(std::begin(data)) && noexcept(std::end(data))) { return as_byte_buffer(std::begin(data), std::end(data)); }
}

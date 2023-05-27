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
	namespace detail
	{
		template<typename T>
		concept byte_like = std::same_as<T, std::byte> || std::same_as<T, std::uint8_t> || std::same_as<T, std::int8_t>;
	}

	/** Concept used to check if type \a T is a contiguous buffer of bytes (`std::byte`, `std::int8_t` or `std::uint8_t`). */
	template<typename T>
	concept io_buffer = std::ranges::contiguous_range<T> && detail::byte_like<std::remove_cv_t<std::ranges::range_value_t<T>>>;

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
	[[nodiscard]] inline std::span<copy_cv_t<T, std::byte>> as_byte_buffer(T *ptr, std::size_t n) noexcept
	{
		return as_byte_buffer(static_cast<copy_cv_t<T, void> *>(ptr), n * sizeof(T));
	}
	/** Returns a span of bytes created from the elements pointed to by the specified contiguous iterators. */
	template<std::contiguous_iterator I, std::sentinel_for<I> S> requires std::integral<std::iter_value_t<I>>
	[[nodiscard]] inline std::span<copy_cv_t<std::iter_reference_t<I>, std::byte>> as_byte_buffer(I first, S last) noexcept
	{
		return as_byte_buffer(std::to_address(first), static_cast<std::size_t>(std::distance(first, last)));
	}
	/** Returns a span of bytes created from the elements of the specified contiguous range. */
	template<std::ranges::contiguous_range R> requires std::integral<std::ranges::range_value_t<R>>
	[[nodiscard]] inline std::span<copy_cv_t<std::ranges::range_reference_t<R>, std::byte>> as_byte_buffer(R &&data) noexcept
	{
		return as_byte_buffer(std::begin(data), std::end(data));
	}
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

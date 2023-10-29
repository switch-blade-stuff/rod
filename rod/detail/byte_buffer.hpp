/*
 * Created by switchblade on 2023-08-19.
 */

#pragma once

#include "handle_base.hpp"

namespace rod
{
	/** Trivial contiguous range used as output IO buffer by file and socket handles.
	 * @note For the purposes of ABI compatibility, `byte_buffer` is layout-compatible with `struct iov`. */
	class byte_buffer
	{
	public:
		using value_type = std::byte;
		using element_type = std::byte;

		using pointer = value_type *;
		using const_pointer = const value_type *;

		using iterator = value_type *;
		using const_iterator = const value_type *;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		using size_type = std::size_t;

	public:
		/** Initializes an empty byte buffer. */
		constexpr byte_buffer() = default;
		/** Initializes a byte buffer from a data pointer and length. */
		constexpr byte_buffer(pointer data, size_type len) noexcept : _data(data), _len(len) {}

		/** Initializes a byte buffer from a contiguous range of bytes defined by [\a begin, \a end). */
		template<std::contiguous_iterator I, std::sentinel_for<I> S> requires std::same_as<std::iter_value_t<I>, std::byte>
		constexpr byte_buffer(I begin, S end) noexcept(noexcept(std::distance(begin, end))) : byte_buffer(std::to_address(begin), std::size_t(std::distance(begin, end))) {}
		/** Initializes a byte buffer from a contiguous range of bytes. */
		template<std::ranges::contiguous_range R> requires(!decays_to_same<R, byte_buffer> && std::same_as<std::ranges::range_value_t<R>, std::byte>)
		constexpr byte_buffer(R &&data) noexcept(noexcept(std::ranges::data(data)) && noexcept(std::ranges::size(data))) : byte_buffer(std::ranges::data(data), std::ranges::size(data)) {}

		/** Returns iterator to the first element of the buffer. */
		[[nodiscard]] constexpr iterator begin() noexcept { return _data; }
		/** Returns const iterator to the first element of the buffer. */
		[[nodiscard]] constexpr const_iterator begin() const noexcept { return _data; }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_iterator cbegin() const noexcept { return _data; }

		/** Returns iterator one past the last element of the buffer. */
		[[nodiscard]] constexpr iterator end() noexcept { return _data + _len; }
		/** Returns const iterator one past the last element of the buffer. */
		[[nodiscard]] constexpr const_iterator end() const noexcept { return _data + _len; }
		/** @copydoc end */
		[[nodiscard]] constexpr const_iterator cend() const noexcept { return _data + _len; }

		/** Returns reverse iterator one past the first element of the buffer. */
		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
		/** Returns const reverse iterator one past the first element of the buffer. */
		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
		/** @copydoc crbegin */
		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }

		/** Returns reverse iterator to the last element of the buffer sequence. */
		[[nodiscard]] constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
		/** Returns const reverse iterator to the last element of the buffer sequence. */
		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
		/** @copydoc crend */
		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

		/** Checks if the buffer is empty. */
		[[nodiscard]] constexpr bool empty() const noexcept { return _len == 0; }
		/** Returns size of the buffer. */
		[[nodiscard]] constexpr size_type size() const noexcept { return _len; }

		/** Returns pointer to the data of the buffer. */
		[[nodiscard]] constexpr pointer data() noexcept { return _data; }
		/** Returns const pointer to the data of the buffer. */
		[[nodiscard]] constexpr const_pointer data() const noexcept { return _data; }

		/** Converts the byte buffer to a span of bytes. */
		[[nodiscard]] constexpr std::span<element_type> as_span() const noexcept { return *this; }
		/** @copydoc as_span */
		[[nodiscard]] constexpr operator std::span<element_type>() const noexcept { return std::span(_data, _len); }

	private:
		pointer _data;
		size_type _len;
	};

	static_assert(std::is_trivial_v<byte_buffer>);
	static_assert(std::is_trivially_copyable_v<byte_buffer>);

	static_assert(std::ranges::contiguous_range<byte_buffer>);
	static_assert(std::constructible_from<byte_buffer, std::byte[1]>);
	static_assert(std::constructible_from<byte_buffer, std::span<std::byte>>);
	static_assert(std::constructible_from<byte_buffer, std::vector<std::byte>>);

	/** Converts a contiguous range of bytes defined by [\a begin, \a end) to `byte_buffer`. */
	template<std::contiguous_iterator I, std::sentinel_for<I> S> requires(!std::is_const_v<std::remove_reference_t<std::iter_reference_t<I>>>)
	[[nodiscard]] inline byte_buffer as_bytes(I begin, S end) noexcept(noexcept(std::span(begin, end))) { return std::as_writable_bytes(std::span(begin, end)); }
	/** Converts a contiguous range of elements to `byte_buffer`. */
	template<std::ranges::contiguous_range R> requires(!std::is_const_v<std::remove_reference_t<std::ranges::range_reference_t<R>>>)
	[[nodiscard]] inline byte_buffer as_bytes(R &&data) noexcept(noexcept(rod::as_bytes(std::ranges::begin(data), std::ranges::end(data)))) { return rod::as_bytes(std::ranges::begin(data), std::ranges::end(data)); }

	static_assert(requires (int *ptr) { { rod::as_bytes(ptr, ptr) } noexcept -> std::same_as<byte_buffer>; });
	static_assert(requires (std::span<int> s) { { rod::as_bytes(s) } noexcept -> std::same_as<byte_buffer>; });
	static_assert(requires (std::array<int, 2> s) { { rod::as_bytes(s) } noexcept -> std::same_as<byte_buffer>; });

	/** Trivial contiguous range used as input IO buffer by file and socket handles.
	 * @note For the purposes of ABI compatibility, `const_byte_buffer` is layout-compatible with `struct iov`. */
	class const_byte_buffer
	{
	public:
		using value_type = std::byte;
		using element_type = const std::byte;

		using pointer = const value_type *;
		using const_pointer = const value_type *;

		using iterator = const value_type *;
		using const_iterator = const value_type *;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		using size_type = std::size_t;

	public:
		/** Initializes an empty byte buffer. */
		constexpr const_byte_buffer() = default;
		/** Initializes a byte buffer from a data pointer and length. */
		constexpr const_byte_buffer(pointer data, size_type len) noexcept : _data(data), _len(len) {}

		/** Initializes a byte buffer from a contiguous range of bytes defined by [\a begin, \a end). */
		template<std::contiguous_iterator I, std::sentinel_for<I> S> requires std::same_as<std::iter_value_t<I>, std::byte>
		constexpr const_byte_buffer(I begin, S end) noexcept(noexcept(std::distance(begin, end))) : const_byte_buffer(std::to_address(begin), std::size_t(std::distance(begin, end))) {}
		/** Initializes a byte buffer from a contiguous range of bytes. */
		template<std::ranges::contiguous_range R> requires(!decays_to_same<R, const_byte_buffer> && std::same_as<std::ranges::range_value_t<R>, std::byte>)
		constexpr const_byte_buffer(R &&data) noexcept(noexcept(std::ranges::data(data)) && noexcept(std::ranges::size(data))) : const_byte_buffer(std::ranges::data(data), std::ranges::size(data)) {}

		/** Returns iterator to the first element of the buffer. */
		[[nodiscard]] constexpr iterator begin() noexcept { return _data; }
		/** Returns const iterator to the first element of the buffer. */
		[[nodiscard]] constexpr const_iterator begin() const noexcept { return _data; }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_iterator cbegin() const noexcept { return _data; }

		/** Returns iterator one past the last element of the buffer. */
		[[nodiscard]] constexpr iterator end() noexcept { return _data + _len; }
		/** Returns const iterator one past the last element of the buffer. */
		[[nodiscard]] constexpr const_iterator end() const noexcept { return _data + _len; }
		/** @copydoc end */
		[[nodiscard]] constexpr const_iterator cend() const noexcept { return _data + _len; }

		/** Returns reverse iterator one past the first element of the buffer. */
		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
		/** Returns const reverse iterator one past the first element of the buffer. */
		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
		/** @copydoc crbegin */
		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }

		/** Returns reverse iterator to the last element of the buffer sequence. */
		[[nodiscard]] constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
		/** Returns const reverse iterator to the last element of the buffer sequence. */
		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
		/** @copydoc crend */
		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

		/** Checks if the buffer is empty. */
		[[nodiscard]] constexpr bool empty() const noexcept { return _len == 0; }
		/** Returns size of the buffer. */
		[[nodiscard]] constexpr size_type size() const noexcept { return _len; }

		/** Returns pointer to the data of the buffer. */
		[[nodiscard]] constexpr pointer data() noexcept { return _data; }
		/** Returns const pointer to the data of the buffer. */
		[[nodiscard]] constexpr const_pointer data() const noexcept { return _data; }

		/** Converts the byte buffer to a span of bytes. */
		[[nodiscard]] constexpr std::span<element_type> as_span() const noexcept { return *this; }
		/** @copydoc as_span */
		[[nodiscard]] constexpr operator std::span<element_type>() const noexcept { return std::span(_data, _len); }

	private:
		pointer _data;
		size_type _len;
	};

	static_assert(std::is_trivial_v<const_byte_buffer>);
	static_assert(std::is_trivially_copyable_v<const_byte_buffer>);

	static_assert(std::ranges::contiguous_range<const_byte_buffer>);
	static_assert(std::constructible_from<const_byte_buffer, byte_buffer>);
	static_assert(std::constructible_from<const_byte_buffer, std::byte[1]>);
	static_assert(std::constructible_from<const_byte_buffer, std::span<std::byte>>);
	static_assert(std::constructible_from<const_byte_buffer, std::vector<std::byte>>);

	/** Converts a contiguous range of bytes defined by [\a begin, \a end) to `const_byte_buffer`. */
	template<std::contiguous_iterator I, std::sentinel_for<I> S> requires std::is_const_v<std::remove_reference_t<std::iter_reference_t<I>>>
	[[nodiscard]] inline const_byte_buffer as_bytes(I begin, S end) noexcept(noexcept(std::span(begin, end))) { return std::as_bytes(std::span(begin, end)); }
	/** Converts a contiguous range of elements to `const_byte_buffer`. */
	template<std::ranges::contiguous_range R> requires std::is_const_v<std::remove_reference_t<std::ranges::range_reference_t<R>>>
	[[nodiscard]] inline const_byte_buffer as_bytes(R &&data) noexcept(noexcept(rod::as_bytes(std::ranges::begin(data), std::ranges::end(data)))) { return rod::as_bytes(std::ranges::begin(data), std::ranges::end(data)); }

	static_assert(requires (const int *ptr) { { rod::as_bytes(ptr, ptr) } noexcept -> std::same_as<const_byte_buffer>; });
	static_assert(requires (std::span<const int> s) { { rod::as_bytes(s) } noexcept -> std::same_as<const_byte_buffer>; });
	static_assert(requires (std::array<const int, 2> s) { { rod::as_bytes(s) } noexcept -> std::same_as<const_byte_buffer>; });
}

template<>
inline constexpr bool std::ranges::enable_borrowed_range<rod::byte_buffer> = true;
template<>
inline constexpr bool std::ranges::enable_borrowed_range<rod::const_byte_buffer> = true;

template<>
inline constexpr bool std::ranges::enable_view<rod::byte_buffer> = true;
template<>
inline constexpr bool std::ranges::enable_view<rod::const_byte_buffer> = true;

static_assert(std::ranges::view<rod::byte_buffer>);
static_assert(std::ranges::view<rod::const_byte_buffer>);

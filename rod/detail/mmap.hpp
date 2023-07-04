/*
 * Created by switch_blade on 2023-07-04.
 */

#pragma once

#include "../result.hpp"

/* Platform-specific implementations. */
#if defined(__unix__)
#include "../unix/detail/mmap.hpp"
#elif defined(_WIN32)
#include "../win32/detail/mmap.hpp"
#else
#error Unsupported platform
#endif

namespace rod
{
	/** Handle to a memory-mapped region. */
	class mmap
	{
		using native_t = detail::system_mmap;

	public:
		using native_handle_type = typename native_t::native_handle_type;

		using mapmode = int;

		/** Memory region is mapped in copy-on-write mode. */
		static constexpr mapmode copy = native_t::copy;
		/** Memory region is mapped for reading. */
		static constexpr mapmode read = native_t::read;
		/** Memory region is mapped for writing. */
		static constexpr mapmode write = native_t::write;
		/** Underlying resource (file or shared memory object) is resized to fit the mapped region.
		 * @note This may be an implied behavior on certain systems. */
		static constexpr mapmode expand = native_t::expand;

	public:
		constexpr mmap() noexcept = default;
		constexpr mmap(mmap &&other) noexcept : _mmap(std::move(other._mmap)) {}
		constexpr mmap &operator=(mmap &&other) noexcept { return (_mmap = std::move(other._mmap), *this); }

		constexpr explicit mmap(native_t &&native) noexcept : _mmap(std::forward<native_t>(native)) {}

		/** Returns pointer to the start of the mapped range. */
		[[nodiscard]] std::byte *begin() noexcept { return static_cast<std::byte *>(data()); }
		/** @copydoc begin */
		[[nodiscard]] const std::byte *begin() const noexcept { return static_cast<const std::byte *>(data()); }
		/** @copydoc begin */
		[[nodiscard]] const std::byte *cbegin() const noexcept { return begin(); }

		/** Returns pointer to the end of the mapped range. */
		[[nodiscard]] std::byte *end() noexcept { return static_cast<std::byte *>(data()) + size(); }
		/** @copydoc begin */
		[[nodiscard]] const std::byte *end() const noexcept { return static_cast<const std::byte *>(data()) + size(); }
		/** @copydoc begin */
		[[nodiscard]] const std::byte *cend() const noexcept { return end(); }

		/** Checks if the handle is empty (does not represent mapped memory). */
		[[nodiscard]] bool empty() const noexcept { return _mmap.data(); }

		/** Returns pointer to the mapped range.
		 * @note Returned pointer may be different from the native (actual) pointer to the underlying memory block.
		 * This may be the case if, for example, an alignment offset is applied to the native pointer. */
		[[nodiscard]] void *data() const noexcept { return _mmap.data(); }
		/** Returns the native (actual) pointer to the underlying memory block. */
		[[nodiscard]] void *native_data() const noexcept { return _mmap.native_data(); }

		/** Returns the size of the mapped range.
		 * @note Returned size may be different from the native (actual) size of the underlying memory block.
		 * This may be the case if, for example, an alignment offset is applied to the native size. */
		[[nodiscard]] std::size_t size() const noexcept { return _mmap.size(); }
		/** Returns the native (actual) size of the underlying memory block. */
		[[nodiscard]] std::size_t native_size() const noexcept { return _mmap.native_size(); }

		/** Returns the underlying native handle to the memory mapping object.
		 * \@note Returned handle is usually (but not required to be) the same as the pointer returned by `native_data`. */
		[[nodiscard]] native_handle_type native_handle() const noexcept { return _mmap.native_handle(); }

		constexpr void swap(mmap &other) noexcept { _mmap.swap(other._mmap); }
		friend constexpr void swap(mmap &a, mmap &b) noexcept { a.swap(b); }

	private:
		native_t _mmap;
	};

	static_assert(std::ranges::contiguous_range<mmap>);
}

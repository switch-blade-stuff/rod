/*
 * Created by switchblade on 2023-05-30.
 */

#pragma once

#include "file.hpp"

/* Platform-specific implementations. */
#if defined(__unix__)
#include "unix/detail/mmap.hpp"
#elif defined(_WIN32)
#include "win32/detail/mmap.hpp"
#else
#error Unkonwn mmap platform
#endif

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	namespace _mmap
	{
		template<typename T>
		concept noexcept_sizeable_range = detail::nothrow_callable<decltype(std::ranges::begin), T> && detail::nothrow_callable<decltype(std::ranges::size), T>;

		class region
		{
			using native_t = detail::system_mmap;

		public:
			using native_file_handle = typename detail::system_file::native_handle_type;

			using mapprot = int;

			/** Memory is mapped with execute permissions. */
			static constexpr mapprot exec = native_t::exec;
			/** Memory is mapped with read permissions. */
			static constexpr mapprot read = native_t::read;
			/** Memory is mapped with write permissions. */
			static constexpr mapprot write = native_t::write;

			using mapmode = int;

			/** Memory is mapped in copy-on-write mode. */
			static constexpr mapmode copy = native_t::copy;
			/** Memory is mapped as a grows-down stack. */
			static constexpr mapmode stack = native_t::stack;
			/** Memory is mapped in shared mode. */
			static constexpr mapmode shared = native_t::shared;

		public:
			/** @brief Creates an unbacked memory mapping.
			 * @param[in] size Size of the created mapping. Actual size will be rounded up to a platform-specific multiple.
			 * @param[in] mode Mode flags to use for the mapping.
			 * @return `mmap_region` handle to the created mapping.
			 * @throw std::system_error On failure to map memory. */
			[[nodiscard]] static region map(std::size_t size, mapmode mode) { return map(nullptr, size, mode); }
			/** @copydoc map
			 * @param[in] hint Pointer to a memory location used as a mapping hint. */
			[[nodiscard]] static region map(void *hint, std::size_t size, mapmode mode)
			{
				std::error_code err;
				if (auto res = map(hint, size, mode, err); err)
					throw std::system_error(err, "rod::mmap_region::map");
				else
					return res;
			}
			/** @copydoc map
			 * @param[in] prot Protection flags to use for the mapping. */
			[[nodiscard]] static region map(void *hint, std::size_t size, mapmode mode, mapprot prot)
			{
				std::error_code err;
				if (auto res = map(hint, size, mode, prot, err); err)
					throw std::system_error(err, "rod::mmap_region::map");
				else
					return res;
			}

			/** @copybrief map
			 * @param[in] size Size of the created mapping. Actual size will be rounded up to a platform-specific multiple.
			 * @param[in] mode Mode flags to use for the mapping.
			 * @param[out] err Reference to the error code set on failure to map memory.
			 * @return `mmap_region` handle to the created mapping. */
			[[nodiscard]] static region map(std::size_t size, mapmode mode, std::error_code &err) { return map(nullptr, size, mode, err); }
			/** @copydoc map
			 * @param[in] hint Pointer to a memory location used as a mapping hint. */
			[[nodiscard]] static region map(void *hint, std::size_t size, mapmode mode, std::error_code &err) { return {native_t::map(hint, size, mode, err)}; }
			/** @copydoc map
			 * @param[in] prot Protection flags to use for the mapping. */
			[[nodiscard]] static region map(void *hint, std::size_t size, mapmode mode, mapprot prot, std::error_code &err) { return {native_t::map(hint, size, mode, prot, err)}; }

			/** @brief Maps a region of memory backed by the specified file.
			 * @param[in] file Native file handle to the file used as a backing for the mapped memory.
			 * @param[in] off Offset into the backing file pointing to the start of the mapped memory. Actual offset will be rounded down to a platform-specific multiple.
			 * @param[in] size Size of the created mapping. Actual size will be rounded up to a platform-specific multiple.
			 * @param[in] mode Mode flags to use for the mapping.
			 * @return `mmap_region` handle to the created mapping.
			 * @throw std::system_error On failure to map memory. */
			[[nodiscard]] static region map(native_file_handle file, std::size_t off, std::size_t size, mapmode mode) { return map(nullptr, file, off, size, mode); }
			/** @copydoc map
			 * @param[in] hint Pointer to a memory location used as a mapping hint. */
			[[nodiscard]] static region map(void *hint, native_file_handle file, std::size_t off, std::size_t size, mapmode mode)
			{
				std::error_code err;
				if (auto res = map(hint, file, off, size, mode, err); err)
					throw std::system_error(err, "rod::mmap_region::map");
				else
					return res;
			}
			/** @copydoc map
			 * @param[in] prot Protection flags to use for the mapping. */
			[[nodiscard]] static region map(void *hint, native_file_handle file, std::size_t off, std::size_t size, mapmode mode, mapprot prot)
			{
				std::error_code err;
				if (auto res = map(hint, file, off, size, mode, prot, err); err)
					throw std::system_error(err, "rod::mmap_region::map");
				else
					return res;
			}

			/** @copybrief map
			 * @param[in] file Native file handle to the file used as a backing for the mapped memory.
			 * @param[in] off Offset into the backing file pointing to the start of the mapped memory.
			 * @param[in] size Size of the created mapping. Actual size will be rounded to a platform-specific multiple.
			 * @param[in] mode Mode flags to use for the mapping.
			 * @param[out] err Reference to the error code set on failure to map memory.
			 * @return `mmap_region` handle to the created mapping. */
			[[nodiscard]] static region map(native_file_handle file, std::size_t off, std::size_t size, mapmode mode, std::error_code &err) { return map(nullptr, file, off, size, mode, err); }
			/** @copydoc map
			 * @param[in] hint Pointer to a memory location used as a mapping hint. */
			[[nodiscard]] static region map(void *hint, native_file_handle file, std::size_t off, std::size_t size, mapmode mode, std::error_code &err) { return {native_t::map(hint, size, file, off, mode, err)}; }
			/** @copydoc map
			 * @param[in] prot Protection flags to use for the mapping. */
			[[nodiscard]] static region map(void *hint, native_file_handle file, std::size_t off, std::size_t size, mapmode mode, mapprot prot, std::error_code &err) { return {native_t::map(hint, size, file, off, mode, prot, err)}; }

		private:
			constexpr region(native_t &&mmap) noexcept : m_mmap(std::move(mmap)) {}

		public:
			/** Initializes an empty mapping handle. */
			constexpr region() noexcept = default;

			/** Initializes a mapping handle from the specified data pointer and size. */
			constexpr region(void *data, std::size_t size) noexcept : m_mmap(data, size) {}
			/** Initializes a mapping handle from the specified base pointer, total size, and base offset. */
			constexpr region(void *data, std::size_t size, std::size_t off) noexcept : m_mmap(data, size, off) {}

			/** @brief Unmaps the underlying memory region.
			 * @param[out] err Reference to the error code set on failure to unmap memory. */
			void unmap(std::error_code &err) noexcept { err = m_mmap.unmap(); }
			/** @copybrief unmap
			  * @throw std::system_error On failure to unmap memory. */
			void unmap() { if (auto err = m_mmap.unmap(); err) throw std::system_error(err, "rod::mmap_region::unmap"); }

			/** Releases the underlying memory mapping.
			 * @return Pair where first is the base pointer to the mapping and second is the total base size of the mapping. */
			constexpr std::pair<void *, std::size_t> release() noexcept { return m_mmap.release(); }

			/** @brief Remaps the underlying memory mapping to the specified size.
			 * @param[in] new_size New size of the underlying memory mapping.
			 * @return Reference to `this`.
			 * @throw std::system_error On failure to remap memory. */
			region &remap(std::size_t new_size)
			{
				std::error_code err;
				if (remap(new_size, err); err)
					throw std::system_error(err, "rod::mmap_region::remap");
				else
					return *this;
			}
			/** @copybrief remap
			 * @param[in] new_size New size of the underlying memory mapping.
			 * @param[out] err Reference to the error code set on failure to remap memory.
			 * @return Reference to `this`. */
			region &remap(std::size_t new_size, std::error_code &err) noexcept { return (err = m_mmap.remap(new_size), *this); }

			/** Checks if the handle is empty (does not reference a mapping). */
			[[nodiscard]] constexpr bool empty() const noexcept { return m_mmap.empty(); }
			/** @copydoc empty */
			[[nodiscard]] constexpr operator bool() const noexcept { return !m_mmap.empty(); }

			/** Returns pointer to the underlying memory mapping.
			 * @note Returned pointer takes into account the base offset. */
			[[nodiscard]] constexpr void *data() const noexcept { return m_mmap.data(); }
			/** Returns the size of the underlying memory mapping.
			 * @note Returned size takes into account the base offset. */
			[[nodiscard]] constexpr std::size_t size() const noexcept { return m_mmap.size(); }

			/** Returns base pointer to the underlying memory mapping. */
			[[nodiscard]] constexpr void *base_data() const noexcept { return m_mmap.base_data(); }
			/** Returns the base size of the underlying memory mapping. */
			[[nodiscard]] constexpr std::size_t base_size() const noexcept { return m_mmap.base_size(); }

			/** Returns a span of bytes pointing to the underlying memory mapping. */
			[[nodiscard]] std::span<std::byte> bytes() const noexcept { return as_byte_buffer(data(), size()); }
			/** @copydoc bytes */
			[[nodiscard]] explicit operator std::span<std::byte>() const noexcept { return bytes(); }

			constexpr void swap(region &other) noexcept { m_mmap.swap(other.m_mmap); }
			friend constexpr void swap(region &a, region &b) noexcept { a.swap(b); }

		private:
			native_t m_mmap;
		};
	}

	/** Handle to a mapped region of memory. */
	using mmap_region = _mmap::region;
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

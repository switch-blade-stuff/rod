/*
 * Created by switchblade on 2023-11-17.
 */

#pragma once

#include "file_handle.hpp"

namespace rod
{
	namespace _mmap
	{
		using extent_pair = std::pair<_handle::extent_type, _handle::extent_type>;

		/** Flags used to control behavior of memory mapping handles and source objects. */
		enum class mmap_flags : std::uint16_t
		{
			/** No access. */
			none = 0,
			/** Map for read-only access. */
			read = 0x1,
			/** Map for executable access. */
			exec = 0x2,
			/** Map for copy-on-write access. Mutually exclusive with `write`.
			 * @note Certain platforms such as Windows do not support write-only mappings. In such cases `copy` and `copy | read` are equivalent. */
			copy = 0x4,
			/** Map for write-only or read-write access. Mutually exclusive with `copy`.
			 * @note Certain platforms such as Windows do not support write-only mappings. In such cases `write` and `write | read` are equivalent. */
			write = 0x8,

			/** Map for read and copy-on-write access. */
			readcopy = read | copy,
			/** Map for read and write access. */
			readwrite = read | write,

			/** Commit mapped pages to the process address space. Mutually exclusive with `reserve`.
			 * @note `commit` is assumed to be default if neither of `commit` or `reserve` is specified. */
			commit = 0x10,
			/** Reserve but dont commit mapped pages from the process address space. Mutually exclusive with `commit`.
			 * @note Reserved pages must be manually comitted to the address space via `mmap_handle::commit`. */
			reserve = 0x20,
			/** Prefault mapped pages. Requires `commit`. */
			prefault = 0x40,

			/** Mapped file is to be treated as a dynamic-linkage binary rather than a file. */
			system_image = 0x100,
			/** All processes mapping the same underlying resource will share the mapped pages. */
			shared_memory = 0x200,
			/** Block until pending changes are flushed to underlying device when a mapping handle of this mapping source is closed. */
			sync_on_close = 0x400,
			/** Hint to the operating system that mapped memory is to be allocated from large pages.
			 * @note May require elevated privileges under Windows. */
			map_large_pages = 0x800,
		};

		[[nodiscard]] constexpr mmap_flags operator~(mmap_flags h) noexcept { return mmap_flags(~std::uint16_t(h)); }
		[[nodiscard]] constexpr mmap_flags operator&(mmap_flags a, mmap_flags b) noexcept { return mmap_flags(std::uint16_t(a) & std::uint16_t(b)); }
		[[nodiscard]] constexpr mmap_flags operator|(mmap_flags a, mmap_flags b) noexcept { return mmap_flags(std::uint16_t(a) | std::uint16_t(b)); }
		[[nodiscard]] constexpr mmap_flags operator^(mmap_flags a, mmap_flags b) noexcept { return mmap_flags(std::uint16_t(a) ^ std::uint16_t(b)); }
		constexpr mmap_flags &operator&=(mmap_flags &a, mmap_flags b) noexcept { return a = a & b; }
		constexpr mmap_flags &operator|=(mmap_flags &a, mmap_flags b) noexcept { return a = a | b; }
		constexpr mmap_flags &operator^=(mmap_flags &a, mmap_flags b) noexcept { return a = a ^ b; }

		/** Object represenging source of a memory mapping such as a file or a memory-backed shared mapping. */
		class mmap_source : public handle_adaptor<mmap_source>
		{
			using adaptor = handle_adaptor<mmap_source>;
			friend adaptor;

		public:
			/** Creates a memory mapping source backed by an anonymous temporary file.
			 * @param size Initial size of the anonymous temporary file.
			 * @param base Temporary directory used to create the backing temporary file.
			 * @param flags Flags used to create the mapping source and map memory. `mmap_flags::readwrite` by default.
			 * @return Handle to the mapping source or a status code on failure. */
			[[nodiscard]] static ROD_API_PUBLIC result<mmap_source> open(extent_type size, const fs::path_handle &base = fs::temp_file_directory(), mmap_flags flags = mmap_flags::readwrite) noexcept;

			/** Creates a memory mapping source backed by the file \a file.
			 * @param file File containing data to be mapped into memory.
			 * @param size Maximum size of the mapping source. If set to `0`, uses full file size.
			 * @param flags Flags used to create the mapping source and map memory.
			 * @return Handle to the mapping source or a status code on failure. */
			[[nodiscard]] static ROD_API_PUBLIC result<mmap_source> open(const fs::file_handle &file, extent_type size, mmap_flags flags) noexcept;
			/** Creates a memory mapping source backed by the file \a file.
			 * @param file File containing data to be mapped into memory.
			 * @param size Maximum size of the mapping source. If set to `0`, uses the entire file.
			 * @return Handle to the mapping source or a status code on failure. */
			[[nodiscard]] static result<mmap_source> open(const fs::file_handle &file, extent_type size = 0) noexcept { return open(file, size, bool(file.flags() & fs::file_flags::write) ? mmap_flags::readwrite : mmap_flags::read); }

		public:
			mmap_source(const mmap_source &) = delete;
			mmap_source &operator=(const mmap_source &) = delete;

			/** Initializes a closed memory mapping source handle. */
			mmap_source() noexcept = default;
			mmap_source(mmap_source &&other) noexcept : adaptor(std::forward<adaptor>(other)) {}
			mmap_source &operator=(mmap_source &&other) noexcept { return (adaptor::operator=(std::forward<adaptor>(other)), *this); }

			/** Initializes memory mapping source handle from a basic handle rvalue and flags. */
			explicit mmap_source(basic_handle &&hnd, mmap_flags flags) noexcept : mmap_source(hnd.release(), flags) {}
			/** Initializes memory mapping source handle from a native handle and flags. */
			explicit mmap_source(typename adaptor::native_handle_type hnd, mmap_flags flags) noexcept : adaptor(typename adaptor::native_handle_type(hnd, std::uint32_t(flags))) {}

			/** Returns the flags of the mapping source handle. */
			[[nodiscard]] constexpr mmap_flags flags() const noexcept { return mmap_flags(native_handle().flags); }

			constexpr void swap(mmap_source &other) noexcept { adaptor::swap(other); }
			friend constexpr void swap(mmap_source &a, mmap_source &b) noexcept { a.swap(b); }

		public:
			template<decays_to_same<endpos_t> Op, decays_to_same<mmap_source> Hnd>
			friend auto tag_invoke(Op, Hnd &&hnd) noexcept { return hnd.do_endpos(); }
			template<decays_to_same<truncate_t> Op, decays_to_same<mmap_source> Hnd, std::convertible_to<extent_type> Ext>
			friend auto tag_invoke(Op, Hnd &&hnd, Ext &&endp) noexcept { return hnd.do_truncate(endp); }

		private:
			auto do_clone() const noexcept { return clone(base()).transform_value([&](basic_handle &&hnd) { return mmap_source(std::move(hnd), flags()); }); }

			ROD_API_PUBLIC result<extent_type> do_endpos() const noexcept;
			ROD_API_PUBLIC result<extent_type> do_truncate(extent_type endp) noexcept;
		};

		static_assert(handle<mmap_source>);

		/** Handle to a view of a memory-mapped region of a file or a memory-backed shared mapping. */
		class mmap_handle
		{
		public:
			using offset_type = typename mmap_source::offset_type;
			using extent_type = typename mmap_source::extent_type;
			using size_type = typename mmap_source::size_type;

		public:
			/* TODO: Document usage */
			[[nodiscard]] static ROD_API_PUBLIC result<mmap_handle> map(size_type size, mmap_flags flags = mmap_flags::readwrite | mmap_flags::commit) noexcept;
			/* TODO: Document usage */
			[[nodiscard]] static ROD_API_PUBLIC result<mmap_handle> map(const mmap_source &src, extent_type offset = 0, size_type size = 0, mmap_flags flags = mmap_flags::readwrite) noexcept;

		public:
			mmap_handle(const mmap_handle &) = delete;
			mmap_handle &operator=(const mmap_handle &) = delete;

			constexpr mmap_handle() noexcept = default;
			constexpr mmap_handle(mmap_handle &&other) noexcept { swap(other); }
			constexpr mmap_handle &operator=(mmap_handle &&other) noexcept { return (swap(other), *this); }

			/** Initializes a memory view not backed by a memory resource.
			 * @param base_ptr Pointer to the start of the memory mapped view (base view).
			 * @param base_off Byte offset from \a base_ptr of the start of the data of the memory mapped view (data view).
			 * @param data_size Size of the data view in bytes.
			 * @param page_size Size of the virtual memory pages used by the base view.
			 * @param reserved Total size of the virtual memory used by the base view.
			 * @param flags Flags of the memory view. */
			constexpr mmap_handle(std::byte *base_ptr, extent_type base_off, size_type data_size, size_type page_size, extent_type reserved, mmap_flags flags) noexcept
					: _flags(flags), _has_source(false), _base_ptr(base_ptr), _base_off(base_off), _reserved(reserved), _data_size(data_size), _page_size(page_size) {}
			/** Initializes a memory view with a backing memory resource.
			 * @param base_ptr Pointer to the start of the memory mapped view (base view).
			 * @param base_off Byte offset from \a base_ptr of the start of the data of the memory mapped view (data view).
			 * @param data_size Size of the data view in bytes.
			 * @param page_size Size of the virtual memory pages used by the base view.
			 * @param reserved Total size of the virtual memory used by the base view.
			 * @param flags Flags of the memory view.
			 * @param src Mapping source the view is mapped from.
			 * @note Lifetime of \a src is not required to exceed the lifetime of the memory view. */
			constexpr mmap_handle(std::byte *base_ptr, extent_type base_off, size_type data_size, size_type page_size, extent_type reserved, mmap_flags flags, const mmap_source &) noexcept
					: _flags(flags), _has_source(true), _base_ptr(base_ptr), _base_off(base_off), _reserved(reserved), _data_size(data_size), _page_size(page_size) {}

			~mmap_handle() { if (!empty()) reset(); }

			/** Checks if the handle points to a valid memory view. */
			[[nodiscard]] constexpr bool empty() const noexcept { return _base_ptr == nullptr; }
			/** Checks if the handle was mapped from a memory resource. */
			[[nodiscard]] constexpr bool has_source() const noexcept { return _has_source; }

			/** Returns the mode of the memory view. */
			[[nodiscard]] constexpr mmap_flags flags() const noexcept { return _flags; }

			/** Returns the base offset of the memory view. */
			[[nodiscard]] constexpr extent_type offset() const noexcept { return _base_off; }
			/** Returns the reserved size of the memory view. */
			[[nodiscard]] constexpr extent_type reserved() const noexcept { return _reserved; }

			/** Returns the page size of the memory view. */
			[[nodiscard]] constexpr size_type page_size() const noexcept { return _page_size; }
			/** Returns the base size of the memory view. */
			[[nodiscard]] constexpr size_type base_size() const noexcept { return _data_size + offset(); }
			/** Returns the data size of the memory view. */
			[[nodiscard]] constexpr size_type data_size() const noexcept { return _data_size; }

			/** Returns pointer to the view base. */
			[[nodiscard]] constexpr std::byte *base() const noexcept { return _base_ptr; }
			/** Returns pointer to the view data at `offset` from view base. */
			[[nodiscard]] constexpr std::byte *data() const noexcept { return base() + offset(); }

			/** Returns the mapped memory view as a span of bytes starting at `0` from view base. */
			[[nodiscard]] constexpr std::span<std::byte> as_base_span() const noexcept { return {base(), base_size()}; }
			/** Returns the mapped memory view as a span of bytes starting at `offset` from view base. */
			[[nodiscard]] constexpr std::span<std::byte> as_data_span() const noexcept { return {data(), data_size()}; }

			/** Un-maps the memory and releases the handle. */
			ROD_API_PUBLIC result<void> reset() noexcept;
			/** Releases the handle and returns the base view pointer.
			 * @note View memory must be manually un-mapped by the user. */
			[[nodiscard]] constexpr std::byte *release() noexcept
			{
				const auto base = _base_ptr;
				operator=(mmap_handle());
				return base;
			}

			/** Flushes range of memory specified by \a ext to the backing resource.
			 * @note Extent offset should be specified relative to `0` from view base.
			 * @note Does not call `sync` on the source file handle and only flushes the mapped view.
			 * @note Flushing is not supported for non-backed views and will return `std::errc::not_supported`. */
			ROD_API_PUBLIC result<void> flush(extent_pair ext) noexcept;
			/** Discards pages in the memory range specified by \a ext.
			 * @note Extent offset should be specified relative to `0` from view base.
			 * @note Discarding pages is not supported for non-backed views and will return `std::errc::not_supported`. */
			ROD_API_PUBLIC result<void> discard(extent_pair ext) noexcept;

			/** Commits reserved range of memory specified by \a ext to process address space.
			 * @note Extent offset should be specified relative to `0` from view base. */
			ROD_API_PUBLIC result<void> commit(extent_pair ext) noexcept;
			/** Decommits range of memory specified by \a ext from process address space.
			 * @note Extent offset should be specified relative to `0` from view base. */
			ROD_API_PUBLIC result<void> decommit(extent_pair ext) noexcept;

			/** Fills memory range specified by \a ext with zeros.
			 * @note Extent offset should be specified relative to `0` from view base. */
			ROD_API_PUBLIC result<void> zero(extent_pair ext) noexcept;
			/** Prefaults ranges of memory specified by \a exts.
			 * @note Extent offset should be specified relative to `0` from view base. */
			ROD_API_PUBLIC result<void> prefault(std::span<const extent_pair> exts) noexcept;
			/** Prefaults the extent \a ext.
			 * @note Extent offset should be specified relative to `0` from view base. */
			result<void> prefault(extent_pair ext) noexcept { return prefault(std::initializer_list<extent_pair>{ext}); }

			constexpr void swap(mmap_handle &other) noexcept
			{
				std::swap(_flags, other._flags);
				std::swap(_has_source, other._has_source);

				std::swap(_base_ptr, other._base_ptr);
				std::swap(_base_off, other._base_off);
				std::swap(_reserved, other._reserved);
				std::swap(_data_size, other._data_size);
				std::swap(_page_size, other._page_size);
			}
			friend constexpr void swap(mmap_handle &a, mmap_handle &b) noexcept { a.swap(b); }

		private:
			/* Source pointer is only used to indicate that we are actually mapped to a resource and are not just virtual memory. */
			mmap_flags _flags = {};
			bool _has_source = {};

			std::byte *_base_ptr = {};
			extent_type _base_off = 0;
			extent_type _reserved = 0;
			size_type _data_size = 0;
			size_type _page_size = 0;
		};
	}

	using _mmap::mmap_flags;
	using _mmap::mmap_source;
	using _mmap::mmap_handle;
}

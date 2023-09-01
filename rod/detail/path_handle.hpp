/*
 * Created by switch_blade on 2023-08-08.
 */

#pragma once

#include "handle_base.hpp"

namespace rod
{
	namespace _path
	{
		/** Lightweight handle to a unique directory within the filesystem. */
		class path_handle : public handle_adaptor<path_handle>
		{
			friend handle_adaptor<path_handle>;

		public:
			/** Opens a path handle for an existing directory specified by \a path. */
			[[nodiscard]] static ROD_API_PUBLIC result<path_handle> open(path_view path) noexcept;
			/** Opens a path handle for an existing directory specified by \a path relative to parent location referenced by \a base. */
			[[nodiscard]] static ROD_API_PUBLIC result<path_handle> open(const path_handle &base, path_view path) noexcept;

		public:
			path_handle(const path_handle &) = delete;
			path_handle &operator=(const path_handle &) = delete;

			/** Initializes a closed path handle. */
			path_handle() noexcept = default;
			path_handle(path_handle &&) noexcept = default;
			path_handle &operator=(path_handle &&) noexcept = default;

			/** Initializes path handle from a native handle. */
			explicit path_handle(native_handle_type hnd) noexcept : handle_adaptor<path_handle>(hnd) {}
			/** Converts a basic handle to a path handle. */
			explicit path_handle(basic_handle &&other) noexcept : handle_adaptor<path_handle>(std::forward<basic_handle>(other)) {}

			constexpr void swap(path_handle &other) noexcept { handle_adaptor<path_handle>::swap(other); }
			friend constexpr void swap(path_handle &a, path_handle &b) noexcept { a.swap(b); }
		};

#if 0
		/** @brief Handle to a unique directory within the filesystem.
		 *
		 * `directory_handle` works as an extended version of `path_handle` (and is a subclass of it),
		 * providing additional support for directory creation, enumeration, and filesystem queries. */
		class directory_handle : public path_handle
		{
		public:
			/** Opens or creates a directory specified by \a path.
			 * @param path Path to the directory.
			 * @param flags Handle flags to open the handle with. Opens directory for reading by default.
			 * @param mode Mode to use for opening the handle. Only opens existing directories by default.
			 * @return Result containing the opened handle or a status code on failure to open the handle.
			 * @note The following flags are not supported and will result in an error: `unlink_on_close`, `no_sparse_files`.
			 * @note The following modes are not supported and will result in an error: `truncate`. */
			[[nodiscard]] static ROD_API_PUBLIC result<directory_handle> open(path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::existing) noexcept;
			/** Opens or creates a directory specified by \a path relative to parent location referenced by \a base.
			 * @param base Handle to the base directory.
			 * @param path Path to the leaf location within \a base.
			 * @param flags Handle flags to open the handle with. Opens directory for reading by default.
			 * @param mode Mode to use for opening the handle. Only opens existing directories by default.
			 * @return Result containing the opened handle or a status code on failure to open the handle.
			 * @note The following flags are not supported and will result in an error: `unlink_on_close`, `no_sparse_files`.
			 * @note The following modes are not supported and will result in an error: `truncate`. */
			[[nodiscard]] static ROD_API_PUBLIC result<directory_handle> open(const path_handle &base, path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::existing) noexcept;

			/** Creates a new uniquely-named directory relative to parent location referenced by \a base.
			 * @note The following flags are not supported and will result in an error: `unlink_on_close`, `no_sparse_files`. */
			[[nodiscard]] static ROD_API_PUBLIC result<directory_handle> open_unique(const path_handle &base, file_flags flags = file_flags::read | file_flags::write) noexcept;
			/** Opens or creates a directory specified by \a path relative to the system's temporary file directory.
			 * @note If \a path is empty, generates an implementation-defined unique name.
			 * @note The following flags are not supported and will result in an error: `unlink_on_close`, `no_sparse_files`.
			 * @note The following modes are not supported and will result in an error: `truncate`. */
			[[nodiscard]] static ROD_API_PUBLIC result<directory_handle> open_temporary(path_view path = {}, file_flags flags = file_flags::read | file_flags::write, open_mode mode = open_mode::always) noexcept;

		public:
			directory_handle(const directory_handle &) = delete;
			directory_handle &operator=(const directory_handle &) = delete;

			/** Initializes a closed directory handle. */
			directory_handle() noexcept = default;
			directory_handle(directory_handle &&) noexcept = default;
			directory_handle &operator=(directory_handle &&) noexcept = default;

			/** Initializes directory handle from a native handle. */
			explicit directory_handle(native_handle_type hnd) noexcept : path_handle(hnd) {}
			/** Initializes directory handle from a native handle and explicit device & inode IDs. */
			explicit directory_handle(native_handle_type hnd, dev_t dev, ino_t ino) noexcept : path_handle(hnd, dev, ino) {}

			/** Converts a filesystem handle to a directory handle. */
			explicit directory_handle(path_handle &&other) noexcept : path_handle(std::forward<path_handle>(other)) {}
			/** Converts a filesystem handle to a directory handle with explicit device & inode IDs. */
			explicit directory_handle(path_handle &&other, dev_t dev, ino_t ino) noexcept : path_handle(std::forward<path_handle>(other), dev, ino) {}

			/** Duplicates the directory handle. */
			[[nodiscard]] result<directory_handle> clone() const noexcept { return result<directory_handle>(path_handle::clone()); }

			constexpr void swap(directory_handle &other) noexcept { path_handle::swap(other); }
			friend constexpr void swap(directory_handle &a, directory_handle &b) noexcept { a.swap(b); }

		private:
			std::uint32_t _flags = 0;
		};
#endif
	}

	using _path::path_handle;
}

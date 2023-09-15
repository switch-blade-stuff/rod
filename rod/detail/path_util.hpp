/*
 * Created by switch_blade on 2023-08-31.
 */

#pragma once

#include "directory_handle.hpp"

namespace rod::fs
{
	/** Returns path of the current directory as if via `getcwd`. */
	[[nodiscard]] ROD_API_PUBLIC auto current_path() noexcept -> result<path>;
	/** Changes path of the current directory to \a path as if via `chdir`. */
	[[nodiscard]] ROD_API_PUBLIC auto current_path(path_view path) noexcept -> result<>;

	/** Checks if \a path references a valid filesystem location.
	 * @errors
	 * <ul>
	 * <li>`std::errc::bad_alloc` on failure to allocate internal buffers.</li>
	 * <li>Other errors returned by the underlying OS functions.</li>
	 * </ul> */
	[[nodiscard]] ROD_API_PUBLIC result<bool> exists(path_view path) noexcept;
	/** Checks if path views \a a and \a b reference the same filesystem object.
	 * @errors Errors returned by `get_stat`. */
	[[nodiscard]] ROD_API_PUBLIC result<bool> equivalent(path_view a, path_view b) noexcept;

	/** Returns the canonical version of \a path. Canonical path has no relative path elements or symlinks and is guaranteed to reference an existing filesystem location.
	 * @return Weakly-canonical version of \a path or a status code on failure.
	 * @errors
	 * <ul>
	 * <li>`std::errc::no_such_file_or_directory` if \a path does not reference a valid filesystem location.</li>
	 * <li>`std::errc::bad_alloc` on failure to allocate path memory.</li>
	 * <li>Errors returned by `to_native_path`.</li>
	 * </ul> */
	[[nodiscard]] ROD_API_PUBLIC result<path> canonical(path_view path) noexcept;
	/** Returns the weakly-canonical version of \a path. Weakly-canonical path is the same as the canonical path except it is not required to reference an existing filesystem location.
	 * @return Weakly-canonical version of \a path or a status code on failure.
	 * @errors
	 * <ul>
	 * <li>`std::errc::bad_alloc` on failure to allocate path memory.</li>
	 * <li>Errors returned by `to_native_path`.</li>
	 * </ul> */
	[[nodiscard]] ROD_API_PUBLIC result<path> weakly_canonical(path_view path) noexcept;

	/** Returns the absolute version of \a path as if via `std::filesystem::current_path() / p`.
	 * @return Weakly-canonical version of \a path or a status code on failure.
	 * @errors
	 * <ul>
	 * <li>`std::errc::no_such_file_or_directory` if \a path does not reference a valid filesystem location.</li>
	 * <li>`std::errc::bad_alloc` on failure to allocate path memory.</li>
	 * <li>Errors returned by `GetFullPathNameW` on Windows.</li>
	 * </ul> */
	[[nodiscard]] ROD_API_PUBLIC result<path> absolute(path_view path) noexcept;
	/* TODO: Document usage. */
	[[nodiscard]] ROD_API_PUBLIC result<path> relative(path_view path) noexcept;
	/* TODO: Document usage. */
	[[nodiscard]] ROD_API_PUBLIC result<path> proximate(path_view path) noexcept;
	/* TODO: Document usage. */
	[[nodiscard]] ROD_API_PUBLIC result<path> relative(path_view path, path_view base) noexcept;
	/* TODO: Document usage. */
	[[nodiscard]] ROD_API_PUBLIC result<path> proximate(path_view path, path_view base) noexcept;

	/* TODO: Implement. */
	/** Creates all directories specified by \a path relative to parent location \a base.
	 * @param base Handle to the parent location. If set to an invalid handle, \a path must be a fully-qualified path.
	 * @param path Path to the target directory relative to \a base if it is a valid handle, otherwise a fully-qualified path.
	 * @param to Optional timeout to use when creating the filesystem object.
	 * @return Amount of directories created or a status code on failure. */
	[[nodiscard]] ROD_API_PUBLIC result<std::size_t> create_all(const path_handle &base, path_view path, const file_timeout &to = file_timeout()) noexcept;

	/** Removes the directory, file, or symlink specified by \a path relative to parent location \a base.
	 * Equivalent to opening the target object handle with sufficient permissions and calling `unlink`.
	 * @param base Handle to the parent location. If set to an invalid handle, \a path must be a fully-qualified path.
	 * @param path Path to the target object relative to \a base if it is a valid handle, otherwise a fully-qualified path.
	 * @param to Optional timeout to use when removing the filesystem object.
	 * @return Amount of filesystem objects removed or a status code on failure.
	 * @errors
	 * <ul>
	 * <li>`std::errc::directory_not_empty` if the specified directory is not empty.</li>
	 * <li>`std::errc::bad_alloc` on failure to allocate internal buffers.</li>
	 * <li>Errors returned by `rmdir` on POSIX or `NtGetInformationFile` and `NtSetInformationFile` on Windows.</li>
	 * </ul> */
	[[nodiscard]] ROD_API_PUBLIC result<std::size_t> remove(const path_handle &base, path_view path, const file_timeout &to = file_timeout()) noexcept;
	/** Removes all directories, files and symlinks specified by \a path relative to parent location \a base.
	 * @param base Handle to the parent location. If set to an invalid handle, \a path must be a fully-qualified path.
	 * @param path Path to the target object relative to \a base if it is a valid handle, otherwise a fully-qualified path.
	 * @param to Optional timeout to use when removing the filesystem objects.
	 * @return Amount of filesystem objects removed or a status code on failure.
	 * @errors
	 * <ul>
	 * <li>`std::errc::bad_alloc` on failure to allocate internal buffers.</li>
	 * <li>Errors returned by `rmdir` on POSIX or `NtQueryDirectoryFile`, `NtGetInformationFile`, and `NtSetInformationFile` on Windows.</li>
	 * </ul> */
	[[nodiscard]] ROD_API_PUBLIC result<std::size_t> remove_all(const path_handle &base, path_view path, const file_timeout &to = file_timeout()) noexcept;

	/** Flags used to control behavior of the `copy_all` function. */
	enum class copy_mode : int
	{
		none = 0,

		/** Enable copying of files. */
		files = 0x1,
		/** Enable copying of directories. */
		directories = 0x2,
		/** Ignore all symlinks. */
		ignore_links = 0x4,
		/** Copy the target of symlinks. */
		follow_links = 0x8,

		/** Overwrite existing files and directories. */
		overwrite = 0x10,
		/** Recursively copy content of subdirectories. */
		recursive = 0x20,

		/** Create symlinks instead of copying contents. Mutually exclusive with `create_hardlinks`. */
		create_symlinks = 0x40,
		/** Create hardlinks instead of copying contents. Mutually exclusive with `create_symlinks`. */
		create_hardlinks = 0x80,
	};

	[[nodiscard]] constexpr copy_mode operator~(copy_mode h) noexcept { return copy_mode(~int(h)); }
	[[nodiscard]] constexpr copy_mode operator&(copy_mode a, copy_mode b) noexcept { return copy_mode(int(a) & int(b)); }
	[[nodiscard]] constexpr copy_mode operator|(copy_mode a, copy_mode b) noexcept { return copy_mode(int(a) | int(b)); }
	[[nodiscard]] constexpr copy_mode operator^(copy_mode a, copy_mode b) noexcept { return copy_mode(int(a) ^ int(b)); }
	constexpr copy_mode &operator&=(copy_mode &a, copy_mode b) noexcept { return a = a & b; }
	constexpr copy_mode &operator|=(copy_mode &a, copy_mode b) noexcept { return a = a | b; }
	constexpr copy_mode &operator^=(copy_mode &a, copy_mode b) noexcept { return a = a ^ b; }

	/* TODO: Implement. */
	/** Duplicates all directories, files and symlinks specified by \a path relative to parent location \a base using mode flags \a mode.
	 * @param base Handle to the parent location. If set to an invalid handle, \a path must be a fully-qualified path.
	 * @param path Path to the target file or directory relative to \a base if it is a valid handle, otherwise a fully-qualified path.
	 * @param to Optional timeout to use when copying the filesystem object.
	 * @return Amount of elements copied or a status code on failure. */
	[[nodiscard]] ROD_API_PUBLIC result<std::size_t> copy_all(const path_handle &base, path_view path, copy_mode mode, const file_timeout &to = file_timeout()) noexcept;
}

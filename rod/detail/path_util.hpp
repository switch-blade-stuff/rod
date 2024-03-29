/*
 * Created by switchblade on 2023-08-31.
 */

#pragma once

#include "directory_handle.hpp"

namespace rod::fs
{
	/** Returns path of the current directory as if via `getcwd`. */
	[[nodiscard]] ROD_API_PUBLIC result<path> current_path() noexcept;
	/** Changes path of the current directory to \a path as if via `chdir`. */
	ROD_API_PUBLIC result<void> current_path(path_view path) noexcept;

	/** Checks if \a path references a valid filesystem location.
	 * @errors
	 * <ul>
	 * <li>`std::errc::not_enough_memory` on failure to allocate internal buffers.</li>
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
	 * <li>`std::errc::not_enough_memory` on failure to allocate path memory.</li>
	 * <li>Errors returned by `to_native_path`.</li>
	 * </ul> */
	[[nodiscard]] ROD_API_PUBLIC result<path> canonical(path_view path) noexcept;
	/** Returns the weakly-canonical version of \a path. Weakly-canonical path is the same as the canonical path except it is not required to reference an existing filesystem location.
	 * @return Weakly-canonical version of \a path or a status code on failure.
	 * @errors
	 * <ul>
	 * <li>`std::errc::not_enough_memory` on failure to allocate path memory.</li>
	 * <li>Errors returned by `to_native_path`.</li>
	 * </ul> */
	[[nodiscard]] ROD_API_PUBLIC result<path> weakly_canonical(path_view path) noexcept;

	/** Returns the absolute version of \a path as if via `std::filesystem::current_path() / p`.
	 * @return Weakly-canonical version of \a path or a status code on failure.
	 * @errors
	 * <ul>
	 * <li>`std::errc::no_such_file_or_directory` if \a path does not reference a valid filesystem location.</li>
	 * <li>`std::errc::not_enough_memory` on failure to allocate path memory.</li>
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

	/** Creates all directories specified by \a path relative to parent location \a base.
	 * @param base Handle to the parent location. If set to an invalid handle, \a path must be a fully-qualified path.
	 * @param path Path to the target directory relative to \a base if it is a valid handle, otherwise a fully-qualified path.
	 * @return Amount of directories created or a status code on failure. */
	ROD_API_PUBLIC result<std::size_t> create_directories(const path_handle &base, path_view path) noexcept;

	/** Flags used to control behavior of the `copy_all` function. */
	enum class copy_mode : int
	{
		none = 0,

		/** Enable copying of files. */
		files = 0x1,
		/** Enable copying of directories.
		 * @note Mutually exclusive with and `copy_mode::create_hardlinks`. */
		directories = 0x2,

		/** Overwrite existing files and directories. */
		overwrite = 0x4,
		/** Recursively copy subdirectory contents. */
		recursive = 0x8,

		/** Ignore links instead of copying. */
		ignore_links = 0x10,
		/** Copy the content of link targets. */
		follow_links = 0x20,

		/** Create symlinks instead of cloning contents.
		 * @note Mutually exclusive with and `copy_mode::create_hardlinks`. */
		create_symlinks = 0x40,
		/** Create hardlinks instead of cloning contents.
		 * @note Mutually exclusive with `copy_mode::create_symlinks` and `copy_mode::directories`. */
		create_hardlinks = 0x80,
	};

	[[nodiscard]] constexpr copy_mode operator~(copy_mode h) noexcept { return copy_mode(~int(h)); }
	[[nodiscard]] constexpr copy_mode operator&(copy_mode a, copy_mode b) noexcept { return copy_mode(int(a) & int(b)); }
	[[nodiscard]] constexpr copy_mode operator|(copy_mode a, copy_mode b) noexcept { return copy_mode(int(a) | int(b)); }
	[[nodiscard]] constexpr copy_mode operator^(copy_mode a, copy_mode b) noexcept { return copy_mode(int(a) ^ int(b)); }
	constexpr copy_mode &operator&=(copy_mode &a, copy_mode b) noexcept { return a = a & b; }
	constexpr copy_mode &operator|=(copy_mode &a, copy_mode b) noexcept { return a = a | b; }
	constexpr copy_mode &operator^=(copy_mode &a, copy_mode b) noexcept { return a = a ^ b; }

	/** TODO: Document usage.
	 * @brief Copies the file, directory or symlink specified by \a src_base and \a src_path to location specified by \a dst_base and \a dst_path. */
	[[nodiscard]] ROD_API_PUBLIC result<std::size_t> copy(const path_handle &src_base, path_view src_path, const path_handle &dst_base, path_view dst_path, copy_mode mode, const file_timeout &to = file_timeout::infinite) noexcept;
	/** TODO: Document usage.
	 * @brief Copies the file, directory or symlink specified by \a src_base and \a src_path to location specified by \a dst_base and \a dst_path. Unlike `copy`, directory contents are copied as well. */
	[[nodiscard]] ROD_API_PUBLIC result<std::size_t> copy_all(const path_handle &src_base, path_view src_path, const path_handle &dst_base, path_view dst_path, copy_mode mode, const file_timeout &to = file_timeout::infinite) noexcept;

	/** Removes the directory, file, or symlink specified by \a path relative to parent location \a base.
	 * Equivalent to opening the target object handle with sufficient permissions and calling `unlink`.
	 * @param base Handle to the parent location. If set to an invalid handle, \a path must be a fully-qualified path.
	 * @param path Path to the target object relative to \a base if it is a valid handle, otherwise a fully-qualified path.
	 * @param to Optional timeout to use when removing the filesystem object.
	 * @return Amount of filesystem objects removed or a status code on failure.
	 * @errors
	 * <ul>
	 * <li>`std::errc::directory_not_empty` if the specified directory is not empty.</li>
	 * <li>`std::errc::not_enough_memory` on failure to allocate internal buffers.</li>
	 * <li>Errors returned by `unlink` or `rmdir` on POSIX or `NtGetInformationFile` and `NtSetInformationFile` on Windows.</li>
	 * </ul> */
	ROD_API_PUBLIC result<std::size_t> remove(const path_handle &base, path_view path, const file_timeout &to = file_timeout::infinite) noexcept;
	/** Removes all directories, files and symlinks specified by \a path relative to parent location \a base.
	 * @param base Handle to the parent location. If set to an invalid handle, \a path must be a fully-qualified path.
	 * @param path Path to the target object relative to \a base if it is a valid handle, otherwise a fully-qualified path.
	 * @param to Optional timeout to use when removing the filesystem objects.
	 * @return Amount of filesystem objects removed or a status code on failure.
	 * @errors
	 * <ul>
	 * <li>`std::errc::not_enough_memory` on failure to allocate internal buffers.</li>
	 * <li>Errors returned by `unlink` or `rmdir` on POSIX or `NtQueryDirectoryFile`, `NtGetInformationFile`, and `NtSetInformationFile` on Windows.</li>
	 * </ul> */
	ROD_API_PUBLIC result<std::size_t> remove_all(const path_handle &base, path_view path, const file_timeout &to = file_timeout::infinite) noexcept;
}

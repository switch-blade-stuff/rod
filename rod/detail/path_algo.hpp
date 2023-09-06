/*
 * Created by switch_blade on 2023-08-31.
 */

#pragma once

#include "handle_stat.hpp"
#include "path_handle.hpp"

namespace rod
{
	/** Returns path of the current directory as if via `getcwd`. */
	[[nodiscard]] ROD_API_PUBLIC auto current_path() noexcept -> result<path>;
	/** Changes path of the current directory to \a path as if via `chdir`. */
	[[nodiscard]] ROD_API_PUBLIC auto current_path(path_view path) noexcept -> result<>;

	/** Checks if \a path references a valid filesystem location. */
	[[nodiscard]] ROD_API_PUBLIC result<bool> exists(path_view path) noexcept;
	/** Checks if path views \a a and \a b reference the same filesystem object. */
	[[nodiscard]] ROD_API_PUBLIC result<bool> equivalent(path_view a, path_view b) noexcept;

	/** Creates all directories specified by \a path relative to parent location \a base.
	 * @return Amount of directories created or a status code on failure. */
	[[nodiscard]] ROD_API_PUBLIC result<std::size_t> create_all(const path_handle &base, path_view path) noexcept;
	/** Creates all directories specified by \a path.
	 * @return Amount of directories created or a status code on failure. */
	[[nodiscard]] inline static result<std::size_t> create_all(path_view path) noexcept { return create_all({}, path); }

	/** Removes all directories, files and symlinks specified by \a path relative to parent location \a base.
	 * @return Amount of elements removed or a status code on failure. */
	[[nodiscard]] ROD_API_PUBLIC result<std::size_t> remove_all(const path_handle &base, path_view path) noexcept;
	/** Removes all directories, files and symlinks specified by \a path.
	 * @return Amount of elements removed or a status code on failure. */
	[[nodiscard]] inline static result<std::size_t> remove_all(path_view path) noexcept { return remove_all({}, path); }

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

		/** Recursively copy subdirectories as well. */
		recursive = 0x10,
		/** Overwrite existing files and directories. */
		overwrite = 0x20,

		/** Create symlinks instead of copying data. Mutually exclusive with `create_hardlinks`. */
		create_symlinks = 0x40,
		/** Create hardlinks instead of copying data. Mutually exclusive with `create_symlinks`. */
		create_hardlinks = 0x80,
	};

	[[nodiscard]] constexpr copy_mode operator~(copy_mode h) noexcept { return copy_mode(~int(h)); }
	[[nodiscard]] constexpr copy_mode operator&(copy_mode a, copy_mode b) noexcept { return copy_mode(int(a) & int(b)); }
	[[nodiscard]] constexpr copy_mode operator|(copy_mode a, copy_mode b) noexcept { return copy_mode(int(a) | int(b)); }
	[[nodiscard]] constexpr copy_mode operator^(copy_mode a, copy_mode b) noexcept { return copy_mode(int(a) ^ int(b)); }
	constexpr copy_mode &operator&=(copy_mode &a, copy_mode b) noexcept { return a = a & b; }
	constexpr copy_mode &operator|=(copy_mode &a, copy_mode b) noexcept { return a = a | b; }
	constexpr copy_mode &operator^=(copy_mode &a, copy_mode b) noexcept { return a = a ^ b; }

	/** Removes all directories, files and symlinks specified by \a path relative to parent location \a base using mode flags \a mode.
	 * @return Amount of elements copied or a status code on failure. */
	[[nodiscard]] ROD_API_PUBLIC result<std::size_t> copy_all(const path_handle &base, path_view path, copy_mode mode) noexcept;
	/** Duplicates all directories, files and symlinks located at \a from path \a to using mode flags \a mode.
	 * @return Amount of elements copied or a status code on failure. */
	[[nodiscard]] inline static result<std::size_t> copy_all(path_view path, copy_mode mode) noexcept { return copy_all({}, path, mode); }
}

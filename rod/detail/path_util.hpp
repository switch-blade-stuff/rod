/*
 * Created by switch_blade on 2023-08-31.
 */

#pragma once

#include "handle_stat.hpp"

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
}

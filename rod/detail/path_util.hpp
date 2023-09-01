/*
 * Created by switch_blade on 2023-08-31.
 */

#pragma once

#include "handle_stat.hpp"

namespace rod
{
	/** Checks if \a path references a valid filesystem location. */
	[[nodiscard]] ROD_API_PUBLIC result<bool> exists(path_view path) noexcept;
	/** Checks if path views \a a and \a b reference the same filesystem object. */
	[[nodiscard]] ROD_API_PUBLIC result<bool> equivalent(path_view a, path_view b) noexcept;
}

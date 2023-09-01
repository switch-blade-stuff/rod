/*
 * Created by switch_blade on 2023-08-31.
 */

#pragma once

#include "handle_stat.hpp"

namespace rod
{
	/** Checks if path views \a a and \a b reference the same filesystem object. */
	[[nodiscard]] ROD_API_PUBLIC result<bool> equivalent(path_view a, path_view b) noexcept;
}

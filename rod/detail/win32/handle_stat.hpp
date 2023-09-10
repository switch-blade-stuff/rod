/*
 * Created by switch_blade on 2023-09-10.
 */

#pragma once

#include <cstring>
#include <cwchar>

#include "../handle_stat.hpp"
#include "path_discovery.hpp"

namespace rod::_handle
{
	constexpr auto basic_info_mask = stat::query::type | stat::query::atime | stat::query::mtime | stat::query::ctime | stat::query::btime | stat::query::is_sparse | stat::query::is_compressed | stat::query::is_reparse_point;
	constexpr auto standard_info_mask = stat::query::size | stat::query::alloc | stat::query::blocks | stat::query::nlink;
	constexpr auto internal_info_mask = stat::query::ino;

	constexpr std::size_t buff_size = 32768;
}

/*
 * Created by switch_blade on 2023-09-02.
 */

#pragma once

#include "../fs_handle_base.hpp"
#include "ntapi.hpp"

namespace rod::_win32
{
	result<> do_link(void *, const path_handle &, path_view, bool replace, const file_timeout &) noexcept;
	result<> do_relink(void *, const path_handle &, path_view, bool replace, const file_timeout &) noexcept;
	result<> do_unlink(void *, const file_timeout &, file_flags) noexcept;
}

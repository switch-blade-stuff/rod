/*
 * Created by switch_blade on 2023-08-20.
 */

#include "../fs_handle_base.hpp"
#include "ntapi.hpp"

namespace rod::_handle
{
	/* TODO: Implement */
	result<> do_link(basic_handle & /*, const path_handle &, path_view */) noexcept {}
	result<> do_relink(basic_handle & /*, const path_handle &, path_view */) noexcept {}
	result<> do_unlink(basic_handle &) noexcept {}

	/* TODO: Implement */
	result<basic_handle> do_reopen(const basic_handle &) noexcept
	{

	}
}

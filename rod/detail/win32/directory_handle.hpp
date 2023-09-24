/*
 * Created by switch_blade on 2023-09-02.
 */

#pragma once

#include "../directory_handle.hpp"
#include "../fs_handle_base.hpp"
#include "ntapi.hpp"

namespace rod::_win32
{
	/* Win32 directory handles are not created with DELETE permission and need to be re-opened for link operations to work. */
	inline static result<directory_handle> reopen_as_deletable(const ntapi &ntapi, const directory_handle &dir, const file_timeout &to) noexcept
	{
		const auto opts = flags_to_opts(dir.flags()) | 1 /*FILE_DIRECTORY_FILE*/;
		const auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
		const auto access = GENERIC_READ | SYNCHRONIZE | DELETE;

		auto iosb = io_status_block();
		auto hnd = ntapi.reopen_file(dir.native_handle(), &iosb, access, share, opts, to);
		if (hnd.has_value()) [[likely]]
			return directory_handle(*hnd, dir.flags());
		else
			return hnd.error();
	}
}

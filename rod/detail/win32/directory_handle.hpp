/*
 * Created by switch_blade on 2023-09-02.
 */

#pragma once

#include "../directory_handle.hpp"
#include "fs_handle_base.hpp"
#include "ntapi.hpp"

namespace rod::_win32
{
	/* Win32 directory handles are not created with DELETE permission and need to be re-opened for link operations to work. */
	inline static result<directory_handle> reopen_as_deletable(const directory_handle &dir, const file_timeout &to) noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		constexpr auto flags = 0x20 | 1 /*FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE*/;
		constexpr auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
		constexpr auto access = GENERIC_READ | SYNCHRONIZE | DELETE;

		auto iosb = io_status_block();
		auto hnd = ntapi->reopen_file(dir.native_handle(), &iosb, access, share, flags, to);
		if (hnd.has_value()) [[likely]]
			return directory_handle(*hnd);
		else
			return hnd.error();
	}
}

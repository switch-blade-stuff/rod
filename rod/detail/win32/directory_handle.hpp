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
	inline static result<directory_handle> reopen_as_deletable(const directory_handle &dir) noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		constexpr auto flags = 0x20 | 1 /*FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE*/;
		constexpr auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
		constexpr auto access = GENERIC_READ | SYNCHRONIZE | DELETE;

		auto obj_attrib = object_attributes();
		auto upath = unicode_string();
		auto iosb = io_status_block();

		obj_attrib.length = sizeof(object_attributes);
		obj_attrib.root_dir = dir.native_handle();
		obj_attrib.name = &upath;

		auto handle = INVALID_HANDLE_VALUE;
		auto status = ntapi->NtOpenFile(&handle, access, &obj_attrib, &iosb, share, flags);
		if (status == STATUS_PENDING)
			status = ntapi->wait_io(handle, &iosb);
		if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return directory_handle(handle);
	}
}

/*
 * Created by switch_blade on 2023-08-09.
 */

#include "../path_handle.hpp"

#include <cstdlib>

#include "../path_discovery.hpp"
#include "ntapi.hpp"

namespace rod
{
	using namespace _win32;

	result<path_handle> path_handle::open(path_view path) noexcept { return open({}, path); }
	result<path_handle> path_handle::open(const path_handle &base, path_view path) noexcept
	{
		constexpr auto flags = 0x20 /* FILE_SYNCHRONOUS_IO_NONALERT */ | 1 /* FILE_DIRECTORY_FILE */;
		constexpr auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
		constexpr auto access = SYNCHRONIZE;

		auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto upath = unicode_string();
		auto guard = ntapi->path_to_nt_string(upath, path, !base.is_open());
		if (guard.has_error()) [[unlikely]]
			return guard.error();

		object_attributes attrib = {};
		LARGE_INTEGER alloc_size = {};
		io_status_block iosb = {};

		attrib.root_dir = base.is_open() ? base.native_handle() : nullptr;
		attrib.length = sizeof(object_attributes);
		attrib.name = &upath;

		/* Wait for completion if NtCreateFile returns STATUS_PENDING. */
		auto handle = INVALID_HANDLE_VALUE;
		auto status = ntapi->NtCreateFile(&handle, access, &attrib, &iosb, &alloc_size, 0, share, file_create, flags, nullptr, 0);
		if (status == STATUS_PENDING) [[unlikely]]
			status = ntapi->wait_io(handle, &iosb);
		if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return path_handle(handle);
	}

#if 0
	result<directory_handle> directory_handle::open(path_view path) noexcept { return open({}, path); }
	result<directory_handle> directory_handle::open(const path_handle &base, path_view path) noexcept
	{
	}

	result<directory_handle> directory_handle::open_unique(const path_handle &base) noexcept
	{
		try { return open(base, generate_unique_name()); }
		catch (const std::system_error &e) { return e.code(); }
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
	}
	result<directory_handle> directory_handle::open_temporary(path_view path) noexcept
	{
		if (path.empty())
			return open_unique(temporary_file_directory());
		else
			return open(temporary_file_directory(), path);
	}
#endif
}

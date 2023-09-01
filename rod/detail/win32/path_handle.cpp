/*
 * Created by switch_blade on 2023-08-09.
 */

#include "../path_handle.hpp"
#include "ntapi.hpp"

namespace rod
{
	using namespace _win32;

	result<path_handle> path_handle::open(path_view path) noexcept { return open({}, path); }
	result<path_handle> path_handle::open(const path_handle &base, path_view path) noexcept
	{
		constexpr auto flags = 0x20 | 1 /*FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE*/;
		constexpr auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

		auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto upath = unicode_string();
		auto guard = ntapi->path_to_nt_string(upath, path, !base.is_open());
		if (guard.has_error()) [[unlikely]]
			return guard.error();

		auto obj_attrib = object_attributes();
		auto iosb = io_status_block();

		obj_attrib.root_dir = base.is_open() ? base.native_handle() : nullptr;
		obj_attrib.length = sizeof(object_attributes);
		obj_attrib.name = &upath;

		auto handle = INVALID_HANDLE_VALUE;
		auto status = ntapi->NtCreateFile(&handle, SYNCHRONIZE, &obj_attrib, &iosb, nullptr, 0, share, file_open, flags, nullptr, 0);
		if (status == STATUS_PENDING) [[unlikely]]
			status = ntapi->wait_io(handle, &iosb);
		if (iosb.info == 5 /*FILE_DOES_NOT_EXIST*/) [[unlikely]]
			return std::make_error_code(std::errc::no_such_file_or_directory);
		else if (is_status_failure(status)) [[unlikely]]
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

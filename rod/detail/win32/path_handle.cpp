/*
 * Created by switch_blade on 2023-08-09.
 */

#include "../path_ahandle.hpp"
#include "ntapi.hpp"

namespace rod::_path
{
	using namespace _win32;

	result<path_handle> path_handle::open(const path_handle &base, path_view path) noexcept
	{
		constexpr auto opts = 0x20 | 1 /*FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE*/;
		constexpr auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = render_as_ustring<true>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto &upath = rpath->first;
		auto guard = ntapi->dos_path_to_nt_path(upath, base.is_open());
		if (guard.has_error()) [[unlikely]]
			return std::make_error_code(std::errc::no_such_file_or_directory);

		auto obj_attrib = object_attributes();
		auto iosb = io_status_block();

		obj_attrib.root_dir = base.is_open() ? base.native_handle() : nullptr;
		obj_attrib.length = sizeof(object_attributes);
		obj_attrib.name = &upath;

		auto hnd = ntapi->create_file(obj_attrib, &iosb, SYNCHRONIZE, 0, share, file_open, opts);
		if (hnd.has_value()) [[likely]]
			return path_handle(*hnd);

		/* Map known error codes. */
		if (auto err = hnd.error(); is_error_file_not_found(err))
			return std::make_error_code(std::errc::no_such_file_or_directory);
		else if (is_error_not_a_directory(err))
			return std::make_error_code(std::errc::not_a_directory);
		else
			return err;
	}
}

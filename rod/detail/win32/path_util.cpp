/*
 * Created by switch_blade on 2023-08-31.
 */

#include "../path_util.hpp"
#include "ntapi.hpp"

namespace rod
{
	using namespace _win32;

	result<bool> exists(path_view path) noexcept
	{
		auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return result(in_place_error, ntapi.error());

		auto basic_info = file_basic_information();
		auto obj_attrib = object_attributes();
		auto upath = unicode_string();

		auto guard = ntapi->path_to_nt_string(upath, path, false);
		if (auto err = guard.error_or({}); err && err.value() != ERROR_PATH_NOT_FOUND) [[unlikely]]
			return result(in_place_error, guard.error());
		else if (err.value() == ERROR_PATH_NOT_FOUND)
			return false;

		obj_attrib.length = sizeof(object_attributes);
		obj_attrib.name = &upath;

		/* If possible, try to get all requested data using `NtQueryAttributesFile` to avoid opening a handle altogether. */
		if (auto status = ntapi->NtQueryAttributesFile(&obj_attrib, &basic_info); is_status_failure(status)) [[unlikely]]
		{
			if (status == 0xc000003a /*STATUS_OBJECT_PATH_NOT_FOUND*/)
				return false;

			/* NtQueryAttributesFile may fail in case the path is a DOS device name. */
			if (!ntapi->RtlIsDosDeviceName_Ustr(&upath)) [[unlikely]]
				return result(in_place_error, status_error_code(status));
			else
				return false;
		}
		return true;
	}
	result<bool> equivalent(path_view a, path_view b) noexcept
	{
		dev_t dev_a, dev_b;
		ino_t ino_a, ino_b;
		stat st;

		if (auto res = get_stat(st, a, stat::query::dev | stat::query::ino); res.has_error()) [[unlikely]]
			return result(in_place_error, res.error());
		dev_a = st.dev;
		ino_a = st.ino;

		if (auto res = get_stat(st, b, stat::query::dev | stat::query::ino); res.has_error()) [[unlikely]]
			return result(in_place_error, res.error());
		dev_b = st.dev;
		ino_b = st.ino;

		return dev_a == dev_b && ino_a == ino_b;
	}
}

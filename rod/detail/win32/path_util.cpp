/*
 * Created by switch_blade on 2023-08-31.
 */

#include "../path_util.hpp"
#include "ntapi.hpp"

namespace rod
{
	using namespace _win32;

	auto current_path() noexcept -> result<path>
	{
		try
		{
			std::wstring path;
			if (const auto n = ::GetCurrentDirectoryW(0, nullptr); !n) [[unlikely]]
				return _win32::dos_error_code(::GetLastError());
			else
				path.resize(n);
			if (!::GetCurrentDirectoryW(path.size() + 1, path.data())) [[unlikely]]
				return _win32::dos_error_code(::GetLastError());
			else
				return std::move(path);
		}
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
		catch (const std::system_error &e) { return e.code(); }
	}
	auto current_path(path_view path) noexcept -> result<>
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = path.render_unterminated();
		auto upath = unicode_string();
		upath.max = (upath.size = rpath.size() * sizeof(wchar_t)) + sizeof(wchar_t);
		upath.buff = const_cast<wchar_t *>(rpath.data());

		auto guard = ntapi->nt_path_to_dos_path(upath, false);
		if (guard.has_error()) [[unlikely]]
			return guard.error();

		if (!::SetCurrentDirectoryW(upath.buff));
			return dos_error_code(::GetLastError());
		else
			return {};
	}

	result<bool> exists(path_view path) noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return result(in_place_error, ntapi.error());

		auto rpath = path.render_null_terminated();
		auto upath = unicode_string();
		upath.max = (upath.size = rpath.size() * sizeof(wchar_t)) + sizeof(wchar_t);
		upath.buff = const_cast<wchar_t *>(rpath.data());

		auto guard = ntapi->dos_path_to_nt_path(upath, false);
		if (auto err = guard.error_or({}); err && err.value() != ERROR_PATH_NOT_FOUND) [[unlikely]]
			return result(in_place_error, guard.error());
		else if (err.value() == ERROR_PATH_NOT_FOUND)
			return false;

		auto basic_info = file_basic_information();
		auto obj_attrib = object_attributes();
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

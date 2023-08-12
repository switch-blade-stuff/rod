/*
 * Created by switch_blade on 2023-08-09.
 */

#include "path.hpp"

#ifdef ROD_WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace rod::_win32
{
	result<path_handle> path_handle::open(fs::path_view path) noexcept { return open({}, path); }
	result<path_handle> path_handle::open(const path_handle &base, fs::path_view path) noexcept
	{
		native_handle_type handle = INVALID_HANDLE_VALUE;
		const auto rpath = path.render_null_terminated();
		const auto &ntapi = _ntapi::api();

		constexpr auto flags = 0x20 /* FILE_SYNCHRONOUS_IO_NONALERT */ | 1 /* FILE_DIRECTORY_FILE */;
		constexpr auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
		constexpr auto access = SYNCHRONIZE;

		_ntapi::object_attributes attrib = {};
		_ntapi::large_integer alloc_size = {};
		_ntapi::io_status_block iosb = {};
		_ntapi::unicode_string uname = {};
		_ntapi::status status;

		attrib.root_dir = base.is_open() ? base.handle : nullptr;
		attrib.length = sizeof(_ntapi::object_attributes);
		attrib.name = &uname;

		if (base.is_open() || std::wstring_view(rpath.data(), rpath.size()).starts_with(L"\\!!"))
		{
			uname.max = (uname.size = static_cast<_ntapi::ushort>(rpath.size() * sizeof(wchar_t))) + sizeof(wchar_t);
			uname.buffer = const_cast<wchar_t *>(rpath.data());

			if (rpath.size() >= 4 && rpath[0] == '\\' && rpath[1] == '!' && rpath[2] == '!' && rpath[3] == '\\')
			{
				uname.size -= 3 * sizeof(wchar_t);
				uname.max -= 3 * sizeof(wchar_t);
				uname.buffer += 3;
			}

			status = ntapi.NtCreateFile(&handle, access, &attrib, &iosb, &alloc_size, 0, share, _ntapi::file_create, flags, nullptr, 0);
		}
		else if (!ntapi.RtlDosPathNameToNtPathName_U(rpath.data(), &uname, nullptr, nullptr)) [[unlikely]]
			return _ntapi::dos_error_code(ERROR_FILE_NOT_FOUND);
		else
		{
			status = ntapi.NtCreateFile(&handle, access, &attrib, &iosb, &alloc_size, 0, share, _ntapi::file_create, flags, nullptr, 0);
			::HeapFree(::GetProcessHeap(), 0, uname.buffer);
		}

		if (status == STATUS_PENDING) [[unlikely]]
			status = ntapi.wait_io(handle, &iosb);
		if (status > _ntapi::message_status_max) [[unlikely]]
			return _ntapi::status_error_code(status);
		else
			return handle;
	}
}

#endif

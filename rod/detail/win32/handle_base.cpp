/*
 * Created by switchblade on 2023-05-20.
 */

#include "../handle_base.hpp"
#include "ntapi.hpp"

namespace rod::_handle
{
	using namespace _win32;

	auto do_close(basic_handle &hnd) noexcept -> result<>
	{
		if (hnd.is_open() && !::CloseHandle(hnd.release())) [[unlikely]]
			return dos_error_code(::GetLastError());
		else
			return {};
	}
	auto do_clone(const basic_handle &hnd) noexcept -> result<basic_handle>
	{
		typename basic_handle::native_handle_type result = INVALID_HANDLE_VALUE;
		if (::DuplicateHandle(::GetCurrentProcess(), hnd.native_handle(), ::GetCurrentProcess(), &result, 0, 0, DUPLICATE_SAME_ACCESS) == 0)
			return dos_error_code(::GetLastError());
		else
			return basic_handle(result);
	}

	result<path> do_to_object_path(const basic_handle &hnd) noexcept
	{
		try
		{
			auto result = std::wstring(32768, '\0');
			std::copy_n(L"\\!!", 3, result.data());

			if (const auto len = ::GetFinalPathNameByHandleW(hnd.native_handle(), result.data() + 3, result.size() - 2, VOLUME_NAME_NT); !len) [[unlikely]]
				return _win32::dos_error_code(::GetLastError());
			else
				result.resize(len / sizeof(wchar_t) + 3);

			/* Detect unlinked files. */
			if (result.find(L"\\$Extend\\$Deleted\\") == std::wstring::npos) [[likely]]
				return std::move(result);
			else
				return {};
		}
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
		catch (const std::system_error &e) { return e.code(); }
	}
	result<path> do_to_native_path(const basic_handle &hnd, native_path_format fmt, dev_t dev, ino_t ino) noexcept
	{
		try
		{
			auto result = std::wstring(32768, '\0');
			auto flags = FILE_NAME_OPENED;
			switch (fmt)
			{
			case native_path_format::generic:
				flags |= VOLUME_NAME_DOS;
				break;
			case native_path_format::system:
				flags |= VOLUME_NAME_NT;
				break;
			case native_path_format::any:
				fmt = native_path_format::volume_id;
			case native_path_format::volume_id:
			case native_path_format::object_id:
				flags |= VOLUME_NAME_GUID;
				break;
			default:
				return std::make_error_code(std::errc::invalid_argument);
			}

			if (const auto len = ::GetFinalPathNameByHandleW(hnd.native_handle(), result.data(), result.size(), flags); !len) [[unlikely]]
				return _win32::dos_error_code(::GetLastError());
			else
				result.resize(len / sizeof(wchar_t) + 3);

			if (fmt == native_path_format::volume_id)
				return std::move(result);
			if (fmt == native_path_format::object_id)
			{
				auto buffer = FILE_OBJECTID_BUFFER();
				auto *id = (GUID *) buffer.ObjectId;
				DWORD bytes;

				if(!::DeviceIoControl(hnd.native_handle(), FSCTL_CREATE_OR_GET_OBJECT_ID, nullptr, 0, &buffer, sizeof(buffer), &bytes, nullptr)) [[unlikely]]
					return dos_error_code(::GetLastError());

				result.resize(87);
				const auto guid_str = L"{%08x-%04hx-%04hx-%02x%02x-%02x%02x%02x%02x%02x%02x}";
				const auto guid_pos = result.data() + 49;
				const auto guid_len = 64;

				swprintf_s(guid_pos, guid_len, guid_str, id->Data1, id->Data2, id->Data3, id->Data4[0], id->Data4[1], id->Data4[2], id->Data4[3], id->Data4[4], id->Data4[5], id->Data4[6], id->Data4[7]);
				return std::move(result);
			}
			if (fmt == native_path_format::system)
			{
				/* Cannot map a non-device WinNT path to a Win32 path. */
				if (result.compare(0, 8, L"\\Device\\")) [[unlikely]]
					return std::make_error_code(std::errc::no_such_file_or_directory);
				else
					result.replace(1, 8, std::wstring_view(L"\\.\\"));
			}

			/* Make sure the resulting DOS path references the same file as `hnd`. */
			const auto share = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;
			const auto attr = FILE_READ_ATTRIBUTES | SYNCHRONIZE;
			const auto tmp = basic_handle(::CreateFileW(result.c_str(), attr, share, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr));
			if (!tmp.is_open()) [[unlikely]]
			    return std::make_error_code(std::errc::no_such_file_or_directory);

			stat st;
			if (auto res = get_stat(st, tmp, stat::query::dev | stat::query::ino); res.has_error()) [[unlikely]]
				return res.error();
			else if (*res == (stat::query::dev | stat::query::ino) && dev == st.dev && ino == st.ino)
			{
				constexpr wchar_t reserved_chars[] = L"\"*/:<>?|";
				constexpr std::wstring_view reserved_names[] =
				{
						L"\\CON\\", L"\\PRN\\", L"\\AUX\\", L"\\NUL\\",
						L"\\COM1\\", L"\\COM2\\", L"\\COM3\\", L"\\COM4\\",
						L"\\COM5\\", L"\\COM6\\", L"\\COM7\\", L"\\COM8\\",
						L"\\COM9\\", L"\\LPT1\\", L"\\LPT2\\", L"\\LPT3\\",
						L"\\LPT4\\", L"\\LPT5\\", L"\\LPT6\\", L"\\LPT7\\",
						L"\\LPT8\\", L"\\LPT9\\"
				};

				bool remove_prefix = result.size() <= 260;
				auto buff = std::wstring_view(result);

				/* Make sure path is in a legal DOS path format. */
				for (std::size_t i = 7; remove_prefix && i < result.size(); ++i)
				{
					/* Test for control characters. */
					if (buff[i] >= 1 && buff[i] <= 31)
					{
						remove_prefix = false;
						break;
					}
					/* Test for reserved characters. */
					for (std::size_t j = 0; j < sizeof(reserved_chars); ++j)
						if (buff[i] == reserved_chars[j])
						{
							remove_prefix = false;
							break;
						}
				}
				if (remove_prefix)
				{
					for (auto name : reserved_names)
						if (buff.find(name) != buff.npos || buff.ends_with(name.substr(0, name.size() - 1)))
						{
							remove_prefix = false;
							break;
						}
				}
				if (remove_prefix) [[likely]]
					result.erase(0, 4);
				return std::move(result);
			}
			return std::make_error_code(std::errc::no_such_file_or_directory);
		}
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
		catch (const std::system_error &e) { return e.code(); }
	}
}

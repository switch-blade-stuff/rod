/*
 * Created by switchblade on 2023-05-20.
 */

#include "../handle_base.hpp"
#include "ntapi.hpp"

namespace rod::_handle
{
	using namespace _win32;

	auto basic_handle::do_close() noexcept -> result<>
	{
		if (is_open() && !::CloseHandle(release())) [[unlikely]]
			return dos_error_code(::GetLastError());
		else
			return {};
	}
	auto basic_handle::do_clone() const noexcept -> result<basic_handle>
	{
		basic_handle::native_handle_type result = INVALID_HANDLE_VALUE;
		if (::DuplicateHandle(::GetCurrentProcess(), _hnd, ::GetCurrentProcess(), &result.value, 0, 0, DUPLICATE_SAME_ACCESS) == 0)
			return dos_error_code(::GetLastError());
		else
			return result;
	}

	result<path> basic_handle::do_to_object_path() const noexcept
	{
		try
		{
			auto result = std::wstring(32767, '\0');
			std::copy_n(L"\\!!", 3, result.data());

			if (const auto len = ::GetFinalPathNameByHandleW(_hnd, result.data() + 3, USHORT(result.size() - 2), VOLUME_NAME_NT); !len) [[unlikely]]
				return _win32::dos_error_code(::GetLastError());
			else
				result.resize(len + 3);

			/* Detect unlinked files. */
			if (result.find(L"\\$Extend\\$Deleted\\") == std::wstring::npos) [[likely]]
				return std::move(result);
			else
				return {};
		}
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
		catch (const std::system_error &e) { return e.code(); }
	}
	result<path> basic_handle::do_to_native_path(native_path_format fmt, dev_t dev, ino_t ino) const noexcept
	{
		try
		{
			auto result = std::wstring(32767, '\0');
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

			if (const auto len = ::GetFinalPathNameByHandleW(_hnd, result.data(), USHORT(result.size()), flags); !len) [[unlikely]]
				return _win32::dos_error_code(::GetLastError());
			else
				result.resize(len + 3);

			if (fmt == native_path_format::volume_id)
				return std::move(result);
			if (fmt == native_path_format::object_id)
			{
				auto buffer = FILE_OBJECTID_BUFFER();
				auto *id = (GUID *) buffer.ObjectId;
				DWORD bytes;

				if(!::DeviceIoControl(_hnd, FSCTL_CREATE_OR_GET_OBJECT_ID, nullptr, 0, &buffer, sizeof(buffer), &bytes, nullptr)) [[unlikely]]
					return dos_error_code(::GetLastError());

				result.resize(87);
				::StringFromGUID2(*id, result.data() + 49, 39);
				return std::move(result);
			}
			if (fmt == native_path_format::system)
			{
				/* Map \Device\ device paths to \\.\, \Global??\ to \\?\ and the rest to \\?\GLOBALROOT */
				if (result.compare(0, 9, L"\\Global??\\") == 0)
					result.replace(1, 9, std::wstring_view(L"\\?\\"));
				else if (result.compare(0, 8, L"\\Device\\") == 0)
					result.replace(1, 8, std::wstring_view(L"\\.\\"));
				else if (result.starts_with(L'\\'))
					result.insert(0, std::wstring_view(L"\\\\?\\GLOBALROOT"));
				return std::move(result);
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
				/* Remove \\?\ prefix if there are no potentially illegal sequences. */
				if (!_path::has_illegal_path_sequences(result)) [[likely]]
					result.erase(0, 4);
				return std::move(result);
			}
			return std::make_error_code(std::errc::no_such_file_or_directory);
		}
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
		catch (const std::system_error &e) { return e.code(); }
	}
}

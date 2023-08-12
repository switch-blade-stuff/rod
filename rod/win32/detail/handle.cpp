/*
 * Created by switchblade on 2023-05-20.
 */

#include "handle.hpp"

#ifdef ROD_WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace rod::_win32
{
	result<> io_handle::close() noexcept
	{
		if (is_open())
		{
			if (!::CloseHandle(handle)) [[unlikely]]
				return _ntapi::dos_error_code(::GetLastError());
			else
				value = io_handle().value;
		}
		return {};
	}

	result<io_handle> io_handle::clone() const noexcept
	{
		native_handle_type result = INVALID_HANDLE_VALUE;
		if (::DuplicateHandle(GetCurrentProcess(), handle, ::GetCurrentProcess(), &result, 0, 0, DUPLICATE_SAME_ACCESS) == 0)
			return _ntapi::dos_error_code(::GetLastError());
		else
			return result;
	}
	result<std::wstring> io_handle::path() const noexcept
	{
		try
		{
			auto result = std::wstring(32768, '\0');
			std::copy_n(L"\\!!", 3, result.data());

			if (const auto len = ::GetFinalPathNameByHandleW(handle, result.data() + 3, (result.size() - 2) * sizeof(wchar_t), VOLUME_NAME_NT); !len) [[unlikely]]
				return _ntapi::dos_error_code(::GetLastError());
			else
				result.resize(len / sizeof(wchar_t) + 3);

			/* Detect unlinked files. */
			if (result.find(L"\\$Extend\\$Deleted\\") == std::wstring::npos) [[likely]]
				return result;
			else
				return {};
		}
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
		catch (const std::system_error &e) { return e.code(); }
	}
}
#endif
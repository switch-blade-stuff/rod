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
	result<path> do_to_native_path(const basic_handle &hnd, native_path_format fmt) noexcept
	{
		try
		{
			/* TODO: Implement */
		}
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
		catch (const std::system_error &e) { return e.code(); }
	}
}

/*
 * Created by switchblade on 2023-05-20.
 */

#include "handle_base.hpp"

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
		typename basic_handle::native_handle_type result = INVALID_HANDLE_VALUE;
		if (is_open() && ::DuplicateHandle(::GetCurrentProcess(), _hnd, ::GetCurrentProcess(), &result.value, 0, 0, DUPLICATE_SAME_ACCESS) == 0)
			return dos_error_code(::GetLastError());
		else
			return result;
	}
}

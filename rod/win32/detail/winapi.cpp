/*
 * Created by switch_blade on 2023-07-04.
 */

#ifdef _WIN32

#include "winapi.hpp"

#include <system_error>

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

namespace rod::detail
{
	void *winapi::get_ntdll()
	{
		if (const auto res = ::GetModuleHandleW(L"ntdll.dll"); res != nullptr) [[likely]]
			return res;

		const auto err = std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
		throw std::system_error(err, "GetModuleHandleW(\"ntdll.dll\")");
	}
	template<typename S>
	void winapi::init_symbol(void *ntdll, S *&ptr, const char *name)
	{
		if (const auto res = ::GetProcAddress(static_cast<HMODULE>(ntdll), name); res == nullptr)
		{
			const auto err = std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
			const auto msg = std::string{"GetProcAddress(ntdll, \""} + name + "\")";
			throw std::system_error(err, msg);
		}
		else if constexpr (std::is_function_v<S>)
			ptr = reinterpret_cast<S *>(reinterpret_cast<void (*)()>(res));
		else
			ptr = reinterpret_cast<S *>(reinterpret_cast<void *>(res));
	}

	const winapi &winapi::instance()
	{
		static const winapi value;
		return value;
	}

	winapi::winapi()
	{
		const auto ntdll = get_ntdll();
		init_symbol(ntdll, NtReadFile, "NtReadFile");
		init_symbol(ntdll, NtWriteFile, "NtWriteFile");
		init_symbol(ntdll, RtlNtStatusToDosError, "RtlNtStatusToDosError");
		init_symbol(ntdll, NtWaitForSingleObject, "NtWaitForSingleObject");
		init_symbol(ntdll, NtCancelIoFileEx, "NtCancelIoFileEx");
		init_symbol(ntdll, NtSetIoCompletion, "NtSetIoCompletion");
		init_symbol(ntdll, NtRemoveIoCompletion, "NtRemoveIoCompletion");
		init_symbol(ntdll, NtRemoveIoCompletionEx, "NtRemoveIoCompletionEx");
	}
}
#endif
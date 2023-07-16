/*
 * Created by switch_blade on 2023-07-04.
 */

#include "ntapi.hpp"

#ifdef ROD_WIN32

#include <system_error>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace rod::detail
{
	const ntapi ntapi::instance;

	ntapi::ntapi()
	{
		ntdll = ::GetModuleHandleW(L"ntdll.dll");
		if (ntdll == nullptr)
		{
			const auto err = std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
			throw std::system_error(err, "GetModuleHandleW(\"ntdll.dll\")");
		}

		const auto init_sym = [this]<typename S>(S *&sym_ptr, const char *sym_name)
		{
			if (const auto ptr = ::GetProcAddress(static_cast<HMODULE>(ntdll), sym_name); ptr == nullptr)
			{
				const auto err = std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
				const auto msg = std::string{R"(GetProcAddress("ntdll.dll", ")"} + sym_name + "\")";
				throw std::system_error(err, msg);
			}
			else if constexpr (std::is_function_v<S>)
				sym_ptr = reinterpret_cast<S *>(reinterpret_cast<void (*)()>(ptr));
			else
				sym_ptr = reinterpret_cast<S *>(reinterpret_cast<void *>(ptr));
		};

		init_sym(NtReadFile, "NtReadFile");
		init_sym(NtWriteFile, "NtWriteFile");
		init_sym(RtlNtStatusToDosError, "RtlNtStatusToDosError");
		init_sym(NtWaitForSingleObject, "NtWaitForSingleObject");
		init_sym(NtCancelIoFileEx, "NtCancelIoFileEx");
		init_sym(NtSetIoCompletion, "NtSetIoCompletion");
		init_sym(NtRemoveIoCompletionEx, "NtRemoveIoCompletionEx");
		init_sym(NtQueryInformationFile, "NtQueryInformationFile");
	}
}
#endif
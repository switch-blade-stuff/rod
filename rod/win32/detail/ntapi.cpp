/*
 * Created by switch_blade on 2023-07-04.
 */

#include "ntapi.hpp"

#ifdef ROD_WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <memory>

namespace rod::_detail
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

	const ntapi::status_category_type::entry *ntapi::status_category_type::find_entry(long ntstatus)
	{
		/* Load NT status table at runtime to use current locale strings. */
		static const auto table = []()
		{
			static constexpr std::pair<ulong, ulong> err_ranges[] = {{0x00000000, 0x0000ffff}, {0x40000000, 0x4000ffff}, {0x80000001, 0x8000ffff}, {0xc0000001, 0xc000ffff}};
			std::vector<ntapi::status_category_type::entry> result;

			constexpr static int buff_size = 32768;
			auto tmp_buffer = std::make_unique<wchar_t[]>(buff_size);
			auto msg_buffer = std::make_unique<char[]>(buff_size);

			for (const auto &range : err_ranges)
				for (ulong i = range.first; i < range.second; ++i)
				{
					/* Decode message string from NT status. Assume status code does not exist if FormatMessageW fails. */
					const auto ntdll = ntapi::instance.ntdll;
					const auto flags = FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
					const auto tmp_length = ::FormatMessageW(flags, ntdll, i, 0, tmp_buffer.get(), buff_size, nullptr);
					if (tmp_length == 0) [[unlikely]]
						continue;
					const auto msg_length = ::WideCharToMultiByte(65001 /* UTF8 */, WC_ERR_INVALID_CHARS, tmp_buffer.get(), int(tmp_length), msg_buffer.get(), buff_size, nullptr, nullptr);
					if (msg_length < 0) [[unlikely]]
						continue;

					/* Decode Win32 & POSIX error codes from NT status. */
					int posix_err = 0;
					const auto win32_err = ntapi::instance.RtlNtStatusToDosError(i);
					const auto cnd = std::error_code(static_cast<int>(win32_err), std::system_category()).default_error_condition();
					if (cnd.category() == std::generic_category())
						posix_err = cnd.value();

					/* Create the resulting entry. */
					result.emplace_back(i, win32_err, posix_err, std::string(msg_buffer.get(), msg_length));
				}

			return result;
		}();

		const auto pos = std::ranges::find_if(table, [=](auto &e) { return e.ntstatus == ntstatus; });
		return pos == table.end() ? nullptr : std::to_address(pos);
	}

	std::string ntapi::status_category_type::message(int value) const
	{
		if (const auto e = find_entry(value); e != nullptr && !e->msg.empty()) [[likely]]
			return e->msg;

		if (static_cast<ulong>(value) >= 0xc000'0000)
			return "Unknown NT error code";
		else if (static_cast<ulong>(value) >= 0x8000'0000)
			return "Unknown NT warning code";
		else if (static_cast<ulong>(value) >= 0x4000'0000)
			return "Unknown NT message code";
		else
			return "Unknown NT success code";
	}

	const std::error_category *ntapi::status_category()
	{
		static status_category_type value;
		return &value;
	}
}
#endif
/*
 * Created by switch_blade on 2023-07-04.
 */

#include "ntapi.hpp"

#ifdef ROD_WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <memory>

namespace rod::_ntapi
{
	inline static auto *load_dll(const char *name)
	{
		auto handle = ::GetModuleHandleA(name);
		if (handle == nullptr)
		{
			const auto err = std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
			throw std::system_error(err, std::string("GetModuleHandleW(\"") + name + "\")");
		}
		return handle;
	}
	template<typename S>
	inline static auto *load_sym(void *dll, const char *dll_name, const char *sym_name)
	{
		if (const auto ptr = ::GetProcAddress(static_cast<HMODULE>(dll), sym_name); ptr == nullptr)
		{
			const auto err = std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
			const auto msg = std::string("GetProcAddress(\"") + dll_name + "\", \"" + sym_name + "\")";
			throw std::system_error(err, msg);
		}
		else if constexpr (std::is_function_v<S>)
			return reinterpret_cast<S>(reinterpret_cast<void (*)()>(ptr));
		else
			return reinterpret_cast<S>(reinterpret_cast<void *>(ptr));
	};

	const api_type &api()
	{
		static const auto value = []()
		{
			api_type result = {.ntdll = load_dll("ntdll.dll")};

			result.RtlNtStatusToDosError = load_sym<RtlNtStatusToDosError_t>(result.ntdll, "ntdll.dll", "RtlNtStatusToDosError");
			result.RtlDosPathNameToNtPathName_U = load_sym<RtlDosPathNameToNtPathName_U_t>(result.ntdll, "ntdll.dll", "RtlDosPathNameToNtPathName_U");

			result.NtReadFile = load_sym<NtReadFile_t>(result.ntdll, "ntdll.dll", "NtReadFile");
			result.NtWriteFile = load_sym<NtWriteFile_t>(result.ntdll, "ntdll.dll", "NtWriteFile");
			result.NtCreateFile = load_sym<NtCreateFile_t>(result.ntdll, "ntdll.dll", "NtCreateFile");

			result.NtQueryInformationFile = load_sym<NtQueryInformationFile_t>(result.ntdll, "ntdll.dll", "NtQueryInformationFile");
			result.NtQueryVolumeInformationFile = load_sym<NtQueryVolumeInformationFile_t>(result.ntdll, "ntdll.dll", "NtQueryVolumeInformationFile");

			result.NtCancelIoFileEx = load_sym<NtCancelIoFileEx_t>(result.ntdll, "ntdll.dll", "NtCancelIoFileEx");
			result.NtSetIoCompletion = load_sym<NtSetIoCompletion_t>(result.ntdll, "ntdll.dll", "NtSetIoCompletion");
			result.NtWaitForSingleObject = load_sym<NtWaitForSingleObject_t>(result.ntdll, "ntdll.dll", "NtWaitForSingleObject");
			result.NtRemoveIoCompletionEx = load_sym<NtRemoveIoCompletionEx_t>(result.ntdll, "ntdll.dll", "NtRemoveIoCompletionEx");

	        return result;
		}();
		return value;
	}

	status api_type::cancel_io(void *handle, io_status_block *iosb) const noexcept
	{
		if (iosb->status != STATUS_PENDING)
			return iosb->status;

		const auto status = NtCancelIoFileEx(handle, iosb, iosb);
		if (status > message_status_max)
		{
			if (status == 0xc0000225 /* STATUS_NOT_FOUND */)
			{
				iosb->status = 0xc0000120 /*STATUS_CANCELLED */;
				return iosb->status;
			}
			return status;
		}
		if (iosb->status == 0)
		{
			iosb->status = 0xc0000120 /* STATUS_CANCELLED */;
			return iosb->status;
		}
	}
	status api_type::wait_io(void *handle, io_status_block *iosb, const large_integer *timeout) const noexcept
	{
		while (iosb->status == 0x00000103 /* STATUS_PENDING */)
		{
			auto status = NtWaitForSingleObject(handle, true, timeout);
			if (status == 0 /* STATUS_SUCCESS */)
				iosb->status = 0;
			else if (status == STATUS_TIMEOUT)
			{
				status = cancel_io(handle, iosb); /* Cancel pending IO on timeout. */
				if (status <= message_status_max || status == 0xc0000120 /* STATUS_CANCELLED */)
					iosb->status = STATUS_TIMEOUT;
				break;
			}
		}
		return iosb->status;
	}

	struct status_entry
	{
		status status = {};
		ulong dos_err = {};
		int posix_err = {};
		std::string msg = {};
	};

	const status_entry *find_status(long status)
	{
		/* Load NT status table at runtime to use current locale strings. */
		static const auto table = []()
		{
			constexpr std::pair<ulong, ulong> err_ranges[] = {{0x0000'0000, 0x0000'ffff}, {0x4000'0000, 0x4000'ffff}, {0x8000'0001, 0x8000'ffff}, {0xc000'0001, 0xc000'ffff}};
			constexpr int buff_size = 32768;

			auto tmp_buffer = std::make_unique<wchar_t[]>(buff_size);
			auto msg_buffer = std::make_unique<char[]>(buff_size);
			std::vector<status_entry> result;

			for (const auto &range : err_ranges)
				for (ulong i = range.first; i < range.second; ++i)
				{
					/* Decode message string from NT status. Assume status code does not exist if FormatMessageW fails. */
					const auto flags = FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
					const auto tmp_length = ::FormatMessageW(flags, api().ntdll, i, 0, tmp_buffer.get(), buff_size, nullptr);
					if (tmp_length == 0) [[unlikely]]
						continue;
					const auto msg_length = ::WideCharToMultiByte(65001 /* UTF8 */, WC_ERR_INVALID_CHARS, tmp_buffer.get(), int(tmp_length), msg_buffer.get(), buff_size, nullptr, nullptr);
					if (msg_length < 0) [[unlikely]]
						continue;

					/* Decode Win32 & POSIX error codes from NT status. */
					int posix_err = 0;
					const auto win32_err = api().RtlNtStatusToDosError(i);
					const auto cnd = std::error_code(static_cast<int>(win32_err), std::system_category()).default_error_condition();
					if (cnd.category() == std::generic_category())
						posix_err = cnd.value();

					/* Create the resulting entry. */
					result.emplace_back(i, win32_err, posix_err, std::string(msg_buffer.get(), msg_length));
				}

			return result;
		}();

		const auto pos = std::ranges::find_if(table, [=](auto &e) { return e.status == status; });
		return pos == table.end() ? nullptr : std::to_address(pos);
	}
	const std::error_category &status_category() noexcept
	{
		static const status_category_type value;
		return value;
	}

	std::string status_category_type::message(int value) const
	{
		if (const auto e = find_status(value); e != nullptr && !e->msg.empty()) [[likely]]
			return e->msg;

		const auto status = static_cast<ulong>(value);
		if (status <= success_status_max)
			return "Unknown NT success code";
		else if (status <= message_status_max)
			return "Unknown NT message code";
		else if (status <= warning_status_max)
			return "Unknown NT warning code";
		else
			return "Unknown NT error code";
	}
	std::error_condition status_category_type::default_error_condition(int value) const noexcept
	{
		if (const auto e = find_status(value); e && e->posix_err)
			return {e->posix_err, std::generic_category()};
		else
			return {value, *this};
	}
	bool status_category_type::equivalent(const std::error_code &code, int value) const noexcept
	{
		if (code.category() == *this || std::strcmp(code.category().name(), name()) == 0)
			return code.value() == value;

		const auto e = find_status(value);
		if (e && (code.category() == std::system_category() || std::strcmp(code.category().name(), std::system_category().name()) == 0))
			return code.value() == static_cast<int>(e->dos_err);
		if (e && (code.category() == std::generic_category() || std::strcmp(code.category().name(), std::generic_category().name()) == 0))
			return code.value() == e->posix_err;
		return false;
	}
	bool status_category_type::equivalent(int value, const std::error_condition &cnd) const noexcept
	{
		if (cnd.category() == *this || std::strcmp(cnd.category().name(), name()) == 0)
			return cnd.value() == value;

		const auto e = find_status(value);
		if (e && (cnd.category() == std::system_category() || std::strcmp(cnd.category().name(), std::system_category().name()) == 0))
			return cnd.value() == static_cast<int>(e->dos_err);
		if (e && (cnd.category() == std::generic_category() || std::strcmp(cnd.category().name(), std::generic_category().name()) == 0))
			return cnd.value() == e->posix_err;
		return false;
	}
}
#endif
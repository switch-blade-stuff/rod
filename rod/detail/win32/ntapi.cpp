/*
 * Created by switchblade on 2023-07-04.
 */

#include "ntapi.hpp"

#if __has_include("nttab.gen.hpp")
#include "nttab.gen.hpp"
#else
#error NTSTATUS table must be generated first!
#endif

namespace rod::_win32
{
	const std::error_category &status_category() noexcept
	{
		static const status_category_type value;
		return value;
	}
	const auto *find_status(long status) noexcept
	{
		/* Load NT status table at runtime to use current locale strings. */
		const auto pos = std::ranges::find_if(ntstatus_table, [=](auto &e) { return e.status == status; });
		return pos == std::ranges::end(ntstatus_table) ? nullptr : std::to_address(pos);
	}

	std::string status_category_type::message(int value) const
	{
		if (const auto e = find_status(value); e != nullptr && !e->msg.empty()) [[likely]]
			return std::string(e->msg);

		const auto status = ULONG(value);
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
			return code.value() == int(e->dos_err);
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
			return cnd.value() == int(e->dos_err);
		if (e && (cnd.category() == std::generic_category() || std::strcmp(cnd.category().name(), std::generic_category().name()) == 0))
			return cnd.value() == e->posix_err;
		return false;
	}

	[[nodiscard]] inline static result<void *> load_library(const char *name) noexcept
	{
		if (auto handle = ::LoadLibraryA(name); handle == nullptr)
			return dos_error_code(::GetLastError());
		else
			return handle;
	}
	template<typename S>
	[[nodiscard]] inline static result<S> load_sym(void *lib, const char *name, bool opt = false) noexcept
	{
		if (const auto ptr = ::GetProcAddress(static_cast<HMODULE>(lib), name); ptr == nullptr && !opt)
			return dos_error_code(::GetLastError());
		else if constexpr (std::is_function_v<S>)
			return reinterpret_cast<S>(reinterpret_cast<void (*)()>(ptr));
		else
			return reinterpret_cast<S>(reinterpret_cast<void *>(ptr));
	}

	const result<ntapi> &ntapi::instance() noexcept
	{
		static const auto value = []() -> result<ntapi>
		{
			ntapi result = {};

			if (auto hnd = load_library("ntdll.dll"); hnd.has_value()) [[likely]]
				result.ntdll = *hnd;
			else
				return hnd.error();
			if (auto hnd = load_library("bcrypt.dll"); hnd.has_value()) [[likely]]
				result.bcrypt = *hnd;
			else
				return hnd.error();
			if (auto hnd = load_library("kernel32.dll"); hnd.has_value()) [[likely]]
				result.kernel32 = *hnd;
			else
				return hnd.error();

			if (auto sym = load_sym<RtlIsDosDeviceName_U_t>(result.ntdll, "RtlIsDosDeviceName_U"); sym.has_value()) [[likely]]
				result.RtlIsDosDeviceName_U = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<RtlNtStatusToDosError_t>(result.ntdll, "RtlNtStatusToDosError"); sym.has_value()) [[likely]]
				result.RtlNtStatusToDosError = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<RtlDosPathNameToNtPathName_U_t>(result.ntdll, "RtlDosPathNameToNtPathName_U"); sym.has_value()) [[likely]]
				result.RtlDosPathNameToNtPathName_U = *sym;
			else
				return sym.error();

			if (auto sym = load_sym<NtOpenFile_t>(result.ntdll, "NtOpenFile"); sym.has_value()) [[likely]]
				result.NtOpenFile = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<NtCreateFile_t>(result.ntdll, "NtCreateFile"); sym.has_value()) [[likely]]
				result.NtCreateFile = *sym;
			else
				return sym.error();

			if (auto sym = load_sym<NtReadFile_t>(result.ntdll, "NtReadFile"); sym.has_value()) [[likely]]
				result.NtReadFile = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<NtWriteFile_t>(result.ntdll, "NtWriteFile"); sym.has_value()) [[likely]]
				result.NtWriteFile = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<NtQueryDirectoryFile_t>(result.ntdll, "NtQueryDirectoryFile"); sym.has_value()) [[likely]]
				result.NtQueryDirectoryFile = *sym;
			else
				return sym.error();

			if (auto sym = load_sym<NtFlushBuffersFile_t>(result.ntdll, "NtFlushBuffersFile"); sym.has_value()) [[likely]]
				result.NtFlushBuffersFile = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<NtFlushBuffersFileEx_t>(result.ntdll, "NtFlushBuffersFileEx", true); sym.has_value()) [[likely]]
				result.NtFlushBuffersFileEx = *sym;
			else
				return sym.error();

			if (auto sym = load_sym<NtQueryAttributesFile_t>(result.ntdll, "NtQueryAttributesFile"); sym.has_value()) [[likely]]
				result.NtQueryAttributesFile = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<NtQueryInformationByName_t>(result.ntdll, "NtQueryInformationByName", true); sym.has_value()) [[likely]]
				result.NtQueryInformationByName = *sym;
			else
				return sym.error();

			if (auto sym = load_sym<NtSetInformationFile_t>(result.ntdll, "NtSetInformationFile"); sym.has_value()) [[likely]]
				result.NtSetInformationFile = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<NtQueryInformationFile_t>(result.ntdll, "NtQueryInformationFile"); sym.has_value()) [[likely]]
				result.NtQueryInformationFile = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<NtQueryVolumeInformationFile_t>(result.ntdll, "NtQueryVolumeInformationFile"); sym.has_value()) [[likely]]
				result.NtQueryVolumeInformationFile = *sym;
			else
				return sym.error();

			if (auto sym = load_sym<NtCancelIoFileEx_t>(result.ntdll, "NtCancelIoFileEx"); sym.has_value()) [[likely]]
				result.NtCancelIoFileEx = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<NtSetIoCompletion_t>(result.ntdll, "NtSetIoCompletion"); sym.has_value()) [[likely]]
				result.NtSetIoCompletion = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<NtWaitForSingleObject_t>(result.ntdll, "NtWaitForSingleObject"); sym.has_value()) [[likely]]
				result.NtWaitForSingleObject = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<NtRemoveIoCompletionEx_t>(result.ntdll, "NtRemoveIoCompletionEx"); sym.has_value()) [[likely]]
				result.NtRemoveIoCompletionEx = *sym;
			else
				return sym.error();

			if (auto sym = load_sym<NtOpenSection_t>(result.ntdll, "NtOpenSection"); sym.has_value()) [[likely]]
				result.NtOpenSection = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<NtCreateSection_t>(result.ntdll, "NtCreateSection"); sym.has_value()) [[likely]]
				result.NtCreateSection = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<NtQuerySection_t>(result.ntdll, "NtQuerySection"); sym.has_value()) [[likely]]
				result.NtQuerySection = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<NtExtendSection_t>(result.ntdll, "NtExtendSection"); sym.has_value()) [[likely]]
				result.NtExtendSection = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<NtFreeVirtualMemory_t>(result.ntdll, "NtFreeVirtualMemory"); sym.has_value()) [[likely]]
				result.NtFreeVirtualMemory = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<NtQueryVirtualMemory_t>(result.ntdll, "NtQueryVirtualMemory"); sym.has_value()) [[likely]]
				result.NtQueryVirtualMemory = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<NtMapViewOfSection_t>(result.ntdll, "NtMapViewOfSection"); sym.has_value()) [[likely]]
				result.NtMapViewOfSection = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<NtUnmapViewOfSection_t>(result.ntdll, "NtUnmapViewOfSection"); sym.has_value()) [[likely]]
				result.NtUnmapViewOfSection = *sym;
			else
				return sym.error();

			if (auto sym = load_sym<BCryptGenRandom_t>(result.bcrypt, "BCryptGenRandom"); sym.has_value()) [[likely]]
				result.BCryptGenRandom = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<BCryptOpenAlgorithmProvider_t>(result.bcrypt, "BCryptOpenAlgorithmProvider"); sym.has_value()) [[likely]]
				result.BCryptOpenAlgorithmProvider = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<BCryptCloseAlgorithmProvider_t>(result.bcrypt, "BCryptCloseAlgorithmProvider"); sym.has_value()) [[likely]]
				result.BCryptCloseAlgorithmProvider = *sym;
			else
				return sym.error();

			if (auto sym = load_sym<DiscardVirtualMemory_t>(result.kernel32, "DiscardVirtualMemory", true); sym.has_value()) [[likely]]
				result.DiscardVirtualMemory = *sym;
			else
				return sym.error();
			if (auto sym = load_sym<PrefetchVirtualMemory_t>(result.kernel32, "PrefetchVirtualMemory", true); sym.has_value()) [[likely]]
				result.PrefetchVirtualMemory = *sym;
			else
				return sym.error();

			return result;
		}();
		return value;
	}

	result<heapalloc_ptr<wchar_t>> ntapi::dos_path_to_nt_path(unicode_string &upath, bool passthrough) const noexcept
	{
		/* Ignore paths that already start with an NT prefix or are relative to a base directory. */
		const auto sv = std::wstring_view(upath.buff, upath.size / sizeof(wchar_t));
		if (passthrough || sv.starts_with(LR"(\!!\)") || sv.starts_with(LR"(\??\)"))
		{
			if (sv.starts_with(LR"(\!!\)"))
			{
				upath.size -= 3 * sizeof(wchar_t);
				upath.max -= 3 * sizeof(wchar_t);
				upath.buff += 3;
			}
			return {};
		}

		if (!RtlDosPathNameToNtPathName_U(upath.buff, &upath, nullptr, nullptr)) [[unlikely]]
			return std::make_error_code(std::errc::no_such_file_or_directory);
		else
			return heapalloc_ptr<wchar_t>(upath.buff);
	}
	result<heapalloc_ptr<wchar_t>> ntapi::canonize_win32_path(unicode_string &upath, bool passthrough) const noexcept
	{
		/* Ignore paths that don't need canonization. */
		const auto sv = std::wstring_view(upath.buff, upath.size / sizeof(wchar_t));
		if (passthrough || !(sv.starts_with(LR"(\!!\)") || sv.starts_with(LR"(\??\)") || sv.starts_with(LR"(\\?\)")) || sv.starts_with(LR"(\\.\)"))
			return {};

		unicode_string src_path = upath;
		heapalloc_ptr<wchar_t> guard;

		/* Duplicate the source string. Make sure to allocate enough space for potential GLOBALROOT\, any other slack and null terminator. */
		upath.max = (upath.size = src_path.size) + (MAX_PATH + 1) * sizeof(wchar_t);
		upath.buff = static_cast<wchar_t *>(::HeapAlloc(::GetProcessHeap(), 0, upath.max));
		if (upath.buff == nullptr) [[unlikely]]
			return std::make_error_code(std::errc::not_enough_memory);
		else
			guard.reset(upath.buff);

		std::memcpy(upath.buff, src_path.buff, src_path.size);
		upath.buff[upath.size / sizeof(wchar_t)] = L'\0';

		/* Use RtlNtPathNameToDosPathName for \??\ namespace paths. */
		if (upath.size >= 8 && std::wcsncmp(upath.buff, LR"(\??\)", 4) == 0)
		{
			auto conv_buff = unicode_string_buffer();
			conv_buff.buffer.buff = (PUCHAR) upath.buff;
			conv_buff.buffer.size = (SIZE_T) upath.size;
			conv_buff.string = upath;

			if (!RtlNtPathNameToDosPathName(0, &conv_buff, nullptr, nullptr)) [[unlikely]]
				return std::make_error_code(std::errc::no_such_file_or_directory);
			else
				upath = conv_buff.string;
		}

		/* Replace \!!\Gobal??\ with \\?\ */
		if (upath.size >= 24 && _wcsnicmp(upath.buff, LR"(\!!\Global??\)", 12) == 0)
		{
			std::memcpy(upath.buff += 8, LR"(\\?)", 6);
			upath.size -= 16;
			upath.max -= 16;
		}
		/* Replace \!!\Device\ with \\.\ */
		if (upath.size >= 22 && _wcsnicmp(upath.buff, LR"(\!!\Device\)", 11) == 0)
		{
			std::memcpy(upath.buff += 7, LR"(\\.)", 6);
			upath.size -= 14;
			upath.max -= 14;
		}
		/* Replace \!!\ with \\?\GLOBALROOT\ */
		if (upath.size >= 8 && std::wcsncmp(upath.buff, LR"(\!!\)", 4) == 0)
		{
			std::move_backward(upath.buff + 3, upath.buff + upath.size / sizeof(wchar_t) + 1, upath.buff + 14);
			std::memcpy(upath.buff, L"\\\\?\\GLOBALROOT", 28);
			upath.size += 22;
		}

		/* Replace \\?\UNC\ with \\, only if there are no illegal sequences. */
		if (upath.size >= 16 && std::wcsncmp(upath.buff, LR"(\\?\UNC\)", 8) == 0 && !_path::has_illegal_path_sequences({upath.buff, upath.size / sizeof(wchar_t)}))
		{
			std::memcpy(upath.buff += 6, L"\\\\", 4);
			upath.size -= 12;
			upath.max -= 12;
		}
		/* Replace \\?\X: with X:, only if there are no illegal sequences. */
		if (upath.size >= 12 && std::wcsncmp(upath.buff, LR"(\\?\)", 4) == 0 && _path::is_drive_letter(upath.buff[4]) && upath.buff[5] == L':' && !_path::has_illegal_path_sequences({upath.buff + 12, upath.size / sizeof(wchar_t) - 12}))
		{
			upath.buff += 4;
			upath.size -= 8;
			upath.max -= 8;
		}
		/* If the path does not start with \\?\ but there are illegal sequences, add \\?\. */
		if (upath.size >= 8 && std::wcsncmp(upath.buff, LR"(\\?\)", 4) != 0 && _path::has_illegal_path_sequences({upath.buff - 8, upath.size / sizeof(wchar_t) - 8}))
		{
			std::move_backward(upath.buff, upath.buff + upath.size / sizeof(wchar_t) + 1, upath.buff + 4);
			std::memcpy(upath.buff, L"\\\\?\\", 8);
			upath.size += 6;
		}

		return std::move(guard);
	}

	result<void *> ntapi::reopen_file(void *handle, io_status_block *iosb, ULONG access, ULONG share, ULONG opts, const fs::file_timeout &to) const noexcept
	{
		auto attr = object_attributes();
		auto path = unicode_string();

		/* Hack to re-open a file by using root handle empty path. */
		attr.length = sizeof(object_attributes);
		attr.root_dir = handle;
		attr.name = &path;

		return open_file(attr, iosb, access, share, opts, to);
	}
	result<void *> ntapi::open_file(const object_attributes &obj, io_status_block *iosb, ULONG access, ULONG share, ULONG opts, const fs::file_timeout &to) const noexcept
	{
		auto handle = INVALID_HANDLE_VALUE;
		*iosb = io_status_block();

		auto status = NtOpenFile(&handle, access, &obj, iosb, share, opts);
		if (status == STATUS_PENDING) [[unlikely]]
			status = wait_io(handle, iosb, to);
		if (iosb->info == 5 /*FILE_DOES_NOT_EXIST*/) [[unlikely]]
			return std::make_error_code(std::errc::no_such_file_or_directory);
		if (status == 0xc0000120 /*STATUS_CANCELLED*/) [[unlikely]]
			return std::make_error_code(std::errc::operation_canceled);
		if (status == STATUS_TIMEOUT) [[unlikely]]
			return std::make_error_code(std::errc::timed_out);
		if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return handle;
	}
	result<void *> ntapi::create_file(const object_attributes &obj, io_status_block *iosb, ULONG access, ULONG attr, ULONG share, disposition disp, ULONG opts, const fs::file_timeout &to) const noexcept
	{
		auto handle = INVALID_HANDLE_VALUE;
		*iosb = io_status_block();

		auto status = NtCreateFile(&handle, access, &obj, iosb, nullptr, attr, share, disp, opts, nullptr, 0);
		if (status == STATUS_PENDING) [[unlikely]]
			status = wait_io(handle, iosb, to);
		if (iosb->info == 5 /*FILE_DOES_NOT_EXIST*/) [[unlikely]]
			return std::make_error_code(std::errc::no_such_file_or_directory);
		if (status == 0xc0000120 /*STATUS_CANCELLED*/) [[unlikely]]
			return std::make_error_code(std::errc::operation_canceled);
		if (status == STATUS_TIMEOUT) [[unlikely]]
			return std::make_error_code(std::errc::timed_out);
		if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return handle;
	}

	ntstatus ntapi::link_file(void *handle, io_status_block *iosb, void *base, unicode_string &upath, bool replace, const fs::file_timeout &to) const noexcept
	{
		auto buff_size = upath.size * sizeof(wchar_t) + sizeof(file_link_information);
		auto buff = ROD_MAKE_BUFFER(std::byte, buff_size);
		if (buff.get() == nullptr) [[unlikely]]
			return STATUS_NO_MEMORY;

		auto link_info = reinterpret_cast<file_link_information *>(buff.get());
		std::memcpy(link_info->name, upath.buff, upath.size);
		link_info->flags = replace ? 0x1 | 0x2 /*FILE_LINK_REPLACE_IF_EXISTS | FILE_LINK_POSIX_SEMANTICS*/ : 0;
		link_info->root_dir = base != INVALID_HANDLE_VALUE ? base : nullptr;
		link_info->name_len = upath.size;

		/* Try FileLinkInformationEx first, which will fail before Win10 RS5, in which case fall back to FileLinkInformation. */
		auto status = set_file_info(handle, iosb, link_info, buff_size, FileLinkInformationEx, to);
		if (is_status_failure(status))
			status = set_file_info(handle, iosb, link_info, buff_size, FileLinkInformation, to);
		return status;
	}
	ntstatus ntapi::relink_file(void *handle, io_status_block *iosb, void *base, unicode_string &upath, bool replace, const fs::file_timeout &to) const noexcept
	{
		auto buff_size = upath.size * sizeof(wchar_t) + sizeof(file_rename_information);
		auto buff = ROD_MAKE_BUFFER(std::byte, buff_size);
		if (buff.get() == nullptr) [[unlikely]]
			return STATUS_NO_MEMORY;

		auto rename_info = reinterpret_cast<file_rename_information *>(buff.get());
		std::memcpy(rename_info->name, upath.buff, upath.size);
		rename_info->flags = replace ? 0x1 | 0x2 /*FILE_RENAME_REPLACE_IF_EXISTS | FILE_RENAME_POSIX_SEMANTICS*/ : 0;
		rename_info->root_dir = base != INVALID_HANDLE_VALUE ? base : nullptr;
		rename_info->name_len = upath.size;

		/* Try FileRenameInformationEx first, which will fail before Win10 RS1, in which case fall back to FileRenameInformation. */
		auto status = set_file_info(handle, iosb, rename_info, buff_size, FileRenameInformationEx, to);
		if (is_status_failure(status))
			status = set_file_info(handle, iosb, rename_info, buff_size, FileRenameInformation, to);
		return status;
	}
	ntstatus ntapi::unlink_file(void *handle, io_status_block *iosb, bool mark_for_delete, const fs::file_timeout &to) const noexcept
	{
		/* Try to unlink with POSIX semantics via the Win10 API. */
		auto disp_info_ex = file_disposition_information_ex{.flags = 0x1 | 0x2}; /*FILE_DISPOSITION_DELETE | FILE_DISPOSITION_POSIX_SEMANTICS*/
		if (auto status = set_file_info(handle, iosb, &disp_info_ex, FileBasicInformation, FileDispositionInformationEx); is_status_failure(status))
		{
			/* Hiding the object is not strictly necessary so discard the error on failure. */
			auto basic_info = file_basic_information{.attributes = FILE_ATTRIBUTE_HIDDEN};
			set_file_info(handle, iosb, &basic_info, FileBasicInformation, to);

			if (mark_for_delete)
			{
				/* Unlike `remove` and `remove_all`, `unlink` requires sufficient access rights, so do not re-try when returned STATUS_ACCESS_DENIED. */
				auto disp_info = file_disposition_information{.del = true};
				return set_file_info(handle, iosb, &disp_info, FileDispositionInformation, to);
			}
		}
		return 0;
	}

	ntstatus ntapi::set_file_info(void *handle, io_status_block *iosb, void *data, std::size_t size, file_info_type type, const fs::file_timeout &to) const noexcept
	{
		*iosb = io_status_block();
		auto status = NtSetInformationFile(handle, iosb, data, ULONG(size), type);
		if (status == STATUS_PENDING)
			status = wait_io(handle, iosb, to);
		return status;
	}
	ntstatus ntapi::get_file_info(void *handle, io_status_block *iosb, void *data, std::size_t size, file_info_type type, const fs::file_timeout &to) const noexcept
	{
		*iosb = io_status_block();
		auto status = NtQueryInformationFile(handle, iosb, data, ULONG(size), type);
		if (status == STATUS_PENDING)
			status = wait_io(handle, iosb, to);
		return status;
	}

	ntstatus ntapi::cancel_io(void *handle, io_status_block *iosb) const noexcept
	{
		if (iosb->status != STATUS_PENDING)
			return iosb->status;

		const auto status = NtCancelIoFileEx(handle, iosb, iosb);
		if (is_status_failure(status))
		{
			if (status == 0xc0000225 /*STATUS_NOT_FOUND*/)
				return iosb->status = 0xc0000120 /*STATUS_CANCELLED*/;
			else
				return status;
		}
		if (iosb->status == 0)
		{
			iosb->status = 0xc0000120; /*STATUS_CANCELLED*/
			return iosb->status;
		}
		return iosb->status;
	}
	ntstatus ntapi::wait_io(void *handle, io_status_block *iosb, const fs::file_timeout &to) const noexcept
	{
		auto time_end = std::chrono::steady_clock::time_point();
		auto timeout_ft = filetime();
		auto timeout = &timeout_ft;

		if (to.is_relative())
			time_end = std::chrono::steady_clock::now() + to.relative({});
		else if (to.is_absolute())
			timeout_ft = tp_to_filetime(to.absolute({}));
		else
			timeout = nullptr;

		while (iosb->status == STATUS_PENDING)
		{
			if (to.is_relative())
			{
				if (auto time_left = time_end - std::chrono::steady_clock::now(); time_left.count() >= 0)
					timeout_ft = tp_to_filetime(time_left.count() / -100);
				else
					timeout_ft = filetime();
			}

			auto status = NtWaitForSingleObject(handle, true, timeout);
			if (status == 0 /* STATUS_SUCCESS */)
				iosb->status = 0;
			else if (status == STATUS_TIMEOUT)
			{
				status = cancel_io(handle, iosb); /* Cancel pending IO on timeout. */
				if (!is_status_failure(status) || status == 0xc0000120 /*STATUS_CANCELLED*/)
					iosb->status = STATUS_TIMEOUT;
				break;
			}
		}
		return iosb->status;
	}

	ntstatus ntapi::free_mapped_pages(std::byte *addr, std::size_t size) const noexcept
	{
		for (auto info = memory_region_information(); size > 0;)
		{
			/* MemoryRegionInformation is undocumented (even in ReactOS), but is supported by most versions of WinNT, and provides much faster
			 * interface than the documented MemoryBasicInformation due to not being required to scan the VMA for page metadata. */
			if (auto status = NtQueryVirtualMemory(::GetCurrentProcess(), addr, MemoryRegionInformation, &info, sizeof(info), nullptr); is_status_failure(status)) [[unlikely]]
				return status;
			if (auto status = NtUnmapViewOfSection(::GetCurrentProcess(), info.base); is_status_failure(status)) [[unlikely]]
				return status;

			addr += info.size;
			if (info.size < size)
				size -= info.size;
			else
				break;
		}
		return 0;
	}
	ntstatus ntapi::free_virtual_pages(std::byte *addr, std::size_t size, ULONG op) const noexcept
	{
		while (size > 0)
		{
			std::size_t region_size = 0;
			void *region_addr = addr;
			if (auto status = NtFreeVirtualMemory(::GetCurrentProcess(), &region_addr, &region_size, op); is_status_failure(status)) [[unlikely]]
				return status;

			size -= region_size;
			addr += region_size;
		}
		return 0;
	}

	result<bool> is_elevated() noexcept
	{
		std::array<std::byte, 1024> owner_buffer, user_buffer;
		DWORD written;
		HANDLE token;

		if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_ALL_ACCESS, &token)) [[unlikely]]
			return {in_place_error, dos_error_code(::GetLastError())};

		auto guard = defer_invoke([&]() noexcept { ::CloseHandle(token); });
		if (!::GetTokenInformation(token, TokenOwner, owner_buffer.data(), DWORD(owner_buffer.size()), &written)) [[unlikely]]
			return {in_place_error, dos_error_code(::GetLastError())};
		if (!::GetTokenInformation(token, TokenUser, user_buffer.data(), DWORD(user_buffer.size()), &written)) [[unlikely]]
			return {in_place_error, dos_error_code(::GetLastError())};

		const auto owner = reinterpret_cast<const TOKEN_OWNER *>(owner_buffer.data());
		const auto user = reinterpret_cast<const TOKEN_USER *>(user_buffer.data());
		return {in_place_value, ::EqualSid(owner->Owner, user->User.Sid) == 0};
	}
}

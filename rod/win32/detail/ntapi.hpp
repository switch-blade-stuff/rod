/*
 * Created by switch_blade on 2023-07-04.
 */

#pragma once

#include "../../detail/config.hpp"

#ifdef ROD_WIN32

#include <system_error>

namespace rod::_ntapi
{
	using ushort = unsigned short;
	using ulong = unsigned long;
	using longlong = long long;
	using status = ulong;

	inline constexpr status error_status_max = 0xffff'ffff;
	inline constexpr status warning_status_max = 0x8fff'ffff;
	inline constexpr status message_status_max = 0x7fff'ffff;
	inline constexpr status success_status_max = 0x3fff'ffff;

	using long_ptr = std::intptr_t;
	using ulong_ptr = std::uintptr_t;

	/* Forward declare these to avoid including windows headers */
	struct unicode_string
	{
		ushort size;
		ushort max;
		wchar_t *buffer;
	};
	union large_integer
	{
		struct
		{
			ulong low;
			long high;
		} u;
		longlong quad;
	};

	struct rtlp_curdir_ref
	{
		long refcount;
		void *handle;
	};
	struct rtl_relative_name_u
	{
		unicode_string name;
		void *containing_dir;
		rtlp_curdir_ref cur_dir;
	};

	using RtlNtStatusToDosError_t = ulong (ROD_NTAPI *)(_In_ status status);
	using RtlDosPathNameToNtPathName_U_t = bool (ROD_NTAPI *)(_In_ const wchar_t *dos_name, _Out_ unicode_string *nt_name, _Out_ const wchar_t **part, _Out_ rtl_relative_name_u *relative);

	struct io_status_block
	{
		union
		{
			ulong status = -1;
			long_ptr ptr;
		};
		ulong_ptr info = {};
	};
	struct io_completion_info
	{
		ulong_ptr key_context = {};
		ulong_ptr apc_context = {};
		io_status_block status;
	};

	struct object_attributes
	{
		ulong length;
		void *root_dir;
		unicode_string *name;
		ulong attr;
		void *security_desc;
		void *security_qos;
	};

	enum disposition : ulong
	{
		file_supersede = 0,
		file_open,
		file_create,
		file_open_if,
		file_overwrite,
		file_overwrite_if,
	};

	using io_apc_routine = void (ROD_NTAPI *)(_In_ ulong_ptr apc_ctx, _In_ io_status_block *iosb, _In_ ulong);

	using NtReadFile_t = status (ROD_NTAPI *)(_In_ void *file, _In_opt_ void *evt, _In_opt_ io_apc_routine apc_func, _In_opt_ ulong_ptr apc_ctx, _Out_ io_status_block *iosb, _Out_ void *buff,
											  _In_ ulong len, _In_opt_ large_integer *off, _In_opt_ ulong *key);
	using NtWriteFile_t = status (ROD_NTAPI *)(_In_ void *file, _In_opt_ void *evt, _In_opt_ io_apc_routine apc_func, _In_opt_ ulong_ptr apc_ctx, _Out_ io_status_block *iosb, _In_ void *buff,
											   _In_ ulong len, _In_opt_ large_integer *off, _In_opt_ ulong *key);
	using NtCreateFile_t = status (ROD_NTAPI *)(_Out_ void **file, _In_ ulong access, _In_ object_attributes *obj_attr, _Out_ io_status_block *iosb, _In_opt_ const large_integer *size,
												_In_ ulong file_attr, _In_ ulong share, _In_ disposition disp, _In_ ulong opts, _In_opt_ void *buff, _In_ ulong len);

	enum file_info_type
	{
		FileModeInformation = 16,
		FileAlignmentInformation = 17,
	};
	enum fs_info_type
	{
		FileFsVolumeInformation = 1,
		FileFsLabelInformation = 2,
		FileFsSizeInformation = 3,
		FileFsDeviceInformation = 4,
		FileFsAttributeInformation = 5,
		FileFsControlInformation = 6,
		FileFsFullSizeInformation = 7,
		FileFsObjectIdInformation = 8,
		FileFsDriverPathInformation = 9,
		FileFsVolumeFlagsInformation = 10,
		FileFsSectorSizeInformation = 11,
		FileFsDataCopyInformation = 12,
		FileFsMetadataSizeInformation = 13,
		FileFsFullSizeInformationEx = 14,
	};

	using file_mode_info = ulong;
	using file_alignment_info = ulong;

	using NtQueryInformationFile_t = status (ROD_NTAPI *)(_In_ void *file, _Out_ io_status_block *status, _Out_ void *info, _In_ ulong len, _In_ file_info_type type);
	using NtQueryVolumeInformationFile_t = status (ROD_NTAPI *)(_In_ void *file, _Out_ io_status_block *status, _Out_ void *info, _In_ ulong len, _In_ fs_info_type type);

	using NtWaitForSingleObject_t = status (ROD_NTAPI *)(_In_ void *hnd, _In_ bool alert, _In_ const large_integer *timeout);
	using NtCancelIoFileEx_t = status (ROD_NTAPI *)(_In_ void *file, _Out_ io_status_block *iosb, _Out_ io_status_block *status);
	using NtSetIoCompletion_t = status (ROD_NTAPI *)(_In_ void *hnd, _In_ ulong key_ctx, _In_ ulong_ptr apc_ctx, _In_ long status, _In_ ulong info);
	using NtRemoveIoCompletionEx_t = status (ROD_NTAPI *)(_In_ void *hnd, _Out_writes_to_(count, *removed) io_completion_info *completion_info, _In_ ulong count, _Out_ ulong *removed, _In_opt_ large_integer *timeout, _In_ bool alert);

	struct api_type
	{
		status cancel_io(void *handle, io_status_block *iosb) const noexcept;
		status wait_io(void *handle, io_status_block *iosb, const large_integer *timeout = nullptr) const noexcept;

		void *ntdll;

		RtlNtStatusToDosError_t RtlNtStatusToDosError;
		RtlDosPathNameToNtPathName_U_t RtlDosPathNameToNtPathName_U;

		NtReadFile_t NtReadFile;
		NtWriteFile_t NtWriteFile;
		NtCreateFile_t NtCreateFile;

		NtQueryInformationFile_t NtQueryInformationFile;
		NtQueryVolumeInformationFile_t NtQueryVolumeInformationFile;

		NtCancelIoFileEx_t NtCancelIoFileEx;
		NtSetIoCompletion_t NtSetIoCompletion;
		NtWaitForSingleObject_t NtWaitForSingleObject;
		NtRemoveIoCompletionEx_t NtRemoveIoCompletionEx;
	};

	static const api_type &api();

	struct status_category_type : std::error_category
	{
		status_category_type() noexcept = default;
		~status_category_type() override = default;
		const char *name() const noexcept override { return "ntapi::status_category"; }

		std::string message(int value) const override;
		std::error_condition default_error_condition(int value) const noexcept override;
		bool equivalent(const std::error_code &code, int value) const noexcept override;
		bool equivalent(int value, const std::error_condition &cnd) const noexcept override;
	};

	static const std::error_category &status_category() noexcept;
	inline static std::error_code dos_error_code(ulong err) noexcept { return {static_cast<int>(err), std::system_category()}; }
	inline static std::error_code status_error_code(status status) noexcept { return {static_cast<int>(status), status_category()}; }
}

#endif
/*
 * Created by switch_blade on 2023-07-04.
 */

#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <combaseapi.h>
#include <winioctl.h>
#include <windows.h>
#include <ShlObj.h>

#ifdef _MSC_VER
#include <sal.h>
#else
#define _In_
#define _Inout_
#define _In_opt_
#define _Out_
#define _Out_writes_to_(A, B)
#define _Out_writes_bytes_(A)
#endif

#define ROD_NTAPI __stdcall

#include "../handle_stat.hpp"
#include "../handle_base.hpp"
#include "../timeout.hpp"

namespace rod::_win32
{
	using ntstatus = ULONG;

	inline constexpr ntstatus error_status_max = 0xffff'ffff;
	inline constexpr ntstatus warning_status_max = 0x8fff'ffff;
	inline constexpr ntstatus message_status_max = 0x7fff'ffff;
	inline constexpr ntstatus success_status_max = 0x3fff'ffff;

	[[nodiscard]] inline constexpr bool is_status_failure(ntstatus st) noexcept { return st > message_status_max; }

	inline constexpr file_type attr_to_type(ULONG file_attr, ULONG reparse_tag = 0) noexcept
	{
		constexpr auto attr_regular = FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | FILE_ATTRIBUTE_SPARSE_FILE | FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_ENCRYPTED | FILE_ATTRIBUTE_TEMPORARY |
		                              FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_OFFLINE | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_SYSTEM;

		if (file_attr & FILE_ATTRIBUTE_REPARSE_POINT && (reparse_tag == IO_REPARSE_TAG_MOUNT_POINT || reparse_tag == IO_REPARSE_TAG_SYMLINK))
			return file_type::symlink;
		if (file_attr & FILE_ATTRIBUTE_DIRECTORY)
			return file_type::directory;
		else if (file_attr & attr_regular)
			return file_type::regular;
		else
			return file_type::unknown;
	}

	struct unicode_string
	{
		USHORT size;
		USHORT max;
		PWCHAR buff;
	};

	template<bool Term = true>
	inline constexpr auto render_as_ustring(path_view_component path) noexcept -> result<std::pair<unicode_string, path_view_component::rendered_path<Term, wchar_t>>>
	{
		try
		{
			auto rpath = path_view_component::rendered_path<Term, wchar_t>(path);
			auto upath = unicode_string();

			upath.max = (upath.size = USHORT(rpath.size() * sizeof(wchar_t))) + sizeof(wchar_t);
			upath.buff = const_cast<wchar_t *>(rpath.data());

			return std::make_pair(std::move(upath), std::move(rpath));
		}
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
	}

	inline static FILETIME tp_to_filetime(typename file_clock::time_point tp) noexcept
	{
		union { FILETIME _ft; std::int64_t _tp; };
		return (_tp = tp.time_since_epoch().count(), _ft);
	}
	inline static typename file_clock::time_point filetime_to_tp(FILETIME ft) noexcept
	{
		union { FILETIME _ft; std::int64_t _tp; };
		return (_ft = ft, file_clock::time_point(_tp));
	}

	template<typename T>
	struct malloca_deleter { void operator()(T *p) const noexcept { _freea(p); } };
	template<typename T>
	using malloca_handle = std::unique_ptr<T, malloca_deleter<T>>;

	template<typename T>
	inline constexpr result<malloca_handle<T>> make_malloca_handle(void *p, std::size_t n) noexcept
	{
		if (p == nullptr && n)
			return std::make_error_code(std::errc::not_enough_memory);
		else
			return malloca_handle<T>(static_cast<T *>(p));
	}

	struct rtl_buffer
	{
		PUCHAR buff;
		PUCHAR static_buff;
		SIZE_T size;
		SIZE_T static_size;
		SIZE_T _reserved0;
		PVOID _reserved1;
	};
	struct unicode_string_buffer
	{
		unicode_string string;
		rtl_buffer buffer;
		UCHAR min_static_buff[sizeof(WCHAR)];
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

	using RtlNtStatusToDosError_t = ULONG (ROD_NTAPI *)(_In_ ntstatus status);
	using RtlIsDosDeviceName_Ustr_t = ULONG (ROD_NTAPI *)(_In_ const unicode_string *ustr);
	using RtlNtPathNameToDosPathName_t = ULONG (ROD_NTAPI *)(_In_ ULONG flags, _Inout_ unicode_string_buffer *path, _Out_ ULONG *type, PULONG);
	using RtlDosPathNameToNtPathName_U_t = bool (ROD_NTAPI *)(_In_ const wchar_t *dos_name, _Out_ unicode_string *nt_name, _Out_ const wchar_t **part, _Out_ rtl_relative_name_u *relative);

	struct io_status_block
	{
		union
		{
			ULONG status = -1;
			LONG_PTR ptr;
		};
		ULONG_PTR info = {};
	};
	struct io_completion_info
	{
		ULONG_PTR key_context = {};
		ULONG_PTR apc_context = {};
		io_status_block status;
	};

	struct object_attributes
	{
		ULONG length;
		void *root_dir;
		unicode_string *name;
		ULONG attr;
		void *security_desc;
		void *security_qos;
	};

	enum disposition : ULONG
	{
		file_supersede = 0,
		file_open,
		file_create,
		file_open_if,
		file_overwrite,
		file_overwrite_if,
	};

	using io_apc_routine = void (ROD_NTAPI *)(_In_ ULONG_PTR apc_ctx, _In_ io_status_block *iosb, _In_ ULONG);

	using NtReadFile_t = ntstatus (ROD_NTAPI *)(_In_ void *file, _In_opt_ void *evt, _In_opt_ io_apc_routine apc_func, _In_opt_ ULONG_PTR apc_ctx, _Out_ io_status_block *iosb, _Out_ void *buff,
	                                            _In_ ULONG len, _In_opt_ LARGE_INTEGER *off, _In_opt_ ULONG *key);
	using NtWriteFile_t = ntstatus (ROD_NTAPI *)(_In_ void *file, _In_opt_ void *evt, _In_opt_ io_apc_routine apc_func, _In_opt_ ULONG_PTR apc_ctx, _Out_ io_status_block *iosb, _In_ void *buff,
	                                             _In_ ULONG len, _In_opt_ LARGE_INTEGER *off, _In_opt_ ULONG *key);

	using NtOpenFile_t = ntstatus (ROD_NTAPI *)(_Out_ void **file, _In_ ULONG access, _In_ const object_attributes *obj, _Out_ io_status_block *iosb, _In_ ULONG share, _In_ ULONG opts);
	using NtCreateFile_t = ntstatus (ROD_NTAPI *)(_Out_ void **file, _In_ ULONG access, _In_ const object_attributes *obj, _Out_ io_status_block *iosb, _In_opt_ const LARGE_INTEGER *size,
	                                              _In_ ULONG file_attr, _In_ ULONG share, _In_ disposition disp, _In_ ULONG opts, _In_opt_ void *buff, _In_ ULONG len);

	enum section_info_type
	{
		SectionBasicInformation,
		SectionImageInformation
	};
	enum object_info_type
	{
		ObjectBasicInformation = 0,
		ObjectNameInformation,
		ObjectTypeInformation2
	};
	enum file_info_type
	{
		FileDirectoryInformation = 1,
		FileFullDirectoryInformation,
		FileBothDirectoryInformation,
		FileBasicInformation,
		FileStandardInformation,
		FileInternalInformation,
		FileEaInformation,
		FileAccessInformation,
		FileNameInformation,
		FileRenameInformation,
		FileLinkInformation,
		FileNamesInformation,
		FileDispositionInformation,
		FilePositionInformation,
		FileFullEaInformation,
		FileModeInformation,
		FileAlignmentInformation,
		FileAllInformation,
		FileAllocationInformation,
		FileEndOfFileInformation,
		FileAlternateNameInformation,
		FileStreamInformation,
		FilePipeInformation,
		FilePipeLocalInformation,
		FilePipeRemoteInformation,
		FileMailslotQueryInformation,
		FileMailslotSetInformation,
		FileCompressionInformation,
		FileObjectIdInformation,
		FileCompletionInformation,
		FileMoveClusterInformation,
		FileQuotaInformation,
		FileReparsePointInformation,
		FileNetworkOpenInformation,
		FileAttributeTagInformation,
		FileTrackingInformation,
		FileIdBothDirectoryInformation,
		FileIdFullDirectoryInformation,
		FileValidDataLengthInformation,
		FileShortNameInformation,
		FileIoCompletionNotificationInformation,
		FileIoStatusBlockRangeInformation,
		FileIoPriorityHintInformation,
		FileSfioReserveInformation,
		FileSfioVolumeInformation,
		FileHardLinkInformation,
		FileProcessIdsUsingFileInformation,
		FileNormalizedNameInformation,
		FileNetworkPhysicalNameInformation,
		FileIdGlobalTxDirectoryInformation,
		FileIsRemoteDeviceInformation,
		FileUnusedInformation,
		FileNumaNodeInformation,
		FileStandardLinkInformation,
		FileRemoteProtocolInformation,
		FileRenameInformationBypassAccessCheck,
		FileLinkInformationBypassAccessCheck,
		FileVolumeNameInformation,
		FileIdInformation,
		FileIdExtdDirectoryInformation,
		FileReplaceCompletionInformation,
		FileHardLinkFullIdInformation,
		FileIdExtdBothDirectoryInformation,
		FileDispositionInformationEx,
		FileRenameInformationEx,
		FileRenameInformationExBypassAccessCheck,
		FileDesiredStorageClassInformation,
		FileStatInformation,
		FileMemoryPartitionInformation,
		FileStatLxInformation,
		FileCaseSensitiveInformation,
		FileLinkInformationEx,
		FileLinkInformationExBypassAccessCheck,
		FileStorageReserveIdInformation,
		FileCaseSensitiveInformationForceAccessCheck,
	};
	enum fs_info_type
	{
		FileFsVolumeInformation = 1,
		FileFsLabelInformation,
		FileFsSizeInformation,
		FileFsDeviceInformation,
		FileFsAttributeInformation,
		FileFsControlInformation,
		FileFsFullSizeInformation,
		FileFsObjectIdInformation,
		FileFsDriverPathInformation,
		FileFsVolumeFlagsInformation,
		FileFsSectorSizeInformation,
		FileFsDataCopyInformation,
		FileFsMetadataSizeInformation,
		FileFsFullSizeInformationEx,
	};

	struct reparse_data_buffer
	{
		ULONG  reparse_tag;
		USHORT data_size;
		USHORT _reserved;
		union
		{
			struct
			{
				USHORT subs_name_off;
				USHORT subs_name_len;
				USHORT print_name_off;
				USHORT print_name_len;
				ULONG flags;
				WCHAR path[1];
			} symlink;
			struct
			{
				USHORT subs_name_off;
				USHORT subs_name_len;
				USHORT print_name_off;
				USHORT print_name_len;
				WCHAR path[1];
			} mount_point;
			struct
			{
				UCHAR data[1];
			} generic;
		};
	};

	struct section_basic_information
	{
		void *base_addr;
		ULONG attributes;
		LONGLONG max_size;
	};
	struct file_basic_information
	{
		FILETIME btime;
		FILETIME atime;
		FILETIME mtime;
		FILETIME ctime;
		ULONG attributes;
	};

	struct file_objectid_information
	{
		LONGLONG file_ref;
		UCHAR object_id[16];
		union
		{
			struct
			{
				UCHAR birth_volume_id[16];
				UCHAR birth_object_id[16];
				UCHAR domain_id[16];
			};
			UCHAR extended_info[48];
		};
	};
	struct file_id_information
	{
		ULONGLONG volume_id;
		UCHAR file_id[16];
	};

	struct file_case_sensitive_information { ULONG flags; };
	struct file_disposition_information_ex { ULONG flags; };
	struct file_disposition_information { BOOLEAN del; };

	struct file_standard_information
	{
		LONGLONG allocation;
		LONGLONG endpos;
		ULONG nlink;
		BOOLEAN delete_pending;
		BOOLEAN directory;
	};
	struct file_rename_information
	{
		ULONG flags;
		HANDLE root_dir;
		ULONG name_len;
		WCHAR name[1];
	};
	struct file_name_information
	{
		ULONG FileNameLength;
		WCHAR FileName[1];
	};
	struct file_link_information
	{
		ULONG flags;
		HANDLE root_dir;
		ULONG name_len;
		WCHAR name[1];
	};
	union file_ea_information
	{
		ULONG reparse_tag;
		ULONG ea_size;
	};

	struct file_internal_information { LONGLONG file_id; };
	using file_position_information = LONGLONG;
	using file_access_information = ACCESS_MASK;
	using file_alignment_information = ULONG;
	using file_mode_information = ULONG;

	struct file_fs_sector_size_information
	{
		ULONG logical_bytes;
		ULONG physical_bytes_atom;
		ULONG physical_bytes_perf;
		ULONG fs_physical_bytes_atom;
		ULONG flags;
		ULONG offset_sector;
		ULONG offset_partition;
	};
	struct file_fs_attribute_information
	{
		ULONG attributes;
		LONG max_component_size;
		ULONG name_len;
		WCHAR name[1];
	};
	struct file_fs_full_size_information
	{
		LONGLONG blk_count;
		LONGLONG blk_avail;
		LONGLONG blk_free;
		ULONG blk_sectors;
		ULONG sector_size;
	};
	struct file_fs_objectid_information
	{
		UCHAR obj_id[16];
		UCHAR ext_info[48];
	};

	struct file_id_full_dir_information
	{
		ULONG next_off;
		ULONG file_idx;
		FILETIME btime;
		FILETIME atime;
		FILETIME mtime;
		FILETIME ctime;
		LONGLONG eof;
		LONGLONG alloc_size;
		ULONG attributes;
		ULONG name_len;
		union
		{
			ULONG ea_size;
			ULONG reparse_tag;
		};
		LONGLONG file_id;
		WCHAR name[1];
	};
	struct file_directory_information
	{
		ULONG next_off;
		ULONG file_idx;
		FILETIME btime;
		FILETIME atime;
		FILETIME mtime;
		FILETIME ctime;
		LONGLONG endpos;
		LONGLONG allocation;
		ULONG attributes;
		ULONG name_len;
		WCHAR name[1];
	};
	struct file_names_information
	{
		ULONG next_off;
		ULONG file_idx;
		ULONG name_len;
		WCHAR name[1];
	};

	struct file_stat_information
	{
		LONGLONG file_id;
		FILETIME btime;
		FILETIME atime;
		FILETIME mtime;
		FILETIME ctime;
		LONGLONG allocation;
		LONGLONG endpos;
		ULONG attributes;
		ULONG reparse_tag;
		ULONG nlink;
		ACCESS_MASK access;
	};
	struct file_all_information
	{
		file_basic_information basic_info;
		file_standard_information standard_info;
		file_internal_information internal_info;
		file_ea_information ea_info;
		file_access_information access_info;
		file_position_information position_info;
		file_mode_information mode_info;
		file_alignment_information alignment_info;
		file_name_information name_info;
	};

	using NtQueryAttributesFile_t = ntstatus (ROD_NTAPI *)(_In_ const object_attributes *obj, _Out_ file_basic_information *info);
	using NtQueryInformationByName_t = ntstatus (ROD_NTAPI *)(_In_ const object_attributes *obj, _Out_ io_status_block *iosb, _Out_ void *info, _In_ ULONG len, _In_ file_info_type type);

	using NtSetInformationFile_t = ntstatus (ROD_NTAPI *)(_In_ void *file, _Out_ io_status_block *iosb, _In_ void *info, _In_ ULONG len, _In_ file_info_type type);
	using NtQueryDirectoryFile_t = ntstatus (ROD_NTAPI *)(_In_ void *file, _In_opt_ void *event, _In_opt_ io_apc_routine apc_func, _In_opt_ ULONG_PTR apc_ctx, _Out_ io_status_block *iosb, _Out_ void *info, _In_ ULONG len, _In_ file_info_type type, _In_ BOOLEAN single, _In_opt_ unicode_string *name, _In_ BOOLEAN restart);
	using NtQueryInformationFile_t = ntstatus (ROD_NTAPI *)(_In_ void *file, _Out_ io_status_block *iosb, _Out_ void *info, _In_ ULONG len, _In_ file_info_type type);
	using NtQueryVolumeInformationFile_t = ntstatus (ROD_NTAPI *)(_In_ void *file, _Out_ io_status_block *iosb, _Out_ void *info, _In_ ULONG len, _In_ fs_info_type type);

	using NtWaitForSingleObject_t = ntstatus (ROD_NTAPI *)(_In_ void *hnd, _In_ bool alert, _In_ const FILETIME *timeout);
	using NtCancelIoFileEx_t = ntstatus (ROD_NTAPI *)(_In_ void *file, _Out_ io_status_block *req, _Out_ io_status_block *iosb);
	using NtSetIoCompletion_t = ntstatus (ROD_NTAPI *)(_In_ void *hnd, _In_ ULONG key_ctx, _In_ ULONG_PTR apc_ctx, _In_ long status, _In_ ULONG info);
	using NtRemoveIoCompletionEx_t = ntstatus (ROD_NTAPI *)(_In_ void *hnd, _Out_writes_to_(count, *removed) io_completion_info *completion_info, _In_ ULONG count, _Out_ ULONG *removed, _In_opt_ LARGE_INTEGER *timeout, _In_ bool alert);

	inline constexpr const wchar_t *bcrypt_primitive_provider = L"Microsoft Primitive Provider";
	inline constexpr const wchar_t *bcrypt_platform_crypto_provider = L"Microsoft Platform Crypto Provider";

	using BCryptOpenAlgorithmProvider_t = ntstatus (ROD_NTAPI *)(_Out_ void **hnd, _In_ const wchar_t *algo, _In_opt_ const wchar_t *impl, _In_ ULONG flags);
	using BCryptGenRandom_t = ntstatus (ROD_NTAPI *)(_In_opt_ void *hnd, _Out_writes_bytes_(n) void *buff, _In_ ULONG size, _In_ ULONG flags);
	using BCryptCloseAlgorithmProvider_t = ntstatus (ROD_NTAPI *)(_Inout_ void *hnd, _In_ ULONG flags);

	inline constexpr auto heapalloc_free = [](auto *p) { ::HeapFree(::GetProcessHeap(), 0, p); };
	template<typename T>
	using heapalloc_ptr = std::unique_ptr<T, decltype(heapalloc_free)>;

	struct ntapi
	{
		[[nodiscard]] static const result<ntapi> &instance() noexcept;

		~ntapi()
		{
			::FreeLibrary(static_cast<HMODULE>(ntdll));
			::FreeLibrary(static_cast<HMODULE>(bcrypt));
		}

		ntstatus cancel_io(void *handle, io_status_block *iosb) const noexcept;
		ntstatus wait_io(void *handle, io_status_block *iosb, const file_timeout &to = file_timeout()) const noexcept;

		result<heapalloc_ptr<wchar_t>> dos_path_to_nt_path(unicode_string &upath, bool passthrough) const noexcept;
		result<heapalloc_ptr<wchar_t>> canonize_win32_path(unicode_string &upath, bool passthrough) const noexcept;

		void *ntdll;
		void *bcrypt;

		RtlNtStatusToDosError_t RtlNtStatusToDosError;
		RtlIsDosDeviceName_Ustr_t RtlIsDosDeviceName_Ustr;
		RtlNtPathNameToDosPathName_t RtlNtPathNameToDosPathName;
		RtlDosPathNameToNtPathName_U_t RtlDosPathNameToNtPathName_U;

		NtReadFile_t NtReadFile;
		NtWriteFile_t NtWriteFile;

		NtOpenFile_t NtOpenFile;
		NtCreateFile_t NtCreateFile;

		NtQueryAttributesFile_t NtQueryAttributesFile;
		NtQueryInformationByName_t NtQueryInformationByName;

		NtSetInformationFile_t NtSetInformationFile;
		NtQueryDirectoryFile_t NtQueryDirectoryFile;
		NtQueryInformationFile_t NtQueryInformationFile;
		NtQueryVolumeInformationFile_t NtQueryVolumeInformationFile;

		NtCancelIoFileEx_t NtCancelIoFileEx;
		NtSetIoCompletion_t NtSetIoCompletion;
		NtWaitForSingleObject_t NtWaitForSingleObject;
		NtRemoveIoCompletionEx_t NtRemoveIoCompletionEx;

		BCryptGenRandom_t BCryptGenRandom;
		BCryptOpenAlgorithmProvider_t BCryptOpenAlgorithmProvider;
		BCryptCloseAlgorithmProvider_t BCryptCloseAlgorithmProvider;
	};

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

	const std::error_category &status_category() noexcept;
	inline static std::error_code dos_error_code(ULONG err) noexcept { return {int(err), std::system_category()}; }
	inline static std::error_code status_error_code(ntstatus status) noexcept { return {int(status), status_category()}; }
}

#define ROD_MAKE_BUFFER(T, n) (rod::_win32::make_malloca_handle<T>(_malloca(n), n))
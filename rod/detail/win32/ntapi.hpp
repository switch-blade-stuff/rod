/*
 * Created by switchblade on 2023-07-04.
 */

#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <combaseapi.h>
#include <winioctl.h>
#include <windows.h>
#include <ShlObj.h>

#if defined(_MSC_VER) || __has_include(<sal.h>)
#include <sal.h>
#else
#ifndef _In_
#define _In_
#endif
#ifndef _Inout_
#define _Inout_
#endif
#ifndef _In_opt_
#define _In_opt_
#endif
#ifndef _Inout_opt_
#define _Inout_opt_
#endif
#ifndef _Out_
#define _Out_
#endif
#ifndef _Out_writes_to_
#define _Out_writes_to_(A, B)
#endif
#ifndef _Out_writes_bytes_
#define _Out_writes_bytes_(A)
#endif
#endif

#if defined(NTAPI)
#define ROD_NTAPI NTAPI
#elif defined(_MSC_VER)
#define ROD_NTAPI __stdcall
#else
#define ROD_NTAPI __attribute__((__stdcall__))
#endif

#include "../fs_handle_base.hpp"
#include "../file_clock.hpp"

namespace rod::_win32
{
	using ntstatus = ULONG;

	inline constexpr ntstatus error_status_max = 0xffff'ffff;
	inline constexpr ntstatus warning_status_max = 0x8fff'ffff;
	inline constexpr ntstatus message_status_max = 0x7fff'ffff;
	inline constexpr ntstatus success_status_max = 0x3fff'ffff;

	[[nodiscard]] inline constexpr bool is_status_failure(ntstatus st) noexcept { return st > message_status_max; }

	inline static _handle::extent_type get_block_size() noexcept
	{
		static const auto result = std::max<_handle::extent_type>(4096, get_page_size());
		return result;
	}
	inline static _handle::extent_type page_align(_handle::extent_type n) noexcept
	{
		const auto page_size = get_page_size();
		const auto rem = n % page_size;
		return n + (rem ? page_size - rem : 0);
	}

	enum disposition : ULONG
	{
		file_supersede = 0,
		file_open,
		file_create,
		file_open_if,
		file_overwrite,
		file_overwrite_if,
	};

	inline constexpr auto mode_to_disp(fs::open_mode mode) noexcept
	{
		switch (mode)
		{
		case fs::open_mode::always: return file_open_if;
		case fs::open_mode::create: return file_create;
		case fs::open_mode::existing: return file_open;
		case fs::open_mode::truncate: return file_overwrite;
		case fs::open_mode::supersede: return file_supersede;
		}
		return disposition();
	}
	inline constexpr auto flags_to_access(fs::file_flags flags) noexcept
	{
		DWORD access = DELETE | SYNCHRONIZE;
		if (bool(flags & fs::file_flags::read))
			access |= STANDARD_RIGHTS_READ;
		if (bool(flags & fs::file_flags::write))
			access |= STANDARD_RIGHTS_WRITE;
		if (bool(flags & fs::file_flags::attr_read))
			access |= FILE_READ_ATTRIBUTES | FILE_READ_EA;
		if (bool(flags & fs::file_flags::attr_write))
			access |= FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA;
		if (bool(flags & fs::file_flags::data_read))
			access |= FILE_READ_DATA;
		if (bool(flags & fs::file_flags::data_write))
			access |= FILE_WRITE_DATA;
		if (bool(flags & fs::file_flags::append))
			access |= FILE_APPEND_DATA;
		return access;
	}

	inline constexpr auto flags_to_opts(fs::file_flags flags) noexcept
	{
		DWORD opts = 0;
		if (!bool(flags & fs::file_flags::non_blocking))
			opts |= 0x20; /*FILE_SYNCHRONOUS_IO_NONALERT*/
		if (bool(flags & fs::file_flags::unlink_on_close))
			opts |= 0x1000; /*FILE_DELETE_ON_CLOSE*/
		return opts;
	}
	inline constexpr auto caching_to_opts(fs::file_caching caching) noexcept
	{
		DWORD opts = 0;
		if (!bool(caching & fs::file_caching::write))
			opts |= 2; /*FILE_WRITE_THROUGH*/
		if (!bool(caching & (fs::file_caching::read | fs::file_caching::write)))
			opts |= 8; /*FILE_NO_INTERMEDIATE_BUFFERING*/
		if (bool(caching & fs::file_caching::avoid_precache))
			opts |= 0x800; /*FILE_RANDOM_ACCESS*/
		if (bool(caching & fs::file_caching::force_precache))
			opts |= 4; /*FILE_SEQUENTIAL_ONLY*/
		return opts;
	}

	inline constexpr auto perm_to_attr(fs::file_perm perm) noexcept
	{
		DWORD attr = 0;
		if (!bool(perm & (fs::file_perm::write | fs::file_perm::exec)))
			attr = FILE_ATTRIBUTE_READONLY;
		return attr;
	}
	inline constexpr auto flags_to_attr(fs::file_flags flags) noexcept
	{
		DWORD attr = 0;
		return attr;
	}
	inline constexpr auto caching_to_attr(fs::file_caching caching) noexcept
	{
		DWORD attr = 0;
		if (bool(caching & fs::file_caching::temporary))
			attr = FILE_ATTRIBUTE_TEMPORARY;
		return attr;
	}

	inline constexpr auto make_handle_opts(fs::file_flags flags, fs::file_caching caching) noexcept
	{
		return flags_to_opts(flags) | caching_to_opts(caching);
	}
	inline constexpr auto make_handle_attr(fs::file_flags flags, fs::file_caching caching, fs::file_perm perm) noexcept
	{
		const auto attr = flags_to_attr(flags) | caching_to_attr(caching) | perm_to_attr(perm);
		return attr ? attr : FILE_ATTRIBUTE_NORMAL;
	}

	inline constexpr auto attr_to_type(ULONG file_attr, ULONG reparse_tag = 0) noexcept
	{
		constexpr auto attr_regular = FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | FILE_ATTRIBUTE_SPARSE_FILE | FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_ENCRYPTED | FILE_ATTRIBUTE_TEMPORARY |
		                              FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_OFFLINE | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_SYSTEM;

		if (file_attr & FILE_ATTRIBUTE_REPARSE_POINT && (reparse_tag == IO_REPARSE_TAG_MOUNT_POINT || reparse_tag == IO_REPARSE_TAG_SYMLINK))
			return fs::file_type::symlink;
		if (file_attr & FILE_ATTRIBUTE_DIRECTORY)
			return fs::file_type::directory;
		else if (file_attr & attr_regular)
			return fs::file_type::regular;
		else
			return fs::file_type::unknown;
	}

	struct unicode_string
	{
		USHORT size;
		USHORT max;
		PWCHAR buff;
	};
	struct filetime
	{
		constexpr filetime() noexcept : ft() {}
		constexpr filetime(FILETIME ft) noexcept : ft(ft) {}
		constexpr filetime(std::int64_t tp) noexcept : tp(tp) {}
		constexpr filetime(LARGE_INTEGER tp) noexcept : tp(tp.QuadPart) {}

		constexpr filetime(const filetime &other) noexcept : ft(other.ft) {}
		constexpr filetime(filetime &&other) noexcept : ft(std::move(other.ft)) {}

		constexpr filetime &operator=(const filetime &other) noexcept { return (ft = other.ft, *this); }
		constexpr filetime &operator=(filetime &&other) noexcept { return (ft = std::move(other.ft), *this); }

		constexpr operator FILETIME() const noexcept { return ft; }
		constexpr operator std::int64_t() const noexcept { return tp; }
		constexpr operator LARGE_INTEGER() const noexcept { return {.QuadPart = tp}; }

		union { FILETIME ft; std::int64_t tp; };
	};

	template<bool Term = true>
	inline constexpr auto render_as_wchar(fs::path_view_component path) noexcept -> result<fs::path_view_component::rendered_path<Term, wchar_t>>
	{
		if (path.empty())
			return {};

		try { return fs::path_view_component::rendered_path<Term, wchar_t>(path); }
		catch (...) { return _detail::current_error(); }
	}
	template<std::ranges::contiguous_range Rng>
	inline constexpr auto make_ustring(Rng path) noexcept
	{
		unicode_string upath;
		upath.max = (upath.size = USHORT(std::ranges::size(path) * sizeof(wchar_t))) + sizeof(wchar_t);
		upath.buff = const_cast<wchar_t *>(std::ranges::data(path));
		return upath;
	}

	inline static filetime tp_to_filetime(typename fs::file_clock::time_point tp) noexcept { return tp.time_since_epoch().count(); }
	inline static typename fs::file_clock::time_point filetime_to_tp(filetime ft) noexcept { return fs::file_clock::time_point(ft); }

#ifdef ROD_FORCE_STACK_BUFFERS
	template<typename T>
	struct alloca_deleter { void operator()(T *) const noexcept {} };
	template<typename T>
	using alloca_handle = std::unique_ptr<T, alloca_deleter<T>>;
#else
	template<typename T>
	struct malloca_deleter { void operator()(T *p) const noexcept { _freea(p); } };
	template<typename T>
	using malloca_handle = std::unique_ptr<T, malloca_deleter<T>>;
#endif

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
	using RtlIsDosDeviceName_U_t = ULONG (ROD_NTAPI *)(_In_ const wchar_t *str);
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

	using io_apc_routine = void (ROD_NTAPI *)(_In_ ULONG_PTR apc_ctx, _In_ io_status_block *iosb, _In_ ULONG);

	struct object_attributes
	{
		ULONG length;
		void *root_dir;
		unicode_string *name;
		ULONG attr;
		void *security_desc;
		void *security_qos;
	};

	using NtOpenFile_t = ntstatus (ROD_NTAPI *)(_Out_ void **file, _In_ ULONG access, _In_ const object_attributes *obj, _Out_ io_status_block *iosb, _In_ ULONG share, _In_ ULONG opts);
	using NtCreateFile_t = ntstatus (ROD_NTAPI *)(_Out_ void **file, _In_ ULONG access, _In_ const object_attributes *obj, _Out_ io_status_block *iosb, _In_opt_ const LARGE_INTEGER *size,
	                                              _In_ ULONG file_attr, _In_ ULONG share, _In_ disposition disp, _In_ ULONG opts, _In_opt_ const void *buff, _In_ ULONG len);

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

	using NtReadFile_t = ntstatus (ROD_NTAPI *)(_In_ void *file, _In_opt_ void *evt, _In_opt_ io_apc_routine apc_func, _In_opt_ ULONG_PTR apc_ctx, _Out_ io_status_block *iosb, _Out_ void *buff,
	                                            _In_ ULONG len, _In_opt_ const LARGE_INTEGER *off, _In_opt_ ULONG *key);
	using NtWriteFile_t = ntstatus (ROD_NTAPI *)(_In_ void *file, _In_opt_ void *evt, _In_opt_ io_apc_routine apc_func, _In_opt_ ULONG_PTR apc_ctx, _Out_ io_status_block *iosb, _In_ const void *buff,
	                                             _In_ ULONG len, _In_opt_ const LARGE_INTEGER *off, _In_opt_ ULONG *key);
	using NtQueryDirectoryFile_t = ntstatus (ROD_NTAPI *)(_In_ void *file, _In_opt_ void *event, _In_opt_ io_apc_routine apc_func, _In_opt_ ULONG_PTR apc_ctx, _Out_ io_status_block *iosb,
	                                                      _Out_ void *info, _In_ ULONG len, _In_ file_info_type type, _In_ bool single, const _In_opt_ unicode_string *name, _In_ bool restart);

	using NtFlushBuffersFile_t = ntstatus (ROD_NTAPI *)(_In_ void *file, _Out_ io_status_block *iosb);
	using NtFlushBuffersFileEx_t = ntstatus (ROD_NTAPI *)(_In_ void *file, _In_ ULONG flags, _In_ void *params, _In_ ULONG params_size, _Out_ io_status_block *iosb);

	struct reparse_data_buffer
	{
		ULONG  reparse_tag;
		USHORT data_size;
		USHORT _reserved;
		union
		{
			struct
			{
				USHORT subst_name_off;
				USHORT subst_name_len;
				USHORT print_name_off;
				USHORT print_name_len;
				ULONG flags;
				WCHAR path[1];
			} symlink;
			struct
			{
				USHORT subst_name_off;
				USHORT subst_name_len;
				USHORT print_name_off;
				USHORT print_name_len;
				WCHAR path[1];
			} junction;
			struct
			{
				UCHAR data[1];
			} generic;
		};
	};

	struct file_basic_information
	{
		filetime btime;
		filetime atime;
		filetime mtime;
		filetime ctime;
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
		filetime btime;
		filetime atime;
		filetime mtime;
		filetime ctime;
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
		filetime btime;
		filetime atime;
		filetime mtime;
		filetime ctime;
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
		filetime btime;
		filetime atime;
		filetime mtime;
		filetime ctime;
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
	using NtQueryInformationFile_t = ntstatus (ROD_NTAPI *)(_In_ void *file, _Out_ io_status_block *iosb, _Out_ void *info, _In_ ULONG len, _In_ file_info_type type);
	using NtQueryVolumeInformationFile_t = ntstatus (ROD_NTAPI *)(_In_ void *file, _Out_ io_status_block *iosb, _Out_ void *info, _In_ ULONG len, _In_ fs_info_type type);

	using NtWaitForSingleObject_t = ntstatus (ROD_NTAPI *)(_In_ void *hnd, _In_ bool alert, _In_ const filetime *timeout);
	using NtCancelIoFileEx_t = ntstatus (ROD_NTAPI *)(_In_ void *file, _Out_ io_status_block *req, _Out_ io_status_block *iosb);
	using NtSetIoCompletion_t = ntstatus (ROD_NTAPI *)(_In_ void *hnd, _In_ ULONG key_ctx, _In_ ULONG_PTR apc_ctx, _In_ long status, _In_ ULONG info);
	using NtRemoveIoCompletionEx_t = ntstatus (ROD_NTAPI *)(_In_ void *hnd, _Out_writes_to_(count, *removed) io_completion_info *completion_info, _In_ ULONG count, _Out_ ULONG *removed, _In_opt_ const LARGE_INTEGER *timeout, _In_ bool alert);

	enum memory_information_type
	{
		MemoryBasicInformation,
		MemoryWorkingSetInformation,
		MemorySectionName,
		MemoryRegionInformation,
		MemoryWorkingSetExInformation,
		MemorySharedCommitInformation,
	};
	enum section_info_type
	{
		SectionBasicInformation,
		SectionImageInformation
	};
	enum section_inherit
	{
		ViewShare = 1,
		ViewUnmap = 2
	};

	struct memory_basic_information
	{
		void *addr;
		void *alloc_addr;
		ULONG alloc_prot;
		USHORT partition_id;
		std::size_t region_size;
		ULONG state;
		ULONG prot;
		ULONG type;
	};
	struct section_basic_information
	{
		void *base_addr;
		ULONG attributes;
		LONGLONG max_size;
	};
	struct memory_region_information
	{
		void *base;
		ULONG prot;
		ULONG type; /*MEM_IMAGE, MEM_MAPPED, MEM_PRIVATE*/
		std::size_t size;
	};
	struct memory_working_set_ex_block
	{
		union
		{
			struct valid_t
			{
				ULONG_PTR valid : 1;
				ULONG_PTR sharecount : 3;
				ULONG_PTR win32prot : 11;
				ULONG_PTR shared : 1;
				ULONG_PTR node : 6;
				ULONG_PTR locked : 1;
				ULONG_PTR largepage : 1;
				ULONG_PTR priority : 3;  // new
				ULONG_PTR _reserved0 : 3;
				ULONG_PTR sharedoriginal : 1;  // new
				ULONG_PTR bad : 1;
#ifdef _WIN64
				ULONG_PTR _reserved1 : 32;
#endif
			} valid;
			struct invalid_t
			{
				ULONG_PTR valid : 1;
				ULONG_PTR _reserved0 : 14;
				ULONG_PTR shared : 1;
				ULONG_PTR _reserved1 : 5;
				ULONG_PTR pagetable : 1;     // new
				ULONG_PTR location : 2;      // new
				ULONG_PTR priority : 3;      // new
				ULONG_PTR modifiedlist : 1;  // new
				ULONG_PTR _reserved2 : 2;
				ULONG_PTR sharedoriginal : 1;  // new
				ULONG_PTR bad : 1;
#ifdef _WIN64
				ULONG_PTR _reserved3 : 32;
#endif
			} invalid;
		};

	};
	struct memory_shared_commit_information
	{
		std::size_t commit;
	};
	struct memory_working_set_ex_information
	{
		void *addr;
		union
		{
			memory_working_set_ex_block attrs;
			ULONG_PTR value;
		};
	};

	using NtOpenSection_t = ntstatus (ROD_NTAPI *)(_Out_ void **hnd, _In_ DWORD access, _In_opt_ const object_attributes *attr);
	using NtCreateSection_t = ntstatus (ROD_NTAPI *)(_Out_ void **hnd, _In_ DWORD access, _In_opt_ const object_attributes *attr, const _In_opt_ LONGLONG *max_size, _In_ ULONG page_prot, _In_ ULONG alloc_attr, _In_ void *file);

	using NtExtendSection_t = ntstatus (ROD_NTAPI *)(_In_ void *hnd, _Inout_ LONGLONG *new_size);
	using NtQuerySection_t = ntstatus (ROD_NTAPI *)(_In_ void *hnd, _In_ section_info_type type, _Out_ void *info, _In_ ULONG len, _Out_opt_ ULONG *out_len);

	using NtMapViewOfSection_t = ntstatus (ROD_NTAPI *)(_In_ void *sec_hnd, _In_ void *proc_hnd, _Inout_ void **base_addr, _In_ ULONG_PTR zero, _In_ SIZE_T commit, _Inout_opt_ LONGLONG *sec_off, _Inout_ SIZE_T *view_size,
														_In_ section_inherit inherit, _In_ ULONG type, _In_ ULONG prot);
	using NtUnmapViewOfSection_t = ntstatus (ROD_NTAPI *)(_In_ void *proc, _In_opt_ void *addr);

	using NtFreeVirtualMemory_t = ntstatus (ROD_NTAPI *)(_In_ void *proc, _Inout_ void **addr, _Inout_ SIZE_T *size, _In_ ULONG op);
	using NtQueryVirtualMemory_t = ntstatus (ROD_NTAPI *)(_In_ void *proc, _In_ void *addr, _In_ memory_information_type type, _Out_ void *info, _In_ SIZE_T len, _Out_opt_ SIZE_T *out_len);

	using DiscardVirtualMemory_t = int (ROD_NTAPI *)(_In_ void *addr, _In_ SIZE_T size);
	using PrefetchVirtualMemory_t = int (ROD_NTAPI *)(_In_ void *proc, _In_ SIZE_T size, _In_ WIN32_MEMORY_RANGE_ENTRY *buff, _In_ ULONG flags);

	inline constexpr const wchar_t *bcrypt_primitive_provider = L"Microsoft Primitive Provider";
	inline constexpr const wchar_t *bcrypt_platform_crypto_provider = L"Microsoft Platform Crypto Provider";

	using BCryptOpenAlgorithmProvider_t = ntstatus (ROD_NTAPI *)(_Out_ void **hnd, _In_ const wchar_t *algo, _In_opt_ const wchar_t *impl, _In_ ULONG flags);
	using BCryptGenRandom_t = ntstatus (ROD_NTAPI *)(_In_opt_ void *hnd, _Out_writes_bytes_(n) void *buff, _In_ ULONG size, _In_ ULONG flags);
	using BCryptCloseAlgorithmProvider_t = ntstatus (ROD_NTAPI *)(_Inout_ void *hnd, _In_ ULONG flags);

	inline constexpr auto heapalloc_free = [](auto *p) { ::HeapFree(::GetProcessHeap(), 0, p); };
	template<typename T>
	using heapalloc_ptr = std::unique_ptr<T, decltype(heapalloc_free)>;

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

	inline static bool is_error_not_a_directory(std::error_code err) noexcept
	{
		static const auto cnds = std::array
		{
			/* DOS errors */
			std::error_condition(ERROR_DIRECTORY, std::system_category()),
			/* POSIX errors */
			std::make_error_condition(std::errc::not_a_directory),
		};
		return std::find(cnds.begin(), cnds.end(), err) != cnds.end();
	}
	inline static bool is_error_file_not_found(std::error_code err) noexcept
	{
		static const auto cnds = std::array
		{
			/* DOS errors */
			std::error_condition(ERROR_FILE_NOT_FOUND, std::system_category()),
			std::error_condition(ERROR_PATH_NOT_FOUND, std::system_category()),
			std::error_condition(ERROR_INVALID_NAME, std::system_category()),
			std::error_condition(ERROR_BAD_NETPATH, std::system_category()),
			/* POSIX errors */
			std::make_error_condition(std::errc::no_such_file_or_directory),
		};
		return std::find(cnds.begin(), cnds.end(), err) != cnds.end();
	}
	inline static bool is_error_access_denied(std::error_code err) noexcept
	{
		static const auto cnds = std::array
		{
			/* DOS errors */
			std::error_condition(ERROR_ACCESS_DENIED, std::system_category()),
			/* POSIX errors */
			std::make_error_condition(std::errc::permission_denied),
		};
		return std::find(cnds.begin(), cnds.end(), err) != cnds.end();
	}
	inline static bool is_error_file_exists(std::error_code err) noexcept
	{
		static const auto cnds = std::array
		{
			/* DOS errors */
			std::error_condition(ERROR_ALREADY_EXISTS, std::system_category()),
			/* POSIX errors */
			std::make_error_condition(std::errc::file_exists),
		};
		return std::find(cnds.begin(), cnds.end(), err) != cnds.end();
	}
	inline static bool is_error_not_empty(std::error_code err) noexcept
	{
		static const auto cnds = std::array
		{
			/* DOS errors */
			std::error_condition(ERROR_NOT_EMPTY, std::system_category()),
			/* POSIX errors */
			std::make_error_condition(std::errc::directory_not_empty),
		};
		return std::find(cnds.begin(), cnds.end(), err) != cnds.end();
	}

	result<bool> is_elevated() noexcept;

	struct ntapi
	{
		[[nodiscard]] static const result<ntapi> &instance() noexcept;

		~ntapi()
		{
			::FreeLibrary(static_cast<HMODULE>(ntdll));
			::FreeLibrary(static_cast<HMODULE>(bcrypt));
		}

		result<heapalloc_ptr<wchar_t>> dos_path_to_nt_path(unicode_string &upath, bool passthrough) const noexcept;
		result<heapalloc_ptr<wchar_t>> canonize_win32_path(unicode_string &upath, bool passthrough) const noexcept;

		result<void *> reopen_file(void *handle, io_status_block *iosb, ULONG access, ULONG share, ULONG opts, const fs::file_timeout &to = fs::file_timeout::infinite) const noexcept;
		result<void *> open_file(const object_attributes &obj, io_status_block *iosb, ULONG access, ULONG share, ULONG opts, const fs::file_timeout &to = fs::file_timeout::infinite) const noexcept;
		result<void *> create_file(const object_attributes &obj, io_status_block *iosb, ULONG access, ULONG attr, ULONG share, disposition disp, ULONG opts, const fs::file_timeout &to = fs::file_timeout::infinite) const noexcept;

		template<typename F>
		result<bool> query_directory(void *handle, std::span<std::byte> buff, unicode_string *filter, bool reset, const fs::file_timeout &to, F &&f) const noexcept
		{
			for (;;)
			{
				auto iosb = io_status_block();
				auto status = NtQueryDirectoryFile(handle, nullptr, nullptr, 0, &iosb, buff.data(), ULONG(buff.size()), FileIdFullDirectoryInformation, true, filter, reset);
				if (status == STATUS_PENDING)
					status = wait_io(handle, &iosb, to);
				if (status == 0x80000006 /*STATUS_NO_MORE_FILES*/)
					return {in_place_value, true};
				if (is_status_failure(status)) [[unlikely]]
					return {in_place_error, status_error_code(status)};

				const auto *full_info = reinterpret_cast<const file_id_full_dir_information *>(buff.data());
				const auto name = std::wstring_view(full_info->name, full_info->name_len / sizeof(wchar_t));
				reset = false;

				/* Skip directory wildcards. */
				if (name.size() >= 1 && name[0] == '.' && (name.size() == 1 || (name.size() == 2 && name[1] == '.')))
					continue;

				auto st = stat(nullptr);
				st.ino = full_info->file_id;
				st.type = attr_to_type(full_info->attributes, full_info->reparse_tag);
				st.atime = filetime_to_tp(full_info->atime);
				st.mtime = filetime_to_tp(full_info->mtime);
				st.ctime = filetime_to_tp(full_info->ctime);
				st.btime = filetime_to_tp(full_info->btime);
				st.size = full_info->eof;
				st.alloc = full_info->alloc_size;
				st.is_sparse = full_info->attributes & FILE_ATTRIBUTE_SPARSE_FILE;
				st.is_compressed = full_info->attributes & FILE_ATTRIBUTE_COMPRESSED;
				st.is_reparse_point = full_info->attributes & FILE_ATTRIBUTE_REPARSE_POINT;

				if (auto res = f(name, st); res.has_error()) [[unlikely]]
					return {in_place_error, std::move(res.error())};
				else if (!(*res))
					return {in_place_value, false};
			}
		}

		ntstatus link_file(void *handle, io_status_block *iosb, void *base, unicode_string &upath, bool replace, const fs::file_timeout &to) const noexcept;
		ntstatus relink_file(void *handle, io_status_block *iosb, void *base, unicode_string &upath, bool replace, const fs::file_timeout &to) const noexcept;
		ntstatus unlink_file(void *handle, io_status_block *iosb, bool mark_for_delete, const fs::file_timeout &to) const noexcept;

		ntstatus set_file_info(void *handle, io_status_block *iosb, void *data, std::size_t size, file_info_type type, const fs::file_timeout &to = fs::file_timeout::infinite) const noexcept;
		ntstatus get_file_info(void *handle, io_status_block *iosb, void *data, std::size_t size, file_info_type type, const fs::file_timeout &to = fs::file_timeout::infinite) const noexcept;

		template<typename T>
		ntstatus set_file_info(void *handle, io_status_block *iosb, T *data, file_info_type type, const fs::file_timeout &to = fs::file_timeout::infinite) const noexcept
		{
			return set_file_info(handle, iosb, data, sizeof(T), type, to);
		}
		template<typename T>
		ntstatus get_file_info(void *handle, io_status_block *iosb, T *data, file_info_type type, const fs::file_timeout &to = fs::file_timeout::infinite) const noexcept
		{
			return get_file_info(handle, iosb, data, sizeof(T), type, to);
		}

		ntstatus cancel_io(void *handle, io_status_block *iosb) const noexcept;
		ntstatus wait_io(void *handle, io_status_block *iosb, const fs::file_timeout &to = fs::file_timeout::infinite) const noexcept;

		template<typename F>
		result<void> apply_virtual_pages(std::byte *addr, std::size_t size, ULONG mask, F &&f) const noexcept
		{
			for (auto info = memory_basic_information(); size > 0;)
			{
				if (auto status = NtQueryVirtualMemory(::GetCurrentProcess(), addr, MemoryBasicInformation, &info, sizeof(info), nullptr); is_status_failure(status)) [[unlikely]]
					return status_error_code(status);

				if ((info.state & mask) != 0)
				{
					if (auto res = f(static_cast<std::byte *>(info.addr), std::min(std::size_t(info.region_size), size)); res.has_error()) [[unlikely]]
						return res.error();
				}

				addr += info.region_size;
				if (info.region_size < size)
					size -= info.region_size;
				else
					break;
			}
			return {};
		}

		ntstatus free_mapped_pages(std::byte *addr, std::size_t size) const noexcept;
		ntstatus free_virtual_pages(std::byte *addr, std::size_t size, ULONG op) const noexcept;

		void *ntdll;
		void *bcrypt;
		void *kernel32;

		RtlIsDosDeviceName_U_t RtlIsDosDeviceName_U;
		RtlNtStatusToDosError_t RtlNtStatusToDosError;
		RtlNtPathNameToDosPathName_t RtlNtPathNameToDosPathName;
		RtlDosPathNameToNtPathName_U_t RtlDosPathNameToNtPathName_U;

		NtOpenFile_t NtOpenFile;
		NtCreateFile_t NtCreateFile;

		NtReadFile_t NtReadFile;
		NtWriteFile_t NtWriteFile;
		NtQueryDirectoryFile_t NtQueryDirectoryFile;

		NtFlushBuffersFile_t NtFlushBuffersFile;
		NtFlushBuffersFileEx_t NtFlushBuffersFileEx;

		NtQueryAttributesFile_t NtQueryAttributesFile;
		NtQueryInformationByName_t NtQueryInformationByName;

		NtSetInformationFile_t NtSetInformationFile;
		NtQueryInformationFile_t NtQueryInformationFile;
		NtQueryVolumeInformationFile_t NtQueryVolumeInformationFile;

		NtCancelIoFileEx_t NtCancelIoFileEx;
		NtSetIoCompletion_t NtSetIoCompletion;
		NtWaitForSingleObject_t NtWaitForSingleObject;
		NtRemoveIoCompletionEx_t NtRemoveIoCompletionEx;

		NtOpenSection_t NtOpenSection;
		NtCreateSection_t NtCreateSection;

		NtQuerySection_t NtQuerySection;
		NtExtendSection_t NtExtendSection;

		NtFreeVirtualMemory_t NtFreeVirtualMemory;
		NtQueryVirtualMemory_t NtQueryVirtualMemory;

		NtMapViewOfSection_t NtMapViewOfSection;
		NtUnmapViewOfSection_t NtUnmapViewOfSection;

		BCryptGenRandom_t BCryptGenRandom;
		BCryptOpenAlgorithmProvider_t BCryptOpenAlgorithmProvider;
		BCryptCloseAlgorithmProvider_t BCryptCloseAlgorithmProvider;

		DiscardVirtualMemory_t DiscardVirtualMemory;
		PrefetchVirtualMemory_t PrefetchVirtualMemory;
	};
}

#ifdef ROD_FORCE_STACK_BUFFERS
#define ROD_MAKE_BUFFER(T, n) (rod::_win32::malloca_handle<T>(static_cast<T *>(alloca(n))))
#else
#define ROD_MAKE_BUFFER(T, n) (rod::_win32::malloca_handle<T>(static_cast<T *>(_malloca(n))))
#endif

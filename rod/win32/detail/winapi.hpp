/*
 * Created by switch_blade on 2023-07-04.
 */

#pragma once

#ifdef _WIN32

#include "../../detail/config.hpp"

#include <cstddef>
#include <cstdint>

namespace rod::detail
{
	struct winapi
	{
		/* Forward declare these to avoid including windows headers */
		struct unicode_string
		{
			unsigned short size;
			unsigned short max;
			wchar_t *buffer;
		};

		union large_integer
		{
			struct
			{
				unsigned long low;
				long high;
			} u;
			long long quad;
		};

		struct io_status_block
		{
			union
			{
				long status;
				void *pointer;
			};
			std::uintptr_t info;
		};

		using io_apc_routine = void (ROD_NTAPI *)(_In_ void *apc_context, _In_ io_status_block *iosb, _In_ unsigned long);

		struct object_attributes
		{
			unsigned long length;
			void *root_directory;
			unicode_string *name;
			unsigned long attrib;
			void *security_dsc;
			void *security_qos;
		};

		struct file_completion_information
		{
			void *port;
			void *key;
		};

		struct file_io_completion_information
		{
			void *key_context;
			void *apc_context;
			io_status_block status;
		};

		using NtReadFile_t = long (ROD_NTAPI *)(_In_ void *file, _In_opt_ void *evt, _In_opt_ io_apc_routine apc_func, _In_opt_ void *apc_ctx, _Out_ io_status_block *iosb, _Out_ void *buff, _In_ unsigned long len, _In_opt_ large_integer *off, _In_opt_ unsigned long *key);
		using NtWriteFile_t = long (ROD_NTAPI *)(_In_ void *file, _In_opt_ void *evt, _In_opt_ io_apc_routine apc_func, _In_opt_ void *apc_ctx, _Out_ io_status_block *iosb, _In_ const void *buff, _In_ unsigned long len, _In_opt_ large_integer *off, _In_opt_ unsigned long *key);

		using RtlNtStatusToDosError_t = unsigned long (ROD_NTAPI *)(_In_ long status);
		using NtCancelIoFileEx_t = long (ROD_NTAPI *)(_In_ void *file, _Out_ io_status_block *in_sb, _Out_ io_status_block *out_sb);
		using NtSetIoCompletion_t = long (ROD_NTAPI *)(_In_ void *hnd, _In_ unsigned long key_ctx, _In_ unsigned long apc_ctx, _In_ long status, _In_ unsigned long info);
		using NtRemoveIoCompletion_t = long (ROD_NTAPI *)(_In_ void *hnd, _Out_ void *key, _Out_ void *apc, _Out_ io_status_block *iosb, _In_opt_ large_integer *timeout);
		using NtRemoveIoCompletionEx_t = long (ROD_NTAPI *)(_In_ void *hnd, _Out_writes_to_(count, *removed) file_io_completion_information *completion_info, _In_ unsigned long count, _Out_ unsigned long *removed, _In_opt_ large_integer *timeout, _In_ bool alert);

		enum file_information_class
		{
			FileDirectoryInformation = 1,
			FileFullDirectoryInformation,                   // 2
			FileBothDirectoryInformation,                   // 3
			FileBasicInformation,                           // 4
			FileStandardInformation,                        // 5
			FileInternalInformation,                        // 6
			FileEaInformation,                              // 7
			FileAccessInformation,                          // 8
			FileNameInformation,                            // 9
			FileRenameInformation,                          // 10
			FileLinkInformation,                            // 11
			FileNamesInformation,                           // 12
			FileDispositionInformation,                     // 13
			FilePositionInformation,                        // 14
			FileFullEaInformation,                          // 15
			FileModeInformation,                            // 16
			FileAlignmentInformation,                       // 17
			FileAllInformation,                             // 18
			FileAllocationInformation,                      // 19
			FileEndOfFileInformation,                       // 20
			FileAlternateNameInformation,                   // 21
			FileStreamInformation,                          // 22
			FilePipeInformation,                            // 23
			FilePipeLocalInformation,                       // 24
			FilePipeRemoteInformation,                      // 25
			FileMailslotQueryInformation,                   // 26
			FileMailslotSetInformation,                     // 27
			FileCompressionInformation,                     // 28
			FileObjectIdInformation,                        // 29
			FileCompletionInformation,                      // 30
			FileMoveClusterInformation,                     // 31
			FileQuotaInformation,                           // 32
			FileReparsePointInformation,                    // 33
			FileNetworkOpenInformation,                     // 34
			FileAttributeTagInformation,                    // 35
			FileTrackingInformation,                        // 36
			FileIdBothDirectoryInformation,                 // 37
			FileIdFullDirectoryInformation,                 // 38
			FileValidDataLengthInformation,                 // 39
			FileShortNameInformation,                       // 40
			FileIoCompletionNotificationInformation,        // 41
			FileIoStatusBlockRangeInformation,              // 42
			FileIoPriorityHintInformation,                  // 43
			FileSfioReserveInformation,                     // 44
			FileSfioVolumeInformation,                      // 45
			FileHardLinkInformation,                        // 46
			FileProcessIdsUsingFileInformation,             // 47
			FileNormalizedNameInformation,                  // 48
			FileNetworkPhysicalNameInformation,             // 49
			FileIdGlobalTxDirectoryInformation,             // 50
			FileIsRemoteDeviceInformation,                  // 51
			FileUnusedInformation,                          // 52
			FileNumaNodeInformation,                        // 53
			FileStandardLinkInformation,                    // 54
			FileRemoteProtocolInformation,                  // 55
			FileRenameInformationBypassAccessCheck,         // 56
			FileLinkInformationBypassAccessCheck,           // 57
			FileVolumeNameInformation,                      // 58
			FileIdInformation,                              // 59
			FileIdExtdDirectoryInformation,                 // 60
			FileReplaceCompletionInformation,               // 61
			FileHardLinkFullIdInformation,                  // 62
			FileIdExtdBothDirectoryInformation,             // 63
			FileDispositionInformationEx,                   // 64
			FileRenameInformationEx,                        // 65
			FileRenameInformationExBypassAccessCheck,       // 66
			FileDesiredStorageClassInformation,             // 67
			FileStatInformation,                            // 68
			FileMemoryPartitionInformation,                 // 69
			FileStatLxInformation,                          // 70
			FileCaseSensitiveInformation,                   // 71
			FileLinkInformationEx,                          // 72
			FileLinkInformationExBypassAccessCheck,         // 73
			FileStorageReserveIdInformation,                // 74
			FileCaseSensitiveInformationForceAccessCheck,   // 75
			FileKnownFolderInformation,                     // 76
			FileMaximumInformation
		};

		struct file_name_information { unsigned long len; wchar_t name[1]; };
		struct file_position_information { large_integer offset; };

		using NtQueryInformationFile_t = long (ROD_NTAPI *)(_In_ void *file, _Out_ io_status_block *status, _Out_ void *info, _In_ unsigned long len, _In_ file_information_class info_class);

		static void *get_ntdll();
		template<typename S>
		static void init_symbol(void *, S *&, const char *);

		ROD_API_PUBLIC static const winapi &instance();

		winapi(const winapi &) = delete;
		winapi &operator=(const winapi &) = delete;

		winapi();

		NtReadFile_t NtReadFile;
		NtWriteFile_t NtWriteFile;
		NtCancelIoFileEx_t NtCancelIoFileEx;
		NtSetIoCompletion_t NtSetIoCompletion;
		NtRemoveIoCompletion_t NtRemoveIoCompletion;
		NtRemoveIoCompletionEx_t NtRemoveIoCompletionEx;
		NtQueryInformationFile_t NtQueryInformationFile;
		RtlNtStatusToDosError_t RtlNtStatusToDosError;
	};
}

#endif
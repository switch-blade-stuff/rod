/*
 * Created by switch_blade on 2023-07-04.
 */

#pragma once

#include "../../detail/config.hpp"

#ifdef ROD_WIN32

#include <system_error>

namespace rod::_detail
{
	struct ntapi
	{
		using ushort = unsigned short;
		using ulong = unsigned long;
		using longlong = long long;

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

		struct io_status_block
		{
			union
			{
				ulong status;
				long_ptr ptr;
			};
			ulong_ptr info;
		};
		struct io_completion_info
		{
			ulong_ptr key_context;
			ulong_ptr apc_context;
			io_status_block status;
		};

		using io_apc_routine = void (ROD_NTAPI *)(_In_ ulong_ptr apc_ctx, _In_ io_status_block *iosb, _In_ ulong);

		using RtlNtStatusToDosError_t = ulong (ROD_NTAPI *)(_In_ ulong status);
		using NtWaitForSingleObject_t = ulong (ROD_NTAPI *)(_In_ void *hnd, _In_ bool alert, _In_ large_integer *timeout);
		using NtReadFile_t = ulong (ROD_NTAPI *)(_In_ void *file, _In_opt_ void *evt, _In_opt_ io_apc_routine apc_func, _In_opt_ ulong_ptr apc_ctx, _Out_ io_status_block *iosb, _Out_ void *buff, _In_ ulong len, _In_opt_ large_integer *off, _In_opt_ ulong *key);
		using NtWriteFile_t = ulong (ROD_NTAPI *)(_In_ void *file, _In_opt_ void *evt, _In_opt_ io_apc_routine apc_func, _In_opt_ ulong_ptr apc_ctx, _Out_ io_status_block *iosb, _In_ void *buff, _In_ ulong len, _In_opt_ large_integer *off, _In_opt_ ulong *key);

		using NtCancelIoFileEx_t = ulong (ROD_NTAPI *)(_In_ void *file, _Out_  io_status_block *iosb, _Out_ io_status_block *status);
		using NtSetIoCompletion_t = ulong (ROD_NTAPI *)(_In_ void *hnd, _In_ ulong key_ctx, _In_ ulong_ptr apc_ctx, _In_ long status, _In_ ulong info);
		using NtRemoveIoCompletionEx_t = ulong (ROD_NTAPI *)(_In_ void *hnd, _Out_writes_to_(count, *removed) io_completion_info *completion_info, _In_ ulong count, _Out_ ulong *removed, _In_opt_ large_integer *timeout, _In_ bool alert);

		enum file_info_type
		{
			FileModeInformation = 16,
			FileAlignmentInformation = 17,
		};

		using file_mode_info = ulong;
		using file_alignment_info = ulong;

		using NtQueryInformationFile_t = ulong (ROD_NTAPI *)(_In_ void *file, _Out_ io_status_block *status, _Out_ void *info, _In_ ulong len, _In_ file_info_type type);

		struct status_category_type : std::error_category
		{
			struct entry
			{
				long ntstatus = {};
				ulong dos_err = {};
				int posix_err = {};
				std::string msg = {};
			};

			static const entry *find_entry(long ntstatus);

			status_category_type() noexcept = default;
			~status_category_type() override = default;
			const char *name() const noexcept override { return "ntapi::status_category"; }

			std::string message(int value) const override;
			std::error_condition default_error_condition(int value) const noexcept override;
			bool equivalent(const std::error_code &code, int value) const noexcept override;
			bool equivalent(int value, const std::error_condition &cnd) const noexcept override;
		};

		static const std::error_category *status_category();
		static const ntapi instance;

		ntapi();

		void *ntdll;
		NtReadFile_t NtReadFile;
		NtWriteFile_t NtWriteFile;
		RtlNtStatusToDosError_t RtlNtStatusToDosError;
		NtWaitForSingleObject_t NtWaitForSingleObject;

		NtCancelIoFileEx_t NtCancelIoFileEx;
		NtSetIoCompletion_t NtSetIoCompletion;
		NtRemoveIoCompletionEx_t NtRemoveIoCompletionEx;
		NtQueryInformationFile_t NtQueryInformationFile;
	};
}

#endif
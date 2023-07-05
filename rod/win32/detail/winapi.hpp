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

		using io_apc_routine = void (ROD_NTAPI *)(_In_ void *apc_ctx, _In_ io_status_block *iosb, _In_ unsigned long);

		using RtlNtStatusToDosError_t = unsigned long (ROD_NTAPI *)(_In_ long status);
		using NtWaitForSingleObject_t = long (ROD_NTAPI *)(_In_ void *hnd, _In_ bool alert, _In_ large_integer *timeout);
		using NtReadFile_t = long (ROD_NTAPI *)(_In_ void *file, _In_opt_ void *evt, _In_opt_ io_apc_routine apc_func, _In_opt_ void *apc_ctx, _Out_ io_status_block *iosb, _Out_ void *buff, _In_ unsigned long len, _In_opt_ large_integer *off, _In_opt_ unsigned long *key);
		using NtWriteFile_t = long (ROD_NTAPI *)(_In_ void *file, _In_opt_ void *evt, _In_opt_ io_apc_routine apc_func, _In_opt_ void *apc_ctx, _Out_ io_status_block *iosb, _In_ const void *buff, _In_ unsigned long len, _In_opt_ large_integer *off, _In_opt_ unsigned long *key);

		using NtCancelIoFileEx_t = long (ROD_NTAPI *)(_In_ void *file, _Out_ io_status_block *in_sb, _Out_ io_status_block *out_sb);
		using NtSetIoCompletion_t = long (ROD_NTAPI *)(_In_ void *hnd, _In_ unsigned long key_ctx, _In_ unsigned long apc_ctx, _In_ long status, _In_ unsigned long info);
		using NtRemoveIoCompletion_t = long (ROD_NTAPI *)(_In_ void *hnd, _Out_ void *key, _Out_ void *apc, _Out_ io_status_block *iosb, _In_opt_ large_integer *timeout);
		using NtRemoveIoCompletionEx_t = long (ROD_NTAPI *)(_In_ void *hnd, _Out_writes_to_(count, *removed) file_io_completion_information *completion_info, _In_ unsigned long count, _Out_ unsigned long *removed, _In_opt_ large_integer *timeout, _In_ bool alert);

		static void *get_ntdll();
		template<typename S>
		static void init_symbol(void *, S *&, const char *);

		static const winapi instance;

		winapi(const winapi &) = delete;
		winapi &operator=(const winapi &) = delete;

		winapi();

		NtReadFile_t NtReadFile;
		NtWriteFile_t NtWriteFile;
		RtlNtStatusToDosError_t RtlNtStatusToDosError;
		NtWaitForSingleObject_t NtWaitForSingleObject;

		NtCancelIoFileEx_t NtCancelIoFileEx;
		NtSetIoCompletion_t NtSetIoCompletion;
		NtRemoveIoCompletion_t NtRemoveIoCompletion;
		NtRemoveIoCompletionEx_t NtRemoveIoCompletionEx;
	};
}

#endif
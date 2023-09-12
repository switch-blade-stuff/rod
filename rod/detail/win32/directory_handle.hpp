/*
 * Created by switch_blade on 2023-09-02.
 */

#pragma once

#include "../directory_handle.hpp"
#include "fs_handle_base.hpp"
#include "ntapi.hpp"

namespace rod::_win32
{
	template<typename F>
	[[nodiscard]] static inline result<bool> query_directory(const ntapi &ntapi, void *hnd, std::span<std::byte> buff, unicode_string *filter, bool reset, const file_timeout &to, F &&f) noexcept
	{
		auto iosb = io_status_block();
		auto status = ntapi.NtQueryDirectoryFile(hnd, nullptr, nullptr, 0, &iosb, buff.data(), buff.size(), FileIdFullDirectoryInformation, false, filter, reset);
		if (status == STATUS_PENDING)
			status = ntapi.wait_io(hnd, &iosb, to);
		if (is_status_failure(status))
			return {in_place_error, status_error_code(status)};

		bool eof = false;
		for (auto pos = buff.data(); !eof;)
		{
			auto full_info = reinterpret_cast<const file_id_full_dir_information *>(pos);
			eof = (full_info->next_off == 0);
			pos += full_info->next_off;

			/* Skip directory wildcards. */
			auto name = std::wstring_view(full_info->name, full_info->name_len);
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

			eof = f(name, st);
		}
		return eof;
	}

	/* Win32 directory handles are not created with DELETE permission and need to be re-opened for link operations to work. */
	inline static result<directory_handle> reopen_as_deletable(const ntapi &ntapi, const directory_handle &dir, const file_timeout &to) noexcept
	{
		constexpr auto flags = 0x20 | 1 /*FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE*/;
		constexpr auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
		constexpr auto access = GENERIC_READ | SYNCHRONIZE | DELETE;

		auto iosb = io_status_block();
		auto hnd = ntapi.reopen_file(dir.native_handle(), &iosb, access, share, flags, to);
		if (hnd.has_value()) [[likely]]
			return directory_handle(*hnd, dir.flags());
		else
			return hnd.error();
	}
}

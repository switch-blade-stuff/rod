/*
 * Created by switch_blade on 2023-08-20.
 */

#include "fs_handle_base.hpp"

namespace rod::_win32
{
	constexpr std::size_t buff_size = 65537;

	inline static result<> do_link(const ntapi &ntapi, void *hnd, const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		auto buff = ROD_MAKE_BUFFER(std::byte, buff_size * sizeof(wchar_t) + sizeof(file_link_information));
		if (buff.has_error()) [[unlikely]]
			return buff.error();

		auto rpath = render_as_ustring<true>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto &upath = rpath->first;
		auto guard = ntapi.dos_path_to_nt_path(upath, base.is_open());
		if (guard.has_error()) [[unlikely]]
			return std::make_error_code(std::errc::no_such_file_or_directory);

		auto link_info = reinterpret_cast<file_link_information *>(buff->get());
		auto iosb = io_status_block();

		std::memcpy(link_info->name, upath.buff, upath.size);
		link_info->flags = replace ? 0x1 | 0x2 /*FILE_LINK_REPLACE_IF_EXISTS | FILE_LINK_POSIX_SEMANTICS*/ : 0;
		link_info->root_dir = base.is_open() ? base.native_handle() : nullptr;
		link_info->name_len = upath.size;

		/* Try FileLinkInformationEx first, which will fail before Win10 RS5, in which case fall back to FileRenameInformation. */
		auto status = ntapi.set_file_info(hnd, &iosb, link_info, sizeof(file_rename_information) + upath.size, FileLinkInformationEx, to);
		if (is_status_failure(status))
			status = ntapi.set_file_info(hnd, &iosb, link_info, sizeof(file_rename_information) + upath.size, FileLinkInformation, to);
		if (is_status_failure(status))
			return status_error_code(status);
		else
			return {};
	}
	inline static result<> do_relink(const ntapi &ntapi, void *hnd, const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		auto buff = ROD_MAKE_BUFFER(std::byte, buff_size * sizeof(wchar_t) + sizeof(file_rename_information));
		if (buff.has_error()) [[unlikely]]
			return buff.error();

		auto rpath = render_as_ustring<true>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto &upath = rpath->first;
		auto guard = ntapi.dos_path_to_nt_path(upath, base.is_open());
		if (guard.has_error()) [[unlikely]]
			return std::make_error_code(std::errc::no_such_file_or_directory);

		auto rename_info = reinterpret_cast<file_rename_information *>(buff->get());
		auto iosb = io_status_block();

		std::memcpy(rename_info->name, upath.buff, upath.size);
		rename_info->flags = replace ? 0x1 | 0x2 /*FILE_RENAME_REPLACE_IF_EXISTS | FILE_RENAME_POSIX_SEMANTICS*/ : 0;
		rename_info->root_dir = base.is_open() ? base.native_handle() : nullptr;
		rename_info->name_len = upath.size;

		/* Try FileRenameInformationEx first, which will fail before Win10 RS1, in which case fall back to FileRenameInformation. */
		auto status = ntapi.set_file_info(hnd, &iosb, rename_info, sizeof(file_rename_information) + upath.size, FileRenameInformationEx, to);
		if (is_status_failure(status))
			status = ntapi.set_file_info(hnd, &iosb, rename_info, sizeof(file_rename_information) + upath.size, FileRenameInformation, to);
		if (is_status_failure(status))
			return status_error_code(status);
		else
			return {};
	}

	result<> do_link(void *hnd, const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		if (const auto &ntapi = ntapi::instance(); ntapi.has_value()) [[likely]]
			return do_link(*ntapi, hnd, base, path, replace, to.absolute());
		else
			return ntapi.error();
	}
	result<> do_relink(void *hnd, const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		if (const auto &ntapi = ntapi::instance(); ntapi.has_value()) [[likely]]
			return do_relink(*ntapi, hnd, base, path, replace, to.absolute());
		else
			return ntapi.error();
	}
	result<> do_unlink(void *hnd, file_flags flags, const file_timeout &to) noexcept
	{
		const auto abs_timeout = to.absolute();
		auto iosb = io_status_block();

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		/* Try to unlink with POSIX semantics via the Win10 API. */
		auto disp_info_ex = file_disposition_information_ex{.flags = 0x1 | 0x2}; /*FILE_DISPOSITION_DELETE | FILE_DISPOSITION_POSIX_SEMANTICS*/
		auto status = ntapi->set_file_info(hnd, &iosb, &disp_info_ex, FileBasicInformation, FileDispositionInformationEx);
		if (!is_status_failure(status))
			return {};

		{
			/* Hiding the object is not strictly necessary so discard the error on failure. */
			auto basic_info = file_basic_information{.attributes = FILE_ATTRIBUTE_HIDDEN};
			ntapi->set_file_info(hnd, &iosb, &basic_info, FileBasicInformation, abs_timeout);
		}
		if (!bool(flags & file_flags::unlink_on_close))
		{
			/* Unlike `remove` and `remove_all`, `unlink` requires sufficient access rights, so do not re-try when returned STATUS_ACCESS_DENIED. */
			auto disp_info = file_disposition_information{.del = true};
			status = ntapi->set_file_info(hnd, &iosb, &disp_info, FileDispositionInformation, abs_timeout);
			if (is_status_failure(status))
				return status_error_code(status);
		}
		return {};
	}
}

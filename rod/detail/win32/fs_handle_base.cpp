/*
 * Created by switch_blade on 2023-08-20.
 */

#include "fs_handle_base.hpp"

namespace rod::_win32
{
	constexpr std::size_t buff_size = 65537;

	inline static result<> do_link(const ntapi &ntapi, void *hnd, const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		auto buff = ROD_MAKE_BUFFER(wchar_t, buff_size * sizeof(wchar_t) + sizeof(file_link_information));
		if (buff.has_error()) [[unlikely]]
			return buff.error();

		auto rpath = path.render_null_terminated();
		auto upath = unicode_string();
		upath.max = (upath.size = USHORT(rpath.size() * sizeof(wchar_t))) + sizeof(wchar_t);
		upath.buff = const_cast<wchar_t *>(rpath.data());

		auto guard = ntapi.dos_path_to_nt_path(upath, base.is_open());
		if (guard.has_error()) [[unlikely]]
			return guard.error();

		auto link_info = reinterpret_cast<file_link_information *>(buff->get());
		auto iosb = io_status_block();

		std::memcpy(link_info->name, upath.buff, upath.size);
		link_info->flags = replace ? 0x1 | 0x2 /*FILE_LINK_REPLACE_IF_EXISTS | FILE_LINK_POSIX_SEMANTICS*/ : 0;
		link_info->root_dir = base.is_open() ? base.native_handle() : nullptr;
		link_info->name_len = upath.size;

		/* Try FileLinkInformationEx first, which will fail before Win10 RS5, in which case fall back to FileRenameInformation. */
		auto status = ntapi.NtSetInformationFile(hnd, &iosb, link_info, sizeof(file_rename_information) + upath.size, FileLinkInformationEx);
		if (is_status_failure(status))
			status = ntapi.NtSetInformationFile(hnd, &iosb, link_info, sizeof(file_rename_information) + upath.size, FileLinkInformation);
		if (status == STATUS_PENDING)
			status = ntapi.wait_io(hnd, &iosb, to);
		if (is_status_failure(status))
			return status_error_code(status);
		else
			return {};
	}
	inline static result<> do_relink(const ntapi &ntapi, void *hnd, const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		auto buff = ROD_MAKE_BUFFER(wchar_t, buff_size * sizeof(wchar_t) + sizeof(file_rename_information));
		if (buff.has_error()) [[unlikely]]
			return buff.error();

		auto rpath = path.render_null_terminated();
		auto upath = unicode_string();
		upath.max = (upath.size = USHORT(rpath.size() * sizeof(wchar_t))) + sizeof(wchar_t);
		upath.buff = const_cast<wchar_t *>(rpath.data());

		auto guard = ntapi.dos_path_to_nt_path(upath, base.is_open());
		if (guard.has_error()) [[unlikely]]
			return guard.error();

		auto rename_info = reinterpret_cast<file_rename_information *>(buff->get());
		auto iosb = io_status_block();

		std::memcpy(rename_info->name, upath.buff, upath.size);
		rename_info->flags = replace ? 0x1 | 0x2 /*FILE_RENAME_REPLACE_IF_EXISTS | FILE_RENAME_POSIX_SEMANTICS*/ : 0;
		rename_info->root_dir = base.is_open() ? base.native_handle() : nullptr;
		rename_info->name_len = upath.size;

		/* Try FileRenameInformationEx first, which will fail before Win10 RS1, in which case fall back to FileRenameInformation. */
		auto status = ntapi.NtSetInformationFile(hnd, &iosb, rename_info, sizeof(file_rename_information) + upath.size, FileRenameInformationEx);
		if (is_status_failure(status))
			status = ntapi.NtSetInformationFile(hnd, &iosb, rename_info, sizeof(file_rename_information) + upath.size, FileRenameInformation);
		if (status == STATUS_PENDING)
			status = ntapi.wait_io(hnd, &iosb, to);
		if (is_status_failure(status))
			return status_error_code(status);
		else
			return {};
	}

	result<> do_link(void *hnd, const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		if (const auto &ntapi = ntapi::instance(); ntapi.has_value()) [[likely]]
			return do_link(*ntapi, hnd, base, path, replace, to);
		else
			return ntapi.error();
	}
	result<> do_relink(void *hnd, const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		if (const auto &ntapi = ntapi::instance(); ntapi.has_value()) [[likely]]
			return do_relink(*ntapi, hnd, base, path, replace, to);
		else
			return ntapi.error();
	}
	result<> do_unlink(void *hnd, const file_timeout &to, file_flags flags) noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto disp_info_ex = file_disposition_information_ex{.flags = 0x1 | 0x2}; /*FILE_DISPOSITION_DELETE | FILE_DISPOSITION_POSIX_SEMANTICS*/
		auto abs_timeout = to.absolute();
		auto iosb = io_status_block();

		/* Try to unlink with POSIX semantics via the Win10 API. */
		auto status = ntapi->NtSetInformationFile(hnd, &iosb, &disp_info_ex, sizeof(disp_info_ex), FileDispositionInformationEx);
		if (status == STATUS_PENDING)
			ntapi->wait_io(hnd, &iosb, abs_timeout);
		if (!is_status_failure(status))
			return {};

		auto basic_info = file_basic_information{.attributes = FILE_ATTRIBUTE_HIDDEN};
		auto disp_info = file_disposition_information{.del = true};
		iosb = io_status_block();

		/* Hiding the object is not strictly necessary so discard the error on failure. */
		if (ntapi->NtSetInformationFile(hnd, &iosb, &basic_info, sizeof(basic_info), FileBasicInformation) == STATUS_PENDING)
			ntapi->wait_io(hnd, &iosb, abs_timeout);

		if (!bool(flags & file_flags::unlink_on_close))
		{
			iosb = io_status_block();
			status = ntapi->NtSetInformationFile(hnd, &iosb, &disp_info, sizeof(disp_info), FileDispositionInformation);
			if (status == STATUS_PENDING)
				ntapi->wait_io(hnd, &iosb, abs_timeout);
			if (is_status_failure(status))
				return status_error_code(status);
		}
		return {};
	}
}

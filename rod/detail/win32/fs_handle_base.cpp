/*
 * Created by switch_blade on 2023-08-20.
 */

#include <utility>

#include "../fs_handle_base.hpp"
#include "ntapi.hpp"

namespace rod::_handle
{
	using namespace _win32;

	constexpr auto buff_size = std::size_t(65536);

	inline static result<> do_link(const ntapi &ntapi, basic_handle &hnd, const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		auto buff = make_info_buffer(sizeof(file_link_information) + buff_size);
		if (buff.has_error()) [[unlikely]]
			return buff.error();

		auto upath = unicode_string();
		auto guard = ntapi.path_to_nt_string(upath, path, base.is_open());
		if (guard.has_error()) [[unlikely]]
			return guard.error();

		auto link_info = reinterpret_cast<file_link_information *>(buff->get());
		auto iosb = io_status_block();

		std::memcpy(link_info->name, upath.buffer, upath.size);
		link_info->flags = replace ? 0x1 | 0x2 /*FILE_LINK_REPLACE_IF_EXISTS | FILE_LINK_POSIX_SEMANTICS*/ : 0;
		link_info->root_dir = base.is_open() ? base.native_handle() : nullptr;
		link_info->name_len = upath.size;

		/* Try FileLinkInformationEx first, which will fail before Win10 RS5, in which case fall back to FileRenameInformation. */
		auto status = ntapi.NtSetInformationFile(hnd.native_handle(), &iosb, link_info, sizeof(file_rename_information) + upath.size, FileLinkInformationEx);
		if (is_status_failure(status))
			status = ntapi.NtSetInformationFile(hnd.native_handle(), &iosb, link_info, sizeof(file_rename_information) + upath.size, FileLinkInformation);
		if (status == STATUS_PENDING)
			status = ntapi.wait_io(hnd.native_handle(), &iosb, to);
		if (is_status_failure(status))
			return status_error_code(status);
		else
			return {};
	}
	inline static result<> do_relink(const ntapi &ntapi, basic_handle &hnd, const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		auto buff = make_info_buffer(sizeof(file_rename_information) + buff_size);
		if (buff.has_error()) [[unlikely]]
			return buff.error();

		auto upath = unicode_string();
		auto guard = ntapi.path_to_nt_string(upath, path, base.is_open());
		if (guard.has_error()) [[unlikely]]
			return guard.error();

		auto rename_info = reinterpret_cast<file_rename_information *>(buff->get());
		auto iosb = io_status_block();

		std::memcpy(rename_info->name, upath.buffer, upath.size);
		rename_info->flags = replace ? 0x1 | 0x2 /*FILE_RENAME_REPLACE_IF_EXISTS | FILE_RENAME_POSIX_SEMANTICS*/ : 0;
		rename_info->root_dir = base.is_open() ? base.native_handle() : nullptr;
		rename_info->name_len = upath.size;

		/* Try FileRenameInformationEx first, which will fail before Win10 RS1, in which case fall back to FileRenameInformation. */
		auto status = ntapi.NtSetInformationFile(hnd.native_handle(), &iosb, rename_info, sizeof(file_rename_information) + upath.size, FileRenameInformationEx);
		if (is_status_failure(status))
			status = ntapi.NtSetInformationFile(hnd.native_handle(), &iosb, rename_info, sizeof(file_rename_information) + upath.size, FileRenameInformation);
		if (status == STATUS_PENDING)
			status = ntapi.wait_io(hnd.native_handle(), &iosb, to);
		if (is_status_failure(status))
			return status_error_code(status);
		else
			return {};
	}

	result<> do_link(basic_handle &hnd, const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		if (const auto &ntapi = ntapi::instance(); ntapi.has_value()) [[likely]]
			return do_link(*ntapi, hnd, base, path, replace, to);
		else
			return ntapi.error();
	}
	result<> do_relink(basic_handle &hnd, const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		if (const auto &ntapi = ntapi::instance(); ntapi.has_value()) [[likely]]
			return do_relink(*ntapi, hnd, base, path, replace, to);
		else
			return ntapi.error();
	}
	result<> do_unlink(basic_handle &hnd, const file_timeout &to, file_flags flags) noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		const auto abs_timeout = to.is_relative ? file_clock::now() + to.relative : to.absolute;
		auto disp_info_ex = file_disposition_information_ex{.flags = 0x1 | 0x2}; /*FILE_DISPOSITION_DELETE | FILE_DISPOSITION_POSIX_SEMANTICS*/
		auto iosb = io_status_block();

		/* Try to unlink with POSIX semantics via the Win10 API. */
		if (is_status_failure(ntapi->NtSetInformationFile(hnd.native_handle(), &iosb, &disp_info_ex, sizeof(disp_info_ex), FileDispositionInformationEx)))
			return {};

		/* Fallback to random rename + delete-on-close to emulate immediate unlink. */
		if (!bool(flags & file_flags::no_instant_unlink))
		{
			auto new_path = do_to_object_path(hnd);
			if (new_path.has_error()) [[unlikely]]
				return new_path.error();

			try
			{
				new_path->replace_extension(std::wstring_view(L".deleted"));
				new_path->replace_stem(generate_unique_name());
			}
			catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
			catch (const std::system_error &e) { return e.code(); }

			auto res = do_relink(*ntapi, hnd, path_handle(), *new_path, false, abs_timeout);
			if (res.has_error() && res.error().value() != 0xc0000043 /*STATUS_SHARING_VIOLATION*/)
				return res.error();
		}
		if (!bool(flags & file_flags::unlink_on_close))
		{
			auto basic_info = file_basic_information{.attributes = FILE_ATTRIBUTE_HIDDEN};
			auto disp_info = file_disposition_information{.del = true};
			iosb = io_status_block();

			/* Hiding the object is not strictly necessary so discard the error on failure. */
			if (ntapi->NtSetInformationFile(hnd.native_handle(), &iosb, &basic_info, sizeof(basic_info), FileBasicInformation) == STATUS_PENDING)
				ntapi->wait_io(hnd.native_handle(), &iosb, abs_timeout);

			iosb = io_status_block();
			auto status = ntapi->NtSetInformationFile(hnd.native_handle(), &iosb, &disp_info, sizeof(disp_info), FileDispositionInformation);
			if (status == STATUS_PENDING)
				ntapi->wait_io(hnd.native_handle(), &iosb, abs_timeout);
			if (is_status_failure(status))
				return status_error_code(status);
		}
		return {};
	}
}

/*
 * Created by switch_blade on 2023-08-27.
 */

#include "directory_handle.hpp"
#include "path_discovery.hpp"

namespace rod::_directory
{
	using namespace _win32;

	result<directory_handle> directory_handle::open_unique(const path_handle &base, file_flags flags) noexcept
	{
		try { return open(base, _handle::generate_unique_name()); }
		catch (const std::system_error &e) { return e.code(); }
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
	}
	result<directory_handle> directory_handle::open_temporary(path_view path, file_flags flags, open_mode mode) noexcept
	{
		if (!path.empty())
			return open(temporary_file_directory(), path, flags, mode);
		else if (mode != open_mode::existing)
			return open_unique(temporary_file_directory(), flags);
		else
			return std::make_error_code(std::errc::invalid_argument);
	}
	result<directory_handle> directory_handle::open(const path_handle &base, path_view path, file_flags flags, open_mode mode) noexcept
	{
		constexpr auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

		if (bool(flags & (file_flags::unlink_on_close | file_flags::no_sparse_files))) [[unlikely]]
			return std::make_error_code(std::errc::invalid_argument);
		if (mode == open_mode::truncate || mode == open_mode::supersede) [[unlikely]]
			return std::make_error_code(std::errc::is_a_directory);

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = path.render_null_terminated();
		auto upath = unicode_string();
		upath.max = (upath.size = USHORT(rpath.size() * sizeof(wchar_t))) + sizeof(wchar_t);
		upath.buff = const_cast<wchar_t *>(rpath.data());

		auto guard = ntapi->dos_path_to_nt_path(upath, base.is_open());
		if (guard.has_error()) [[unlikely]]
			return guard.error();

		DWORD access = 0;
		if (bool(flags & file_flags::read))
			access |= STANDARD_RIGHTS_READ;
		if (bool(flags & file_flags::write))
			access |= STANDARD_RIGHTS_WRITE;
		if (bool(flags & file_flags::attr_read))
			access |= FILE_READ_ATTRIBUTES | FILE_READ_EA;
		if (bool(flags & file_flags::attr_write))
			access |= FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA;
		if (bool(flags & file_flags::data_read))
			access |= FILE_LIST_DIRECTORY;
		if (bool(flags & file_flags::data_write))
			access |= FILE_WRITE_DATA;
		if (bool(flags & file_flags::append))
			access |= FILE_APPEND_DATA;

		DWORD ntflags = 1; /*FILE_DIRECTORY_FILE*/
		if (!bool(flags & file_flags::non_blocking))
		{
			ntflags |= 0x20; /*FILE_SYNCHRONOUS_IO_NONALERT*/
			access |= SYNCHRONIZE;
		}

		auto disp = disposition();
		switch (mode)
		{
		case open_mode::always:
			disp = file_open_if;
			break;
		case open_mode::create:
			disp = file_create;
			break;
		case open_mode::existing:
			disp = file_open;
			break;
		}

		auto obj_attrib = object_attributes();
		auto handle = native_handle_type();
		auto iosb = io_status_block();

		obj_attrib.root_dir = base.is_open() ? base.native_handle() : nullptr;
		obj_attrib.length = sizeof(object_attributes);
		obj_attrib.name = &upath;

		auto status = ntapi->NtCreateFile(&handle.value, access, &obj_attrib, &iosb, nullptr, 0, share, disp, ntflags, nullptr, 0);
		if (status == STATUS_PENDING) [[unlikely]]
			status = ntapi->wait_io(handle, &iosb);
		if (iosb.info == 5 /*FILE_DOES_NOT_EXIST*/) [[unlikely]]
			return std::make_error_code(std::errc::no_such_file_or_directory);
		else if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);

		/* Set case-sensitive flag on newly created directories if requested. */
		if (bool(flags & file_flags::case_sensitive) && mode == open_mode::create || (mode == open_mode::always && iosb.info == 2/*FILE_CREATED*/))
		{
			auto case_info = file_case_sensitive_information{.flags = 1/*FILE_CS_FLAG_CASE_SENSITIVE_DIR*/};
			iosb = io_status_block();
			status = ntapi->NtSetInformationFile(handle.value, &iosb, &case_info, sizeof(case_info), FileCaseSensitiveInformation);
			if (status == STATUS_PENDING) [[unlikely]]
				status = ntapi->wait_io(handle, &iosb);
		}

		handle.flags = std::uint32_t(flags);
		return directory_handle(handle);
	}

	result<> directory_handle::do_link(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		if (auto hnd = reopen_as_deletable(*this); hnd.has_value()) [[likely]]
			return _win32::do_link(hnd->native_handle(), base, path, replace, to);
		else
			return hnd.error();
	}
	result<> directory_handle::do_relink(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		if (auto hnd = reopen_as_deletable(*this); hnd.has_value()) [[likely]]
			return _win32::do_relink(hnd->native_handle(), base, path, replace, to);
		else
			return hnd.error();
	}
	result<> directory_handle::do_unlink(const file_timeout &to) noexcept
	{
		if (auto hnd = reopen_as_deletable(*this); hnd.has_value()) [[likely]]
			return _win32::do_unlink(hnd->native_handle(), to, flags());
		else
			return hnd.error();
	}
}

/*
 * Created by switch_blade on 2023-08-27.
 */

#include "directory_handle.hpp"
#include "detail/directory_handle.hpp"

namespace rod::_directory
{
	using namespace _win32;

	inline constexpr auto stats_mask = stat::query::ino | stat::query::type | stat::query::atime | stat::query::mtime | stat::query::ctime | stat::query::btime | stat::query::size | stat::query::alloc | stat::query::is_sparse | stat::query::is_compressed | stat::query::is_reparse_point;
	inline constexpr std::size_t buff_size = 65536;

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

	io_result_t<directory_handle, read_some_t> directory_handle::do_read_some(io_request<read_some_t> &&req, const file_timeout &to) const noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto buff = make_info_buffer(buff_size);
		if (buff.has_error()) [[unlikely]]
			return buff.error();

		auto rfilter = req.filter.render_null_terminated();
		auto ufilter = unicode_string();
		auto g = std::lock_guard(*this);

		ufilter.max = (ufilter.size = USHORT(rfilter.size() * sizeof(wchar_t))) + sizeof(wchar_t);
		ufilter.buff = const_cast<wchar_t *>(rfilter.data());

		wchar_t *result_buff = req.buffs._buff.release();
		std::size_t buffer_size = 0, result_size = 0;
		bool reset_pos = !req.resume, eof = false;

		while (result_size < req.buffs.size() && !eof)
		{
			auto iosb = io_status_block();
			auto status = ntapi->NtQueryDirectoryFile(native_handle(), nullptr, nullptr, 0, &iosb, buff->get(), buff_size * sizeof(wchar_t), FileIdFullDirectoryInformation, false, req.filter.empty() ? nullptr : &ufilter, reset_pos);
			if (status == STATUS_PENDING)
				status = ntapi->wait_io(native_handle(), &iosb, to);
			if (is_status_failure(status))
			{
				eof = status == 0x80000006 /*STATUS_NO_MORE_FILES*/;
				goto finish;
			}

			reset_pos = eof = false;
			for (auto pos = reinterpret_cast<std::byte *>(buff->get()); !eof && result_size < req.buffs.size(); ++result_size)
			{
				auto full_info = reinterpret_cast<file_id_full_dir_information *>(pos);
				auto &entry = req.buffs[result_size];

				/* Copy or reserve entry path. */
				if (entry._buff.size() >= full_info->name_len)
				{
					/* Try to terminate path for efficiency. */
					if ((entry._has_null_terminator = entry._buff.size() > full_info->name_len))
						entry._buff[entry._buff.size()] = '\0';

					/* If the provided space is big enough, directly copy to the user buffer. */
					std::memcpy(entry._buff.data(), full_info->name, full_info->name_len * sizeof(wchar_t));
					entry._buff = entry._buff.subspan(0, full_info->name_len);
				}
				else
				{
					auto old_size = buffer_size, new_size = buffer_size + full_info->name_len + 1;
					auto old_buff = result_buff, new_buff = result_buff;

					if (new_size > req.buffs._buff_max)
					{
						if (old_buff)
							new_buff = static_cast<wchar_t *>(std::realloc(old_buff, new_size * sizeof(wchar_t)));
						else
							new_buff = static_cast<wchar_t *>(std::malloc(new_size * sizeof(wchar_t)));
						if (new_buff == nullptr) [[unlikely]]
							goto finish;
					}

					/* Since WinNT paths are 32767 chars max, we can use a negative number to indicate an offset. */
					std::memcpy(result_buff + old_size, full_info->name, full_info->name_len * sizeof(wchar_t));
					auto off = reinterpret_cast<wchar_t *>(old_size);
					auto len = ULONG(-LONG(full_info->name_len));
					entry._buff = std::span(off, len);

					result_buff = new_buff;
					buffer_size = new_size;
				}

				/* Fill requested entry metadata. */
				entry._metadata = stat(nullptr);
				if (bool(entry._mask &= stats_mask))
				{
					if (bool(entry._mask & stat::query::ino))
						entry._metadata.ino = full_info->file_id;
					if (bool(entry._mask & stat::query::type))
						entry._metadata.type = attr_to_type(full_info->attributes, full_info->reparse_tag);
					if (bool(entry._mask & stat::query::atime))
						entry._metadata.atime = filetime_to_tp(full_info->atime);
					if (bool(entry._mask & stat::query::mtime))
						entry._metadata.mtime = filetime_to_tp(full_info->mtime);
					if (bool(entry._mask & stat::query::ctime))
						entry._metadata.ctime = filetime_to_tp(full_info->ctime);
					if (bool(entry._mask & stat::query::btime))
						entry._metadata.btime = filetime_to_tp(full_info->btime);
					if (bool(entry._mask & stat::query::size))
						entry._metadata.size = full_info->eof;
					if (bool(entry._mask & stat::query::alloc))
						entry._metadata.alloc = full_info->alloc_size;
					if (bool(entry._mask & stat::query::is_sparse))
						entry._metadata.is_sparse = full_info->attributes & FILE_ATTRIBUTE_SPARSE_FILE;
					if (bool(entry._mask & stat::query::is_compressed))
						entry._metadata.is_compressed = full_info->attributes & FILE_ATTRIBUTE_COMPRESSED;
					if (bool(entry._mask & stat::query::is_reparse_point))
						entry._metadata.is_reparse_point = full_info->attributes & FILE_ATTRIBUTE_REPARSE_POINT;
				}

				/* Advance to next entry. */
				eof = (full_info->next_off == 0);
				pos += full_info->next_off;
			}
		}

	finish:
		/* Restore placeholder offsets. */
		for (std::size_t i = 0; i < result_size; ++i)
		{
			auto &entry = req.buffs[result_size];
			if (LONG(entry._buff.size()) >= 0)
				continue;

			const auto off = reinterpret_cast<std::size_t>(entry._buff.data());
			const auto len = LONG(-LONG(entry._buff.size()));
			entry._buff = std::span(result_buff + off, len);
		}

		/* Save the buffer max size for later re-use. */
		req.buffs._buff_max = std::max(req.buffs._buff_max, buffer_size);
		req.buffs._buff.reset(result_buff);

		/* Always truncate to result_size to avoid empty-entry checks. */
		req.buffs._data = req.buffs._data.subspan(0, result_size);
		return std::move(req.buffs);
	}
}

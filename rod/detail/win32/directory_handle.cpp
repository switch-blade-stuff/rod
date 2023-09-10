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

			eof = f(*full_info, st);
		}
		return eof;
	}

	io_result_t<directory_handle, read_some_t> directory_handle::do_read_some(io_request<read_some_t> &&req, const file_timeout &to) noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto buff = ROD_MAKE_BUFFER(std::byte, buff_size * sizeof(wchar_t));
		if (buff.has_error()) [[unlikely]]
			return buff.error();

		const auto abs_timeout = to.absolute();
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
			auto bytes = std::span{buff->get(), buff_size * sizeof(wchar_t)};
			eof = query_directory(*ntapi, native_handle(), bytes, req.filter.empty() ? nullptr : &ufilter, reset_pos, abs_timeout, [&](auto &info, auto &st)
			{
				auto &entry = req.buffs[result_size++];
				if (entry._buff.size() >= info.name_len)
				{
					/* Try to terminate path for efficiency. */
					if ((entry._is_terminated = entry._buff.size() > info.name_len))
						entry._buff[entry._buff.size()] = '\0';

					std::memcpy(entry._buff.data(), info.name, info.name_len * sizeof(wchar_t));
					entry._buff = entry._buff.subspan(0, info.name_len);
				}
				else
				{
					auto old_size = buffer_size, new_size = buffer_size + info.name_len + 1;
					auto old_buff = result_buff, new_buff = result_buff;

					if (new_size > req.buffs._buff_max)
					{
						if (old_buff)
							new_buff = static_cast<wchar_t *>(std::realloc(old_buff, new_size * sizeof(wchar_t)));
						else
							new_buff = static_cast<wchar_t *>(std::malloc(new_size * sizeof(wchar_t)));
						if (new_buff == nullptr) [[unlikely]]
							return false;
					}

					/* Since WinNT paths are 32767 chars max, we can use a negative number to indicate an offset. */
					std::memcpy(result_buff + old_size, info.name, info.name_len * sizeof(wchar_t));
					auto off = reinterpret_cast<wchar_t *>(old_size);
					auto len = ULONG(-LONG(info.name_len));
					entry._buff = std::span(off, len);

					result_buff = new_buff;
					buffer_size = new_size;
				}

				entry._query = stats_mask;
				entry._st = st;
				return true;
			}).value_or(false);
		}

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
	result<> directory_iterator::next(const file_timeout &to) noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto buff = ROD_MAKE_BUFFER(std::byte, buff_size * sizeof(wchar_t));
		if (buff.has_error()) [[unlikely]]
			return buff.error();

		const auto abs_timeout = to.absolute();
		auto &str = _entry.to_path_string();
		auto res = result<>();

		auto bytes = std::span{buff->get(), buff_size * sizeof(wchar_t)};
		auto err = query_directory(*ntapi, _dir_hnd.native_handle(), bytes, nullptr, false, abs_timeout, [&](auto &info, auto &st)
		{
			_entry._query = stats_mask;
			_entry._st = st;

			try { str.assign(info.name, info.name_len); }
			catch (const std::bad_alloc &) { res = std::make_error_code(std::errc::not_enough_memory); }
			return false;
		});
		if (err.has_error()) [[unlikely]]
			return err;
		else
			return res;
	}
}

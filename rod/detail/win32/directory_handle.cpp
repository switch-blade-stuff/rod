/*
 * Created by switchblade on 2023-08-27.
 */

#include "directory_handle.hpp"

namespace rod::_dir
{
	using namespace _win32;
	using namespace fs;

	inline constexpr auto stats_mask = stat::query::ino | stat::query::type | stat::query::atime | stat::query::mtime | stat::query::ctime | stat::query::btime | stat::query::size | stat::query::alloc | stat::query::is_sparse | stat::query::is_compressed | stat::query::is_reparse_point;
	inline constexpr auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
	inline constexpr std::size_t buff_size = 65536;

	result<directory_handle> directory_handle::open(const path_handle &base, path_view path, file_flags flags, open_mode mode) noexcept
	{
		if (bool(flags & (file_flags::unlink_on_close | file_flags::no_sparse_files))) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		if (mode == open_mode::truncate || mode == open_mode::supersede) [[unlikely]]
			return std::make_error_code(std::errc::is_a_directory);

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = render_as_ustring<true>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto &upath = rpath->first;
		auto guard = ntapi->dos_path_to_nt_path(upath, base.is_open());
		if (guard.has_error()) [[unlikely]]
			return std::make_error_code(std::errc::no_such_file_or_directory);

		auto obj_attrib = object_attributes();
		auto access = flags_to_access(flags) & ~DELETE;
		auto opts = flags_to_opts(flags) | 1; /*FILE_DIRECTORY_FILE*/
		auto disp = mode_to_disp(mode);
		auto iosb = io_status_block();

		obj_attrib.root_dir = base.is_open() ? base.native_handle() : nullptr;
		obj_attrib.length = sizeof(object_attributes);
		obj_attrib.name = &upath;

		auto hnd = ntapi->create_file(obj_attrib, &iosb, access, 0, share, disp, opts);
		if (hnd.has_error()) [[unlikely]]
		{
			/* Map known error codes. */
			if (auto err = hnd.error(); is_error_file_not_found(err))
				return std::make_error_code(std::errc::no_such_file_or_directory);
			else if (is_error_not_a_directory(err))
				return std::make_error_code(std::errc::not_a_directory);
			else if (is_error_file_exists(err) && mode == open_mode::create)
				return std::make_error_code(std::errc::file_exists);
			else
				return err;
		}

		/* Optionally set case-sensitive flag on newly created directories if requested. */
		if (bool(flags & file_flags::case_sensitive) && mode == open_mode::create || (mode == open_mode::always && iosb.info == 2 /*FILE_CREATED*/))
		{
			auto case_info = file_case_sensitive_information{.flags = 1 /*FILE_CS_FLAG_CASE_SENSITIVE_DIR*/};
			ntapi->set_file_info(*hnd, &iosb, &case_info, FileCaseSensitiveInformation);
		}
		return directory_handle(*hnd, flags);
	}
	result<directory_handle> directory_handle::reopen(const path_handle &other, file_flags flags) noexcept
	{
		/* Try to clone if possible. */
		if (flags == file_flags(other.native_handle().flags))
		{
			if (auto hnd = clone(other); hnd.has_value()) [[likely]]
				return directory_handle(std::move(*hnd), flags);
			else
				return hnd.error();
		}

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto access = flags_to_access(flags) & ~DELETE;
		auto opts = flags_to_opts(flags);
		auto iosb = io_status_block();

		auto hnd = ntapi->reopen_file(other.native_handle(), &iosb, access, share, opts);
		if (hnd.has_value()) [[likely]]
			return directory_handle(*hnd, flags);
		else
			return hnd.error();
	}

	result<> directory_handle::do_link(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		if (!bool(flags() & file_flags::non_blocking) && to != timeout_type())
			return std::make_error_code(std::errc::not_supported);

		const auto abs_timeout = to.absolute();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = render_as_ustring<false>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto del_hnd = reopen_as_deletable(*ntapi, *this, abs_timeout);
		if (del_hnd.has_error()) [[unlikely]]
			return del_hnd.error();

		auto iosb = io_status_block();
		if (auto status = ntapi->link_file(del_hnd->native_handle(), &iosb, base.native_handle(), rpath->first, replace, abs_timeout); is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return {};
	}
	result<> directory_handle::do_relink(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		if (!bool(flags() & file_flags::non_blocking) && to != timeout_type())
			return std::make_error_code(std::errc::not_supported);

		const auto abs_timeout = to.absolute();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = render_as_ustring<false>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto del_hnd = reopen_as_deletable(*ntapi, *this, abs_timeout);
		if (del_hnd.has_error()) [[unlikely]]
			return del_hnd.error();

		auto iosb = io_status_block();
		if (auto status = ntapi->relink_file(del_hnd->native_handle(), &iosb, base.native_handle(), rpath->first, replace, abs_timeout); is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return {};
	}
	result<> directory_handle::do_unlink(const file_timeout &to) noexcept
	{
		if (!bool(flags() & file_flags::non_blocking) && to != timeout_type())
			return std::make_error_code(std::errc::not_supported);

		const auto abs_timeout = to.absolute();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto del_hnd = reopen_as_deletable(*ntapi, *this, abs_timeout);
		if (del_hnd.has_error()) [[unlikely]]
			return del_hnd.error();

		auto iosb = io_status_block();
		if (auto status = ntapi->unlink_file(del_hnd->native_handle(), &iosb, !bool(flags() & file_flags::unlink_on_close), abs_timeout); is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return {};
	}

	io_result_t<directory_handle, read_some_t> directory_handle::do_read_some(io_request<read_some_t> &&req, const file_timeout &to) noexcept
	{
		if (req.buffs.empty()) [[unlikely]]
			return std::make_pair(std::move(req.buffs), false);

		const auto abs_timeout = to.absolute();
		auto g = std::lock_guard(*this);

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto buff = ROD_MAKE_BUFFER(std::byte, buff_size * sizeof(wchar_t));
		if (buff.get() == nullptr) [[unlikely]]
			return std::make_error_code(std::errc::not_enough_memory);

		auto rfilter = render_as_ustring<true>(req.filter);
		if (rfilter.has_error()) [[unlikely]]
			return rfilter.error();

		std::size_t buffer_size = req.buffs._buff_max, result_size = 0;
		wchar_t *result_buff = req.buffs._buff.release();
		auto &ufilter = rfilter->first;
		bool reset_pos = !req.resume;
		result<bool> eof_result;

		while (result_size < req.buffs.size() && !eof_result.value_or(false))
		{
			auto bytes = std::span{buff.get(), buff_size * sizeof(wchar_t)};
			eof_result = ntapi->query_directory(native_handle(), bytes, req.filter.empty() ? nullptr : &ufilter, reset_pos, abs_timeout, [&](auto sv, auto &st) noexcept -> result<bool>
			{
				auto &entry = req.buffs[result_size++];
				if (entry._buff.size() >= sv.size())
				{
					/* Try to terminate path for efficiency. */
					if ((entry._is_terminated = entry._buff.size() > sv.size()))
						entry._buff[entry._buff.size()] = '\0';

					std::memcpy(entry._buff.data(), sv.data(), sv.size() * sizeof(wchar_t));
					entry._buff = entry._buff.subspan(0, sv.size());
				}
				else
				{
					auto old_size = buffer_size, new_size = buffer_size + sv.size() + 1;
					auto old_buff = result_buff, new_buff = result_buff;

					if (new_size > old_size)
					{
						if (old_buff)
							new_buff = static_cast<wchar_t *>(std::realloc(old_buff, new_size * sizeof(wchar_t)));
						else
							new_buff = static_cast<wchar_t *>(std::malloc(new_size * sizeof(wchar_t)));
						if (new_buff == nullptr) [[unlikely]]
							return {in_place_error, std::make_error_code(std::errc::not_enough_memory)};
					}

					/* Since WinNT paths are 32767 chars max, we can use a negative number to indicate an offset. */
					std::memcpy(result_buff + old_size, sv.data(), sv.size() * sizeof(wchar_t));
					auto off = reinterpret_cast<wchar_t *>(old_size);
					auto len = ULONG(-LONG(sv.size()));
					entry._buff = std::span(off, len);

					result_buff = new_buff;
					buffer_size = new_size;
				}

				entry._query = stats_mask;
				entry._st = st;
				return {in_place_value, false};
			});
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

		/* Always truncate to result_size to avoid empty-entry checks. */
		req.buffs._data = req.buffs._data.subspan(0, result_size);
		/* Save the buffer max size for later re-use. */
		req.buffs._buff_max = std::max(req.buffs._buff_max, buffer_size);
		req.buffs._buff.reset(result_buff);

		if (eof_result.has_value()) [[likely]]
			return std::make_pair(std::move(req.buffs), *eof_result);
		else
			return eof_result.error();
	}

	result<directory_iterator> directory_iterator::from_handle(const path_handle &other) noexcept
	{
		if (auto hnd = directory_handle::reopen(other, file_flags::read); hnd.has_value()) [[likely]]
			return directory_iterator(hnd->release());
		else
			return hnd.error();
	}
	result<directory_iterator> directory_iterator::from_path(const path_handle &base, path_view path) noexcept
	{
		if (auto hnd = directory_handle::open(base, path, file_flags::read); hnd.has_value()) [[likely]]
			return directory_iterator(hnd->release());
		else
			return hnd.error();
	}

	result<> directory_iterator::next(const file_timeout &to) noexcept
	{
		const auto abs_timeout = to.absolute();
		auto &&str = _entry.to_path_string();

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto buff = ROD_MAKE_BUFFER(std::byte, buff_size * sizeof(wchar_t));
		if (buff.get() == nullptr) [[unlikely]]
			return std::make_error_code(std::errc::not_enough_memory);

		auto bytes = std::span{buff.get(), buff_size * sizeof(wchar_t)};
		auto eof_result = ntapi->query_directory(_dir_hnd.native_handle(), bytes, nullptr, false, abs_timeout, [&](auto sv, auto &st) -> result<bool>
		{
			_entry._query = stats_mask;
			_entry._st = st;

			try { str.assign(sv.data(), sv.size()); }
			catch (...) { return {in_place_error, _detail::current_error()}; }
			return {in_place_value, false};
		});
		if (eof_result.has_error()) [[unlikely]]
			return eof_result.error();

		/* Reset the iterator to sentinel on EOF so that end iterator comparisons are equal. */
		if (*eof_result)
			*this = end();
		return {};
	}
}

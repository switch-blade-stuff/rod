/*
 * Created by switch_blade on 2023-08-27.
 */

#include "directory_handle.hpp"
#include "handle_stat.hpp"

namespace rod::_directory
{
	using namespace _win32;

	inline constexpr auto stats_mask = stat::query::ino | stat::query::type | stat::query::atime | stat::query::mtime | stat::query::ctime | stat::query::btime | stat::query::size | stat::query::alloc | stat::query::is_sparse | stat::query::is_compressed | stat::query::is_reparse_point;
	inline constexpr auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
	inline constexpr std::size_t buff_size = 65536;

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

		auto access = flags_to_access(flags);
		auto opts = flags_to_opts(flags);
		auto iosb = io_status_block();

		auto hnd = ntapi->reopen_file(other.native_handle(), &iosb, access, share, opts);
		if (hnd.has_value()) [[likely]]
			return directory_handle(*hnd, flags);
		else
			return hnd.error();
	}
	result<directory_handle> directory_handle::open(const path_handle &base, path_view path, file_flags flags, open_mode mode) noexcept
	{
		if (bool(flags & (file_flags::unlink_on_close | file_flags::no_sparse_files))) [[unlikely]]
			return std::make_error_code(std::errc::invalid_argument);
		if (mode == open_mode::truncate || mode == open_mode::supersede) [[unlikely]]
			return std::make_error_code(std::errc::is_a_directory);

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rend = render_as_ustring<true>(path);
		if (rend.has_error()) [[unlikely]]
			return rend.error();

		auto &[upath, rpath] = *rend;
		auto guard = ntapi->dos_path_to_nt_path(upath, base.is_open());
		if (guard.has_error()) [[unlikely]]
			return guard.error();

		auto obj_attrib = object_attributes();
		auto access = flags_to_access(flags);
		auto opts = flags_to_opts(flags);
		auto disp = mode_to_disp(mode);
		auto iosb = io_status_block();

		obj_attrib.root_dir = base.is_open() ? base.native_handle() : nullptr;
		obj_attrib.length = sizeof(object_attributes);
		obj_attrib.name = &upath;

		auto hnd = ntapi->create_file(obj_attrib, &iosb, access, 0, share, disp, opts);
		if (hnd.has_error()) [[unlikely]]
			return hnd.error();

		/* Optionally set case-sensitive flag on newly created directories if requested. */
		if (bool(flags & file_flags::case_sensitive) && mode == open_mode::create || (mode == open_mode::always && iosb.info == 2 /*FILE_CREATED*/))
		{
			auto case_info = file_case_sensitive_information{.flags = 1/*FILE_CS_FLAG_CASE_SENSITIVE_DIR*/};
			ntapi->set_file_info(*hnd, &iosb, &case_info, FileCaseSensitiveInformation);
		}
		return directory_handle(*hnd, flags);
	}

	result<> directory_handle::do_link(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		const auto abs_timeout = to.absolute();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		if (auto hnd = reopen_as_deletable(*ntapi, *this, abs_timeout); hnd.has_value()) [[likely]]
			return _win32::do_link(hnd->native_handle(), base, path, replace, abs_timeout);
		else
			return hnd.error();
	}
	result<> directory_handle::do_relink(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		const auto abs_timeout = to.absolute();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		if (auto hnd = reopen_as_deletable(*ntapi, *this, abs_timeout); hnd.has_value()) [[likely]]
			return _win32::do_relink(hnd->native_handle(), base, path, replace, abs_timeout);
		else
			return hnd.error();
	}
	result<> directory_handle::do_unlink(const file_timeout &to) noexcept
	{
		const auto abs_timeout = to.absolute();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		if (auto hnd = reopen_as_deletable(*ntapi, *this, abs_timeout); hnd.has_value()) [[likely]]
			return _win32::do_unlink(hnd->native_handle(), abs_timeout, flags());
		else
			return hnd.error();
	}

	io_result_t<directory_handle, read_some_t> directory_handle::do_read_some(io_request<read_some_t> &&req, const file_timeout &to) noexcept
	{
		if (req.buffs.empty()) [[unlikely]]
			return std::move(req.buffs);

		const auto abs_timeout = to.absolute();
		auto g = std::lock_guard(*this);

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto buff = ROD_MAKE_BUFFER(std::byte, buff_size * sizeof(wchar_t));
		if (buff.has_error()) [[unlikely]]
			return buff.error();

		auto rend = render_as_ustring<true>(req.filter);
		if (rend.has_error()) [[unlikely]]
			return {in_place_error, rend.error()};

		std::size_t buffer_size = req.buffs._buff_max, result_size = 0;
		wchar_t *result_buff = req.buffs._buff.release();
		bool reset_pos = !req.resume, eof = false;
		auto &ufilter = rend->first;

		while (result_size < req.buffs.size() && !eof)
		{
			auto bytes = std::span{buff->get(), buff_size * sizeof(wchar_t)};
			eof = query_directory(*ntapi, native_handle(), bytes, req.filter.empty() ? nullptr : &ufilter, reset_pos, abs_timeout, [&](auto sv, auto &st)
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
							return true;
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
				return false;
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
		auto res = result<>();

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto buff = ROD_MAKE_BUFFER(std::byte, buff_size * sizeof(wchar_t));
		if (buff.has_error()) [[unlikely]]
			return buff.error();

		auto bytes = std::span{buff->get(), buff_size * sizeof(wchar_t)};
		auto err = query_directory(*ntapi, _dir_hnd.native_handle(), bytes, nullptr, false, abs_timeout, [&](auto sv, auto &st)
		{
			_entry._query = stats_mask;
			_entry._st = st;

			try { str.assign(sv.data(), sv.size()); }
			catch (const std::bad_alloc &) { res = std::make_error_code(std::errc::not_enough_memory); }
			return false;
		});

		/* Reset the iterator to sentinel on EOF so that end iterator comparisons are equal. */
		if (auto code = err.error_or({}).value(); code == 0x80000006/*STATUS_NO_MORE_FILES*/)
		{
			_entry = directory_entry{};
			_dir_hnd = basic_handle{};
			return {};
		}
		else if (code != 0) [[unlikely]]
			return err;
		else
			return res;
	}
}

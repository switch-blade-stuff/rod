/*
 * Created by switchblade on 2023-08-27.
 */

#include "directory_handle.hpp"

namespace rod::_dir
{
	using namespace fs;

	result<directory_handle> directory_handle::open(const path_handle &base, path_view path, file_flags flags, open_mode mode) noexcept
	{
		if (bool(flags & (file_flags::unlink_on_close | file_flags::no_sparse_files | file_flags::non_blocking))) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		if (mode == open_mode::truncate || mode == open_mode::supersede) [[unlikely]]
			return std::make_error_code(std::errc::is_a_directory);

		try
		{
			const auto base_hnd = base.native_handle() ? base.native_handle() : native_handle_type(AT_FDCWD);
			const auto rpath = (path.empty() && base.is_open() ? "." : path).render_null_terminated();
			auto fd_flags = _unix::make_fd_flags(flags & ~file_flags::write, mode);
#ifdef O_DIRECTORY
			fd_flags |= O_DIRECTORY;
#endif

			if (bool(fd_flags & O_CREAT) && ::mkdirat(base_hnd, rpath.c_str(), S_IRWXU | S_IRWXG) < 0) [[unlikely]]
			{
				const auto err = errno;
				if (err != EEXIST || bool(fd_flags & O_EXCL)) [[likely]]
					return std::error_code(err, std::system_category());
			}
			if (const auto hnd = ::openat(base_hnd, rpath.c_str(), fd_flags); hnd < 0) [[unlikely]]
				return std::error_code(errno, std::system_category());
			else
				return directory_handle(hnd, flags);
		}
		catch (...) { return _detail::current_error(); }
	}
	result<directory_handle> directory_handle::reopen(const path_handle &other, file_flags flags) noexcept
	{
		if (flags == file_flags(other.native_handle().flags))
		{
			if (auto hnd = clone(other); hnd.has_value()) [[likely]]
				return directory_handle(std::move(*hnd), flags);
			else
				return hnd.error();
		}
		return open(other, ".", flags);
	}

	result<> directory_handle::do_relink(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		const auto abs_timeout = to.is_infinite() ? file_timeout() : to.absolute();
		if (const auto res = _unix::relink_file(native_handle(), base, path, replace, bool(flags() & file_flags::unlink_stat_check), abs_timeout); res.has_value()) [[likely]]
			return release(*res), result<>();
		else
			return res.error();
	}
	result<> directory_handle::do_unlink(const file_timeout &to) noexcept
	{
		const auto abs_timeout = to.is_infinite() ? file_timeout() : to.absolute();
		if (const auto res = _unix::unlink_file(native_handle(), true, bool(flags() & file_flags::unlink_stat_check), abs_timeout); res.has_value()) [[likely]]
			return release(*res), result<>();
		else
			return res.error();
	}

	io_result_t<directory_handle, read_some_t> directory_handle::do_read_some(io_request<read_some_t> &&req, const file_timeout &to) noexcept
	{
		if (req.buffs.empty()) [[unlikely]]
			return std::make_pair(std::move(req.buffs), false);

		try
		{
			const auto abs_timeout = to.is_infinite() ? file_timeout() : to.absolute();
			const auto buff_init = req.buffs.size() * (sizeof(_unix::dirent) + 256);

			auto dir_offset = _unix::seek_pos(native_handle(), 0, req.reset ? SEEK_SET : SEEK_CUR);
			if (dir_offset < 0) [[unlikely]]
				return std::error_code(errno, std::system_category());

			const auto rfilter = req.filter.render_null_terminated();
			auto buff_mem = std::move(req.buffs._buff);
			auto buff_len = req.buffs._buff_len;
			auto result_len = std::size_t();
			auto result_eof = false;

			for (std::size_t i = 0; result_len < req.buffs.size() && !result_eof; ++i)
			{
				if (const auto new_len = buff_init << i; new_len == 0) [[unlikely]]
					return std::make_error_code(std::errc::not_enough_memory);
				else if (buff_len < new_len)
				{
					auto *old_mem = buff_mem.release();
					void *new_mem = nullptr;

					if (old_mem != nullptr)
						new_mem = std::realloc(old_mem, new_len);
					else
						new_mem = std::malloc(new_len);
					if (new_mem == nullptr) [[unlikely]]
						return (buff_mem.reset(old_mem), std::make_error_code(std::errc::not_enough_memory));

					buff_mem.reset(static_cast<char *>(new_mem));
					buff_len = new_len;
				}

				auto next_len = result_len;
				const auto off = _unix::for_each_dir_entry(native_handle(), {buff_mem.get(), buff_len}, abs_timeout, [&](_unix::dirent entry_data) noexcept -> result<bool>
				{
					if (next_len >= req.buffs.size())
						return false;

					auto match_flags = FNM_PATHNAME;
#ifdef FNM_EXTMATCH
					match_flags |= FNM_EXTMATCH;
#endif
					if (!req.filter.empty() && ::fnmatch(rfilter.c_str(), entry_data.d_name, match_flags) != 0)
						return false;

					auto &entry = (req.buffs._data[next_len] = io_buffer<read_some_t>());
					if (entry._buff.data() != nullptr)
						entry._buff = std::span<char>(entry_data.d_name, _detail::strlen(entry_data.d_name));
					else
						entry._buff = std::span(entry._buff.data(), entry_data.d_name - buff_mem.get());

					entry._st.type = _unix::type_from_dirent_type(entry_data.d_type);
					entry._st.ino = entry_data.d_ino;

					entry._query = stat::query::ino;
					if ((entry._st.type) != file_type::unknown)
						entry._query |= stat::query::type;

					next_len += 1;
					return true;
				});
				if (next_len >= req.buffs.size() || (off.has_value() && (result_eof = !(dir_offset = *off))))
					result_len = next_len;

				/* Always seek to start to ensure atomicity. */
				const auto res = _unix::seek_pos(native_handle(), dir_offset, SEEK_SET);
				if (off.has_error() && off.error().value() != EINVAL) [[unlikely]]
					return off.error();
				if (res < 0) [[unlikely]]
					return std::error_code(errno, std::system_category());
			}

			for (auto &entry : req.buffs)
			{
				if (entry._buff.data() == nullptr)
					continue;

				const auto str = buff_mem.get() + entry._buff.size();
				entry._buff = std::span(str, _detail::strlen(str));
			}

			req.buffs._buff = std::move(buff_mem);
			req.buffs._buff_len = std::max(req.buffs._buff_len, buff_len);
			req.buffs._data = req.buffs._data.subspan(0, result_len);
			return std::make_pair(std::move(req.buffs), result_eof);
		}
		catch (...) { return _detail::current_error(); }
	}

	result<directory_iterator> directory_iterator::from_handle(const path_handle &other) noexcept
	{
		auto hnd = directory_handle::reopen(other, file_flags::read);
		if (hnd.has_error()) [[unlikely]]
			return hnd.error();

		auto iter = directory_iterator(hnd->release());
		if (auto res = iter.next(); res.has_value()) [[likely]]
			return std::move(iter);
		else
			return res.error();
	}
	result<directory_iterator> directory_iterator::from_path(const path_handle &base, path_view path) noexcept
	{
		auto hnd = directory_handle::open(base, path, file_flags::read);
		if (hnd.has_error()) [[unlikely]]
			return hnd.error();

		auto iter = directory_iterator(hnd->release());
		if (auto res = iter.next(); res.has_value()) [[likely]]
			return std::move(iter);
		else
			return res.error();
	}

	result<> directory_iterator::next(const file_timeout &to) noexcept
	{
		try
		{
			const auto abs_timeout = to.is_infinite() ? file_timeout() : to.absolute();
			auto dir_offset = _unix::seek_pos(_base_hnd.native_handle(), 0, SEEK_CUR);
			if (dir_offset < 0) [[unlikely]]
				return std::error_code(errno, std::system_category());

			constexpr auto buff_init = sizeof(_unix::dirent) + 256;
			auto &result_buff = _entry.to_path_string();
			bool has_result = false;

			for (std::size_t i = 0; !has_result; ++i)
			{
				if (const auto new_len = buff_init << i; new_len == 0) [[unlikely]]
					return std::make_error_code(std::errc::not_enough_memory);
				else if (result_buff.size() < new_len)
					result_buff.resize(new_len);

				const auto off = _unix::for_each_dir_entry(_base_hnd.native_handle(), result_buff, abs_timeout, [&](_unix::dirent entry_data) -> result<int>
				{
					_entry._st.type = _unix::type_from_dirent_type(entry_data.d_type);
					_entry._st.ino = entry_data.d_ino;

					_entry._query = stat::query::ino;
					if ((_entry._st.type) != file_type::unknown)
						_entry._query |= stat::query::type;

					result_buff.erase(entry_data.d_name - result_buff.data());
					result_buff.resize(_detail::strlen(entry_data.d_name));
					has_result = true;
					return -1;
				});
				if (off.has_value() && (dir_offset = *off) == 0)
					break;

				/* Always seek to start to ensure atomicity. */
				const auto res = _unix::seek_pos(_base_hnd.native_handle(), dir_offset, SEEK_SET);
				if (off.has_error() && off.error().value() != EINVAL) [[unlikely]]
					return off.error();
				if (res < 0) [[unlikely]]
					return std::error_code(errno, std::system_category());
			}
			if (!has_result && dir_offset == 0)
			{
				_base_hnd = {};
				_entry = {};
			}
			return {};
		}
		catch (...) { return _detail::current_error(); }
	}
}

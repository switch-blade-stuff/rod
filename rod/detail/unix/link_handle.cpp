/*
 * Created by switchblade on 2023-10-29.
 */

#include "link_handle.hpp"

namespace rod::_link
{
	inline static result<> make_symlink(int base, const char *path, const char *tgt, bool replace, const file_timeout &to) noexcept
	{
		while (replace)
		{
			const auto name = _handle::generate_unique_name();
			if (name.has_error()) [[unlikely]]
				return name.error();

			if (::symlinkat(tgt, base, name->c_str()) < 0) [[unlikely]]
			{
				if (const auto err = errno; err != EEXIST) [[unlikely]]
					return std::error_code(err, std::system_category());

				if (to.is_infinite())
					continue;
				if (const auto now = file_clock::now(); now >= to.absolute(now)) [[unlikely]]
					return std::make_error_code(std::errc::timed_out);
			}
			if (::renameat(base, name->c_str(), base, path) < 0) [[unlikely]]
				return std::error_code(errno, std::system_category());
			else
				return {};
		}
		if (::symlinkat(tgt, base, path) < 0) [[unlikely]]
			return std::error_code(errno, std::system_category());
		else
			return {};
	}

	result<link_handle> link_handle::open(const path_handle &base, path_view path, file_flags flags, open_mode mode) noexcept
	{
		if (bool(flags & (file_flags::append | file_flags::no_sparse_files | file_flags::non_blocking | file_flags::case_sensitive))) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		if (mode == open_mode::truncate) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);

		path_handle base_tmp;
		fs::path leaf;

		const auto *base_ptr = &base_tmp;
		auto dir_fd = AT_FDCWD;

		try
		{
			const auto path_parent = path.parent_path();
			leaf = fs::path(path.filename());

			if (base.native_handle() && path_parent.empty())
			{
#ifdef ROD_HAS_SYMLINK_HANDLE
				base_ptr = &base;
#else
				if (auto new_hnd = clone(base); new_hnd.has_value()) [[likely]]
					base_tmp = std::move(*new_hnd);
				else
					return new_hnd.error();
#endif
				dir_fd = base_ptr->native_handle();
			}
			else
#ifdef ROD_HAS_SYMLINK_HANDLE
			if (!path_parent.empty())
#endif
			{
				auto new_hnd = path_handle::open(base, path_parent.empty() ? "." : path_parent);
				if (new_hnd.has_error()) [[unlikely]]
					base_tmp = std::move(*new_hnd);
				else
					return new_hnd.error();

				dir_fd = base_ptr->native_handle();
			}
		}
		catch (...) { return _detail::current_error(); }

		switch (mode)
		{
		case open_mode::existing:
		{
			struct ::stat st = {};
			if (::fstatat(dir_fd, leaf.c_str(), &st, AT_SYMLINK_NOFOLLOW) < 0) [[unlikely]]
				return std::error_code(errno, std::system_category());

			break;
		}
		case open_mode::always:
		case open_mode::create:
		case open_mode::supersede:
		{
#if defined(__linux__) || defined(__APPLE__)
			constexpr auto tgt = ".";
#else
			constexpr auto tgt = "";
#endif
			auto res = make_symlink(dir_fd, leaf.c_str(), tgt, mode == open_mode::supersede, file_timeout::fallback);
			if (res.has_error() && (res.error().value() != EEXIST || mode != open_mode::create)) [[unlikely]]
				return res.error();
		}
		default:
			break;
		}

#ifdef ROD_HAS_SYMLINK_HANDLE
		const auto fd_flags = _unix::make_fd_flags(flags & ~file_flags::write, mode);
		if (auto fd = ::openat(dir_fd, leaf.c_str(), fd_flags | O_PATH | O_NOFOLLOW, 0660); fd < 0) [[unlikely]]
			return std::error_code(errno, std::system_category());
		else
			return link_handle(fd, flags);
#else
		return link_handle(std::move(base_tmp), std::move(leaf), flags);
#endif
	}
	result<link_handle> link_handle::reopen(const link_handle &other, file_flags flags) noexcept
	{
		if (bool(flags & (file_flags::append | file_flags::no_sparse_files | file_flags::non_blocking | file_flags::case_sensitive))) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);

#ifdef ROD_HAS_SYMLINK_HANDLE
		if (flags == other.flags())
			return clone(other);

		for (const auto abs_timeout = file_timeout::fallback.absolute();;)
		{
			auto curr_path = _unix::get_fd_path(other.native_handle());
			if (curr_path.has_error()) [[unlikely]]
				return curr_path.error();

			auto new_hnd = open({}, *curr_path, other.flags(), open_mode::existing);
			if (new_hnd.has_error() && new_hnd.error() != std::make_error_condition(std::errc::no_such_file_or_directory)) [[unlikely]]
				return new_hnd.error();

			constexpr auto required_query = stat::query::ino | stat::query::dev;
			auto new_st = stat(nullptr);
			auto old_st = stat(nullptr);

			auto new_st_query = get_stat(new_st, *new_hnd, required_query).value_or({});
			auto old_st_query = get_stat(old_st, other, required_query).value_or({});

			if (new_st_query == old_st_query && new_st.ino == old_st.ino && new_st.dev == old_st.dev)
				return new_hnd;
			if (const auto now = file_clock::now(); now >= abs_timeout) [[unlikely]]
				return std::make_error_code(std::errc::timed_out);
		}
#else
		return clone(other);
#endif
	}

	result<> link_handle::do_link(const path_handle &base, path_view path, bool replace, [[maybe_unused]] const file_timeout &to) noexcept
	{
#ifdef ROD_HAS_SYMLINK_HANDLE
		auto res = _unix::link_file(native_handle(), base, path, replace, bool(flags() & file_flags::unlink_stat_check), to);
		if (res.has_error()) [[unlikely]]
			return res.error();
		else
			release(*res);
#else
		try
		{
			auto rpath = path.render_null_terminated();
			auto res = _unix::link_file(native_handle(), _path.native().c_str(), base.native_handle(), rpath.c_str(), replace);
			if (res.has_error()) [[unlikely]]
				return res.error();
		}
		catch (...) { return _detail::current_error(); }
#endif

		return {};
	}
	result<> link_handle::do_relink(const path_handle &base, path_view path, bool replace, [[maybe_unused]] const file_timeout &to) noexcept
	{
#ifdef ROD_HAS_SYMLINK_HANDLE
		auto res = _unix::relink_file(native_handle(), base, path, replace, bool(flags() & file_flags::unlink_stat_check), to);
		if (res.has_error()) [[unlikely]]
			return res.error();
		else
			release(*res);
#else
		try
		{
			auto rpath = path.render_null_terminated();
			auto res = _unix::relink_file(native_handle(), _path.native().c_str(), base.native_handle(), rpath.c_str(), replace);
			if (res.has_error()) [[unlikely]]
				return res.error();

			auto path_parent = path.parent_path();
			_path = fs::path(path.filename());

			if (base.native_handle() && path_parent.empty())
			{
				if (auto new_base = clone(base); new_base.has_value()) [[likely]]
					link_handle::base() = std::move(*new_base);
				else
					return new_base.error();
			}
			else if (!path_parent.empty())
			{
				if (auto new_base = path_handle::open(base, path_parent); new_base.has_value()) [[likely]]
					link_handle::base() = std::move(*new_base);
				else
					return new_base.error();
			}
		}
		catch (...) { return _detail::current_error(); }
#endif

		return {};
	}
	result<> link_handle::do_unlink([[maybe_unused]] const file_timeout &to) noexcept
	{
#ifdef ROD_HAS_SYMLINK_HANDLE
		auto res = _unix::unlink_file(native_handle(), false, bool(flags() & file_flags::unlink_stat_check), to);
		if (res.has_error()) [[unlikely]]
			return res.error();
		else
			release(*res);
#else
		auto res = _unix::unlink_file(native_handle(), _path.native().c_str(), false);
		if (res.has_error()) [[unlikely]]
			return res.error();

		close(link_handle::base());
		_path.clear();
#endif

		return {};
	}

	read_some_result_t<link_handle> link_handle::do_read_some(io_request<read_some_t> &&req, const file_timeout &) noexcept
	{
		if (!bool(flags() & file_flags::read)) [[unlikely]]
			return std::make_error_code(std::errc::bad_file_descriptor);
		if (req.buffs.empty()) [[unlikely]]
			return std::move(req.buffs);

		for (std::size_t path_len = PATH_MAX;; path_len += PATH_MAX)
		{
			if (req.buffs._buff_len < path_len)
			{
				void *old_buff = req.buffs._buff.release(), *new_buff;
				if (old_buff != nullptr)
					new_buff = std::realloc(old_buff, path_len + 1);
				else
					new_buff = std::malloc(path_len + 1);
				if (new_buff == nullptr) [[unlikely]]
					return (std::free(old_buff), std::make_error_code(std::errc::not_enough_memory));

				req.buffs._buff.reset(static_cast<char *>(new_buff));
				req.buffs._buff_len = path_len;
			}

#ifdef ROD_HAS_SYMLINK_HANDLE
			auto len = ::readlinkat(native_handle(), "", req.buffs._buff.get(), req.buffs._buff_len);
			if (std::size_t(len) == req.buffs._buff_len)
				len = ::readlinkat(native_handle(), "", req.buffs._buff.get(), req.buffs._buff_len + 1);
#else
			auto len = ::readlinkat(native_handle(), _path.c_str(), req.buffs._buff.get(), req.buffs._buff_len);
			if (std::size_t(len) == req.buffs._buff_len)
				len = ::readlinkat(native_handle(), _path.c_str(), req.buffs._buff.get(), req.buffs._buff_len + 1);
#endif
			if (len < 0) [[unlikely]]
				return std::error_code(errno, std::system_category());

			if (std::size_t(len) <= (path_len = req.buffs._buff_len)) [[likely]]
				path_len = std::size_t(len);
			else
				continue;

			for (auto path_data = std::span(req.buffs._buff.get(), path_len); auto &dst : req.buffs)
			{
				/* If the path buffer is empty, clear the destination buffer. */
				if (path_data.empty())
				{
					/* Terminate the buffer before truncation to make path_view more efficient. */
					if ((dst._is_terminated = !dst._buff.empty()))
						dst._buff.front() = '\0';

					dst._buff = dst._buff.subspan(0, 0);
					continue;
				}

				/* Use the internal buffer if the destination buffer is empty. */
				dst._buff = dst._buff.empty() ? path_data : dst._buff;
				auto copy_len = std::min(dst._buff.size(), path_len);

				if ((dst._is_terminated = dst._buff.size() >= copy_len))
					dst._buff[copy_len] = '\0';
				if (dst._buff.data() != path_data.data())
					std::copy_n(path_data.data(), copy_len, dst._buff.data());

				/* Advance the path buffer & truncate the destination buffer. */
				dst._buff = dst._buff.subspan(0, copy_len);
				path_data = path_data.subspan(copy_len);
			}
			return std::move(req.buffs);
		}
	}
	write_some_result_t<link_handle> link_handle::do_write_some(io_request<write_some_t> &&req, const file_timeout &to) noexcept
	{
		if (!bool(flags() & file_flags::write)) [[unlikely]]
			return std::make_error_code(std::errc::bad_file_descriptor);
		if (req.buffs._type != link_type::symbolic) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);

		const auto abs_timeout = to.is_infinite() ? to : to.absolute();
		auto &buff_len = req.buffs._buff_len;
		auto &buff_mem = req.buffs._buff;
		auto buff_pos = std::size_t(0);

		auto path_len = std::accumulate(req.buffs.begin(), req.buffs.end(), std::size_t(0), [](auto i, auto &b) { return i + b.size(); });
		if (path_len == std::numeric_limits<std::size_t>::max()) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);

		/* Reserve a buffer large enough for all path components and reparse_data_buffer. */
		if (const auto new_len = path_len + 1; req.buffs._buff_len < new_len)
		{
			void *old_mem = req.buffs._buff.release(), *new_mem;
			if (old_mem != nullptr)
				new_mem = std::realloc(old_mem, new_len);
			else
				new_mem = std::malloc(new_len);
			if (new_mem == nullptr) [[unlikely]]
				return (std::free(old_mem), std::make_error_code(std::errc::not_enough_memory));

			buff_mem.reset(static_cast<char *>(new_mem));
			buff_len = new_len;
		}

		for (const auto &src : req.buffs)
		{
			std::copy_n(src.data(), src.size(), buff_mem.get() + buff_pos);
			buff_pos += src.size();
		}
		buff_mem[path_len] = '\0';

#ifdef ROD_HAS_SYMLINK_HANDLE
		auto base_hnd = native_handle_type(AT_FDCWD);
		auto leaf_str = std::string();

		auto base = _unix::get_fd_base(native_handle(), &leaf_str, bool(flags() & file_flags::unlink_stat_check), to);
		if (base.has_value()) [[likely]]
			base_hnd = base->native_handle();
		else
			return base.error();
#else
		auto base_hnd = base().native_handle() ? base().native_handle() : native_handle_type(AT_FDCWD);
		auto &leaf_str = _path.native();
#endif

		if (auto res = make_symlink(base_hnd, leaf_str.c_str(), buff_mem.get(), true, abs_timeout); res.has_error()) [[unlikely]]
			return res.error();

#ifdef ROD_HAS_SYMLINK_HANDLE
		{
			/* Mask out unlink-on-close for the current handle, since it is being replaced. */
			auto old_hnd = release();
			old_hnd.flags &= ~std::uint32_t(file_flags::unlink_on_close);
			release(old_hnd);

			/* Open the newly linked handle. */
			auto new_hnd = open(*base, leaf_str, flags(), open_mode::existing);
			if (new_hnd.has_error()) [[unlikely]]
				return new_hnd.error();
			else
				swap(*new_hnd);
		}
#endif
		return std::move(req.buffs);
	}
}

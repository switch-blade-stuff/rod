/*
 * Created by switchblade on 2023-09-15.
 */

#include "path_util.hpp"
#include "file_handle.hpp"
#include "link_handle.hpp"

namespace rod::fs
{
	result<bool> equivalent(path_view a, path_view b) noexcept
	{
		dev_t dev_a, dev_b;
		ino_t ino_a, ino_b;
		stat st;

		if (auto res = get_stat(st, {}, a, stat::query::dev | stat::query::ino); res.has_error()) [[unlikely]]
			return {in_place_error, res.error()};
		else if (*res != (stat::query::dev | stat::query::ino))
			return false;
		dev_a = st.dev;
		ino_a = st.ino;

		if (auto res = get_stat(st, {}, b, stat::query::dev | stat::query::ino); res.has_error()) [[unlikely]]
			return {in_place_error, res.error()};
		else if (*res != (stat::query::dev | stat::query::ino))
			return false;
		dev_b = st.dev;
		ino_b = st.ino;

		return dev_a == dev_b && ino_a == ino_b;
	}

	result<path> relative(path_view path) noexcept
	{
		if (auto curr = current_path(); curr.has_value()) [[likely]]
			return relative(path, *curr);
		else
			return curr.error();
	}
	result<path> proximate(path_view path) noexcept
	{
		if (auto curr = current_path(); curr.has_value()) [[likely]]
			return proximate(path, *curr);
		else
			return curr.error();
	}
	result<path> relative(path_view path, path_view base) noexcept
	{
		auto path_canon = weakly_canonical(path);
		if (path_canon.has_error()) [[unlikely]]
			return path_canon.error();

		auto base_canon = weakly_canonical(base);
		if (base_canon.has_error()) [[unlikely]]
			return base_canon.error();

		try { return path_canon->lexically_relative(*base_canon); }
		catch (...) { return _detail::current_error(); }
	}
	result<path> proximate(path_view path, path_view base) noexcept
	{
		auto path_canon = weakly_canonical(path);
		if (path_canon.has_error()) [[unlikely]]
			return path_canon.error();

		auto base_canon = weakly_canonical(base);
		if (base_canon.has_error()) [[unlikely]]
			return base_canon.error();

		try { return path_canon->lexically_proximate(*base_canon); }
		catch (...) { return _detail::current_error(); }
	}

	inline static result<directory_handle> try_create_dir(const path_handle &base, path_view path) noexcept
	{
		/* Try to create a new directory. If failed, try to get the path's type. If it is a directory, return success. */
		if (auto res = directory_handle::open(base, path, file_flags::read, open_mode::create); res.has_value())
			return std::move(*res);

		auto st = stat(nullptr);
		if (auto res = get_stat(st, base, path, stat::query::type); res.has_error() || st.type != file_type::directory)
			return res.error_or(std::make_error_code(std::errc::file_exists));
		else
			return {};
	}
	template<typename Str>
	inline static result<std::size_t> do_create(const path_handle &base, Str view, auto fmt) noexcept
	{
		const auto sep_pred = [fmt](auto ch) { return is_separator(ch, fmt); };
		const auto root_end = std::find_if_not(view.begin() + root_name_size(view, fmt), view.end(), sep_pred);
		auto cmp_off = std::distance(view.begin(), root_end);
		auto cmp_pos = view.begin() + cmp_off;

#ifdef ROD_WIN32
		/* Remove drive leter if there is a \\?\ prefix. */
		if (cmp_off && view.size() - cmp_off >= 3 && root_end[1] == ':' && _path::is_drive_letter(*root_end) && is_separator(root_end[2], fmt))
			cmp_pos += 2;
#endif

		/* Go through every component and create the directory. Capture only the relevant error. */
		std::error_code errs[2] = {};
		std::size_t new_dirs = 0;
		directory_handle dir_hnd;
		auto *base_ptr = &base;

		for (auto path_pos = view.begin(); cmp_pos != view.end();)
		{
			const auto cmp_end = std::find_if(std::find_if_not(cmp_pos, view.end(), sep_pred), view.end(), sep_pred);
			const auto path = path_view(std::to_address(path_pos), std::size_t(cmp_end - path_pos), false, fmt);
			cmp_pos = std::find_if_not(cmp_end, view.end(), sep_pred);

			auto create_res = try_create_dir(*base_ptr, path);
			if (create_res.has_error()) [[unlikely]]
			{
				errs[0] = create_res.error();
				if (errs[0] != std::make_error_condition(std::errc::no_such_file_or_directory))
					errs[1] = errs[0];
			}

			if (create_res->is_open())
			{
				base_ptr = &implicit_cast<const path_handle &>(dir_hnd);
				dir_hnd = std::move(*create_res);
				path_pos = cmp_pos;
				new_dirs++;
			}
		}
		if (errs[0]) [[unlikely]]
			return errs[1] ? errs[1] : errs[0];
		else
			return new_dirs;
	}
	template<typename Str> requires decays_to_same<std::ranges::range_value_t<Str>, std::byte>
	inline static result<std::size_t> do_create(const path_handle &base, Str view, auto fmt) noexcept
	{
		return try_create_dir(base, path_view(view.data(), view.size(), false, fmt)).transform_value([](auto &hnd) { return std::size_t(hnd.is_open()); });
	}

	result<std::size_t> create_directories(const path_handle &base, path_view path) noexcept
	{
		if (!path.empty()) [[likely]]
			return visit([&](auto v) { return do_create(base, v, path.format()); }, path);
		else
			return std::make_error_code(std::errc::no_such_file_or_directory);
	}

	inline constexpr auto copy_stats_mask = stat::query::type | stat::query::dev | stat::query::ino | stat::query::perm;

	inline static result<std::size_t> do_copy(const path_handle &, path_view, const path_handle &, path_view, copy_mode, const file_timeout &, bool) noexcept;
	inline static result<std::size_t> do_copy(const path_handle &, path_view, stat, const path_handle &, path_view, stat, copy_mode, const file_timeout &, bool) noexcept;

	inline static result<std::size_t> copy_symlink(const path_handle &src_base, path_view src_path, stat , const path_handle &dst_base, path_view dst_path, stat dst_st, copy_mode mode, const file_timeout &to) noexcept
	{
		if (bool(mode & copy_mode::ignore_links))
			return 0;

		const auto transform = [](auto &&...) noexcept -> std::size_t { return 1; };
		const auto overwrite = bool(mode & copy_mode::overwrite);

		if (dst_st.type != file_type::none || (overwrite && dst_st.type != file_type::symlink))
			return std::make_error_code(std::errc::file_exists);

		auto src_hnd = link_handle::open(src_base, src_path, file_flags::read, open_mode::existing);
		if (src_hnd.has_error()) [[unlikely]]
			return src_hnd.error();

		auto src_buff = link_handle::io_buffer<read_some_t>();
		auto src_res = read_some(*src_hnd, {{&src_buff, 1}}, to);
		if (src_res.has_error()) [[unlikely]]
			return src_res.error();

		auto dst_buff = link_handle::io_buffer<write_some_t>(src_buff.data(), src_buff.size());
		if (auto dst_hnd = link_handle::open(dst_base, dst_path, file_flags::write, overwrite ? open_mode::always : open_mode::create); dst_hnd.has_value())
			return write_some(*dst_hnd, {{std::move(*src_res), &dst_buff, 1, src_res->type()}}, to).transform_value(transform);
		else if (dst_hnd.error() != std::make_error_condition(std::errc::file_exists) || overwrite)
			return dst_hnd.error();
		else
			return 0;
	}
	inline static result<std::size_t> copy_regular(const path_handle &src_base, path_view src_path, stat src_st, const path_handle &dst_base, path_view dst_path, stat dst_st, copy_mode mode, const file_timeout &to) noexcept
	{
		constexpr auto src_caching = file_caching::meta | file_caching::force_precache | file_caching::read;
		constexpr auto dst_caching = file_caching::meta | file_caching::force_precache | file_caching::write;

		if (!bool(mode & copy_mode::files))
			return 0;

		const auto transform = [](auto &&...) noexcept -> std::size_t { return 1; };
		const auto overwrite = bool(mode & copy_mode::overwrite);

		auto src_hnd = file_handle::open(src_base, src_path, file_flags::read, open_mode::existing, src_caching);
		if (src_hnd.has_error()) [[unlikely]]
			return src_hnd.error();

		if (bool(mode & copy_mode::create_hardlinks))
		{
			if (auto link_res = link(*src_hnd, dst_base, dst_path, overwrite, to); link_res.has_value())
				return 1;
			else if (link_res.error() != std::make_error_condition(std::errc::file_exists) || overwrite)
				return link_res.error();
		}
		else if (bool(mode & copy_mode::create_symlinks))
		{
			if (dst_st.type != file_type::none || (overwrite && dst_st.type != file_type::symlink))
				return std::make_error_code(std::errc::file_exists);

			auto full_path = to_object_path(*src_hnd);
			if (full_path.has_error()) [[unlikely]]
				return full_path.error();

			auto dst_buff = link_handle::io_buffer<write_some_t>(full_path->native());
			if (auto dst_hnd = link_handle::open(dst_base, dst_path, file_flags::write ^ file_flags::append, overwrite ? open_mode::always : open_mode::create); dst_hnd.has_value())
				return write_some(*dst_hnd, {{&dst_buff, 1, link_type::symbolic}}, to).transform_value(transform);
			else if (dst_hnd.error() != std::make_error_condition(std::errc::file_exists) || overwrite)
				return dst_hnd.error();
		}
		else
		{
			if (dst_st.type != file_type::none || (overwrite && dst_st.type != file_type::regular))
				return std::make_error_code(std::errc::file_exists);

#if defined(ROD_WIN32) && 0
			const auto dst_perm = src_st.perm | file_perm::write;
#else
			const auto dst_perm = src_st.perm;
#endif

			if (auto dst_hnd = file_handle::open(dst_base, dst_path, file_flags::attr_read | file_flags::write, overwrite ? open_mode::supersede : open_mode::create, dst_caching, dst_perm); dst_hnd.has_value())
				return clone_extents_to(*src_hnd, {.extent = {-1, -1}, .dst = *dst_hnd}, to).transform_value(transform);
			else if (dst_hnd.error() != std::make_error_condition(std::errc::file_exists) || overwrite)
				return dst_hnd.error();
		}
		return 0;
	}
	inline static result<std::size_t> copy_directory(const path_handle &src_base, path_view src_path, stat, const path_handle &dst_base, path_view dst_path, stat dst_st, copy_mode mode, const file_timeout &to, bool enter) noexcept
	{
		if (!bool(mode & copy_mode::directories))
			return 0;

		const auto nofollow = !bool(mode & copy_mode::follow_links);
		const auto overwrite = bool(mode & copy_mode::overwrite);
		const auto recursive = bool(mode & copy_mode::recursive);

		if (dst_st.type != file_type::none || (overwrite && dst_st.type != file_type::directory))
			return std::make_error_code(std::errc::file_exists);

		/* Create destination first, source is ignored unless we need to copy contents. */
		auto dst_hnd = directory_handle::open(dst_base, dst_path, file_flags::write, overwrite ? open_mode::supersede : open_mode::create);
		if (dst_hnd.has_error()) [[unlikely]]
			return dst_hnd.error();
		else if (!enter)
			return 1;

		auto src_hnd = directory_handle::open(src_base, src_path, file_flags::read, open_mode::existing);
		if (src_hnd.has_error()) [[unlikely]]
			return src_hnd.error();

		auto buff = directory_handle::io_buffer<read_some_t>(copy_stats_mask);
		auto seq = directory_handle::io_buffer_sequence<read_some_t>(&buff, 1);
		auto num = std::size_t(1);

		for (;;)
		{
			auto read_res = read_some(*src_hnd, {.buffs = std::move(seq), .resume = true}, to);
			if (read_res.has_error()) [[unlikely]]
				return read_res.error();
			else if (!read_res->first.empty())
				seq = std::move(read_res->first);
			else
				break;

			auto [src_entry_st, src_entry_mask] = seq.front().st();
			auto src_entry_path = seq.front().path();

			/* Get the stats for the source & destination entries. */
			if ((src_entry_mask & copy_stats_mask) != copy_stats_mask) [[unlikely]]
			{
				if (auto res = get_stat(src_entry_st, *src_hnd, src_entry_path, copy_stats_mask, nofollow); res.has_error()) [[unlikely]]
					return res.error();
				else if ((*res & copy_stats_mask) != copy_stats_mask) [[unlikely]]
					return std::make_error_code(std::errc::not_supported);
			}
			auto dst_entry_st = stat(nullptr);
			if (auto res = get_stat(dst_entry_st, *dst_hnd, src_entry_path, copy_stats_mask, true); res.has_error()) [[unlikely]]
			{
				/* Destination file may not exist, in which case we will create it instead of failing. */
				if (auto err = res.error(); err != std::make_error_condition(std::errc::no_such_file_or_directory)) [[unlikely]]
					return err;
			}
			else if ((*res & copy_stats_mask) != copy_stats_mask) [[unlikely]]
				return std::make_error_code(std::errc::not_supported);

			/* Recursively copy the directory entry. */
			auto copy_res = do_copy(*src_hnd, src_entry_path, src_entry_st, *dst_hnd, src_entry_path, dst_entry_st, mode, to, recursive);
			if (copy_res.has_error()) [[unlikely]]
				return copy_res.error();
			else
				num += *copy_res;
			if (read_res->second)
				break;
		}
		return num;
	}

	inline static result<std::size_t> do_copy(const path_handle &src_base, path_view src_path, const path_handle &dst_base, path_view dst_path, copy_mode mode, const file_timeout &to, bool enter) noexcept
	{
		/* Assert mutually exclusive flags. */
		if (bool(mode & copy_mode::create_hardlinks) && bool(mode & (copy_mode::create_symlinks | copy_mode::directories)))
			return std::make_error_code(std::errc::invalid_argument);

		auto src_st = stat(nullptr);
		if (auto res = get_stat(src_st, src_base, src_path, copy_stats_mask, !bool(mode & copy_mode::follow_links)); res.has_error()) [[unlikely]]
			return res.error();
		else if ((*res & copy_stats_mask) != copy_stats_mask) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);

		auto dst_st = stat(nullptr);
		if (auto res = get_stat(dst_st, dst_base, dst_path, copy_stats_mask, true); res.has_error()) [[unlikely]]
		{
			/* Destination file may not exist, in which case we will create it instead of failing. */
			if (auto err = res.error(); err != std::make_error_condition(std::errc::no_such_file_or_directory)) [[unlikely]]
				return err;
		}
		else if ((*res & copy_stats_mask) != copy_stats_mask) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);

		return do_copy(src_base, src_path, src_st, dst_base, dst_path, dst_st, mode, to, enter);
	}
	inline static result<std::size_t> do_copy(const path_handle &src_base, path_view src_path, stat src_st, const path_handle &dst_base, path_view dst_path, stat dst_st, copy_mode mode, const file_timeout &to, bool enter) noexcept
	{
		/* Make sure file types are valid. */
		if (src_st.type != file_type::regular && src_st.type != file_type::symlink && src_st.type != file_type::directory) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		/* There is nothing to do if the files are equivalent. */
		if (src_st.dev != dst_st.dev || src_st.ino != dst_st.ino)
		{
			if (src_st.type == file_type::regular)
				return copy_regular(src_base, src_path, src_st, dst_base, dst_path, dst_st, mode, to);
			if (src_st.type == file_type::symlink)
				return copy_symlink(src_base, src_path, src_st, dst_base, dst_path, dst_st, mode, to);
			if (src_st.type == file_type::directory)
				return copy_directory(src_base, src_path, src_st, dst_base, dst_path, dst_st, mode, to, enter);
		}
		return 0;
	}

	result<std::size_t> copy(const path_handle &src_base, path_view src_path, const path_handle &dst_base, path_view dst_path, copy_mode mode, const file_timeout &to) noexcept
	{
		return do_copy(src_base, src_path, dst_base, dst_path, mode, to != file_timeout() ? to.absolute() : file_timeout(), false);
	}
	result<std::size_t> copy_all(const path_handle &src_base, path_view src_path, const path_handle &dst_base, path_view dst_path, copy_mode mode, const file_timeout &to) noexcept
	{
		return do_copy(src_base, src_path, dst_base, dst_path, mode, to != file_timeout() ? to.absolute() : file_timeout(), true);
	}
}

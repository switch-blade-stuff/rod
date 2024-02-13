/*
 * Created by switchblade on 2023-08-31.
 */

#include "path_util.hpp"

rod::result<std::string> rod::_unix::expand_path(std::string_view str) noexcept
{
	if (str.find('$') == std::string_view::npos)
		return str;

	try
	{
		auto buff = std::string();
#ifdef USE_WORDEXP
		auto exp = ::wordexp_t();
		buff = str;

		switch (::wordexp(buff.data(), &exp, WRDE_UNDEF))
		{
		case WRDE_NOSPACE:
			return std::make_error_code(std::errc::not_enough_memory);
		case WRDE_BADCHAR:
		case WRDE_SYNTAX:
			return std::make_error_code(std::errc::invalid_argument);
		case WRDE_BADVAL:
			return std::make_error_code(std::errc::not_supported);
		default: [[likely]]
			break;
		}

		const auto g = defer_invoke([&] { ::wordfree(&exp); });
		buff.clear();

		for (std::size_t i = 0; i < exp.we_wordc; ++i)
			buff += exp.we_wordv[i];
#else
		auto cmd = std::string("echo \"", 7 + str.size());
		(cmd += str) += '\"';
		buff.resize(4096);

		auto pid = ::pid_t();
		int link[2] = {};

		if (::pipe(link) == -1) [[unlikely]]
			return std::error_code(errno, std::system_category());
		if ((pid = ::fork()) == -1) [[unlikely]]
			return std::error_code(errno, std::system_category());

		if (pid == ::pid_t(0))
		{
			if (::dup2(link[1], STDOUT_FILENO) == STDOUT_FILENO) [[likely]]
			{
				::close(link[0]);
				::close(link[1]);
				::execl("/bin/sh", cmd.data(), static_cast<char *>(nullptr));
			}
			const auto err = errno;
			int err_msg[2] = {};

			std::memcpy(err_msg + 1, &err, 4);
			::write(link[1], err_msg, 8);
			std::_Exit(err);
		}
		else
		{
			const auto g = defer_invoke([&] { ::close(link[0]); });
			::close(link[1]);

			if (::read(link[0], buff.data(), buff.size()) == 8 && buff[0] == '\0') [[unlikely]]
				return std::error_code(reinterpret_cast<int *>(buff.data())[1], std::system_category());
			else
				buff.resize(::strlen(buff.data()));
		}
#endif
		return buff;
	}
	catch (...) { return _detail::current_error(); }
}

namespace rod::fs
{
	result<path> current_path() noexcept
	{
		auto buff_size = std::size_t(4096);
		auto buff_data = make_malloc_ptr_for_overwrite<char[]>(buff_size);

		for (;;)
		{
			if (buff_data.get() == nullptr) [[unlikely]]
				return std::make_error_code(std::errc::not_enough_memory);
			if (::getcwd(buff_data.get(), buff_size) == buff_data.get())
				return buff_data.get();

			const auto err = errno;
			if (err != ERANGE) [[unlikely]]
				return std::error_code(err, std::system_category());

			const auto old_data = buff_data.release();
			const auto new_data = std::realloc(old_data, buff_size *= 2);

			if (new_data != nullptr) [[likely]]
				buff_data.reset(static_cast<char *>(new_data));
			else
				std::free(old_data);
		}
	}
	result<void> current_path(path_view path) noexcept
	{
		try
		{
			if (const auto rpath = path.render_null_terminated(); ::chdir(rpath.c_str()) != 0) [[unlikely]]
				return std::error_code(errno, std::system_category());
			else
				return {};
		}
		catch (...) { return _detail::current_error(); }
	}

	result<bool> exists(path_view path) noexcept
	{
		try
		{
			/* If access to the file is denied, check if the parent directory exists. */
			auto rpath = path.render_null_terminated();
			auto error = 0;

			if (::access(rpath.c_str(), F_OK) && ((error = errno) != EACCES || !path.has_parent_path())) [[unlikely]]
				return {in_place_error, std::error_code(error, std::system_category())};

			rpath = path.parent_path().render_null_terminated();
			error = 0;

			if (::access(rpath.c_str(), F_OK) && (error = errno) != ENOENT) [[unlikely]]
				return result{in_place_error, std::error_code(error, std::system_category())};
			else
				return error != ENOENT;
		}
		catch (...) { return {in_place_error, _detail::current_error()}; }
	}
	result<path> absolute(path_view path) noexcept
	{
		if (path.empty()) [[unlikely]]
			return path;

		try { return current_path().into_value([&](auto &&curr) { return curr / path; }); }
		catch (...) { return _detail::current_error(); }
	}

	result<path> canonical(path_view path) noexcept
	{
		try
		{
			if (const auto rpath = malloc_ptr<char[]>(::realpath(path.render_null_terminated().c_str(), nullptr)); !rpath) [[unlikely]]
				return std::error_code(errno, std::system_category());
			else
				return rpath.get();
		}
		catch (...) { return _detail::current_error(); }
	}
	result<path> weakly_canonical(path_view path) noexcept
	{
		auto res = canonical(path);
		if (res.has_value() || res.error().value() != ENOENT)
			return res;

		try
		{
			const auto norm = fs::path(path).lexically_normal();
			const auto rel = path_view(norm).relative_path();
			res.emplace_value(norm.root_path());
			bool canonize = true;

			/* Convert as many valid components as possible. */
			for (auto &comp : rel)
			{
				res->append(comp);
				if (!canonize)
					continue;

				if (auto tmp = canonical(*res); tmp.has_value()) [[likely]]
					res = std::move(tmp);
				else if (tmp.error().value() == ENOENT)
					canonize = false;
				else
					return tmp;
			}
			return res;
		}
		catch (...) { return _detail::current_error(); }
	}

	inline static result<std::size_t> do_remove(const path_handle &base, path_view path, bool isdir, const file_timeout &) noexcept
	{
		const auto rpath = path.render_null_terminated();
		if (!::unlinkat(base.native_handle(), rpath.c_str(), isdir ? AT_REMOVEDIR : 0)) [[likely]]
			return 1;
		if (const auto err = errno; err != ENOENT)
			return std::error_code(err, std::system_category());
		else
			return 0;
	}
	inline static result<std::size_t> do_remove_directory(const path_handle &base, path_view path, const file_timeout &to) noexcept
	{
		std::size_t removed = 0;
		{
			auto hnd = directory_handle::open(base, path, file_flags::readwrite);
			if (hnd.has_error()) [[unlikely]]
			{
				if (auto err = hnd.error(); err != std::make_error_condition(std::errc::no_such_file_or_directory))
					return err;
				else
					return 0;
			}

			auto ent = std::array{directory_handle::io_buffer<read_some_t>()};
			auto seq = directory_handle::io_buffer_sequence<read_some_t>(ent);

			for (;;)
			{
				auto read_res = read_some(*hnd, {.buffs = std::move(seq), .resume = false}, to);
				if (read_res.has_error()) [[unlikely]]
					return read_res.error();
				else if (!read_res->first.empty())
					seq = std::move(read_res->first);
				else
					break;

				auto [leaf_st, mask] = seq.front().st();
				if (!bool(mask & stat::query::type)) [[unlikely]]
					continue; /* Should never happen. */


				/* Recursively call remove on the subdirectory. */
				result<std::size_t> remove_res;
				if (leaf_st.type == file_type::directory)
					remove_res = do_remove_directory(*hnd, seq.front().path(), to);
				else
					remove_res = do_remove(*hnd, seq.front().path(), false, to);

				if (remove_res.has_value()) [[likely]]
					removed += *remove_res;
				else
					return remove_res;
				if (read_res->second)
					break;
			}
		}

		/* Finally, try to remove the directory itself. */
		constexpr std::size_t spin_max = 10;
		for (std::size_t i = 0;; ++i)
		{
			auto res = do_remove(base, path, true, to);
			if (res.has_value()) [[likely]]
				return removed + *res;

			/* EBUSY - Directory is being used by something else. */
			if (file_clock::now() >= to.absolute()) [[unlikely]]
				return std::make_error_code(std::errc::timed_out);
			if (auto err = res.error(); err.value() != EBUSY)
				return err;
			if (i > spin_max) [[unlikely]]
				::sched_yield();
		}
	}

	result<std::size_t> remove(const path_handle &base, path_view path, const file_timeout &to) noexcept
	{
		const auto abs_timeout = to != file_timeout() ? to.absolute() : file_timeout();

		auto st = stat(nullptr);
		if (auto res = get_stat(st, base, path, stat::query::type); res.has_error()) [[unlikely]]
		{
			if (auto err = res.error(); err != std::make_error_condition(std::errc::no_such_file_or_directory))
				return err;
			else
				return 0;
		}
		return do_remove(base, path, st.type == file_type::directory, abs_timeout);
	}
	result<std::size_t> remove_all(const path_handle &base, path_view path, const file_timeout &to) noexcept
	{
		const auto abs_timeout = to != file_timeout() ? to.absolute() : file_timeout();

		auto st = stat(nullptr);
		if (auto res = get_stat(st, base, path, stat::query::type); res.has_error()) [[unlikely]]
		{
			if (auto err = res.error(); err != std::make_error_condition(std::errc::no_such_file_or_directory))
				return err;
			else
				return 0;
		}
		if (auto res = do_remove(base, path, st.type == file_type::directory, abs_timeout); res.has_error() && st.type == file_type::directory)
			return do_remove_directory(base, path, abs_timeout);
		else
			return res;
	}
}

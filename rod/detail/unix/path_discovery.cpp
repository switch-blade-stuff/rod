/*
 * Created by switchblade on 2023-08-20.
 */

#include "../file_handle.hpp"
#include "path_discovery.hpp"

rod::result<std::string> rod::_unix::read_xdg_path(std::string_view name) noexcept
{
#if defined(__linux__) || defined(__FreeBSD__)
	auto dirs_file = fs::file_handle::open({}, "~/.config/user-dirs.dirs");
	if (dirs_file.has_error()) [[unlikely]]
		return dirs_file.error();

	auto st = stat(nullptr);
	if (auto res = get_stat(st, *dirs_file, stat::query::size); res.has_error()) [[unlikely]]
		return res.error();

	const auto buff_mem = make_malloc_ptr_for_overwrite<char[]>(st.size + 1);
	if (buff_mem.get() == nullptr) [[unlikely]]
		return std::make_error_code(std::errc::not_enough_memory);

	auto buff_raw = as_bytes(buff_mem.get(), buff_mem.get() + st.size);
	if (auto res = read_some_at(*dirs_file, {{&buff_raw, 1}}); res.has_error()) [[unlikely]]
		return res.error();

	/* Parse until we get the required key. */
	const auto str = std::string_view(buff_mem.get(), st.size);

	const auto name_pos = str.find(name);
	if (name_pos == std::string_view::npos) [[unlikely]]
		return std::make_error_code(std::errc::not_supported);

	const auto path_beg = str.find_first_not_of('\"', name_pos + name.size());
	if (path_beg == std::string_view::npos) [[unlikely]]
		return std::make_error_code(std::errc::not_supported);

	const auto path_end = str.find_first_of('\"', path_beg + 1);
	return expand_path(str.substr(path_beg + 1, path_end - path_beg - 1));
#else
	return std::make_error_code(std::errc::not_supported);
#endif
}

namespace rod::_detail
{
	using namespace _unix;

	result<> find_temp_dirs(std::vector<fs::discovered_path> &dirs) noexcept
	{
		try
		{
			if (const auto uid = ::getuid(), euid = ::geteuid(); uid == euid) [[likely]]
			{
				if (const auto env_path = std::string_view(::getenv(TMPDIR_ENV)); !env_path.empty()) [[likely]]
					dirs.push_back({env_path, fs::discovery_source::environment});

#ifndef __APPLE__
				dirs.push_back({"/run/user/" + std::to_string(uid), fs::discovery_source::system});
#endif
				dirs.push_back({"/var/tmp", fs::discovery_source::system});
				dirs.push_back({"/tmp", fs::discovery_source::system});
			}

			dirs.push_back({"/var/tmp", fs::discovery_source::fallback});
			dirs.push_back({"/tmp", fs::discovery_source::fallback});
			return {};
		}
		catch (...) { return _detail::current_error(); }
	}
	result<> find_data_dirs(std::vector<fs::discovered_path> &dirs) noexcept
	{
		try
		{
			if (const auto uid = ::getuid(), euid = ::geteuid(); uid == euid) [[likely]]
			{
				for_each_in_env_list(::getenv("XDG_DATA_DIRS"), [&](auto value)
				{
					dirs.push_back({value, fs::discovery_source::environment});
					return true;
				});
				if (const auto value = std::string_view(::getenv("XDG_DATA_HOME")); !value.empty()) [[likely]]
					dirs.push_back({value, fs::discovery_source::environment});
			}

			dirs.push_back({"/usr/local/share", fs::discovery_source::fallback});
			dirs.push_back({"/usr/share", fs::discovery_source::fallback});
			return {};
		}
		catch (...) { return _detail::current_error(); }
	}
	result<> find_state_dirs(std::vector<fs::discovered_path> &dirs) noexcept
	{
		try
		{
			if (const auto uid = ::getuid(), euid = ::geteuid(); uid == euid) [[likely]]
			{
				for_each_in_env_list(::getenv("XDG_DATA_DIRS"), [&](auto value)
				{
					dirs.push_back({value, fs::discovery_source::environment});
					return true;
				});
				if (const auto value = std::string_view(::getenv("XDG_STATE_HOME")); !value.empty()) [[likely]]
					dirs.push_back({value, fs::discovery_source::environment});
				if (const auto value = std::string_view(::getenv("XDG_DATA_HOME")); !value.empty()) [[likely]]
					dirs.push_back({value, fs::discovery_source::environment});
				if (const auto value = std::string_view(::getenv("HOME")); !value.empty()) [[likely]]
					dirs.push_back({fs::path(value) / ".local/state", fs::discovery_source::system});
				else
					dirs.push_back({"~/.local/state", fs::discovery_source::system});
			}

			dirs.push_back({"/usr/local/share", fs::discovery_source::fallback});
			dirs.push_back({"/usr/share", fs::discovery_source::fallback});
			return {};
		}
		catch (...) { return _detail::current_error(); }
	}
	result<> find_config_dirs(std::vector<fs::discovered_path> &dirs) noexcept
	{
		try
		{
			if (const auto uid = ::getuid(), euid = ::geteuid(); uid == euid) [[likely]]
			{
				for_each_in_env_list(::getenv("XDG_CONFIG_DIRS"), [&](auto value)
				{
					dirs.push_back({value, fs::discovery_source::environment});
					return true;
				});
				if (const auto value = std::string_view(::getenv("XDG_CONFIG_HOME")); !value.empty()) [[likely]]
					dirs.push_back({value, fs::discovery_source::environment});
			}

			dirs.push_back({"/etc/xdg", fs::discovery_source::fallback});
			return {};
		}
		catch (...) { return _detail::current_error(); }
	}

	namespace /* HACK: Capturing argv[0] via a manual entry into .init_array for later use with find_install_dir. */
	{
		int capture_argv(int argc, char **argv, char **) noexcept;

#if defined(__APPLE__) || defined(__MACH__)
		__attribute__((used, section("__DATA,__mod_init_func"))) auto ctr = &capture_argv;
#else
		__attribute__((used, section(".init_array"))) auto ctr = &capture_argv;
#endif
		std::string argv0_captured;

		int capture_argv(int argc, char **argv, char **) noexcept
		{
			if (argc > 0) [[likely]]
				argv0_captured = argv[0];
			return 0;
		}
	}

	result<fs::path> find_install_dir() noexcept
	{
#ifdef PROC_SELF
		malloc_ptr<char[]> buff = {};
		std::size_t buff_size = 4096;

		buff = make_malloc_ptr_for_overwrite<char[]>(buff_size);
		if (buff.get() == nullptr) [[unlikely]]
			return std::make_error_code(std::errc::not_enough_memory);

		if ((buff_size = std::size_t(readlink(PROC_SELF, buff.get(), buff_size))) != 0) [[likely]]
			return std::string_view(buff.get(), buff_size);
		if (const auto err = errno; err == ENOMEM || err == ENAMETOOLONG)
			return std::make_error_code(std::errc::not_enough_memory);
		else if (err != ENOENT && err != EINVAL)
			return std::error_code(err, std::system_category());
#elif defined(__FreeBSD__) || defined(__DragonFly__)
		const int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
		if (std::size_t n = 0; !::sysctl(&mib, nullptr, &n, nullptr, 0)) [[likely]]
		{
			auto buff = make_malloc_ptr_for_overwrite<char[]>(n);
			if (buff.get() == nullptr) [[unlikely]]
				return std::make_error_code(std::errc::not_enough_memory);
			if (!::sysctl(&mib, buff.get(), &n, nullptr, 0)) [[likely]]
				return std::string_view(buff.get(), ::strnlen(buff.get(), n));
		}
		if (const auto err = errno; err != ENOENT && err != EINVAL)
			return std::error_code(err, std::system_category());
#elif defined(__NetBSD__)
		const int mib[4] = {CTL_KERN, KERN_PROC_ARGS, -1, KERN_PROC_PATHNAME};
		if (std::size_t n = 0; !::sysctl(&mib, nullptr, &n, nullptr, 0)) [[likely]]
		{
			auto buff = make_malloc_ptr_for_overwrite<char[]>(n);
			if (buff.get() == nullptr) [[unlikely]]
				return std::make_error_code(std::errc::not_enough_memory);
			if (!::sysctl(&mib, buff.get(), &n, nullptr, 0)) [[likely]]
				return std::string_view(buff.get(), ::strnlen(buff.get(), n));
		}
		if (const auto err = errno; err != ENOENT && err != EINVAL)
			return std::error_code(err, std::system_category());
#elif defined(__APPLE__) && defined(__MACH__)
        if (std::uint32_t n = 0; _NSGetExecutablePath(nullptr, &n))
		{
			auto buff = make_malloc_ptr_for_overwrite<char[]>(n);
			if (buff.get() == nullptr) [[unlikely]]
				return std::make_error_code(std::errc::not_enough_memory);
			if (_NSGetExecutablePath(buff.get(), &n)) [[likely]]
				return std::string_view(buff.get(), ::strnlen(buff.get(), n));
		}
#endif

		/* Inspect argv[0] as backup */
		if (!argv0_captured.empty()) [[likely]]
		{
			if (fs::path_view(argv0_captured).is_absolute())
				return fs::path_view(argv0_captured).parent_path();
			if (fs::path_view(argv0_captured).has_parent_path())
			{
				auto result = fs::current_path();
				if (result.has_value()) [[likely]]
				{
					*result /= argv0_captured;
					return result->parent_path();
				}
			}

			/* Search $PATH for the executable in argv[0] */
			auto target = fs::path();
			for_each_in_env_list(::getenv("PATH"), [&](auto value)
			{
				if ((target = value) /= argv0_captured; ::access(target.native().data(), X_OK))
				{
					target.clear();
					return true;
				}
				return false;
			});
			return target.parent_path();
		}
		return {};
	}
	result<fs::path> find_runtime_dir() noexcept
	{
		return with_env_var(TMPDIR_ENV, [](auto value, auto uid, auto euid) noexcept -> result<fs::path>
		{
			if (uid != euid) [[unlikely]]
				return "/tmp";

#ifdef __APPLE__
			if (value.empty()) [[unlikely]]
				return "/tmp";
#else
			if (value.empty()) [[unlikely]]
				return "/run/user/" + std::to_string(uid);
#endif

			return value;
		});
	}

	result<fs::path> find_user_home_dir() noexcept
	{
		return with_env_var("HOME", [](auto value, auto uid, auto euid) noexcept -> result<fs::path>
		{
			if (!value.empty() && uid == euid) [[likely]]
				return value;

			/* We are setuid or environment is bad. */
			malloc_ptr<char[]> buff = {};
			std::size_t buff_size = 4096;
			passwd pwd, *pwd_ptr;

			for (;;)
			{
				if (buff.get() != nullptr)
					buff.reset(static_cast<char *>(std::realloc(buff.release(), buff_size *= 2)));
				else
					buff = make_malloc_ptr_for_overwrite<char[]>(buff_size);

				if (buff.get() == nullptr) [[unlikely]]
					return std::make_error_code(std::errc::not_enough_memory);
				if (getpwuid_r(uid, &pwd, buff.get(), buff_size, &pwd_ptr)) [[unlikely]]
				{
					const auto err = errno;
					if (err != ERANGE && err != ENOMEM)
						return std::error_code(errno, std::system_category());
					else
						continue;
				}
				if (pwd_ptr == &pwd) [[likely]]
					value = pwd.pw_dir;
				break;
			}

			/* Fall back to the home dir wildcard if other methods failed. */
			return value.empty() ? value : "~";
		});
	}
	result<fs::path> find_data_home_dir() noexcept
	{
		return with_env_var("XDG_DATA_HOME", [](auto value, auto, auto) noexcept -> result<fs::path>
		{
			if (value.empty()) [[unlikely]]
				return find_user_home_dir().into_value([](auto p) { return p / ".local/share"; });
			else
				return value;
		});
	}
	result<fs::path> find_cache_home_dir() noexcept
	{
		return with_env_var("XDG_CACHE_HOME", [](auto value, auto, auto) noexcept -> result<fs::path>
		{
			if (value.empty()) [[unlikely]]
				return find_user_home_dir().into_value([](auto p) { return p / ".cache"; });
			else
				return value;
		});
	}
	result<fs::path> find_state_home_dir() noexcept
	{
		return with_env_var("XDG_STATE_HOME", [](auto value, auto, auto) noexcept -> result<fs::path>
		{
			if (value.empty()) [[unlikely]]
				return find_user_home_dir().into_value([](auto p) { return p / ".local/state"; });
			else
				return value;
		});
	}
	result<fs::path> find_config_home_dir() noexcept
	{
		return with_env_var("XDG_CONFIG_HOME", [](auto value, auto, auto) noexcept -> result<fs::path>
		{
			if (value.empty()) [[unlikely]]
				return find_user_home_dir().into_value([](auto p) { return p / ".config"; });
			else
				return value;
		});
	}

	result<fs::path> find_share_dir() noexcept { return read_xdg_path("XDG_PUBLICSHARE_DIR").or_else([]{ return std::string("~/Public"); }); }
	result<fs::path> find_music_dir() noexcept { return read_xdg_path("XDG_MUSIC_DIR").or_else([]{ return std::string("~/Music"); }); }
	result<fs::path> find_videos_dir() noexcept { return read_xdg_path("XDG_VIDEOS_DIR").or_else([]{ return std::string("~/Videos"); }); }
	result<fs::path> find_desktop_dir() noexcept { return read_xdg_path("XDG_DESKTOP_DIR").or_else([]{ return std::string("~/Desktop"); }); }
	result<fs::path> find_pictures_dir() noexcept { return read_xdg_path("XDG_PICTURES_DIR").or_else([]{ return std::string("~/Pictures"); }); }
	result<fs::path> find_downloads_dir() noexcept { return read_xdg_path("XDG_DOWNLOAD_DIR").or_else([]{ return std::string("~/Downloads"); }); }
	result<fs::path> find_documents_dir() noexcept { return read_xdg_path("XDG_DOCUMENTS_DIR").or_else([]{ return std::string("~/Documents"); }); }
	result<fs::path> find_templates_dir() noexcept { return read_xdg_path("XDG_TEMPLATES_DIR").or_else([]{ return std::string("~/Templates"); }); }

	result<fs::path> preferred_temp_file_dir() noexcept { return find_runtime_dir(); }
	result<fs::path> preferred_temp_pipe_dir() noexcept { return find_runtime_dir(); }
}

/*
 * Created by switchblade on 2023-08-20.
 */

#include "path_discovery.hpp"

namespace rod::_detail
{
	using namespace _win32;

	result<> find_temp_dirs(std::vector<fs::discovered_path> &dirs) noexcept
	{
		try
		{
			/* If not running with elevated privileges, find %TMP%, %TEMP%, %LOCALAPPDATA%\Temp */
			if (!is_elevated().value_or(false))
			{
				with_env_var(L"LOCALAPPDATA", [&dirs](auto &&dir) { dirs.push_back({fs::path(dir, fs::path::native_format) + L"\\Temp", fs::discovery_source::environment}); });
				with_env_var(L"TEMP", [&dirs](auto &&dir) { dirs.push_back({fs::path(dir, fs::path::native_format), fs::discovery_source::environment}); });
				with_env_var(L"TMP", [&dirs](auto &&dir) { dirs.push_back({fs::path(dir, fs::path::native_format), fs::discovery_source::environment}); });
			}

			/* Find %LOCALAPPDATA%\Temp */
			with_shell_path(FOLDERID_LocalAppData, [&dirs](auto p) { dirs.push_back({fs::path(p, fs::path::native_format) + L"\\Temp", fs::discovery_source::system}); });
			/* Find %USERPROFILE%\AppData\Local\Temp */
			with_shell_path(FOLDERID_Profile, [&dirs](auto p) { dirs.push_back({fs::path(p, fs::path::native_format) + LR"(\AppData\Local\Temp)", fs::discovery_source::system}); });

			{ /* Find GetWindowsDirectoryW()\Temp */
				auto buffer = std::wstring(32767, L'\0');
				auto len = ::GetWindowsDirectoryW(buffer.data(), DWORD(buffer.size()));
				if (len && len < buffer.size())
				{
					buffer.resize(len);
					buffer.append(L"\\Temp");
					dirs.push_back({fs::path(std::move(buffer), fs::path::native_format), fs::discovery_source::fallback});
				}
			}
			{ /* Find %SYSTEMDRIVE%\Temp */
				auto buffer = std::wstring(32767, L'\0');
				auto len = ::GetSystemWindowsDirectoryW(buffer.data(), DWORD(buffer.size()));
				if (len && len < buffer.size())
				{
					buffer.resize(buffer.find_last_of(L'\\', len));
					buffer.append(L"\\Temp");
					dirs.push_back({fs::path(std::move(buffer), fs::path::native_format), fs::discovery_source::fallback});
				}
			}
			return {};
		}
		catch (...) { return _detail::current_error(); }
	}
	result<> find_data_dirs(std::vector<fs::discovered_path> &dirs) noexcept
	{
		const auto ids = std::array{FOLDERID_LocalAppData, FOLDERID_RoamingAppData, FOLDERID_AppDataProgramData, FOLDERID_ProgramData};
		return find_shell_dirs(dirs, ids);
	}
	result<> find_state_dirs(std::vector<fs::discovered_path> &dirs) noexcept
	{
		const auto ids = std::array{FOLDERID_LocalAppData};
		return find_shell_dirs(dirs, ids);
	}
	result<> find_config_dirs(std::vector<fs::discovered_path> &dirs) noexcept
	{
		const auto ids = std::array{FOLDERID_LocalAppData, FOLDERID_RoamingAppData, FOLDERID_AppDataProgramData, FOLDERID_ProgramData};
		return find_shell_dirs(dirs, ids);
	}

	result<fs::path> find_install_dir() noexcept
	{
		try
		{
			auto path = std::wstring(MAX_PATH, '\0');
			auto n = std::size_t(0);

			for (;;)
			{
				if (!(n = ::GetModuleFileNameW(nullptr, path.data(), DWORD(path.size())))) [[unlikely]]
					return dos_error_code(::GetLastError());
				else if (n >= path.size())
					path.resize(n + MAX_PATH);
				else
					break;
			}

			/* Strip module file name. */
			const auto pos = path.find_last_of(L"\\/", n);
			path.resize(pos == std::wstring::npos ? n : pos);
			return std::move(path);
		}
		catch (...) { return _detail::current_error(); }
	}
	/* Windows does not have a user-specific temporary runtime directory. */
	result<fs::path> find_runtime_dir() noexcept { return {}; }

	result<fs::path> find_user_home_dir() noexcept { return with_env_var(L"USERPROFILE", [](auto sv) { return fs::path(sv, fs::path::native_format); }); }
	result<fs::path> find_data_home_dir() noexcept { return find_localappdata(); }
	result<fs::path> find_cache_home_dir() noexcept { return find_localappdata(); }
	result<fs::path> find_state_home_dir() noexcept { return find_localappdata(); }
	result<fs::path> find_config_home_dir() noexcept { return find_localappdata(); }

	result<fs::path> find_share_dir() noexcept { return with_shell_path(FOLDERID_Public, [](auto p) { return fs::path(p, fs::path::native_format); }); }
	result<fs::path> find_music_dir() noexcept { return with_shell_path(FOLDERID_Music, [](auto p) { return fs::path(p, fs::path::native_format); }); }
	result<fs::path> find_videos_dir() noexcept { return with_shell_path(FOLDERID_Videos, [](auto p) { return fs::path(p, fs::path::native_format); }); }
	result<fs::path> find_desktop_dir() noexcept { return with_shell_path(FOLDERID_Desktop, [](auto p) { return fs::path(p, fs::path::native_format); }); }
	result<fs::path> find_pictures_dir() noexcept { return with_shell_path(FOLDERID_Pictures, [](auto p) { return fs::path(p, fs::path::native_format); }); }
	result<fs::path> find_downloads_dir() noexcept { return with_shell_path(FOLDERID_Downloads, [](auto p) { return fs::path(p, fs::path::native_format); }); }
	result<fs::path> find_documents_dir() noexcept { return with_shell_path(FOLDERID_Documents, [](auto p) { return fs::path(p, fs::path::native_format); }); }
	result<fs::path> find_templates_dir() noexcept { return with_shell_path(FOLDERID_Templates, [](auto p) { return fs::path(p, fs::path::native_format); }); }

	/* Windows uses \Device\NamedPipe for all named pipes, and has no "standard" temp file directory. */
	result<fs::path> preferred_temp_file_dir() noexcept { return {}; }
	result<fs::path> preferred_temp_pipe_dir() noexcept { try { return fs::path(L"\\!!\\Device\\NamedPipe", fs::path::native_format); } catch (...) { return _detail::current_error(); } }
}

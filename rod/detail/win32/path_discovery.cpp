/*
 * Created by switch_blade on 2023-08-20.
 */

#include "path_discovery.hpp"

namespace rod::_detail
{
	using namespace _win32;

	inline static auto find_shell_dirs(std::span<const GUID> ids, std::vector<discovered_path> &dirs) noexcept
	{
		dirs.clear();
		for (auto id : ids)
			with_shell_path(id, [&](auto dir)
			{
				auto entry = discovered_path{.path = path(dir, path::native_format), .source = discovery_source::system};
				if (query_discovered_dir(discovery_mode::all, entry).value_or(false)) [[likely]]
					dirs.emplace_back(std::move(entry));
			});
		return result<>();
	}
	inline static auto find_localappdata() noexcept { return with_shell_path(FOLDERID_LocalAppData, [](auto sv) { return path(sv); }); }

	result<> find_temp_dirs(std::vector<discovered_path> &dirs) noexcept
	{
		try
		{
			dirs.clear();

			/* TODO: Check if not SUID. */
			/* Find %TMP%, %TEMP%, %LOCALAPPDATA%\Temp */
			for (auto env: {L"TMP", L"TEMP", L"LOCALAPPDATA"})
				with_env_var(env, [&](auto dir)
				{
					auto entry = discovered_path{.path = path(dir, path::native_format), .source = discovery_source::environment};
					if (env[0] == L'L') entry.path += L"\\Temp";
					if (query_discovered_dir(discovery_mode::all, entry).value_or(false)) [[likely]]
						dirs.emplace_back(std::move(entry));
				});

			/* Find %LOCALAPPDATA%\Temp */
			with_shell_path(FOLDERID_LocalAppData, [&](auto dir)
			{
				auto entry = discovered_path{.path = path(dir, path::native_format), .source = discovery_source::system};
				entry.path += L"\\Temp";
				if (query_discovered_dir(discovery_mode::all, entry).value_or(false)) [[likely]]
					dirs.emplace_back(std::move(entry));
			});

			{ /* Find GetWindowsDirectoryW()\Temp */
				auto buffer = std::wstring(32767, L'\0');
				auto len = ::GetWindowsDirectoryW(buffer.data(), DWORD(buffer.size()));
				if (len && len < buffer.size())
				{
					buffer.resize(len);
					buffer.append(L"\\Temp");
					auto entry = discovered_path{.path = path(std::move(buffer), path::native_format), .source = discovery_source::fallback};
					if (query_discovered_dir(discovery_mode::all, entry).value_or(false)) [[likely]]
						dirs.emplace_back(std::move(entry));
				}
			}
			{ /* Find %SYSTEMDRIVE%\Temp */
				auto buffer = std::wstring(32767, L'\0');
				auto len = ::GetSystemWindowsDirectoryW(buffer.data(), DWORD(buffer.size()));
				if (len && len < buffer.size())
				{
					buffer.resize(buffer.find_last_of(L'\\', len));
					buffer.append(L"\\Temp");
					auto entry = discovered_path{.path = path(std::move(buffer), path::native_format), .source = discovery_source::fallback};
					if (query_discovered_dir(discovery_mode::all, entry).value_or(false)) [[likely]]
						dirs.emplace_back(std::move(entry));
				}
			}
			return {};
		}
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
		catch (const std::system_error &e) { return e.code(); }
	}
	result<> find_data_dirs(std::vector<discovered_path> &dirs) noexcept
	{
		const auto ids = std::array{FOLDERID_LocalAppData, FOLDERID_RoamingAppData, FOLDERID_AppDataProgramData, FOLDERID_ProgramData};
		return find_shell_dirs(ids, dirs);
	}
	result<> find_state_dirs(std::vector<discovered_path> &dirs) noexcept
	{
		const auto ids = std::array{FOLDERID_LocalAppData};
		return find_shell_dirs(ids, dirs);
	}
	result<> find_config_dirs(std::vector<discovered_path> &dirs) noexcept
	{
		const auto ids = std::array{FOLDERID_LocalAppData, FOLDERID_RoamingAppData, FOLDERID_AppDataProgramData, FOLDERID_ProgramData};
		return find_shell_dirs(ids, dirs);
	}

	result<path> find_install_dir() noexcept
	{
		try
		{
			std::wstring path;
			std::size_t n;

			do
			{
				if (!(n = ::GetModuleFileNameW(nullptr, path.data(), DWORD(path.size() * sizeof(wchar_t))))) [[unlikely]]
					return dos_error_code(::GetLastError());
				else
					path.resize(n + MAX_PATH);
			}
			while (n >= path.size());

			path.resize(n);
			return std::move(path);
		}
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
		catch (const std::system_error &e) { return e.code(); }
	}
	/* Windows does not have a user-specific temporary runtime directory. */
	result<path> find_runtime_dir() noexcept { return {}; }

	result<path> find_temp_pipe_dir() noexcept
	{
		try { return path(L"\\!!\\Device\\NamedPipe\\"); }
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
		catch (const std::system_error &e) { return e.code(); }
	}

	result<path> find_user_home_dir() noexcept { return with_env_var(L"USERPROFILE", [](auto sv) { return path(sv); }); }
	result<path> find_data_home_dir() noexcept { return find_localappdata(); }
	result<path> find_cache_home_dir() noexcept { return find_localappdata(); }
	result<path> find_state_home_dir() noexcept { return find_localappdata(); }
	result<path> find_config_home_dir() noexcept { return find_localappdata(); }
}

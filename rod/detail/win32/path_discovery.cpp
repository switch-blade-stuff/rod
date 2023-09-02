/*
 * Created by switch_blade on 2023-08-20.
 */

#include "../path_discovery.hpp"
#include "ntapi.hpp"

namespace rod::_detail
{
	using namespace _win32;

	template<typename F>
	inline static result<std::invoke_result_t<F, std::wstring_view>> with_env_var(const wchar_t *env, F f) noexcept
	{
		try
		{
			std::wstring path;
			if (const auto n = ::GetEnvironmentVariableW(env, nullptr, 0); !n) [[unlikely]]
				return dos_error_code(::GetLastError());
			else
				path.resize(n - 1);

			if (!::GetEnvironmentVariableW(env, path.data(), DWORD(path.size()))) [[unlikely]]
				return dos_error_code(::GetLastError());
			else
				return f(std::wstring_view(path));
		}
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
		catch (const std::system_error &e) { return e.code(); }
	}
	template<typename F>
	inline static result<std::invoke_result_t<F, std::wstring_view>> with_shell_path(GUID id, F f) noexcept
	{
		try
		{
			if (wchar_t *path; ::SHGetKnownFolderPath(id, 0, nullptr, &path) == S_OK) [[likely]]
			{
				if constexpr (std::is_void_v<std::invoke_result_t<F, std::wstring_view>>)
				{
					f(std::wstring_view(path));
					::CoTaskMemFree(path);
					return {};
				}
				else
				{
					auto result = f(std::wstring_view(path));
					::CoTaskMemFree(path);
					return result;
				}
			}
			return _win32::dos_error_code(::GetLastError());
		}
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
		catch (const std::system_error &e) { return e.code(); }
	}

	inline static result<> init_candidates(std::span<std::tuple<GUID, int, int>> candidates, typename path::string_type &buff, std::vector<discovered_path> &dirs) noexcept
	{
		buff.clear();
		dirs.clear();
		for (auto &[id, pos, len] : candidates)
		{
			const auto add_dir = [&](auto path)
			{
				len = int(path.size());
				pos = int(buff.size());
				buff.insert(pos, path);
			};
			if (auto res = with_shell_path(id, add_dir); res.has_error()) [[unlikely]]
				return res;
		}

		dirs.reserve(candidates.size());
		for (auto &[_, pos, len] : candidates)
		{
			discovered_path path;
			path.path = path_view(buff.data() + pos, len, buff[pos + len] == '\0', path_view::native_format);
			path.source = discovery_source::system;

			/* TODO: Init directory flags. */
			dirs.push_back(path);
		}

		return {};
	}

	result<> find_temp_dirs(typename path::string_type &buff, std::vector<discovered_path> &dirs) noexcept
	{
		/* TODO: Implement */
		return {};
	}
	result<> find_data_dirs(typename path::string_type &buff, std::vector<discovered_path> &dirs) noexcept
	{
		auto candidates = std::array{std::tuple{FOLDERID_LocalAppData, 0, 0}, std::tuple{FOLDERID_RoamingAppData, 0, 0}, std::tuple{FOLDERID_AppDataProgramData, 0, 0}, std::tuple{FOLDERID_ProgramData, 0, 0}};
		return init_candidates(candidates, buff, dirs);
	}
	result<> find_state_dirs(typename path::string_type &buff, std::vector<discovered_path> &dirs) noexcept
	{
		auto candidates = std::array{std::tuple{FOLDERID_LocalAppData, 0, 0}};
		return init_candidates(candidates, buff, dirs);
	}
	result<> find_config_dirs(typename path::string_type &buff, std::vector<discovered_path> &dirs) noexcept
	{
		auto candidates = std::array{std::tuple{FOLDERID_LocalAppData, 0, 0}, std::tuple{FOLDERID_RoamingAppData, 0, 0}, std::tuple{FOLDERID_AppDataProgramData, 0, 0}, std::tuple{FOLDERID_ProgramData, 0, 0}};
		return init_candidates(candidates, buff, dirs);
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
	result<path> find_runtime_dir() noexcept { return {}; }

	result<path> find_temp_file_dir() noexcept
	{
		/* TODO: Implement */
		return {};
	}
	result<path> find_temp_pipe_dir() noexcept
	{
		/* TODO: Implement */
		return {};
	}

	inline static result<path> find_localappdata() noexcept { return with_shell_path(FOLDERID_LocalAppData, [](auto sv) { return path(sv); }); }

	result<path> find_user_home_dir() noexcept { return with_env_var(L"USERPROFILE", [](auto sv) { return path(sv); }); }
	result<path> find_data_home_dir() noexcept { return find_localappdata(); }
	result<path> find_cache_home_dir() noexcept { return find_localappdata(); }
	result<path> find_state_home_dir() noexcept { return find_localappdata(); }
	result<path> find_config_home_dir() noexcept { return find_localappdata(); }
}

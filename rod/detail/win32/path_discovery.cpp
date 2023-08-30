/*
 * Created by switch_blade on 2023-08-20.
 */

#include "../path_discovery.hpp"

#include "ntapi.hpp"
#include <ShlObj.h>

namespace rod::_detail
{
	template<typename F>
	inline static result<std::invoke_result_t<F, wchar_t *>> with_shell_path(GUID id, F f) noexcept
	{
		try
		{
			if (wchar_t *path; ::SHGetKnownFolderPath(id, 0, nullptr, &path) == S_OK) [[likely]]
			{
				auto result = f(path);
				::CoTaskMemFree(path);
				return result;
			}
			return _win32::dos_error_code(::GetLastError());
		}
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
		catch (const std::system_error &e) { return e.code(); }
	}

	result<> find_temp_dirs(typename path::string_type &buff, std::vector<discovered_path> &dirs) noexcept
	{
	}
	result<> find_data_dirs(typename path::string_type &buff, std::vector<discovered_path> &dirs) noexcept
	{
	}
	result<> find_state_dirs(typename path::string_type &buff, std::vector<discovered_path> &dirs) noexcept
	{
	}
	result<> find_config_dirs(typename path::string_type &buff, std::vector<discovered_path> &dirs) noexcept
	{
	}

	result<path> find_working_dir() noexcept
	{
		try
		{
			std::wstring path;
			if (const auto n = ::GetCurrentDirectoryW(0, nullptr); !n) [[unlikely]]
				return _win32::dos_error_code(::GetLastError());
			else
				path.resize(n);
			if (!::GetCurrentDirectoryW(path.size() + 1, path.data())) [[unlikely]]
				return _win32::dos_error_code(::GetLastError());
			else
				return std::move(path);
		}
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
		catch (const std::system_error &e) { return e.code(); }
	}
	result<path> find_install_dir() noexcept
	{
		try
		{
			std::wstring path;
			std::size_t n;

			do
			{
				if (!(n = ::GetModuleFileNameW(nullptr, path.data(), path.size() * sizeof(wchar_t)))) [[unlikely]]
					return _win32::dos_error_code(::GetLastError());
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

	/* TODO: Implement these */
	result<path> find_temp_file_dir() noexcept {}
	result<path> find_temp_pipe_dir() noexcept {}

	result<path> find_user_home_dir() noexcept
	{
		try
		{
			std::wstring path;
			if (const auto n = ::GetEnvironmentVariableW(L"USERPROFILE", nullptr, 0); !n) [[unlikely]]
				return _win32::dos_error_code(::GetLastError());
			else
				path.resize(n - 1);

			if (!::GetEnvironmentVariableW(L"USERPROFILE", path.data(), path.size())) [[unlikely]]
				return _win32::dos_error_code(::GetLastError());
			else
				return std::move(path);
		}
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
		catch (const std::system_error &e) { return e.code(); }
	}

	/* All of these use %LocalAppData% */
	result<path> find_runtime_dir() noexcept { return with_shell_path(FOLDERID_LocalAppData, [](auto *p) { return path(p); }); }
	result<path> find_data_home_dir() noexcept { return find_runtime_dir(); }
	result<path> find_cache_home_dir() noexcept { return find_runtime_dir(); }
	result<path> find_state_home_dir() noexcept { return find_runtime_dir(); }
	result<path> find_config_home_dir() noexcept { return find_runtime_dir(); }
}

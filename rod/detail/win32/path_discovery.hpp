/*
 * Created by switchblade on 2023-09-02.
 */

#pragma once

#include "../path_discovery.hpp"
#include "ntapi.hpp"

namespace rod::_win32
{
	template<typename F, typename Res = result<std::invoke_result_t<F, std::wstring_view>>>
	inline static Res with_env_var(const wchar_t *env, F f) noexcept
	{
		try
		{
			std::wstring path;
			if (const auto n = ::GetEnvironmentVariableW(env, nullptr, 0); !n) [[unlikely]]
				return Res(in_place_error, dos_error_code(::GetLastError()));
			else
				path.resize(n - 1);

			if (!::GetEnvironmentVariableW(env, path.data(), DWORD(path.size() + 1))) [[unlikely]]
				return Res(in_place_error, dos_error_code(::GetLastError()));
			else if constexpr (!std::is_void_v<std::invoke_result_t<F, std::wstring_view>>)
				return f(std::wstring_view(path));
			else
			{
				f(std::wstring_view(path));
				return {};
			}
		}
		catch (...) { return _detail::current_error(); }
	}
	template<typename F, typename Res = result<std::invoke_result_t<F, std::wstring_view>>>
	inline static Res with_shell_path(GUID id, F f) noexcept
	{
		try
		{
			if (wchar_t *path; ::SHGetKnownFolderPath(id, 0, nullptr, &path) == S_OK) [[likely]]
			{
				if constexpr (std::is_void_v<std::invoke_result_t<F, std::wstring_view>>)
				{
					f(std::wstring_view(path));
					::CoTaskMemFree(path);
					return Res();
				}
				else
				{
					auto result = f(std::wstring_view(path));
					::CoTaskMemFree(path);
					return result;
				}
			}
			return Res(in_place_error, dos_error_code(::GetLastError()));
		}
		catch (...) { return _detail::current_error(); }
	}
}
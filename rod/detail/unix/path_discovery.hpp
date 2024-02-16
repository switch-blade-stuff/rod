/*
 * Created by switchblade on 2023-09-02.
 */

#pragma once

#include <cstdlib>

#if __has_include("mach-o/dyld.h")
#include <mach-o/dyld.h>
#endif
#if __has_include("sys/sysctl.h")
#include <sys/sysctl.h>
#endif
#if __has_include("sys/param.h")
#include <sys/param.h>
#endif

#if defined(__linux__)
#define PROC_SELF "/proc/self/exe"
#elif defined(__unix__) && defined(__sun__)
#define PROC_SELF "/proc/self/path/a.out"
#endif

#ifdef __APPLE__
#define TMPDIR_ENV "TMPDIR"
#else
#define TMPDIR_ENV "XDG_RUNTIME_DIR"
#endif

#include <unistd.h>
#include <pwd.h>

#include "../path_discovery.hpp"
#include "../file_handle.hpp"
#include "path_util.hpp"

namespace rod::_unix
{
	result<std::string> read_link_path(const char *link) noexcept;
	result<std::string> read_xdg_path(std::string_view name) noexcept;

	template<std::invocable<std::string_view> F> requires(std::convertible_to<std::invoke_result_t<F, std::string_view>, bool>)
	inline static void for_each_in_env_list(std::string_view env_list, F f) noexcept(std::is_nothrow_invocable_v<F, std::string_view>)
	{
		if (env_list.empty()) [[unlikely]]
			return;

		for (std::size_t pos = 0, end;; pos = end + 1)
		{
			end = env_list.find_first_of(':', pos);
			if (!f(env_list.substr(pos, end - pos)))
				break;
			if (pos > env_list.size())
				break;
		}
	}
	template<std::invocable<std::string_view, uid_t, uid_t> F, typename Res = std::invoke_result_t<F, std::string_view, uid_t, uid_t>>
	inline static Res with_env_var(const char *env, F f) noexcept(std::is_nothrow_invocable_v<F, std::string_view, uid_t, uid_t>)
	{
		if (const auto uid = ::getuid(), euid = ::geteuid(); uid == euid) [[likely]]
			return f(std::string_view(::getenv(env)), uid, euid);
		else
			return f(std::string_view(), uid, euid);
	}
}

/*
 * Created by switchblade on 2023-10-30.
 */

#pragma once

#include <unistd.h>
#include <fcntl.h>

#if __has_include("wordexp.h")
#include <wordexp.h>
#define USE_WORDEXP
#endif

#include "../path_util.hpp"

namespace rod::_unix
{
	inline constexpr int make_fd_mode(fs::file_perm perm) noexcept
	{
		int result = 0;
		if (bool(perm & fs::file_perm::owner_read))
			result |= S_IRUSR;
		if (bool(perm & fs::file_perm::owner_write))
			result |= S_IWUSR;
		if (bool(perm & fs::file_perm::owner_exec))
			result |= S_IXUSR;
		if (bool(perm & fs::file_perm::group_read))
			result |= S_IRGRP;
		if (bool(perm & fs::file_perm::group_write))
			result |= S_IWGRP;
		if (bool(perm & fs::file_perm::group_exec))
			result |= S_IXGRP;
		if (bool(perm & fs::file_perm::other_read))
			result |= S_IROTH;
		if (bool(perm & fs::file_perm::other_write))
			result |= S_IWOTH;
		if (bool(perm & fs::file_perm::other_exec))
			result |= S_IXOTH;
#ifdef S_ISUID
		if (bool(perm & fs::file_perm::setuid))
			result |= S_ISUID;
#endif
#ifdef S_ISGID
		if (bool(perm & fs::file_perm::setgid))
			result |= S_ISGID;
#endif
#ifdef S_ISVTX
		if (bool(perm & fs::file_perm::sticky))
			result |= S_ISVTX;
#endif

		return result;
	}
	inline constexpr int make_fd_flags(fs::file_flags flags, fs::open_mode mode = fs::open_mode::existing, fs::file_caching caching = fs::file_caching::all) noexcept
	{
		int result = O_RDONLY;
		if (bool(flags & fs::file_flags::write))
		{
			if (!bool(flags & fs::file_flags::read))
				result = O_WRONLY;
			else
				result = O_RDWR;
		}
		if (bool(flags & fs::file_flags::non_blocking))
			result |= O_NONBLOCK;

		if (mode == fs::open_mode::always)
			result |= O_CREAT;
		else if (mode == fs::open_mode::create)
			result |= O_CREAT | O_EXCL;
		else if (mode == fs::open_mode::truncate)
			result |= O_TRUNC;
		else if (mode == fs::open_mode::supersede)
			result |= O_TRUNC | O_CREAT;

		if (!bool(caching & fs::file_caching::meta))
			result |= O_SYNC;
		else if (!bool(caching & fs::file_caching::write))
			result |= O_DSYNC;
#ifdef O_DIRECT
		if (!bool(caching & (fs::file_caching::read | fs::file_caching::write)))
			result |= O_DIRECT;
#endif

		return result;
	}

	result<std::string> exec_cmd(std::string_view cmd) noexcept;
	result<std::string> expand_path(std::string_view str) noexcept;
}

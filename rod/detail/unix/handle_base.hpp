/*
 * Created by switchblade on 2023-10-30.
 */

#pragma once

#include <fcntl.h>

#include "../handle_base.hpp"
#include "handle_stat.hpp"

namespace rod::_unix
{
	using native_handle_type = basic_handle::native_handle_type;

	inline static auto seek_pos(int fd, auto off, int dir) noexcept
	{
#if defined(__linux__) && defined(_LARGEFILE64_SOURCE)
		return ::lseek64(fd, ::off64_t(off), dir);
#else
		return ::lseek(fd, ::off_t(off), dir);
#endif
	}
	inline static auto tell_pos(int fd) noexcept { return seek_pos(fd, 0, SEEK_CUR); }

	result<native_handle_type> link_file(native_handle_type hnd, const path_handle &base, path_view path, bool replace, bool check_stat, const file_timeout &to) noexcept;
	result<native_handle_type> relink_file(native_handle_type hnd, const path_handle &base, path_view path, bool replace, bool check_stat, const file_timeout &to) noexcept;
	result<native_handle_type> unlink_file(native_handle_type hnd, bool rmdir, bool check_stat, const file_timeout &to) noexcept;
}

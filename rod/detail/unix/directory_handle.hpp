/*
 * Created by switchblade on 2023-09-02.
 */

#pragma once

#if __has_include("dirent.h")
#include <dirent.h>
#else
#include <unistd.h>
#endif

#include <sys/stat.h>
#include <fnmatch.h>

#include "../directory_handle.hpp"
#include "handle_base.hpp"

namespace rod::_unix
{
#if defined(__linux__)
	using dirent = ::dirent64;
	using diroff = ::off64_t;
#else
	using dirent = ::dirent;
	using diroff = ::off_t;
#endif

	inline static int getdents(int fd, std::span<char> buff) noexcept
	{
#if defined(__linux__)
		return ::getdents64(fd, buff.data(), static_cast<unsigned int>(buff.size()));
#elif !(defined(__APPLE__) && defined(__MACH__))
		return ::getdents(fd, buff.data(), static_cast<unsigned int>(buff.size()), &dir_pos);
#else
		::off_t off;
		return ::syscall(SYS_getdirentries64, fd, buff.data(), static_cast<unsigned int>(buff.size()), &off);
#endif
	}

	template<typename F>
	inline static result<diroff> for_each_dir_entry(int fd, std::span<char> buff, const file_timeout &to, F &&f) noexcept
	{
		/* Fill the buffer and iterate over the entries. */
		const auto ents = diroff(getdents(fd, buff));
		if (ents < 0) [[unlikely]]
			return std::error_code(errno, std::system_category());

		auto last_err = std::error_code();
		auto next_off = diroff();

		for (std::size_t i = 0, off = 0; i < std::size_t(ents); ++i)
		{
			const auto ent = reinterpret_cast<dirent *>(buff.data() + off);
			off += ent->d_reclen;

			result<int> accept = 0;
			if (ent->d_name[0] != '.' || (ent->d_name[1] != 0 && ent->d_name[1] != '.'))
				accept = f(*ent);

			if (const auto now = file_clock::now(); now >= to.absolute(now) && !last_err) [[unlikely]]
				last_err = std::make_error_code(std::errc::timed_out);
			if (accept.has_error() && !last_err) [[unlikely]]
				last_err = accept.error();

			if (last_err || (next_off = ent->d_off) == 0 || *accept == -1)
				break;
			if (*accept == 1) [[likely]]
				continue;

			std::memmove(ent, buff.data() + off, buff.size() - off);
		}
		if (last_err) [[unlikely]]
			return last_err;
		else
			return next_off;
	}
}

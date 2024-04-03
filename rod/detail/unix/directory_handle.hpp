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

	inline static ::ssize_t getdents(int fd, std::span<char> buff) noexcept
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
	inline static result<diroff> for_each_dir_entry(int fd, std::span<char> buff, const file_timeout &to, F &&f)
	{
		/* Fill the buffer and iterate over the entries. */
		auto ents = getdents(fd, buff);
		if (ents < 0) [[unlikely]]
			return std::error_code(errno, std::system_category());

		auto last_err = std::error_code();
		auto next_off = diroff();
		auto pos = ::ssize_t();

		while (pos < ents)
		{
			const auto ent = reinterpret_cast<dirent *>(buff.data() + pos);
			const auto next_pos = pos + ent->d_reclen;

			result<int> accept = 0;
			if (ent->d_name[0] != '.' && (ent->d_name[1] == '\0' || (ent->d_name[1] != '.' && ent->d_name[2] != '\0')))
				accept = f(*ent);

			if (const auto now = file_clock::now(); now >= to.absolute(now) && !last_err) [[unlikely]]
				last_err = std::make_error_code(std::errc::timed_out);
			if (accept.has_error() && !last_err) [[unlikely]]
				last_err = accept.error();

			if (last_err || *accept == -1)
				break;

			next_off = ent->d_off;
			pos = next_pos;
			if (*accept == 1) [[likely]]
			{
				continue;
			}

//			std::memmove(ent, buff.data() + next_pos, buff.size() - next_pos);
//			ents -= ent->d_reclen;
		}
		if (last_err) [[unlikely]]
			return last_err;
		if (pos < ents)
			return next_off;
		else
			return 0;
	}
}

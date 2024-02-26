/*
 * Created by switchblade on 2023-10-30.
 */

#pragma once

#include <sys/stat.h>
#include <fcntl.h>

#ifdef __linux__
#include <sys/sysmacros.h>
#include <sys/statfs.h>
#include <sys/mount.h>
#include <mntent.h>
#endif

#if defined(STATX_BASIC_STATS) && defined(STATX_BTIME) && defined(STATX_ATTR_COMPRESSED)
#define HAS_STATX 1
#else
#define HAS_STATX 0
#endif
#if HAS_STATX || defined(__APPLE__) || defined(__FreeBSD__)
#define HAS_BTIME 1
#else
#define HAS_BTIME 0
#endif

#include "../handle_stat.hpp"
#include "path_discovery.hpp"

namespace rod::_unix
{
	using namespace fs;

	result<std::string> get_path(int fd) noexcept;
	result<stat::query> get_stat(stat &st, int base, const char *leaf, stat::query q, bool nofollow) noexcept;
	result<stat::query> set_stat(const stat &st, int base, const char *leaf, stat::query q, bool nofollow) noexcept;
	result<fs_stat::query> get_fs_stat(fs_stat &st, int base, const char *leaf, fs_stat::query q, bool nofollow) noexcept;

	constexpr file_type type_from_mode(std::uint16_t mode) noexcept
	{
		switch ((mode & S_IFMT))
		{
		case S_IFREG: return file_type::regular;
		case S_IFLNK: return file_type::symlink;
		case S_IFSOCK: return file_type::socket;
		case S_IFIFO: return file_type::fifo;
		case S_IFBLK: return file_type::block;
		case S_IFDIR: return file_type::directory;
		case S_IFCHR: return file_type::character;
		}
		return file_type::unknown;
	}
	constexpr file_perm perm_from_mode(std::uint16_t mode) noexcept
	{
		auto perm = file_perm::none;
		if (bool(mode & S_ISUID))
			perm |= file_perm::setuid;
		if (bool(mode & S_ISGID))
			perm |= file_perm::setgid;
		if (bool(mode & S_ISVTX))
			perm |= file_perm::sticky;
		if (bool(mode & S_IRUSR))
			perm |= file_perm::owner_read;
		if (bool(mode & S_IWUSR))
			perm |= file_perm::owner_write;
		if (bool(mode & S_IXUSR))
			perm |= file_perm::owner_exec;
		if (bool(mode & S_IRGRP))
			perm |= file_perm::group_read;
		if (bool(mode & S_IWGRP))
			perm |= file_perm::group_write;
		if (bool(mode & S_IXGRP))
			perm |= file_perm::group_exec;
		if (bool(mode & S_IROTH))
			perm |= file_perm::other_read;
		if (bool(mode & S_IWOTH))
			perm |= file_perm::other_write;
		if (bool(mode & S_IXOTH))
			perm |= file_perm::other_exec;
		return perm;
	}
	constexpr std::uint16_t mode_from_perm(file_perm perm) noexcept
	{
		std::uint16_t mode = 0;
		if (bool(perm & file_perm::setuid))
			mode |= S_ISUID;
		if (bool(perm & file_perm::setgid))
			mode |= S_ISGID;
		if (bool(perm & file_perm::sticky))
			mode |= S_ISVTX;
		if (bool(perm & file_perm::owner_read))
			mode |= S_IRUSR;
		if (bool(perm & file_perm::owner_write))
			mode |= S_IWUSR;
		if (bool(perm & file_perm::owner_exec))
			mode |= S_IXUSR;
		if (bool(perm & file_perm::group_read))
			mode |= S_IRGRP;
		if (bool(perm & file_perm::group_write))
			mode |= S_IWGRP;
		if (bool(perm & file_perm::group_exec))
			mode |= S_IXGRP;
		if (bool(perm & file_perm::other_read))
			mode |= S_IROTH;
		if (bool(perm & file_perm::other_write))
			mode |= S_IWOTH;
		if (bool(perm & file_perm::other_exec))
			mode |= S_IXOTH;
		return mode;
	}

	constexpr file_time_point time_from_timespec(struct ::timespec ts) noexcept
	{
		constexpr auto n_ticks = static_cast<std::uint64_t>(file_clock::period::den) / file_clock::period::num;
		constexpr auto mul = n_ticks >= 1000000000ull ? n_ticks / 1000000000ull : 1;
		constexpr auto div = n_ticks >= 1000000000ull ? 1 : 1000000000ull / n_ticks;
		return file_time_point(file_clock::duration(ts.tv_sec * n_ticks + ts.tv_nsec * mul / div));
	}
	constexpr struct ::timespec timespec_from_time(file_time_point tp) noexcept
	{
		constexpr auto n_ticks = static_cast<std::uint64_t>(file_clock::period::den) / file_clock::period::num;
		constexpr auto mul = n_ticks >= 1000000000ull ? n_ticks / 1000000000ull : 1;
		constexpr auto div = n_ticks >= 1000000000ull ? 1 : 1000000000ull / n_ticks;

		const auto dur = file_clock::duration(tp.time_since_epoch());
		return {static_cast<time_t>(dur.count() / n_ticks), static_cast<long>((dur.count() % n_ticks) * div / mul)};
	}

	constexpr file_time_point time_from_sec_nsec(std::int64_t sec, std::int64_t nsec) noexcept
	{
		return time_from_timespec({static_cast<time_t>(sec), static_cast<long>(nsec)});
	}
	constexpr std::uint64_t dev_from_major_minor(std::uint32_t maj, std::uint32_t min) noexcept
	{
		return (std::uint64_t(maj) << 32) | std::uint64_t(min);
	}
}

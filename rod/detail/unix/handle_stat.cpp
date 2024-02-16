/*
 * Created by switchblade on 2023-08-29.
 */

#include "handle_stat.hpp"

namespace rod
{
	namespace _unix
	{
		result<std::string> get_path(int fd) noexcept
		{
			try
			{
#if defined(__linux__)
				char link[64];
				::snprintf(link, 64, "/proc/self/fd/%d", fd);

				if (auto path = _unix::read_link_path(link); path.has_value() && path->find(" (deleted)") != std::string::npos)
					return link;
				else
					return path;
#elif defined(__APPLE__)
				auto path = std::string(32769, '\0');
				if (::fcntl(fd, F_GETPATH, path.data()) [[unlikely]]
				{
					if (const auto err = errno; err != ENOENT)
						return std::error_code(err, std::system_category());
					else
						return {};
				}
				path.resize(std::strchr(path.data(), 0) - path.data());
				if (struct stat ls; !::lstat(path.data(), &ls))
					return path;
				else
					return {};
#elif defined(__FreeBSD__)
				const int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_FILEDESC, ::getpid()};
				auto path = std::string();

				if (std::size_t n = 0; !::sysctl(&mib, 4, nullptr, &n, nullptr, 0)) [[likely]]
				{
					auto buff = make_malloc_ptr_for_overwrite<char[]>(n);
					if (buff.get() == nullptr) [[unlikely]]
						return std::make_error_code(std::errc::not_enough_memory);
					if (!::sysctl(&mib, 4, buff.get(), &n, nullptr, 0)) [[likely]]
						return std::error_code(errno, std::system_category());
					else
						path.resize(n);
				}
				for (auto *p = path.data(); p < path.data() + path.size();)
				{
					if (const auto kif = reinterpret_cast<struct kinfo_file *>(p); kif->kf_fd != fd)
						p += kif->kf_structsize;
					else
						return kif->kf_path;
				}
				return {};
#else
				return std::make_error_code(std::errc::not_supported);
#endif
			}
			catch (...) { return _detail::current_error(); }
		}
		result<stat::query> get_stat(stat &st, int base, const char *leaf, stat::query q, bool nofollow) noexcept
		{
			if (q == stat::query::none)
				return q;
			if (base < 0 && leaf)
				base = AT_FDCWD;

			auto done = stat::query::none;
#if HAS_STATX
			struct ::statx st_data = {};
			unsigned int mask = 0;

			if (bool(q & stat::query::ino))
				mask |= STATX_INO;
			if (bool(q & stat::query::uid))
				mask |= STATX_UID;
			if (bool(q & stat::query::gid))
				mask |= STATX_GID;
			if (bool(q & stat::query::type))
				mask |= STATX_TYPE;
			if (bool(q & stat::query::perm))
				mask |= STATX_MODE;
			if (bool(q & stat::query::nlink))
				mask |= STATX_NLINK;
			if (bool(q & (stat::query::size | stat::query::is_sparse)))
				mask |= STATX_SIZE;
			if (bool(q & (stat::query::blocks | stat::query::alloc | stat::query::is_sparse)))
				mask |= STATX_BLOCKS;
			if (bool(q & stat::query::atime))
				mask |= STATX_ATIME;
			if (bool(q & stat::query::mtime))
				mask |= STATX_MTIME;
			if (bool(q & stat::query::ctime))
				mask |= STATX_CTIME;
			if (bool(q & stat::query::btime))
				mask |= STATX_BTIME;

			if (::statx(base, leaf ? leaf : "", AT_NO_AUTOMOUNT | AT_STATX_SYNC_AS_STAT | (leaf ? 0 : AT_EMPTY_PATH) | (nofollow ? AT_SYMLINK_NOFOLLOW : 0), mask, &st_data)) [[unlikely]]
				return std::error_code(errno, std::system_category());

			if (bool(q & stat::query::ino) && bool(st_data.stx_mask & STATX_INO))
			{
				st.ino = st_data.stx_ino;
				done |= stat::query::ino;
			}
			if (bool(q & stat::query::dev))
			{
				st.dev = dev_from_major_minor(st_data.stx_dev_major,  st_data.stx_dev_minor);
				done |= stat::query::dev;
			}
			if (bool(q & stat::query::rdev))
			{
				st.rdev = dev_from_major_minor(st_data.stx_rdev_major,  st_data.stx_rdev_minor);
				done |= stat::query::rdev;
			}
			if (bool(q & stat::query::blksize))
			{
				st.blksize = st_data.stx_blksize;
				done |= stat::query::blksize;
			}
			if (bool(q & stat::query::uid) && bool(st_data.stx_mask & STATX_UID))
			{
				st.uid = st_data.stx_uid;
				done |= stat::query::uid;
			}
			if (bool(q & stat::query::gid) && bool(st_data.stx_mask & STATX_GID))
			{
				st.gid = st_data.stx_gid;
				done |= stat::query::gid;
			}
			if (bool(q & stat::query::type) && bool(st_data.stx_mask & STATX_TYPE))
			{
				st.type = type_from_mode(st_data.stx_mode);
				done |= stat::query::type;
			}
			if (bool(q & stat::query::perm) && bool(st_data.stx_mask & STATX_MODE))
			{
				st.perm = perm_from_mode(st_data.stx_mode);
				done |= stat::query::perm;
			}
			if (bool(q & stat::query::size) && bool(st_data.stx_mask & STATX_SIZE))
			{
				st.size = st_data.stx_size;
				done |= stat::query::size;
			}
			if (bool(q & stat::query::alloc) && bool(st_data.stx_mask & STATX_BLOCKS))
			{
				st.alloc = st_data.stx_blocks * 512;
				done |= stat::query::alloc;
			}
			if (bool(q & stat::query::nlink) && bool(st_data.stx_mask & STATX_NLINK))
			{
				st.nlink = st_data.stx_nlink;
				done |= stat::query::nlink;
			}
			if (bool(q & stat::query::blocks) && bool(st_data.stx_mask & STATX_BLOCKS))
			{
				st.alloc = st_data.stx_blocks;
				done |= stat::query::blocks;
			}
			if (bool(q & stat::query::atime) && bool(st_data.stx_mask & STATX_ATIME))
			{
				st.atime = time_from_sec_nsec(st_data.stx_atime.tv_sec, st_data.stx_atime.tv_nsec);
				done |= stat::query::atime;
			}
			if (bool(q & stat::query::mtime) && bool(st_data.stx_mask & STATX_MTIME))
			{
				st.mtime = time_from_sec_nsec(st_data.stx_mtime.tv_sec, st_data.stx_mtime.tv_nsec);
				done |= stat::query::mtime;
			}
			if (bool(q & stat::query::ctime) && bool(st_data.stx_mask & STATX_CTIME))
			{
				st.ctime = time_from_sec_nsec(st_data.stx_ctime.tv_sec, st_data.stx_ctime.tv_nsec);
				done |= stat::query::ctime;
			}
			if (bool(q & stat::query::btime) && bool(st_data.stx_mask & STATX_BTIME))
			{
				st.btime = time_from_sec_nsec(st_data.stx_btime.tv_sec, st_data.stx_btime.tv_nsec);
				done |= stat::query::btime;
			}
			if (bool(q & stat::query::is_compressed) && bool(st_data.stx_attributes_mask & STATX_ATTR_COMPRESSED))
			{
				st.is_compressed = st_data.stx_attributes & STATX_ATTR_COMPRESSED;
				done |= stat::query::is_compressed;
			}
			if (bool(q & stat::query::is_sparse) && bool(st_data.stx_mask & STATX_SIZE) && bool(st_data.stx_mask & STATX_BLOCKS))
			{
				st.is_sparse = st_data.stx_size > st_data.stx_blocks * 512;
				done |= stat::query::is_sparse;
			}
#else
			struct ::stat st_data = {};
			bool stat_fail = false;

			if (leaf != nullptr)
				stat_fail = ::fstatat(base, leaf, &st_data, AT_NO_AUTOMOUNT | (nofollow ? AT_SYMLINK_NOFOLLOW : 0);
			else
				stat_fail = ::fstat(base, &st_data);
			if (stat_fail) [[unlikely]]
				return std::error_code(errno, std::system_category());

			if (bool(q & stat::query::ino))
			{
				st.ino = st_data.st_ino;
				done |= stat::query::ino;
			}
			if (bool(q & stat::query::dev))
			{
				st.dev = st_data.st_dev;
				done |= stat::query::dev;
			}
			if (bool(q & stat::query::rdev))
			{
				st.rdev = st_data.st_rdev;
				done |= stat::query::rdev;
			}
			if (bool(q & stat::query::uid))
			{
				st.uid = st_data.st_uid;
				done |= stat::query::uid;
			}
			if (bool(q & stat::query::gid))
			{
				st.gid = st_data.st_gid;
				done |= stat::query::gid;
			}
			if (bool(q & stat::query::type))
			{
				st.type = type_from_mode(st_data.st_mode);
				done |= stat::query::type;
			}
			if (bool(q & stat::query::perm))
			{
				st.perm = perm_from_mode(st_data.st_mode);
				done |= stat::query::perm;
			}
			if (bool(q & stat::query::size))
			{
				st.size = st_data.st_size;
				done |= stat::query::size;
			}
			if (bool(q & stat::query::alloc))
			{
				st.alloc = st_data.st_blocks * 512;
				done |= stat::query::alloc;
			}
			if (bool(q & stat::query::nlink))
			{
				st.nlink = st_data.st_nlink;
				done |= stat::query::nlink;
			}
			if (bool(q & stat::query::blocks))
			{
				st.alloc = st_data.st_blocks;
				done |= stat::query::blocks;
			}
			if (bool(q & stat::query::blksize))
			{
				st.blksize = st_data.st_blksize;
				done |= stat::query::blksize;
			}
#if defined(__ANDROID__)
			if (bool(q & stat::query::atime))
			{
				st.atime = time_from_timespec(reinterpret_cast<struct ::timespec &>(st_data.st_atime));
				done |= stat::query::atime;
			}
			if (bool(q & stat::query::mtime))
			{
				st.mtime = time_from_timespec(reinterpret_cast<struct ::timespec &>(st_data.st_mtime));
				done |= stat::query::mtime;
			}
			if (bool(q & stat::query::ctime))
			{
				st.ctime = time_from_timespec(reinterpret_cast<struct ::timespec &>(st_data.st_ctime));
				done |= stat::query::ctime;
			}
			if (bool(q & stat::query::btime))
			{
				st.btime = time_from_timespec(reinterpret_cast<struct ::timespec &>(st_data.st_btime));
				done |= stat::query::btime;
			}
#elif defined(__APPLE__)
			if (bool(q & stat::query::atime))
			{
				st.atime = time_from_timespec(st_data.st_atimespec);
				done |= stat::query::atime;
			}
			if (bool(q & stat::query::mtime))
			{
				st.mtime = time_from_timespec(st_data.st_mtimespec);
				done |= stat::query::mtime;
			}
			if (bool(q & stat::query::ctime))
			{
				st.ctime = time_from_timespec(st_data.st_ctimespec);
				done |= stat::query::ctime;
			}
			if (bool(q & stat::query::btime))
			{
				st.btime = time_from_timespec(st_data.st_birthtimespec);
				done |= stat::query::btime;
			}
#else
			if (bool(q & stat::query::atime))
			{
				st.atime = time_from_timespec(st_data.st_atime);
				done |= stat::query::atime;
			}
			if (bool(q & stat::query::mtime))
			{
				st.mtime = time_from_timespec(st_data.st_mtime);
				done |= stat::query::mtime;
			}
			if (bool(q & stat::query::ctime))
			{
				st.ctime = time_from_timespec(st_data.st_ctime);
				done |= stat::query::ctime;
			}
			if (bool(q & stat::query::btime))
			{
#if defined(__FreeBSD__)
				st.btime = time_from_timespec(st_data.st_birthtim);
#else
				st.btime = time_from_timespec(st_data.st_ctime);
#endif
				done |= stat::query::btime;
			}
#endif
			if (bool(q & stat::query::is_sparse))
			{
				st.is_sparse = st_data.stx_size > st_data.stx_blocks * 512;
				done |= stat::query::is_sparse;
			}
#endif

			return done;
		}
		result<stat::query> set_stat(const stat &st, int base, const char *leaf, stat::query q, bool nofollow) noexcept
		{
			if (bool(q & ~(stat::query::perm | stat::query::uid | stat::query::gid | stat::query::atime | stat::query::mtime | (HAS_BTIME ? stat::query::btime : stat::query::none)))) [[unlikely]]
				return std::make_error_code(std::errc::not_supported);
			if (q == stat::query::none)
				return q;
			if (base < 0 && leaf)
				base = AT_FDCWD;

			auto done = stat::query::none;
			if (bool(q & stat::query::perm))
			{
				bool has_err;
				if (leaf != nullptr)
					has_err = ::fchmodat(base, leaf, mode_from_perm(st.perm), nofollow ? AT_SYMLINK_NOFOLLOW : 0);
				else
					has_err = ::fchmod(base, mode_from_perm(st.perm));
				if (has_err) [[unlikely]]
					return std::error_code(errno, std::system_category());
				else
					done |= stat::query::perm;
			}
			if (bool(q & (stat::query::uid | stat::query::gid)))
			{
				const auto uid = bool(q & stat::query::uid) ? st.uid : -1;
				const auto gid = bool(q & stat::query::gid) ? st.gid : -1;
				bool has_err;

				if (leaf != nullptr)
					has_err = ::fchownat(base, leaf, uid, gid, nofollow ? AT_SYMLINK_NOFOLLOW : 0);
				else
					has_err = ::fchown(base, uid, gid);
				if (has_err) [[unlikely]]
					return std::error_code(errno, std::system_category());
				else
					done |= q & (stat::query::uid | stat::query::gid);
			}

			struct ::timespec ts[2] = {{0, UTIME_OMIT}, {0, UTIME_OMIT}};
			if (bool(q & stat::query::atime))
				ts[0] = timespec_from_time(st.atime);
			if (bool(q & stat::query::mtime))
				ts[1] = timespec_from_time(st.mtime);
			if (bool(q & stat::query::btime))
			{
				if (!bool(q & stat::query::mtime))
				{
					auto new_st = stat();
					if (auto res = get_stat(new_st, base, leaf, stat::query::mtime, nofollow); res.has_value()) [[likely]]
						ts[1] = timespec_from_time(new_st.mtime);
					else
						return res;
				}

				struct ::timespec btime_ts[2] = {{0, UTIME_OMIT}, timespec_from_time(st.btime)};
				bool has_err;
				if (leaf != nullptr)
					has_err = ::utimensat(base, leaf, btime_ts, nofollow ? AT_SYMLINK_NOFOLLOW : 0);
				else
					has_err = ::futimens(base, btime_ts);
				if (has_err) [[unlikely]]
					return std::error_code(errno, std::system_category());
			}
			if (bool(q & (stat::query::atime | stat::query::mtime | stat::query::btime)))
			{
				bool has_err;
				if (leaf != nullptr)
					has_err = ::utimensat(base, leaf, ts, nofollow ? AT_SYMLINK_NOFOLLOW : 0);
				else
					has_err = ::futimens(base, ts);
				if (!has_err) [[likely]]
					done |= q & (stat::query::atime | stat::query::mtime | stat::query::btime);
				else
					return std::error_code(errno, std::system_category());
			}
			return done;
		}
		result<fs_stat::query> get_fs_stat(fs_stat &st, int base, const char *leaf, fs_stat::query q, bool nofollow) noexcept;
	}

	namespace _handle
	{
		result<stat::query> do_get_stat(stat &st, basic_handle::native_handle_type hnd, stat::query q) noexcept { return _unix::get_stat(st, hnd, nullptr, q, true); }
		result<stat::query> do_get_stat(stat &st, const fs::path_handle &base, fs::path_view path, stat::query q, bool nofollow) noexcept { return _unix::get_stat(st, base.native_handle(), path.render_null_terminated().c_str(), q, nofollow); }

		result<stat::query> do_set_stat(const stat &st, basic_handle::native_handle_type hnd, stat::query q) noexcept { return _unix::set_stat(st, hnd, nullptr, q, true); }
		result<stat::query> do_set_stat(const stat &st, const fs::path_handle &base, fs::path_view path, stat::query q, bool nofollow) noexcept { return _unix::set_stat(st, base.native_handle(), path.render_null_terminated().c_str(), q, nofollow); }

		result<fs_stat::query> do_get_fs_stat(fs_stat &st, basic_handle::native_handle_type hnd, fs_stat::query q) noexcept { return _unix::get_fs_stat(st, hnd, nullptr, q, true); }
		result<fs_stat::query> do_get_fs_stat(fs_stat &st, const fs::path_handle &base, fs::path_view path, fs_stat::query q, bool nofollow) noexcept { return _unix::get_fs_stat(st, base.native_handle(), path.render_null_terminated().c_str(), q, nofollow); }
	}

	namespace _path
	{
		result<path> do_to_object_path(basic_handle::native_handle_type hnd) noexcept { return _unix::get_path(hnd); }
		result<path> do_to_native_path(basic_handle::native_handle_type hnd, native_path_format, fs::dev_t, fs::ino_t) noexcept { return _unix::get_path(hnd); }
	}
}
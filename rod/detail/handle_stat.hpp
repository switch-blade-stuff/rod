/*
 * Created by switch_blade on 2023-08-15.
 */

#pragma once

#include "path_view.hpp"
#include "timeout.hpp"
#include "../tag.hpp"

namespace rod
{
	namespace _handle
	{
		class basic_handle;

		using extent_type = std::uint64_t;
		using size_type = std::size_t;

		/** Filesystem device ID type. */
		using dev_t = std::uint64_t;
		/** Filesystem inode ID type. */
		using ino_t = std::uint64_t;

		/** Type of a filesystem object. */
		enum class file_type : std::int16_t
		{
			none = 0,
			not_found,

			/** Handle or path references a regular file. */
			regular,
			/** Handle or path references a symbolic link. */
			symlink,
			/** Handle or path references a regular directory. */
			directory,
			/** Handle or path references a block device. */
			block,
			/** Handle or path references a character device. */
			character,
			/** Handle or path references a named pipe/fifo. */
			fifo,
			/** Handle or path references a socket (such as POSIX socket or WSA). */
			socket,
			/** Handle or path does not reference a known object type. */
			unknown = -1,
		};

		/** POSIX permission flags of a filesystem handle. */
		enum class file_perm : std::int16_t
		{
			none = 0,
			all = -1,

			/** Owner has execute permission. */
			owner_exec = 0100,
			/** Owner has read permission. */
			owner_read = 0400,
			/** Owner has write permission. */
			owner_write = 0200,
			/** Owner has all permissions. */
			owner_all = owner_exec | owner_read | owner_write,

			/** Group has execute permission. */
			group_exec = 0010,
			/** Group has read permission. */
			group_read = 0040,
			/** Group has write permission. */
			group_write = 0020,
			/** Group has all permissions. */
			group_all = group_exec | group_read | group_write,

			/** Others have execute permission. */
			other_exec = 0001,
			/** Others have read permission. */
			other_read = 0004,
			/** Others have write permission. */
			other_write = 0002,
			/** Others have all permissions. */
			other_all = other_exec | other_read | other_write,

			/** Owner, group & others have execute permissions. */
			exec = owner_exec | group_exec | other_exec,
			/** Owner, group & others have read permissions. */
			read = owner_read | group_read | other_read,
			/** Owner, group & others have write permissions. */
			write = owner_write | group_write | other_write,
		};

		[[nodiscard]] constexpr file_perm operator~(file_perm h) noexcept { return file_perm(~std::int16_t(h)); }
		[[nodiscard]] constexpr file_perm operator&(file_perm a, file_perm b) noexcept { return file_perm(std::int16_t(a) & std::int16_t(b)); }
		[[nodiscard]] constexpr file_perm operator|(file_perm a, file_perm b) noexcept { return file_perm(std::int16_t(a) | std::int16_t(b)); }
		[[nodiscard]] constexpr file_perm operator^(file_perm a, file_perm b) noexcept { return file_perm(std::int16_t(a) ^ std::int16_t(b)); }
		constexpr file_perm &operator&=(file_perm &a, file_perm b) noexcept { return a = a & b; }
		constexpr file_perm &operator|=(file_perm &a, file_perm b) noexcept { return a = a | b; }
		constexpr file_perm &operator^=(file_perm &a, file_perm b) noexcept { return a = a ^ b; }

		/** Structure containing file stats of a filesystem object, derived from POSIX `struct stat`. */
		struct stat
		{
			/** Query flags used to select stat fields. */
			enum class query : int
			{
				/** Select all of the stat fields. */
				all = -1,
				/** Select none of the stat fields. */
				none = 0,
				/** Select `dev` field. */
				dev = 1 << 0,
				/** Select `ino` field. */
				ino = 1 << 1,
				/** Select `rdev` field. */
				rdev = 1 << 2,
				/** Select `uid` field. */
				uid = 1 << 3,
				/** Select `gid` field. */
				gid = 1 << 4,
				/** Select `nlink` field. */
				nlink = 1 << 5,
				/** Select `type` field. */
				type = 1 << 6,
				/** Select `perm` field. */
				perm = 1 << 7,
				/** Select `atime` field. */
				atime = 1 << 8,
				/** Select `mtime` field. */
				mtime = 1 << 9,
				/** Select `ctime` field. */
				ctime = 1 << 10,
				/** Select `btime` field. */
				btime = 1 << 11,
				/** Select `size` field. */
				size = 1 << 12,
				/** Select `alloc` field. */
				alloc = 1 << 13,
				/** Select `blocks` field. */
				blocks = 1 << 14,
				/** Select `blksize` field. */
				blksize = 1 << 15,
				/** Select `sparse` field. */
				sparse = 1 << 16,
				/** Select `compressed` field. */
				compressed = 1 << 17,
				/** Select `reparse_point` field. */
				reparse_point = 1 << 18,
				/** Select `gen` field. */
				gen = 1 << 19,
				/** Select `flags` field. */
				flags = 1 << 20,
			};

			/** Device ID. */
			dev_t dev;
			/** Object ID. */
			ino_t ino;
			/** ID of a special file (POSIX only). */
			dev_t rdev;
			/** User id of the object (POSIX only). */
			std::int16_t uid;
			/** Group id of the object (POSIX only). */
			std::int16_t gid;
			/** Total number of hard links. */
			std::int16_t nlink;
			/** Type of the object. */
			file_type type;
			/** Permissions of the object (POSIX only). */
			file_perm perm;

			/** Time of last access. */
			union { typename file_clock::time_point atime; };
			/** Time of last modification. */
			union { typename file_clock::time_point mtime; };
			/** Time of last status change. */
			union { typename file_clock::time_point ctime; };
			/** File creation (birth) time. */
			union { typename file_clock::time_point btime; };

			/** Size of the file in bytes. */
			_handle::extent_type size;
			/** Number of bytes allocated for the file. */
			_handle::extent_type alloc;
			/** Number of blocks allocated for the file. */
			_handle::extent_type blocks;
			/** Block size used by the device. */
			std::uint16_t blksize;

			/** Either the file is sparse or the directory can handle sparse files. */
			bool sparse : 1;
			/** Either the file is compressed or the directory can handle compressed files. */
			bool compressed : 1;
			/** The object is a reparse point (Windows only). */
			bool reparse_point : 1;

			/** General information, if supported by the system. */
			std::uint32_t gen;
			/** User-defined flags, if supported by the system. */
			std::uint32_t flags;

			/** Default-initializes the stat structure. */
			constexpr stat() noexcept {}
			/** Zero-initializes the stat structure. */
			constexpr stat(std::nullptr_t) noexcept : dev(), ino(), rdev(), uid(), gid(), nlink(), type(), perm(), atime(), mtime(), ctime(), btime(), size(), alloc(), blocks(), blksize(), sparse(), compressed(), reparse_point(), gen(), flags() {}
		};

		[[nodiscard]] constexpr stat::query operator~(stat::query h) noexcept { return stat::query(~int(h)); }
		[[nodiscard]] constexpr stat::query operator&(stat::query a, stat::query b) noexcept { return stat::query(int(a) & int(b)); }
		[[nodiscard]] constexpr stat::query operator|(stat::query a, stat::query b) noexcept { return stat::query(int(a) | int(b)); }
		[[nodiscard]] constexpr stat::query operator^(stat::query a, stat::query b) noexcept { return stat::query(int(a) ^ int(b)); }
		constexpr stat::query &operator&=(stat::query &a, stat::query b) noexcept { return a = a & b; }
		constexpr stat::query &operator|=(stat::query &a, stat::query b) noexcept { return a = a | b; }
		constexpr stat::query &operator^=(stat::query &a, stat::query b) noexcept { return a = a ^ b; }

		/* Default implementation for path types. */
		ROD_API_PUBLIC result<stat::query> do_get_stat(stat &st, path_view path, stat::query q, bool nofollow) noexcept;
		inline static result<stat::query> do_get_stat(stat &st, const path &path, stat::query q, bool nofollow) noexcept { return do_get_stat(st, path_view(path), q, nofollow); }

		template<typename Res>
		concept stat_result = is_result_v<Res> && std::constructible_from<typename Res::template rebind_value<stat::query>, Res>;

		struct get_stat_t
		{
			template<typename Path> requires one_of<std::decay_t<Path>, path, path_view>
			result<stat::query> operator()(stat &st, Path &&path, stat::query q = stat::query::all, bool nofollow = false) const noexcept { return do_get_stat(st, std::forward<Path>(path), q, nofollow); }
			template<typename Hnd> requires tag_invocable<get_stat_t, stat &, const Hnd &, stat::query>
			stat_result auto operator()(stat &st, const Hnd &hnd, stat::query q = stat::query::all) const noexcept { return tag_invoke(*this, st, hnd, q); }
		};
		struct set_stat_t
		{
			template<typename Hnd> requires tag_invocable<set_stat_t, const stat &, Hnd &, stat::query>
			stat_result auto operator()(const stat &st, Hnd &hnd, stat::query q = stat::query::all) const noexcept { return tag_invoke(*this, st, hnd, q); }
		};
	}

	using _handle::file_perm;
	using _handle::file_type;
	using _handle::dev_t;
	using _handle::ino_t;
	using _handle::stat;

	using _handle::get_stat_t;
	using _handle::set_stat_t;

	/** Customization point object used to query selected stats for the specified filesystem object.
	 * @note Some of the queried fields may not be supported by the platform or the filesystems, which will be indicated by a cleared bit in the returned mask.
	 *
	 * @overload Queries stats of a handle.
	 * @param hnd Handle to query stats from.
	 * @param q Query flags used to select requested stats.
	 * @return Mask of the obtained stats, or a status code on failure.
	 *
	 * @overload Queries stats of a path.
	 * @param path Path to query stats for.
	 * @param q Query flags used to select requested stats.
	 * @param nofollow If set to `true`, will not follow symlinks. `false` by default.
	 * @return Mask of the obtained stats, or a status code on failure. */
	inline constexpr auto get_stat = get_stat_t{};
	/** Customization point object used to modify selected stats for the specified filesystem object.
	 * Only the following fields can be modified:
	 * <ul>
	 * <li>`perm` (POSIX only)</li>
	 * <li>`uid` (POSIX only)</li>
	 * <li>`gid` (POSIX only)</li>
	 * <li>`atime`</li>
	 * <li>`mtime`</li>
	 * <li>`btime`</li>
	 * </ul>
	 *
	 * @param hnd Handle who's stats to modify.
	 * @param q Query flags used to select modified stats.
	 * @return Mask of the modified stats, or a status code on failure. */
	inline constexpr auto set_stat = set_stat_t{};
}

/*
 * Created by switchblade on 2023-08-15.
 */

#pragma once

#include "handle_base.hpp"
#include "file_clock.hpp"
#include "path_view.hpp"

namespace rod
{
	namespace _path { class path_handle; }

	namespace fs
	{
		/** Enumeration used to control behavior of handle open functions. */
		enum class open_mode : std::uint8_t
		{
			/** Open if exists or create if missing. */
			always,
			/** Create only if does not exist and fail otherwise. */
			create,

			/** Open only if already exists and fail otherwise. */
			existing,
			/** Open and overwrite contents only if already exists and fail otherwise. */
			truncate,
			/** Replace if already exists or create if missing. */
			supersede,
		};

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
			all = -1,
			none = 0,

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
	}

	namespace _handle
	{
		/** Structure containing stats of a filesystem object, derived from POSIX `struct stat`. */
		struct stat
		{
			/** Query flags used to select stat fields. */
			enum class query : int
			{
				all = -1,
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
				/** Select `is_sparse` field. */
				is_sparse = 1 << 16,
				/** Select `is_compressed` field. */
				is_compressed = 1 << 17,
				/** Select `is_reparse_point` field. */
				is_reparse_point = 1 << 18,
				/** Select `gen` field. */
				gen = 1 << 19,
				/** Select `flags` field. */
				flags = 1 << 20,
			};

			/** Device ID. */
			fs::dev_t dev;
			/** Object ID. */
			fs::ino_t ino;
			/** ID of a special file (POSIX only). */
			fs::dev_t rdev;
			/** User id of the object (POSIX only). */
			std::int16_t uid;
			/** Group id of the object (POSIX only). */
			std::int16_t gid;
			/** Total number of hard links. */
			std::int16_t nlink;
			/** Type of the object. */
			fs::file_type type;
			/** Permissions of the object (POSIX only). */
			fs::file_perm perm;

			/** Time of last access. */
			union { typename fs::file_clock::time_point atime; };
			/** Time of last modification. */
			union { typename fs::file_clock::time_point mtime; };
			/** Time of last status change. */
			union { typename fs::file_clock::time_point ctime; };
			/** File creation (birth) time. */
			union { typename fs::file_clock::time_point btime; };

			/** Size of the file in bytes. */
			_handle::extent_type size;
			/** Number of bytes allocated for the file. */
			_handle::extent_type alloc;
			/** Number of blocks allocated for the file. */
			_handle::extent_type blocks;
			/** Block size used by the device. */
			std::uint16_t blksize;

			/** Either the file is sparse or the directory can handle sparse files. */
			bool is_sparse : 1;
			/** Either the file is compressed or the directory can handle compressed files. */
			bool is_compressed : 1;
			/** The object is a reparse point (Windows only). */
			bool is_reparse_point : 1;

			/** General information, if supported by the system. */
			std::uint32_t gen;
			/** User-defined flags, if supported by the system. */
			std::uint32_t flags;

			/** Default-initializes the stat structure. */
			constexpr stat() noexcept {}
			/** Zero-initializes the stat structure. */
			constexpr stat(std::nullptr_t) noexcept : dev(), ino(), rdev(), uid(), gid(), nlink(), type(), perm(), atime(), mtime(), ctime(), btime(), size(), alloc(), blocks(), blksize(), is_sparse(), is_compressed(), is_reparse_point(), gen(), flags() {}
		};

		[[nodiscard]] constexpr stat::query operator~(stat::query h) noexcept { return stat::query(~int(h)); }
		[[nodiscard]] constexpr stat::query operator&(stat::query a, stat::query b) noexcept { return stat::query(int(a) & int(b)); }
		[[nodiscard]] constexpr stat::query operator|(stat::query a, stat::query b) noexcept { return stat::query(int(a) | int(b)); }
		[[nodiscard]] constexpr stat::query operator^(stat::query a, stat::query b) noexcept { return stat::query(int(a) ^ int(b)); }
		constexpr stat::query &operator&=(stat::query &a, stat::query b) noexcept { return a = a & b; }
		constexpr stat::query &operator|=(stat::query &a, stat::query b) noexcept { return a = a | b; }
		constexpr stat::query &operator^=(stat::query &a, stat::query b) noexcept { return a = a ^ b; }

		template<typename Res>
		concept stat_result = instance_of<Res, result> && std::constructible_from<typename Res::template rebind_value<stat::query>, Res>;

		ROD_API_PUBLIC result<stat::query> do_get_stat(stat &st, basic_handle::native_handle_type hnd, stat::query q) noexcept;
		ROD_API_PUBLIC result<stat::query> do_get_stat(stat &st, const _path::path_handle &base, fs::path_view path, stat::query q, bool nofollow) noexcept;

		struct get_stat_t
		{
			template<typename Path> requires one_of<std::decay_t<Path>, fs::path, fs::path_view>
			result<stat::query> operator()(stat &st, const _path::path_handle &base, Path &&path, stat::query q = stat::query::all, bool nofollow = false) const noexcept { return do_get_stat(st, base, std::forward<Path>(path), q, nofollow); }
			template<typename Hnd> requires tag_invocable<get_stat_t, stat &, const Hnd &, stat::query>
			stat_result auto operator()(stat &st, const Hnd &hnd, stat::query q = stat::query::all) const noexcept { return tag_invoke(*this, st, hnd, q); }
		};

		ROD_API_PUBLIC result<stat::query> do_set_stat(const stat &st, basic_handle::native_handle_type hnd, stat::query q) noexcept;
		ROD_API_PUBLIC result<stat::query> do_set_stat(const stat &st, const _path::path_handle &base, fs::path_view path, stat::query q, bool nofollow) noexcept;

		struct set_stat_t
		{
			template<typename Path> requires one_of<std::decay_t<Path>, fs::path, fs::path_view>
			result<stat::query> operator()(const stat &st, const _path::path_handle &base, Path &&path, stat::query q = stat::query::all, bool nofollow = false) const noexcept { return do_set_stat(st, base, std::forward<Path>(path), q, nofollow); }
			template<typename Hnd> requires tag_invocable<set_stat_t, const stat &, Hnd &, stat::query>
			stat_result auto operator()(const stat &st, Hnd &hnd, stat::query q = stat::query::all) const noexcept { return tag_invoke(*this, st, hnd, q); }
		};
	}

	using _handle::stat;
	using _handle::get_stat_t;
	using _handle::set_stat_t;

	/** Customization point object used to query selected stats for the specified filesystem object.
	 * @note Some of the queried fields may not be supported by the platform or the filesystems, which will be indicated by a cleared bit in the returned mask.
	 *
	 * @overload Queries stats of an object referenced by \a hnd.
	 * @param st Reference to the `stat` structure receiving values of the selected stats.
	 * @param hnd Handle to the target object.
	 * @param q Query flags used to select requested stats.
	 * @return Mask of the obtained stats, or a status code on failure.
	 *
	 * @overload Queries stats of an object referenced by path \a path relative to \a base.
	 * @param st Reference to the `stat` structure receiving values of the selected stats.
	 * @param base Handle to the parent location. If set to an invalid handle, \a path must be a fully-qualified path.
	 * @param path Path to the target object relative to \a base if it is a valid handle, otherwise a fully-qualified path.
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
	 * @overload Modifies stats of an object referenced by \a hnd.
	 * @param st Reference to the `stat` structure containing new values of the selected stats.
	 * @param hnd Handle to the target object.
	 * @param q Query flags used to select modified stats.
	 * @return Mask of the modified stats, or a status code on failure.
	 *
	 * @overload Modifies stats of an object referenced by path \a path relative to \a base.
	 * @param st Reference to the `stat` structure containing new values of the selected stats.
	 * @param base Handle to the parent location. If set to an invalid handle, \a path must be a fully-qualified path.
	 * @param path Path to the target object relative to \a base if it is a valid handle, otherwise a fully-qualified path.
	 * @param q Query flags used to select modified stats.
	 * @param nofollow If set to `true`, will not follow symlinks. `false` by default.
	 * @return Mask of the modified stats, or a status code on failure. */
	inline constexpr auto set_stat = set_stat_t{};

	namespace _handle
	{
		/** Status and capability flags of a filesystem. */
		enum class fs_flags : std::int16_t
		{
			all = -1,
			none = 0,

			/* Status flags */
			/** Cannot execute files of the filesystem (POSIX only). */
			noexec = 0x1,
			/** Cannot become superuser on the filesystem (POSIX only). */
			nosuid = 0x2,
			/** Filesystem is read-only (writes will always fail). */
			rdonly = 0x4,
			/** Filesystem can operate over a network. */
			network = 0x8,

			/* Feature flags */
			/** Filesystem supports sparse files. */
			sparse_files = 0x10,
			/** Filesystem supports per-file compression. */
			file_compression = 0x20,
			/** Filesystem supports per-volume compression. */
			volume_compression = 0x40,
			/** Filesystem supports extended attributes. */
			extended_attributes = 0x80,
			/** Filesystem supports access-control lists. */
			access_control_list = 0x100,
		};

		[[nodiscard]] constexpr fs_flags operator~(fs_flags h) noexcept { return fs_flags(~int(h)); }
		[[nodiscard]] constexpr fs_flags operator&(fs_flags a, fs_flags b) noexcept { return fs_flags(int(a) & int(b)); }
		[[nodiscard]] constexpr fs_flags operator|(fs_flags a, fs_flags b) noexcept { return fs_flags(int(a) | int(b)); }
		[[nodiscard]] constexpr fs_flags operator^(fs_flags a, fs_flags b) noexcept { return fs_flags(int(a) ^ int(b)); }
		constexpr fs_flags &operator&=(fs_flags &a, fs_flags b) noexcept { return a = a & b; }
		constexpr fs_flags &operator|=(fs_flags &a, fs_flags b) noexcept { return a = a | b; }
		constexpr fs_flags &operator^=(fs_flags &a, fs_flags b) noexcept { return a = a ^ b; }

		/** Structure containing stats of a filesystem. */
		struct fs_stat
		{
			/** Query flags used to select fs_stat fields. */
			enum class query : int
			{
				all = -1,
				none = 0,

				/** Select `flags` field. */
				flags = 0x1,
				/** Select `filename_max` field. */
				filename_max = 0x2,
				/** Select `io_size` field. */
				io_size = 0x04,
				/** Select `blk_size` field. */
				blk_size = 0x08,

				/** Select `blk_count` field. */
				blk_count = 0x010,
				/** Select `blk_avail` field. */
				blk_avail = 0x020,
				/** Select `blk_free` field. */
				blk_free = 0x040,

				/** Select `ino_count` field. */
				ino_count = 0x100,
				/** Select `ino_avail` field. */
				ino_avail = 0x200,
				/** Select `ino_free` field. */
				ino_free = 0x400,

				/** Select `fs_id` field. */
				fs_id = 0x1000,
				/** Select `fs_type` field. */
				fs_type = 0x2000,
				/** Select `fs_name` field. */
				fs_name = 0x4000,
				/** Select `fs_path` field. */
				fs_path = 0x8000,
			};

			/** Status & feature flags. */
			fs_flags flags = {};
			/** Maximum filename length. */
			std::size_t filename_max = 0;

			/** Optimal suggested size of IO requests. */
			extent_type io_size = 0;
			/** Size of filesystem blocks. */
			extent_type blk_size = 0;

			/** Total number of blocks in the filesystem. */
			extent_type blk_count = 0;
			/** Number of unoccupied blocks available to non-privileged users. */
			extent_type blk_avail = 0;
			/** Number of unoccupied blocks in the filesystem. */
			extent_type blk_free = 0;

			/** Total number of inodes in the filesystem (POSIX only). */
			extent_type ino_count = 0;
			/** Number of unused inodes available to non-privileged users (POSIX only). */
			extent_type ino_avail = 0;
			/** Number of unused inodes in the filesystem (POSIX only). */
			extent_type ino_free = 0;

			/** Internal filesystem ID. */
			std::uint64_t fs_id[2] = {};
			/** Type of the mounted filesystem. */
			std::string fs_type;
			/** Name of the mounted filesystem. */
			std::string fs_name;
			/** Path to the filesystem mount directory. */
			fs::path fs_path;
		};

		[[nodiscard]] constexpr fs_stat::query operator~(fs_stat::query h) noexcept { return fs_stat::query(~int(h)); }
		[[nodiscard]] constexpr fs_stat::query operator&(fs_stat::query a, fs_stat::query b) noexcept { return fs_stat::query(int(a) & int(b)); }
		[[nodiscard]] constexpr fs_stat::query operator|(fs_stat::query a, fs_stat::query b) noexcept { return fs_stat::query(int(a) | int(b)); }
		[[nodiscard]] constexpr fs_stat::query operator^(fs_stat::query a, fs_stat::query b) noexcept { return fs_stat::query(int(a) ^ int(b)); }
		constexpr fs_stat::query &operator&=(fs_stat::query &a, fs_stat::query b) noexcept { return a = a & b; }
		constexpr fs_stat::query &operator|=(fs_stat::query &a, fs_stat::query b) noexcept { return a = a | b; }
		constexpr fs_stat::query &operator^=(fs_stat::query &a, fs_stat::query b) noexcept { return a = a ^ b; }

		template<typename Res>
		concept fs_stat_result = instance_of<Res, result> && std::constructible_from<typename Res::template rebind_value<fs_stat::query>, Res>;

		ROD_API_PUBLIC result<fs_stat::query> do_get_fs_stat(fs_stat &st, basic_handle::native_handle_type hnd, fs_stat::query q) noexcept;
		ROD_API_PUBLIC result<fs_stat::query> do_get_fs_stat(fs_stat &st, const _path::path_handle &base, fs::path_view path, fs_stat::query q, bool nofollow) noexcept;

		struct get_fs_stat_t
		{
			template<typename Path> requires one_of<std::decay_t<Path>, fs::path, fs::path_view>
			result<fs_stat::query> operator()(fs_stat &st, const _path::path_handle &base, Path &&path, fs_stat::query q = fs_stat::query::all, bool nofollow = false) const noexcept { return do_get_fs_stat(st, base, std::forward<Path>(path), q, nofollow); }
			template<typename Hnd> requires tag_invocable<get_fs_stat_t, fs_stat &, const Hnd &, fs_stat::query>
			fs_stat_result auto operator()(fs_stat &st, const Hnd &hnd, fs_stat::query q = fs_stat::query::all) const noexcept { return tag_invoke(*this, st, hnd, q); }
		};
	}

	namespace fs
	{
		using _handle::fs_flags;
		using _handle::fs_stat;
		using _handle::get_fs_stat_t;

		/** TODO: Document usage. */
		inline constexpr auto get_fs_stat = get_fs_stat_t{};
	}

	namespace _path
	{
		/** Enumeration used to select representation of the path returned by `to_native_path`. */
		enum class native_path_format
		{
			/** Use any valid implementation-defined path representation. */
			any = 0,
			/** Use a generic path representation (ex. use DOS path format under Windows). */
			generic,
			/** Use system-native path representation (ex. use Win32 path format under Windows).
			 * @note Some systems (ex. Linux) do not support different path representations, in which case `system` is same as `generic`. */
			system,

			/** Use a unique volume ID path representation (Windows only). */
			volume_id,
			/** Use a unique object ID path representation (Windows only). */
			object_id,
		};

		template<typename Res>
		concept path_result = instance_of<Res, result> && std::constructible_from<typename Res::template rebind_value<fs::path>, result<fs::path>>;

		/* Default implementation for handles. */
		ROD_API_PUBLIC result<path> do_to_object_path(basic_handle::native_handle_type hnd) noexcept;
		ROD_API_PUBLIC result<path> do_to_native_path(basic_handle::native_handle_type hnd, native_path_format fmt, fs::dev_t dev, fs::ino_t ino) noexcept;

		struct to_object_path_t
		{
			template<typename Hnd> requires tag_invocable<to_object_path_t, const Hnd &>
			[[nodiscard]] path_result auto operator()(const Hnd &hnd) const noexcept { return tag_invoke(*this, hnd); }
		};
		struct to_native_path_t
		{
			template<typename Hnd> requires tag_invocable<to_native_path_t, const Hnd &, native_path_format, fs::dev_t, fs::ino_t>
			[[nodiscard]] auto operator()(const Hnd &hnd, native_path_format fmt, fs::dev_t dev, fs::ino_t ino) const noexcept -> tag_invoke_result_t<to_native_path_t, const Hnd &, native_path_format, fs::dev_t, fs::ino_t>
			{
				return tag_invoke(*this, hnd, fmt, dev, ino);
			}
			template<typename Hnd> requires(!tag_invocable<to_native_path_t, const Hnd &, native_path_format> && _detail::callable<get_stat_t, stat &, const Hnd &, stat::query> && tag_invocable<to_native_path_t, const Hnd &, native_path_format, fs::dev_t, fs::ino_t>)
			[[nodiscard]] auto operator()(const Hnd &hnd, native_path_format fmt = native_path_format::any) const noexcept -> tag_invoke_result_t<to_native_path_t, const Hnd &, native_path_format, fs::dev_t, fs::ino_t>
			{
				stat st;
				if (auto res = get_stat(st, hnd, stat::query::dev | stat::query::ino); res.has_value()) [[likely]]
					return tag_invoke(*this, hnd, fmt, st.dev, st.ino);
				else
					return res.error();
			}

			template<typename Hnd> requires tag_invocable<to_native_path_t, const Hnd &, native_path_format>
			[[nodiscard]] path_result auto operator()(const Hnd &hnd, native_path_format fmt = native_path_format::any) const noexcept { return tag_invoke(*this, hnd, fmt); }
		};
	}

	namespace fs
	{
		using _path::native_path_format;
		using _path::to_object_path_t;
		using _path::to_native_path_t;

		/* TODO: Document usage */
		inline constexpr auto to_object_path = to_object_path_t{};
		/* TODO: Document usage */
		inline constexpr auto to_native_path = to_native_path_t{};
	}
}

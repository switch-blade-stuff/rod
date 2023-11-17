/*
 * Created by switchblade on 2023-08-15.
 */

#pragma once

#include <atomic>
#include <vector>
#include <span>

#include "directory_handle.hpp"

namespace rod
{
	namespace fs
	{
		/** Type of the source providing the discovered path. */
		enum class discovery_source : int
		{
			unknown = 0,
			/** Discovered path is a user-specified override. */
			override,
			/** Discovered path comes from an environment variable. */
			environment,
			/** Discovered path comes from an implementation-defined system source. */
			system,
			/** Discovered path is an implementation-defined or user-specified fallback. */
			fallback,
		};

		/** Structure representing metadata of a discovered path. */
		struct discovered_path
		{
			/** Path to the discovered directory. */
			fs::path path;
			/** Source type of the discovered directory. */
			discovery_source source = {};

			/** Device ID of the discovered directory. */
			fs::dev_t dev = 0;
			/** Inode ID of the discovered directory. */
			fs::ino_t ino = 0;

			/** Flag indicating whether the target directory is backed by storage. */
			bool storage_backed : 1 = false;
			/** Flag indicating whether the target directory is backed by network. */
			bool network_backed : 1 = false;
			/** Flag indicating whether the target directory is backed by memory. */
			bool memory_backed : 1 = false;
		};
	}

	namespace _detail
	{
		bool match_memory_backed(std::string_view) noexcept;
		bool match_storage_backed(std::string_view) noexcept;
		bool match_network_backed(std::string_view) noexcept;

		result<fs::path> find_install_dir() noexcept;
		result<fs::path> find_runtime_dir() noexcept;

		result<fs::path> find_data_home_dir() noexcept;
		result<fs::path> find_user_home_dir() noexcept;
		result<fs::path> find_cache_home_dir() noexcept;
		result<fs::path> find_state_home_dir() noexcept;
		result<fs::path> find_config_home_dir() noexcept;

		result<fs::path> find_share_dir() noexcept;
		result<fs::path> find_music_dir() noexcept;
		result<fs::path> find_videos_dir() noexcept;
		result<fs::path> find_desktop_dir() noexcept;
		result<fs::path> find_downloads_dir() noexcept;
		result<fs::path> find_documents_dir() noexcept;
		result<fs::path> find_templates_dir() noexcept;

		result<fs::path> preferred_temp_file_dir() noexcept;
		result<fs::path> preferred_temp_pipe_dir() noexcept;

		struct discovery_cache
		{
			enum status_t
			{
				none = 0,
				busy = 1,
				init = 2,
			};

			template<auto discovery_cache::*Path>
			[[nodiscard]] auto open_cached_dir() const noexcept -> fs::directory_handle;
			template<auto discovery_cache::*Dirs, typename F>
			[[nodiscard]] auto refresh_dirs(F &&) noexcept -> result<>;

			typename fs::path::string_type working_dir;
			typename fs::path::string_type install_dir;
			typename fs::path::string_type runtime_dir;
			typename fs::path::string_type user_home_dir;
			typename fs::path::string_type data_home_dir;
			typename fs::path::string_type cache_home_dir;
			typename fs::path::string_type state_home_dir;
			typename fs::path::string_type config_home_dir;

			typename fs::path::string_type share_dir;
			typename fs::path::string_type music_dir;
			typename fs::path::string_type videos_dir;
			typename fs::path::string_type desktop_dir;
			typename fs::path::string_type downloads_dir;
			typename fs::path::string_type documents_dir;
			typename fs::path::string_type templates_dir;

			std::vector<fs::discovered_path> temp_dirs;
			std::vector<fs::discovered_path> data_dirs;
			std::vector<fs::discovered_path> state_dirs;
			std::vector<fs::discovered_path> config_dirs;
		};

		/* Platform-specific discovery hooks. */
		result<> find_temp_dirs(std::vector<fs::discovered_path> &) noexcept;
		result<> find_data_dirs(std::vector<fs::discovered_path> &) noexcept;
		result<> find_state_dirs(std::vector<fs::discovered_path> &) noexcept;
		result<> find_config_dirs(std::vector<fs::discovered_path> &) noexcept;
	}

	namespace fs
	{
		/** Regular expression used to match temporary directories that are backed by physical storage. */
		static constexpr const char storage_backed_regex[] = "ufs|xfs|zfs|hfs|ext[2-4]|f2fs|apfs|jfs|lxfs|cifs|btrfs|unionfs|lustre|overlay2?|nilf2|msdosfs|fat|vfat|fat32|exfat|ntfs|smbfs|nfs[1-9]?|newnfs";
		/** Regular expression used to match temporary directories that are backed by a networked filesystem. */
		static constexpr const char network_backed_regex[] = "cifs|nfs[1-9]?|lustre|smbfs";
		/** Regular expression used to match temporary directories that are backed by memory. */
		static constexpr const char memory_backed_regex[] = "tmpfs|ramfs";

		/** Returns handle to the current working directory. */
		[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_working_directory() noexcept;
		/** Returns handle to the program install directory. Install directory is the parent directory of the current process' executable. */
		[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_install_directory() noexcept;
		/** Returns handle to the current user's runtime directory. Runtime directory is a local filesystem directory used to store user-specific runtime files such as named pipes and sockets.
		 * This directory corresponds to <a href="https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html">$XDG_RUNTIME_DIR</a>.
		 * @note Not all operating systems have support for user runtime directories. `temp_file_directory` and `temp_pipe_directory` may yield a more accurate result. */
		[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_runtime_directory() noexcept;

		/** Returns handle to the working directory acquired during static initialization. */
		[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_working_directory() noexcept;
		/** Returns handle to the install directory acquired during static initialization. Install directory is the parent directory of the current process' executable. */
		[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_install_directory() noexcept;
		/** Returns handle to the user's runtime directory acquired during static initialization. Runtime directory is a local filesystem directory used to store user-specific runtime files such as named pipes and sockets.
		 * This directory corresponds to <a href="https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html">$XDG_RUNTIME_DIR</a>.
		 * @note Not all operating systems have support for user runtime directories. `temp_file_directory` and `temp_pipe_directory` may yield a more accurate result. */
		[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_runtime_directory() noexcept;

		/** Returns handle to the current user's home directory. */
		[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_user_home_directory() noexcept;
		/** Returns handle to the current user's data home directory. Data home directory is used to store user-specific data files.
		 * This directory corresponds to <a href="https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html">$XDG_DATA_HOME</a>. */
		[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_data_home_directory() noexcept;
		/** Returns handle to the current user's cache home directory. Cache home directory is used to store user-specific cache files.
		 * This directory corresponds to <a href="https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html">XDG_CACHE_HOME</a>. */
		[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_cache_home_directory() noexcept;
		/** Returns handle to the current user's state home directory. State home directory is used to store user-specific state files.
		 * This directory corresponds to <a href="https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html">XDG_STATE_HOME</a>. */
		[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_state_home_directory() noexcept;
		/** Returns handle to the current user's config home directory. Config home directory is used to store user-specific config files.
		 * This directory corresponds to <a href="https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html">$XDG_CONFIG_HOME</a>. */
		[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_config_home_directory() noexcept;

		/** Returns handle to the user's home directory acquired during static initialization. */
		[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_user_home_directory() noexcept;
		/** Returns handle to the user's data home directory acquired during static initialization. Data home directory is used to store user-specific data files.
		 * This directory corresponds to <a href="https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html">$XDG_DATA_HOME</a>. */
		[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_data_home_directory() noexcept;
		/** Returns handle to the user's cache home directory acquired during static initialization. Cache home directory is used to store user-specific cache files.
		 * This directory corresponds to <a href="https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html">XDG_CACHE_HOME</a>. */
		[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_cache_home_directory() noexcept;
		/** Returns handle to the user's state home directory acquired during static initialization. State home directory is used to store user-specific state files.
		 * This directory corresponds to <a href="https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html">XDG_STATE_HOME</a>. */
		[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_state_home_directory() noexcept;
		/** Returns handle to the user's config home directory acquired during static initialization. Config home directory is used to store user-specific config files.
		 * This directory corresponds to <a href="https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html">$XDG_CONFIG_HOME</a>. */
		[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_config_home_directory() noexcept;

		/** Returns handle to the current user's share directory. This directory corresponds to `$XDG_PUBLICSHARE_DIR`. */
		[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_share_directory() noexcept;
		/** Returns handle to the current user's music directory. This directory corresponds to `$XDG_MUSIC_DIR`. */
		[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_music_directory() noexcept;
		/** Returns handle to the current user's videos directory. This directory corresponds to `$XDG_VIDEOS_DIR`. */
		[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_videos_directory() noexcept;
		/** Returns handle to the current user's desktop directory. This directory corresponds to `$XDG_DESKTOP_DIR`. */
		[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_desktop_directory() noexcept;
		/** Returns handle to the current user's downloads directory. This directory corresponds to `$XDG_DOWNLOAD_DIR`. */
		[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_downloads_directory() noexcept;
		/** Returns handle to the current user's documents directory. This directory corresponds to `$XDG_DOCUMENTS_DIR`. */
		[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_documents_directory() noexcept;
		/** Returns handle to the current user's templates directory. This directory corresponds to `$XDG_TEMPLATES_DIR`. */
		[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_templates_directory() noexcept;

		/** Returns handle to the user's share directory acquired during static initialization. This directory corresponds to `$XDG_PUBLICSHARE_DIR`. */
		[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_share_directory() noexcept;
		/** Returns handle to the user's music directory acquired during static initialization. This directory corresponds to `$XDG_MUSIC_DIR`. */
		[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_music_directory() noexcept;
		/** Returns handle to the user's videos directory acquired during static initialization. This directory corresponds to `$XDG_VIDEOS_DIR`. */
		[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_videos_directory() noexcept;
		/** Returns handle to the user's desktop directory acquired during static initialization. This directory corresponds to `$XDG_DESKTOP_DIR`. */
		[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_desktop_directory() noexcept;
		/** Returns handle to the user's downloads directory acquired during static initialization. This directory corresponds to `$XDG_DOWNLOAD_DIR`. */
		[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_downloads_directory() noexcept;
		/** Returns handle to the user's documents directory acquired during static initialization. This directory corresponds to `$XDG_DOCUMENTS_DIR`. */
		[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_documents_directory() noexcept;
		/** Returns handle to the user's templates directory acquired during static initialization. This directory corresponds to `$XDG_TEMPLATES_DIR`. */
		[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_templates_directory() noexcept;

		/** Returns handle to a directory used for creation of temporary files. */
		[[nodiscard]] ROD_API_PUBLIC const directory_handle &temp_file_directory() noexcept;
		/** Returns handle to a directory used for creation of temporary named pipes. */
		[[nodiscard]] ROD_API_PUBLIC const directory_handle &temp_pipe_directory() noexcept;

		/** Returns a vector of system-specific temporary file directories sorted by discovery source.
		 * @param override Span of directory paths used for `discovery_source::override` source.
		 * @param fallback Span of directory paths used for `discovery_source::fallback` source.
		 * @return Vector of `discovered_path` structures describing discovered temporary directories or a status code on failure. */
		[[nodiscard]] ROD_API_PUBLIC result<std::vector<discovered_path>> temp_directory_paths(std::span<const path_view> override = {}, std::span<const path_view> fallback = {}, bool refresh_cache = false) noexcept;
		/** Returns a vector of system-specific data directories sorted by discovery source.
		 * @param override Span of directory paths used for `discovery_source::override` source.
		 * @param fallback Span of directory paths used for `discovery_source::fallback` source.
		 * @return Vector of `discovered_path` structures describing discovered data directories or a status code on failure. */
		[[nodiscard]] ROD_API_PUBLIC result<std::vector<discovered_path>> data_directory_paths(std::span<const path_view> override = {}, std::span<const path_view> fallback = {}, bool refresh_cache = false) noexcept;
		/** Returns a vector of system-specific state directories sorted by discovery source.
		 * @param override Span of directory paths used for `discovery_source::override` source.
		 * @param fallback Span of directory paths used for `discovery_source::fallback` source.
		 * @return Vector of `discovered_path` structures describing discovered state directories or a status code on failure. */
		[[nodiscard]] ROD_API_PUBLIC result<std::vector<discovered_path>> state_directory_paths(std::span<const path_view> override = {}, std::span<const path_view> fallback = {}, bool refresh_cache = false) noexcept;
		/** Returns a vector of system-specific config directories sorted by discovery source.
		 * @param override Span of directory paths used for `discovery_source::override` source.
		 * @param fallback Span of directory paths used for `discovery_source::fallback` source.
		 * @return Vector of `discovered_path` structures describing discovered config directories or a status code on failure. */
		[[nodiscard]] ROD_API_PUBLIC result<std::vector<discovered_path>> config_directory_paths(std::span<const path_view> override = {}, std::span<const path_view> fallback = {}, bool refresh_cache = false) noexcept;
	}

	result<fs::directory_handle> fs::directory_handle::open_temporary(fs::path_view path, fs::file_flags flags, fs::open_mode mode) noexcept
	{
		if (!path.empty())
			return open(fs::temp_file_directory(), path, flags, mode);
		else if (mode != fs::open_mode::existing)
			return open_unique(fs::temp_file_directory(), flags);
		else
			return std::make_error_code(std::errc::invalid_argument);
	}
}
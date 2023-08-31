/*
 * Created by switch_blade on 2023-08-15.
 */

#pragma once

#include <atomic>
#include <vector>
#include <span>

#include "path_handle.hpp"
#include "handle_stat.hpp"

namespace rod
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

	/** Flags used to control path discovery behavior. */
	enum class discovery_mode : int
	{
		/** Storage-backed directories will be considered for discovery. */
		storage_backed = 1,
		/** Network-backed directories will be considered for discovery. */
		network_backed = 2,
		/** Memory-backed directories will be considered for discovery. */
		memory_backed = 4,
		/** Discovery mode with all flags set. */
		all = storage_backed | network_backed | memory_backed,
	};

	[[nodiscard]] constexpr discovery_mode operator~(discovery_mode h) noexcept { return discovery_mode(~int(h)); }
	[[nodiscard]] constexpr discovery_mode operator&(discovery_mode a, discovery_mode b) noexcept { return discovery_mode(int(a) & int(b)); }
	[[nodiscard]] constexpr discovery_mode operator|(discovery_mode a, discovery_mode b) noexcept { return discovery_mode(int(a) | int(b)); }
	[[nodiscard]] constexpr discovery_mode operator^(discovery_mode a, discovery_mode b) noexcept { return discovery_mode(int(a) ^ int(b)); }
	constexpr discovery_mode &operator&=(discovery_mode &a, discovery_mode b) noexcept { return a = a & b; }
	constexpr discovery_mode &operator|=(discovery_mode &a, discovery_mode b) noexcept { return a = a | b; }
	constexpr discovery_mode &operator^=(discovery_mode &a, discovery_mode b) noexcept { return a = a ^ b; }

	/** Structure representing metadata of a discovered path. */
	struct discovered_path
	{
		/** Value of the discovered path. */
		path_view path;
		/** Source type of the discovered path. */
		discovery_source source = {};
		/** Flag indicating whether the target directory is backed by storage. */
		bool storage_backed : 1 = false;
		/** Flag indicating whether the target directory is backed by network. */
		bool network_backed : 1 = false;
		/** Flag indicating whether the target directory is backed by memory. */
		bool memory_backed : 1 = false;
	};

	namespace _detail
	{
		static bool match_storage_backed(typename path::string_view_type) noexcept;
		static bool match_memory_backed(typename path::string_view_type) noexcept;

		/* Platform-specific discovery hooks. */
		static result<> find_temp_dirs(typename path::string_type &, std::vector<discovered_path> &) noexcept;
		static result<> find_data_dirs(typename path::string_type &, std::vector<discovered_path> &) noexcept;
		static result<> find_state_dirs(typename path::string_type &, std::vector<discovered_path> &) noexcept;
		static result<> find_config_dirs(typename path::string_type &, std::vector<discovered_path> &) noexcept;

		static result<path> find_working_dir() noexcept;
		static result<path> find_install_dir() noexcept;
		static result<path> find_runtime_dir() noexcept;
		static result<path> find_temp_file_dir() noexcept;
		static result<path> find_temp_pipe_dir() noexcept;
		static result<path> find_user_home_dir() noexcept;
		static result<path> find_data_home_dir() noexcept;
		static result<path> find_cache_home_dir() noexcept;
		static result<path> find_state_home_dir() noexcept;
		static result<path> find_config_home_dir() noexcept;

		struct discovery_cache
		{
			[[nodiscard]] inline static std::vector<discovered_path> filter_paths(discovery_mode, std::span<const discovered_path>, std::span<const path_view>, std::span<const path_view>) noexcept;
			template<typename F>
			[[nodiscard]] inline static result<> refresh_dirs(F &&, typename path::string_type &, std::vector<discovered_path> &) noexcept;
			template<auto discovery_cache::*Path>
			[[nodiscard]] inline static const directory_handle &cached_dir_handle() noexcept;

			static discovery_cache &instance();
			inline discovery_cache();

			void unlock() noexcept
			{
				guard.clear(std::memory_order_release);
				guard.notify_one();
			}
			void lock() noexcept
			{
				while (guard.test_and_set(std::memory_order_acq_rel))
					guard.wait(true);
			}

			[[nodiscard]] result<> refresh_temp_dirs() noexcept { return refresh_dirs(find_temp_dirs, temp_dirs_buff, temp_dirs); }
			[[nodiscard]] result<> refresh_data_dirs() noexcept { return refresh_dirs(find_data_dirs, data_dirs_buff, data_dirs); }
			[[nodiscard]] result<> refresh_state_dirs() noexcept { return refresh_dirs(find_state_dirs, state_dirs_buff, state_dirs); }
			[[nodiscard]] result<> refresh_config_dirs() noexcept { return refresh_dirs(find_config_dirs, config_dirs_buff, config_dirs); }

			std::atomic_flag guard;

			typename path::string_type working_dir;
			typename path::string_type install_dir;
			typename path::string_type runtime_dir;
			typename path::string_type user_home_dir;
			typename path::string_type data_home_dir;
			typename path::string_type cache_home_dir;
			typename path::string_type state_home_dir;
			typename path::string_type config_home_dir;

			typename path::string_type temp_dirs_buff;
			typename path::string_type data_dirs_buff;
			typename path::string_type state_dirs_buff;
			typename path::string_type config_dirs_buff;

			std::vector<discovered_path> temp_dirs;
			std::vector<discovered_path> data_dirs;
			std::vector<discovered_path> state_dirs;
			std::vector<discovered_path> config_dirs;
		};
	}

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
	/** Returns handle to the current user's runtime directory. Runtime directory is a local filesystem directory used to store user-specific runtime files such as named pipes and sockets. */
	[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_runtime_directory() noexcept;

	/** Returns handle to the working directory acquired during static initialization. */
	[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_working_directory() noexcept;
	/** Returns handle to the install directory acquired during static initialization. Install directory is the parent directory of the current process' executable. */
	[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_install_directory() noexcept;
	/** Returns handle to the user's runtime directory acquired during static initialization. Runtime directory is a local filesystem directory used to store user-specific runtime files such as named pipes and sockets. */
	[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_runtime_directory() noexcept;

	/** Returns handle to the current user's home directory. */
	[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_user_home_directory() noexcept;
	/** Returns handle to the current user's data home directory. Data home directory is used to store user-specific data files. */
	[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_data_home_directory() noexcept;
	/** Returns handle to the current user's cache home directory. Cache home directory is used to store user-specific cache files. */
	[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_cache_home_directory() noexcept;
	/** Returns handle to the current user's state home directory. State home directory is used to store user-specific state files. */
	[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_state_home_directory() noexcept;
	/** Returns handle to the current user's config home directory. Config home directory is used to store user-specific config files. */
	[[nodiscard]] ROD_API_PUBLIC result<directory_handle> current_config_home_directory() noexcept;

	/** Returns handle to the user's home directory acquired during static initialization. */
	[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_user_home_directory() noexcept;
	/** Returns handle to the user's data home directory acquired during static initialization. Data home directory is used to store user-specific data files. */
	[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_data_home_directory() noexcept;
	/** Returns handle to the user's cache home directory acquired during static initialization. Cache home directory is used to store user-specific cache files. */
	[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_cache_home_directory() noexcept;
	/** Returns handle to the user's state home directory acquired during static initialization. State home directory is used to store user-specific state files. */
	[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_state_home_directory() noexcept;
	/** Returns handle to the user's config home directory acquired during static initialization. Config home directory is used to store user-specific config files. */
	[[nodiscard]] ROD_API_PUBLIC const directory_handle &starting_config_home_directory() noexcept;

	/** Returns handle to a directory used for creation of temporary files. */
	[[nodiscard]] ROD_API_PUBLIC const directory_handle &temporary_file_directory() noexcept;
	/** Returns handle to a directory used for creation of temporary named pipes. */
	[[nodiscard]] ROD_API_PUBLIC const directory_handle &temporary_pipe_directory() noexcept;

	/** Returns path of the current directory as if via `getcwd`. */
	[[nodiscard]] ROD_API_PUBLIC result<path> current_path() noexcept;
	/** Changes path of the current directory to \a path as if via `chdir`. */
	[[nodiscard]] ROD_API_PUBLIC fs_result<> current_path(path_view path) noexcept;

	/** Returns a span of system-specific temporary directories, sorted by discovery source.
	 * @param refresh If set to `true`, system temporary directory cache is refreshed.
	 * @return Span of `discovered_path` structures describing discovered temporary directories. */
	[[nodiscard]] ROD_API_PUBLIC std::span<const discovered_path> temporary_directory_paths(bool refresh = false) noexcept;
	/** Returns a span of system-specific temporary directories, sorted by discovery source and filtered using discovery mode \a mode.
	 * @param mode Discovery mode flags used to validate resulting temporary directories.
	 * @param refresh If set to `true`, system temporary directory cache is refreshed.
	 * @return Vector of `discovered_path` structures describing discovered temporary directories. */
	[[nodiscard]] ROD_API_PUBLIC std::vector<discovered_path> temporary_directory_paths(discovery_mode mode, bool refresh = false);

	/** Returns a vector of system-specific temporary directories with overrides & fallbacks, sorted by discovery source.
	 * @param override Span of directory paths used for `discovery_source::override` source.
	 * @param fallback Span of directory paths used for `discovery_source::fallback` source.
	 * @param refresh If set to `true`, system temporary directory cache is refreshed.
	 * @return Vector of `discovered_path` structures describing discovered temporary directories. */
	[[nodiscard]] ROD_API_PUBLIC std::vector<discovered_path> temporary_directory_paths(std::span<const path_view> override, std::span<const path_view> fallback = {}, bool refresh = false);
	/** Returns a vector of system-specific temporary directories with overrides & fallbacks, sorted by discovery source and filtered using discovery mode \a mode.
	 * @param mode Discovery mode flags used to validate resulting temporary directories.
	 * @param override Span of directory paths used for `discovery_source::override` source.
	 * @param fallback Span of directory paths used for `discovery_source::fallback` source.
	 * @param refresh If set to `true`, system temporary directory cache is refreshed.
	 * @return Vector of `discovered_path` structures describing discovered temporary directories. */
	[[nodiscard]] ROD_API_PUBLIC std::vector<discovered_path> temporary_directory_paths(discovery_mode mode, std::span<const path_view> override, std::span<const path_view> fallback = {}, bool refresh = false);
}
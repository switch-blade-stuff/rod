/*
 * Created by switch_blade on 2023-08-15.
 */

#include "path_discovery.hpp"

#if defined(ROD_WIN32)
#include <direct.h>
#endif

#include <cerrno>
#include <mutex>

namespace rod
{
	namespace _detail
	{
		/* Not using regex here since std::regex is slow AF, and I don't want to depend on external regex lib. */
		template<typename C, std::size_t N>
		inline static bool match_masks(std::basic_string_view<C> name, std::initializer_list<std::array<C, N>> masks) noexcept
		{
			if (name.size() <= 2 || name.size() > N)
				return false;

			std::array<C, N> buff = {};
			for (std::size_t i = 0; i < name.size(); ++i)
				buff[i] = name[i] | ' ';

			for (auto &mask : masks)
			{
				if (std::memcmp(mask.data(), buff.data(), sizeof(C) * N) == 0)
					return true;
			}
			return false;
		}
		bool match_storage_backed(typename path::string_view_type name) noexcept
		{
			using buff_t = std::array<typename path::value_type, 8>;
			constexpr buff_t overlay2_mask = {'o', 'v', 'e', 'r', 'l', 'a', 'y', '2'};
			constexpr buff_t overlay_mask = {'o', 'v', 'e', 'r', 'l', 'a', 'y'};
			constexpr buff_t msdosfs_mask = {'m', 's', 'd', 'o', 's', 'f', 's'};
			constexpr buff_t unionfs_mask = {'u', 'n', 'i', 'o', 'n', 'f', 's'};
			constexpr buff_t lustre_mask = {'l', 'u', 's', 't', 'r', 'e'};
			constexpr buff_t newnfs_mask = {'n', 'e', 'w', 'n', 'f', 's'};
			constexpr buff_t btrfs_mask = {'b', 't', 'r', 'f', 's'};
			constexpr buff_t exfat_mask = {'e', 'x', 'f', 'a', 't'};
			constexpr buff_t fat32_mask = {'f', 'a', 't', '3', '2'};
			constexpr buff_t nilf2_mask = {'n', 'i', 'l', 'f', '2'};
			constexpr buff_t smbfs_mask = {'s', 'm', 'b', 'f', 's'};
			constexpr buff_t apfs_mask = {'a', 'p', 'f', 's'};
			constexpr buff_t cifs_mask = {'c', 'i', 'f', 's'};
			constexpr buff_t ext2_mask = {'e', 'x', 't', '2'};
			constexpr buff_t ext3_mask = {'e', 'x', 't', '3'};
			constexpr buff_t ext4_mask = {'e', 'x', 't', '4'};
			constexpr buff_t f2fs_mask = {'f', '2', 'f', 's'};
			constexpr buff_t lxfs_mask = {'l', 'x', 'f', 's'};
			constexpr buff_t nfs1_mask = {'n', 'f', 's', '1'};
			constexpr buff_t nfs2_mask = {'n', 'f', 's', '2'};
			constexpr buff_t nfs3_mask = {'n', 'f', 's', '3'};
			constexpr buff_t nfs4_mask = {'n', 'f', 's', '4'};
			constexpr buff_t nfs5_mask = {'n', 'f', 's', '5'};
			constexpr buff_t nfs6_mask = {'n', 'f', 's', '6'};
			constexpr buff_t nfs7_mask = {'n', 'f', 's', '7'};
			constexpr buff_t nfs8_mask = {'n', 'f', 's', '8'};
			constexpr buff_t nfs9_mask = {'n', 'f', 's', '9'};
			constexpr buff_t ntfs_mask = {'n', 't', 'f', 's'};
			constexpr buff_t vfat_mask = {'v', 'f', 'a', 't'};
			constexpr buff_t fat_mask = {'f', 'a', 't'};
			constexpr buff_t hfs_mask = {'h', 'f', 's'};
			constexpr buff_t jfs_mask = {'j', 'f', 's'};
			constexpr buff_t nfs_mask = {'n', 'f', 's'};
			constexpr buff_t ufs_mask = {'u', 'f', 's'};
			constexpr buff_t xfs_mask = {'x', 'f', 's'};
			constexpr buff_t zfs_mask = {'z', 'f', 's'};

			return match_masks(name, {overlay2_mask, overlay_mask, msdosfs_mask, unionfs_mask, lustre_mask, newnfs_mask, btrfs_mask, exfat_mask, fat32_mask, nilf2_mask, smbfs_mask, apfs_mask, cifs_mask,
			                          ext2_mask, ext3_mask, ext4_mask, f2fs_mask, lxfs_mask, nfs1_mask, nfs2_mask, nfs3_mask, nfs4_mask, nfs5_mask, nfs6_mask, nfs7_mask, nfs8_mask, nfs9_mask, ntfs_mask,
			                          vfat_mask, fat_mask, hfs_mask, jfs_mask, nfs_mask, ufs_mask, xfs_mask, zfs_mask});
		}
		bool match_memory_backed(typename path::string_view_type name) noexcept
		{
			constexpr std::array<typename path::value_type, 5> ramfs_mask = {'r', 'a', 'm', 'f', 's'};
			constexpr std::array<typename path::value_type, 5> tmpfs_mask = {'t', 'm', 'p', 'f', 's'};
			return match_masks(name, {tmpfs_mask, ramfs_mask});
		}

		static constinit std::atomic<int> cache_init;
		static discovery_cache cache_data;

		discovery_cache &discovery_cache::instance()
		{
			for (;;)
			{
				/* Try to prematurely initialize the cache if required. This is technically UB but should be ok. */
				const auto flag = cache_init.load(std::memory_order_acquire);
				if (flag == 2)
					break;
				if (flag == 0)
				{
					new (&cache_data) discovery_cache();
					return cache_data;
				}
				cache_init.wait(flag);
			}
		}
		discovery_cache::discovery_cache()
		{
			working_dir = find_working_dir().value();
			install_dir = find_install_dir().value();
			runtime_dir = find_runtime_dir().value();

			cache_init.store(2, std::memory_order_release);
			cache_init.notify_all();
		}

		std::vector<discovered_path> discovery_cache::filter_paths(discovery_mode mode, std::span<const discovered_path> cache, std::span<const path_view> override, std::span<const path_view> fallback) noexcept
		{
			std::vector<discovered_path> result;
			result.reserve(cache.size() + override.size() + fallback.size());

			/* TODO: Go through each path, get it's file & volume stats and apply mode filter. */
		}
		template<typename F>
		result<> discovery_cache::refresh_dirs(F &&find, typename path::string_type &buff, std::vector<discovered_path> &dirs) noexcept
		{
			if (auto res = find(buff, dirs); res.has_error()) [[unlikely]]
				return res;

			/* Remove duplicates & sort paths using the source type. */
			dirs.erase(std::unique(dirs.begin(), dirs.end(), [](auto &a, auto &b) { return a.source == b.source && a.path == b.path; }), dirs.end());
			std::ranges::sort(dirs, [](auto &a, auto &b) { return a.source < b.source && a.path < b.path; });
			return {};
		}
		template<auto discovery_cache::*Path>
		const directory_handle &discovery_cache::cached_dir_handle() noexcept
		{
			static const auto handle = open_directory(instance().*Path).value_or({});
			return handle;
		}
	}

	const directory_handle &starting_working_directory() noexcept { return _detail::discovery_cache::cached_dir_handle<&_detail::discovery_cache::working_dir>(); }
	const directory_handle &starting_install_directory() noexcept { return _detail::discovery_cache::cached_dir_handle<&_detail::discovery_cache::install_dir>(); }
	const directory_handle &starting_runtime_directory() noexcept { return _detail::discovery_cache::cached_dir_handle<&_detail::discovery_cache::runtime_dir>(); }

	result<path> current_path() noexcept { return _detail::find_working_dir(); }
	result<> current_path(path_view path) noexcept
	{
		const auto rpath = path.render_null_terminated();
#if defined(ROD_WIN32)
		const auto res = _wchdir(rpath.c_str());
#else
		const auto res = chdir(rpath.c_str());
#endif
		if (res) [[unlikely]]
			return std::error_code(errno, std::generic_category());
		else
			return {};
	}

	std::span<const discovered_path> temporary_directory_paths(bool refresh) noexcept
	{
		auto &cache = _detail::discovery_cache::instance();
		auto g = std::lock_guard(cache);

		if (refresh || cache.temp_dirs.empty()) [[unlikely]]
			(void) cache.refresh_temp_dirs();
		return cache.temp_dirs;
	}
	std::vector<discovered_path> temporary_directory_paths(discovery_mode mode, bool refresh)
	{
		return temporary_directory_paths(mode, {}, {}, refresh);
	}
	std::vector<discovered_path> temporary_directory_paths(std::span<const path_view> override, std::span<const path_view> fallback, bool refresh)
	{
		return temporary_directory_paths(discovery_mode::all, override, fallback, refresh);
	}
	std::vector<discovered_path> temporary_directory_paths(discovery_mode mode, std::span<const path_view> override, std::span<const path_view> fallback, bool refresh)
	{
		return _detail::discovery_cache::filter_paths(mode, temporary_directory_paths(refresh), override, fallback);
	}
}
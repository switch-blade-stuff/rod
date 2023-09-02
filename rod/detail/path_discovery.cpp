/*
 * Created by switch_blade on 2023-08-15.
 */

#include "path_discovery.hpp"
#include "path_util.hpp"

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

		result<discovery_cache> &discovery_cache::instance()
		{
			static auto value = []() -> result<discovery_cache>
			{
				discovery_cache result;
				if (auto path = current_path(); path.has_value()) [[likely]]
					result.working_dir = std::move(*path);
				else
					return path.error();
				if (auto path = find_install_dir(); path.has_value()) [[likely]]
					result.install_dir = std::move(*path);
				else
					return path.error();
				if (auto path = find_runtime_dir(); path.has_value()) [[likely]]
					result.runtime_dir = std::move(*path);
				else
					return path.error();
				return std::move(result);
			}();
			return value;
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
			static const auto handle = []() -> directory_handle
			{
				if (const auto &cache = instance(); cache.has_value() && !((*cache).*Path).empty()) [[likely]]
					return directory_handle::open((*cache).*Path).value_or({});
				else
					return {};
			}();
			return handle;
		}
	}

	const directory_handle &starting_working_directory() noexcept { return _detail::discovery_cache::cached_dir_handle<&_detail::discovery_cache::working_dir>(); }
	const directory_handle &starting_install_directory() noexcept { return _detail::discovery_cache::cached_dir_handle<&_detail::discovery_cache::install_dir>(); }
	const directory_handle &starting_runtime_directory() noexcept { return _detail::discovery_cache::cached_dir_handle<&_detail::discovery_cache::runtime_dir>(); }

	result<std::vector<discovered_path>> temporary_directory_paths(discovery_mode mode, std::span<const path_view> override, std::span<const path_view> fallback, bool refresh) noexcept
	{
		auto &cache = _detail::discovery_cache::instance();
		if (cache.has_error()) [[unlikely]]
			return cache.error();

		auto guard = std::lock_guard(*cache);
		if (result<> res; (refresh || cache->temp_dirs.empty()) && (res = cache->refresh_temp_dirs()).has_error()) [[unlikely]]
			return res.error();

		try
		{
			/* TODO: Go through cache, override & fallback dirs and apply the filter. */
			return cache->temp_dirs;
		}
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
		catch (const std::system_error &e) { return e.code(); }
	}

	const directory_handle &temporary_file_directory() noexcept
	{
		static const directory_handle value;
		return value;
	}
	const directory_handle &temporary_path_directory() noexcept
	{
		static const directory_handle value;
		return value;
	}
}
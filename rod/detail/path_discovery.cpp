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
		template<std::size_t N>
		inline static bool match_masks(std::string_view name, std::initializer_list<std::array<char, N>> masks) noexcept
		{
			if (name.size() <= 2 || name.size() > N)
				return false;

			std::array<char, N> buff = {};
			for (std::size_t i = 0; i < name.size(); ++i)
				buff[i] = name[i] | ' ';

			for (auto &mask : masks)
			{
				if (std::memcmp(mask.data(), buff.data(), N) == 0)
					return true;
			}
			return false;
		}
		bool match_memory_backed(std::string_view name) noexcept
		{
			constexpr auto ramfs_mask = std::array{'r', 'a', 'm', 'f', 's'};
			constexpr auto tmpfs_mask = std::array{'t', 'm', 'p', 'f', 's'};
			return match_masks(name, {tmpfs_mask, ramfs_mask});
		}
		bool match_storage_backed(std::string_view name) noexcept
		{
			using buff_t = std::array<char, 8>;
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
		bool match_network_backed(std::string_view name) noexcept
		{
			using buff_t = std::array<char, 6>;
			constexpr buff_t lustre_mask = {'l', 'u', 's', 't', 'r', 'e'};
			constexpr buff_t smbfs_mask = {'s', 'm', 'b', 'f', 's'};
			constexpr buff_t nfs1_mask = {'n', 'f', 's', '1'};
			constexpr buff_t nfs2_mask = {'n', 'f', 's', '2'};
			constexpr buff_t nfs3_mask = {'n', 'f', 's', '3'};
			constexpr buff_t nfs4_mask = {'n', 'f', 's', '4'};
			constexpr buff_t nfs5_mask = {'n', 'f', 's', '5'};
			constexpr buff_t nfs6_mask = {'n', 'f', 's', '6'};
			constexpr buff_t nfs7_mask = {'n', 'f', 's', '7'};
			constexpr buff_t nfs8_mask = {'n', 'f', 's', '8'};
			constexpr buff_t nfs9_mask = {'n', 'f', 's', '9'};
			constexpr buff_t cifs_mask = {'c', 'i', 'f', 's'};
			return match_masks(name, {lustre_mask, smbfs_mask, nfs1_mask, nfs2_mask, nfs3_mask, nfs4_mask, nfs5_mask, nfs6_mask, nfs7_mask, nfs8_mask, nfs9_mask, cifs_mask});
		}

		result<bool> query_discovered_dir(fs::discovery_mode mode, fs::discovered_path &dir) noexcept
		{
			/* TODO: Try to create a temporary file in the target directory to see if it's suitable and to do `get_fs_stat`. */

			/* Apply mode filter. */
			if (bool(mode & fs::discovery_mode::storage_backed) < dir.storage_backed)
				return false;
			if (bool(mode & fs::discovery_mode::network_backed) < dir.network_backed)
				return false;
			if (bool(mode & fs::discovery_mode::memory_backed) < dir.memory_backed)
				return false;

			return true;
		}

		result<discovery_cache> &discovery_cache::instance()
		{
			static auto value = []() -> result<discovery_cache>
			{
				discovery_cache result;
				if (auto path = fs::current_path(); path.has_value()) [[likely]]
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

		template<auto discovery_cache::*Path>
		auto discovery_cache::open_cached_dir() const noexcept -> fs::directory_handle
		{
			if (!(this->*Path).empty()) [[likely]]
				return fs::directory_handle::open({}, this->*Path).value_or({});
			else
				return {};
		}
		template<auto discovery_cache::*Dirs, typename F>
		auto discovery_cache::refresh_dirs(F &&find) noexcept -> result<>
		{
			auto &dirs = this->*Dirs;
			if (auto res = find(dirs); res.has_error()) [[unlikely]]
				return res;

			/* Remove duplicates & sort paths using the source type. */
			dirs.erase(std::unique(dirs.begin(), dirs.end(), [](auto &a, auto &b) { return a.source == b.source && a.path == b.path; }), dirs.end());
			std::ranges::sort(dirs, [](auto &a, auto &b) { return a.source < b.source && a.path < b.path; });
			return {};
		}
	}

	namespace fs
	{
		using _detail::discovery_cache;

		const directory_handle &starting_working_directory() noexcept
		{
			static const auto value = discovery_cache::instance().transform([](auto &c) { return c.template open_cached_dir<&discovery_cache::working_dir>(); }).value_or({});
			return value;
		}
		const directory_handle &starting_install_directory() noexcept
		{
			static const auto value = discovery_cache::instance().transform([](auto &c) { return c.template open_cached_dir<&discovery_cache::install_dir>(); }).value_or({});
			return value;
		}
		const directory_handle &starting_runtime_directory() noexcept
		{
			static const auto value = discovery_cache::instance().transform([](auto &c) { return c.template open_cached_dir<&discovery_cache::runtime_dir>(); }).value_or({});
			return value;
		}

		inline static result<bool> filter_dir(discovery_mode mode, discovered_path &dir, const discovered_path *entry) noexcept
		{
			result<directory_handle> hnd;
			if (entry != nullptr)
			{
//				dir.memory_backed = _detail::match_memory_backed(entry->fs_type);
//				dir.storage_backed = _detail::match_storage_backed(entry->fs_type);
				dir.network_backed = entry->network_backed;
			}
			else
			{
				fs_stat st;
				if (hnd = directory_handle::open({}, dir.path, file_flags::attr_read); hnd.has_error()) [[unlikely]]
					return {in_place_error, hnd.error()};
				if (auto res = get_fs_stat(st, *hnd, fs_stat::query::fs_type | fs_stat::query::flags); res.has_error()) [[unlikely]]
					return {in_place_error, res.error()};

				dir.memory_backed = _detail::match_memory_backed(st.fs_type);
				dir.storage_backed = _detail::match_storage_backed(st.fs_type);
				dir.network_backed = bool(st.flags & fs_flags::network);
			}


			/* TODO: check if we can create a file in the directory. */
#if 0
			auto tmp = file_handle::open_unique(dir.path, file_flags::unlink_on_close);
			if (tmp.)
#endif

			if (entry != nullptr)
			{
				dir.dev = entry->dev;
				dir.ino = entry->ino;
			}
			else
			{
				stat st;
				if (auto res = get_stat(st, *hnd, stat::query::dev | stat::query::ino); res.has_error()) [[unlikely]]
					return {in_place_error, res.error()};

				dir.dev = st.dev;
				dir.ino = st.ino;
			}
			return true;
		}

		result<std::vector<discovered_path>> temporary_directory_paths(discovery_mode mode, std::span<const path_view> override, std::span<const path_view> fallback, bool refresh) noexcept
		{
			auto &cache = discovery_cache::instance();
			if (cache.has_error()) [[unlikely]]
				return cache.error();

			auto guard = std::lock_guard(*cache);
			if (refresh || cache->temp_dirs.empty()) [[unlikely]]
			{
				auto res = cache->refresh_dirs<&discovery_cache::temp_dirs>(_detail::find_temp_dirs);
				if (res.has_error()) [[unlikely]]
					return res.error();
			}

			try
			{
				std::vector<discovered_path> result;
				result.reserve(override.size() + fallback.size() + cache->temp_dirs.size());

				for (auto &entry: override)
				{
					auto dir = discovered_path{.path = path(entry), .source = discovery_source::override};
					if (auto res = filter_dir(mode, dir, nullptr); res.has_value() && *res)
						result.push_back(dir);
					else if (res.has_error()) [[unlikely]]
						return res.error();
				}
				for (auto &entry: cache->temp_dirs)
				{
					auto dir = discovered_path{.path = entry.path, .source = entry.source};
					if (auto res = filter_dir(mode, dir, &entry); res.has_value() && *res)
						result.push_back(dir);
					else if (res.has_error()) [[unlikely]]
						return res.error();
				}
				for (auto &entry: fallback)
				{
					auto dir = discovered_path{.path = path(entry), .source = discovery_source::fallback};
					if (auto res = filter_dir(mode, dir, nullptr); res.has_value() && *res)
						result.push_back(dir);
					else if (res.has_error()) [[unlikely]]
						return res.error();
				}

				return std::move(result);
			}
			catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
			catch (const std::system_error &e) { return e.code(); }
		}

		const directory_handle &temporary_file_directory() noexcept
		{
			static const directory_handle value;
			return value;
		}
		const directory_handle &temporary_pipe_directory() noexcept
		{
			static const directory_handle value;
			return value;
		}
	}
}
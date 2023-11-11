/*
 * Created by switchblade on 2023-08-15.
 */

#include "path_discovery.hpp"
#include "file_handle.hpp"
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

		inline static auto init_cache() noexcept -> result<discovery_cache>;
		inline static auto lock_cache() noexcept;

		static std::atomic<discovery_cache::status_t> cache_status = {};
		static result<discovery_cache> cache_instance = init_cache();

		inline static auto init_cache() noexcept -> result<discovery_cache>
		{
			for (;;)
			{
				auto expected = discovery_cache::none;
				discovery_cache result;

				if (cache_status.compare_exchange_strong(expected, discovery_cache::busy, std::memory_order_acq_rel))
				{
					const auto guard = defer_invoke([]() noexcept
					{
						cache_status.store(discovery_cache::init, std::memory_order_release);
						cache_status.notify_one();
					});

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
				}
				if (expected == discovery_cache::busy)
					cache_status.wait(expected);
				if (expected == discovery_cache::init)
					return std::move(cache_instance);
			}
		}
		inline static auto lock_cache() noexcept
		{
			for (auto expected = discovery_cache::init;;)
			{
				if (cache_status.compare_exchange_strong(expected, discovery_cache::busy, std::memory_order_acq_rel))
					return defer_invoke([]() noexcept
					{
						cache_status.store(discovery_cache::init, std::memory_order_release);
						cache_status.notify_one();
					});
				if (expected == discovery_cache::busy)
					cache_status.wait(expected);
				if (expected != discovery_cache::init)
					cache_instance = init_cache();
			}
		}

		inline static auto acquire_cache() noexcept { return std::make_pair(std::reference_wrapper(cache_instance), lock_cache()); }

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
			dirs.clear();
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

		result<directory_handle> current_working_directory() noexcept
		{
			if (auto path = current_path(); path.has_value()) [[likely]]
				return directory_handle::open({}, *path);
			else
				return path.error();
		}
		result<directory_handle> current_install_directory() noexcept
		{
			if (auto path = _detail::find_install_dir(); path.has_value()) [[likely]]
				return directory_handle::open({}, *path);
			else
				return path.error();
		}
		result<directory_handle> current_runtime_directory() noexcept
		{
			if (auto path = _detail::find_runtime_dir(); path.has_value()) [[likely]]
				return directory_handle::open({}, *path);
			else
				return path.error();
		}

		const directory_handle &starting_working_directory() noexcept
		{
			static const auto value = _detail::acquire_cache().first.transform_value([](auto &c) { return c.template open_cached_dir<&discovery_cache::working_dir>(); }).value_or({});
			return value;
		}
		const directory_handle &starting_install_directory() noexcept
		{
			static const auto value = _detail::acquire_cache().first.transform_value([](auto &c) { return c.template open_cached_dir<&discovery_cache::install_dir>(); }).value_or({});
			return value;
		}
		const directory_handle &starting_runtime_directory() noexcept
		{
			static const auto value = _detail::acquire_cache().first.transform_value([](auto &c) { return c.template open_cached_dir<&discovery_cache::runtime_dir>(); }).value_or({});
			return value;
		}

		inline static bool verify_discovered_path(discovered_path &dir, discovery_mode mode) noexcept
		{
			constexpr auto fs_stat_mask = fs_stat::query::fs_type | fs_stat::query::flags;
			constexpr auto dir_stat_mask = stat::query::dev | stat::query::ino;

			fs_stat fs_st;
			stat dir_st;

			/* Open a handle to the directory and check if we can create a file. */
			auto path_hnd = path_handle::open({}, dir.path);
			if (path_hnd.has_error()) [[unlikely]]
				return false;
			auto file_hnd = file_handle::open_unique(*path_hnd, file_flags::attr_read | file_flags::unlink_on_close);
			if (file_hnd.has_error()) [[unlikely]]
				return false;

			/* Get filesystem and directory stats. */
			if ((get_stat(dir_st, *path_hnd, dir_stat_mask).value_or({}) & dir_stat_mask) != dir_stat_mask) [[unlikely]]
				return false;
			if ((get_fs_stat(fs_st, *file_hnd, fs_stat_mask).value_or({}) & fs_stat_mask) != fs_stat_mask) [[unlikely]]
				return false;

			/* Fill the discovery metadata. */
			dir.memory_backed = _detail::match_memory_backed(fs_st.fs_type);
			dir.storage_backed = _detail::match_storage_backed(fs_st.fs_type);
			dir.network_backed = bool(fs_st.flags & fs_flags::network);
			dir.dev = dir_st.dev;
			dir.ino = dir_st.ino;

			/* Compare against the discovery mode mask. */
			if (dir.memory_backed && !bool(mode & discovery_mode::memory_backed))
				return false;
			if (dir.storage_backed && !bool(mode & discovery_mode::storage_backed))
				return false;
			if (dir.network_backed && !bool(mode & discovery_mode::network_backed))
				return false;
			return true;
		}

		result<std::vector<discovered_path>> temporary_directory_paths(discovery_mode mode, std::span<const path_view> override, std::span<const path_view> fallback, bool refresh) noexcept
		{
			auto &&[cache, guard] = _detail::acquire_cache();
			if (cache.has_error()) [[unlikely]]
				return cache.error();

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
					if (verify_discovered_path(dir, mode))
						result.push_back(std::move(dir));
				}
				for (auto &entry: cache->temp_dirs)
				{
					auto dir = discovered_path{.path = entry.path, .source = entry.source};
					if (verify_discovered_path(dir, mode))
						result.push_back(std::move(dir));
				}
				for (auto &entry: fallback)
				{
					auto dir = discovered_path{.path = path(entry), .source = discovery_source::fallback};
					if (verify_discovered_path(dir, mode))
						result.push_back(std::move(dir));
				}

				return std::move(result);
			}
			catch (...) { return _detail::current_error(); }
		}

		const directory_handle &temporary_file_directory() noexcept
		{
			static const directory_handle value = []() noexcept
			{
				constexpr auto search_mode = discovery_mode::storage_backed | discovery_mode::network_backed | discovery_mode::memory_backed;

				if (auto preferred = _detail::preferred_temp_file_dir(); preferred.has_value() && !preferred->empty()) [[unlikely]]
					return directory_handle::open({}, *preferred).value_or({});
				if (auto discovered = temporary_directory_paths(search_mode, {}, {}, true); discovered.has_value() && !discovered->empty()) [[unlikely]]
					return directory_handle::open({}, discovered->front().path).value_or({});

				return directory_handle();
			}();
			return value;
		}
		const directory_handle &temporary_pipe_directory() noexcept
		{
			static const directory_handle value = []() noexcept
			{
				constexpr auto search_mode = discovery_mode::storage_backed | discovery_mode::memory_backed;

				if (auto preferred = _detail::preferred_temp_pipe_dir(); preferred.has_value() && !preferred->empty()) [[unlikely]]
					return directory_handle::open({}, *preferred).value_or({});
				if (auto discovered = temporary_directory_paths(search_mode, {}, {}, true); discovered.has_value() && !discovered->empty()) [[unlikely]]
					return directory_handle::open({}, discovered->front().path).value_or({});

				return directory_handle();
			}();
			return value;
		}
	}
}
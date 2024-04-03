/*
 * Created by switchblade on 2023-11-23.
 */

#include "mmap_handle.hpp"

namespace rod::_mmap
{
	inline static result<std::byte *> map_region(void *addr, int map_fd, std::int64_t &off, std::uint64_t &len, std::uint64_t &page_size, mmap_flags flags, int extra_flags) noexcept
	{
		if (map_fd < 0)
		{
			if (off != 0) [[unlikely]]
				return std::make_error_code(std::errc::not_supported);
			else
				map_fd = -1;
		}

		auto map_prot = PROT_NONE;
		if (bool(flags & mmap_flags::exec))
			map_prot |= PROT_EXEC;
		if (bool(flags & mmap_flags::read))
			map_prot |= PROT_READ;
		if (bool(flags & mmap_flags::write))
			map_prot |= PROT_WRITE;

		if ((flags & (mmap_flags::reserve | mmap_flags::commit)) == mmap_flags::none)
			flags |= mmap_flags::commit;

		/* Always shared by default to ensure write-through is accomplished on flush. */
		auto map_flags = bool(flags & mmap_flags::copy) ? MAP_PRIVATE : MAP_SHARED;
		if (map_fd == -1)
			map_flags |= MAP_ANONYMOUS;
#ifdef MAP_NOSYNC
		else if (_file::file_caching(source.flags >> 16) & _file::file_caching::temporary)
			map_flags |= MAP_NOSYNC;
#endif

		if (bool(flags & mmap_flags::copy))
		{
			map_flags = (map_flags & ~MAP_SHARED) | MAP_PRIVATE;
			map_prot |= PROT_READ | PROT_WRITE;
		}
		if (!bool(flags & mmap_flags::commit))
		{
#ifdef MAP_GUARD
			if (bool(map_flags & MAP_ANONYMOUS) && map_prot == PROT_NONE)
				map_flags |= MAP_GUARD;
#endif
#ifdef MAP_NORESERVE
			map_flags |= MAP_NORESERVE;
#endif
		}
		if (bool(flags & mmap_flags::prefault))
		{
#ifdef MAP_PREFAULT_READ
			map_flags |= MAP_PREFAULT_READ;
#endif
#ifdef MAP_POPULATE
			map_flags |= MAP_POPULATE;
#endif
		}

		auto map_page_size = page_size;
		if (const auto page_sizes = get_page_sizes(); map_page_size == 0)
		{
			if (bool(flags & mmap_flags::map_large_pages) && page_sizes.size() > 1)
				map_page_size = page_sizes.back();
			else
				map_page_size = page_sizes.front();
		}
		if (map_page_size > get_page_size())
		{
			const auto top = (__CHAR_BIT__ * sizeof(unsigned long)) - __builtin_clzl(static_cast<unsigned long>(map_page_size));
#if defined(__linux__)
#ifdef MAP_HUGE_SHIFT
			map_flags |= top << MAP_HUGE_SHIFT;
#endif
			map_flags |= MAP_HUGETLB;
#elif defined(__FreeBSD__)
			map_flags |= MAP_ALIGNED(top);
#elif defined(__APPLE__)
			if (bool(map_flags & MAP_ANONYMOUS))
				map_fd = ((top - 20) << VM_FLAGS_SUPERPAGE_SHIFT);
#endif
		}

		const auto map_len = len + (off & (map_page_size - 1));
		const auto map_off = off & ~(map_page_size - 1);
#ifdef __linux__
		const auto map_mem = static_cast<std::byte *>(::mmap64(addr, map_len, map_prot, map_flags | extra_flags, map_fd, map_off));
#else
		const auto map_mem = static_cast<std::byte *>(::mmap(addr, map_len, map_prot, map_flags | extra_flags, map_fd, map_off));
#endif
		if (map_mem == MAP_FAILED) [[unlikely]]
			return std::error_code(errno, std::system_category());

#ifdef MADV_FREE_REUSABLE
		if (bool(map_prot & PROT_WRITE) && !bool(flags & mmap_flags::commit))
		{
			if (::madvise(map_mem, map_len, MADV_FREE_REUSABLE) < 0) [[unlikely]]
				return std::error_code(errno, std::system_category());
		}
#endif

		len = map_len;
		off = map_off;
		page_size = map_page_size;
		return map_mem;
	}

	result<mmap_source> mmap_source::open(extent_type size, const fs::path_handle &base, mmap_flags flags) noexcept
	{
		if ((bool(flags & mmap_flags::copy) && bool(flags & mmap_flags::write)) || (bool(flags & (mmap_flags::commit | mmap_flags::prefault)) && bool(flags & mmap_flags::reserve))) [[unlikely]]
			return std::make_error_code(std::errc::invalid_argument);
		if (bool(flags & mmap_flags::reserve) && bool(flags & mmap_flags::map_large_pages)) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
#ifdef __linux__
		if (size > extent_type(std::numeric_limits<::off64_t>::max())) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);
#else
		if (size > extent_type(std::numeric_limits<::off_t>::max())) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);
#endif
		if ((flags & (mmap_flags::reserve | mmap_flags::commit)) == mmap_flags::none)
			flags |= mmap_flags::commit;

		auto tmp_file = fs::file_handle::open_anonymous(base);
		if (tmp_file.has_error()) [[unlikely]]
			return tmp_file.error();

		auto trunc_res = truncate(*tmp_file, size);
		if (trunc_res.has_error()) [[unlikely]]
			return trunc_res.error();

		return mmap_source(tmp_file->release(), flags);
	}
	result<mmap_source> mmap_source::open(const fs::file_handle &file, extent_type size, mmap_flags flags) noexcept
	{
		if ((bool(flags & mmap_flags::copy) && bool(flags & mmap_flags::write)) || (bool(flags & (mmap_flags::commit | mmap_flags::prefault)) && bool(flags & mmap_flags::reserve))) [[unlikely]]
			return std::make_error_code(std::errc::invalid_argument);
#ifdef __linux__
		if (size > extent_type(std::numeric_limits<::off64_t>::max())) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);
#else
		if (size > extent_type(std::numeric_limits<::off_t>::max())) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);
#endif
		if ((flags & (mmap_flags::reserve | mmap_flags::commit)) == mmap_flags::none)
			flags |= mmap_flags::commit;

		auto open_flags = file.flags();
		if (bool(flags & mmap_flags::read | mmap_flags::exec))
			open_flags |= fs::file_flags::read;
		if (bool(flags & mmap_flags::write | mmap_flags::copy))
			open_flags |= fs::file_flags::write;

		auto tmp_file = fs::file_handle::reopen(file, open_flags, file.caching());
		if (tmp_file.has_error()) [[unlikely]]
			return tmp_file.error();

		if (size != 0)
		{
			auto trunc_res = truncate(*tmp_file, size);
			if (trunc_res.has_error()) [[unlikely]]
				return trunc_res.error();
		}
		return mmap_source(tmp_file->release(), flags);
	}

	result<mmap_source::extent_type> mmap_source::do_endpos() const noexcept
	{
#ifdef __linux__
		struct ::stat64 st = {};
		const auto res = ::fstat64(native_handle(), &st);
#else
		struct ::stat st = {};
		const auto res = ::fstat(native_handle(), &st);
#endif
		if (res < 0) [[unlikely]]
			return std::error_code(errno, std::system_category());
		else
			return extent_type(st.st_size);
	}
	result<mmap_source::extent_type> mmap_source::do_truncate(extent_type endp) noexcept
	{
#ifdef __linux__
		if (endp > extent_type(std::numeric_limits<::off64_t>::max())) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);
		const auto res = ::ftruncate64(native_handle(), ::off64_t(endp));
#else
		if (endp > extent_type(std::numeric_limits<::off_t>::max())) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);
		const auto res = ::ftruncate(native_handle(), ::off_t(endp));
#endif
		if (res < 0) [[unlikely]]
			return std::error_code(errno, std::system_category());
		else
			return endp;
	}

	result<mmap_handle> mmap_handle::map(size_type size, mmap_flags flags) noexcept
	{
		if ((bool(flags & mmap_flags::copy) && bool(flags & mmap_flags::write)) || (bool(flags & (mmap_flags::commit | mmap_flags::prefault)) && bool(flags & mmap_flags::reserve))) [[unlikely]]
			return std::make_error_code(std::errc::invalid_argument);

		std::uint64_t page_size = 0, cap = size;
		std::int64_t off = 0;
		if (auto mem = map_region(nullptr, -1, off, cap, page_size, flags, 0); mem.has_value()) [[likely]]
			return mmap_handle(*mem, off, size, page_size, cap, flags);
		else
			return mem.error();
	}
	result<mmap_handle> mmap_handle::map(const mmap_source &src, extent_type offset, size_type size, mmap_flags flags) noexcept
	{
		if ((bool(flags & mmap_flags::copy) && bool(flags & mmap_flags::write)) || (bool(flags & (mmap_flags::commit | mmap_flags::prefault)) && bool(flags & mmap_flags::reserve)) || !src.is_open()) [[unlikely]]
			return std::make_error_code(std::errc::invalid_argument);
#ifdef __linux__
		if (offset > extent_type(std::numeric_limits<::off64_t>::max()) || size > extent_type(std::numeric_limits<::off64_t>::max())) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);
#else
		if (offset > extent_type(std::numeric_limits<::off_t>::max()) || size > extent_type(std::numeric_limits<::off_t>::max())) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);
#endif

		auto map_size = endpos(src);
		if (map_size.has_error()) [[unlikely]]
			return map_size.error();

		if (*map_size > offset)
			*map_size -= offset;
		else
			*map_size = 0;
		if (size == 0) /* Make sure not to overflow when casting to size_type. */
			size = size_type(std::min(*map_size, extent_type(std::numeric_limits<size_type>::max())));

		std::uint64_t page_size = 0, cap = size;
		std::int64_t off = offset;
		if (auto mem = map_region(nullptr, src.native_handle(), off, cap, page_size, flags, 0); mem.has_value()) [[likely]]
			return mmap_handle(*mem, off, size, page_size, cap, flags, {});
		else
			return mem.error();
	}

	result<void> mmap_handle::reset() noexcept
	{
		if (!empty()) [[likely]]
		{
			if (has_source() && bool(flags() & (mmap_flags::write | mmap_flags::copy)) && bool(flags() & mmap_flags::sync_on_close))
			{
				const auto res = flush({0, 0});
				if (res.has_error()) [[unlikely]]
					return res.error();
			}
			if (::munmap(base(), reserved()) < 0) [[unlikely]]
				return std::error_code(errno, std::system_category());
		}

		_flags = {};
		_base_ptr = {};
		_base_off = {};
		_reserved = {};
		_data_size = {};
		_page_size = {};
		_has_source = {};
		return {};
	}

	inline constexpr extent_pair clamp_extent(extent_pair ext, _handle::extent_type max) noexcept
	{
		if (ext.second == 0)
			ext.second = max;
		if (ext.first >= max)
			ext.first = 0;
		return ext;
	}
	inline constexpr extent_pair over_align_extent(extent_pair ext, _handle::extent_type mul) noexcept
	{
		const auto mask = mul - 1;
		ext.second = (ext.second + mask) & ~mask;
		ext.first = ext.first & ~mask;
		return ext;
	}
	inline constexpr extent_pair under_align_extent(extent_pair ext, _handle::extent_type mul) noexcept
	{
		const auto mask = mul - 1;
		ext.first = (ext.first + mask) & ~mask;
		ext.second = ext.second & ~mask;
		return ext;
	}

	result<void> mmap_handle::flush(extent_pair ext) noexcept
	{
		if (!has_source()) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);

		ext = clamp_extent(ext, reserved());
		if (const auto endp = ext.first + ext.second; endp < ext.first || endp > reserved()) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);

		ext = over_align_extent(ext, page_size());
		if (::msync(base() + ext.first, ext.second, MS_SYNC) < 0) [[unlikely]]
			return std::error_code(errno, std::system_category());
		else
			return {};
	}
	result<void> mmap_handle::discard(extent_pair ext) noexcept
	{
		if (!has_source()) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		ext = clamp_extent(ext, reserved());
		if (const auto endp = ext.first + ext.second; endp < ext.first || endp > reserved()) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);

		ext = under_align_extent(ext, page_size());
#ifdef MADV_FREE
		if (ext.second > 0 && ::madvise(base() + ext.first, ext.second, MADV_FREE) >= 0) [[likely]]
			return {};
#endif
#ifdef MADV_REMOVE
		if (ext.second > 0 && ::madvise(base() + ext.first, ext.second, MADV_REMOVE) >= 0) [[likely]]
			return {};
#endif
		return std::make_error_code(std::errc::not_supported);
	}

	result<void> mmap_handle::commit(extent_pair ext) noexcept
	{
		ext = clamp_extent(ext, reserved());
		if (const auto endp = ext.first + ext.second; endp < ext.first || endp > reserved()) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);

		/* Update mapped pages with correct access flags. */
		/* TODO: Make it work for file-backed pages as well. */
		ext = over_align_extent(ext, page_size());
		if (!has_source())
		{
			std::uint64_t page_size = _page_size, cap = ext.second;
			std::int64_t off = offset() + ext.first;
#ifdef MAP_FIXED_NOREPLACE
			const auto res = map_region(base(), -1, off, cap, page_size, flags(), MAP_FIXED_NOREPLACE);
#else
			const auto res = map_region(base(), -1, off, cap, page_size, flags(), MAP_FIXED);
#endif
			if (res.has_error()) [[unlikely]]
				return res.error();
		}
		if (has_source() && ext.second > 0 && ::madvise(base() + ext.first, ext.second, MADV_WILLNEED) < 0) [[unlikely]]
			return std::error_code(errno, std::system_category());
		else
			return {};
	}
	result<void> mmap_handle::decommit(extent_pair ext) noexcept
	{
		ext = clamp_extent(ext, reserved());
		if (const auto endp = ext.first + ext.second; endp < ext.first || endp > reserved()) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);

		ext = under_align_extent(ext, page_size());
		if (has_source() && ext.second > 0 && ::madvise(base() + ext.first, ext.second, MADV_DONTNEED) < 0) [[unlikely]]
			return std::error_code(errno, std::system_category());

		/* Update un-mapped pages with correct access flags. */
		/* TODO: Make it work for file-backed pages as well. */
		if (!has_source())
		{
			std::uint64_t page_size = _page_size, cap = ext.second;
			std::int64_t off = offset() + ext.first;
#ifdef MAP_FIXED_NOREPLACE
			const auto res = map_region(base(), -1, off, cap, page_size, mmap_flags::none, MAP_FIXED_NOREPLACE);
#else
			const auto res = map_region(base(), -1, off, cap, page_size, mmap_flags::none, MAP_FIXED);
#endif
			if (res.has_error()) [[unlikely]]
				return res.error();
		}
		return {};
	}

	result<void> mmap_handle::zero(extent_pair ext) noexcept
	{
		ext = clamp_extent(ext, reserved());
		if (const auto endp = ext.first + ext.second; endp < ext.first || endp > reserved()) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);

#ifdef MADV_FREE_REUSABLE
		std::memset(base() + ext.first, 0, ext.second);
		if (const auto aligned = under_align_extent(ext, page_size()); has_source() && aligned.second > 0)
			::madvise(base() + aligned.first, aligned.second, MADV_FREE_REUSABLE);
#else
#ifdef MADV_REMOVE
		if (const auto aligned = under_align_extent(ext, page_size()); aligned.second > 0) [[likely]]
			::madvise(base() + aligned.first, aligned.second, MADV_REMOVE);
#endif
		std::memset(base() + ext.first, 0, ext.second);
#endif
		return {};
	}
	result<void> mmap_handle::prefault(std::span<const extent_pair> exts) noexcept
	{
		for (auto ext : exts)
		{
			ext = over_align_extent(ext, page_size());
			if (const auto endp = ext.first + ext.second; endp > base_size()) [[unlikely]]
				ext.second = base_size() - ext.first;
			if (::madvise(base() + ext.first, ext.second, MADV_WILLNEED) < 0) [[unlikely]]
				return std::error_code(errno, std::system_category());
		}
		return {};
	}
}

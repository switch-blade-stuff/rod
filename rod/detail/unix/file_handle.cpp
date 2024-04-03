/*
 * Created by switchblade on 2023-09-15.
 */

#include "file_handle.hpp"

namespace rod::_file
{
	result<file_handle> file_handle::do_open(const path_handle &base, path_view path, file_flags flags, open_mode mode, file_caching caching, file_perm perm) noexcept
	{
		if (!bool(caching & (file_caching::read | file_caching::write)) && bool(flags & file_flags::append)) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		if (bool(flags & file_flags::case_sensitive)) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);

		try
		{
			const auto fd_flags = _unix::make_fd_flags(flags, mode, caching);
			const auto fd_mode = _unix::mode_from_perm(perm);
			const auto rpath = path.render_null_terminated();

			auto hnd = base.is_open() ? ::openat(base.native_handle(), rpath.c_str(), fd_flags, fd_mode) : ::open(rpath.c_str(), fd_flags, fd_mode);
			if (hnd < 0) [[unlikely]]
			{
				const auto err = errno;
				if (bool(flags & file_flags::write) && mode == open_mode::supersede && err == EEXIST)
				{
					auto base_path = path.parent_path();
					auto base_hnd = path_handle();

					if (base.is_open() && base_path.empty())
					{
						if (auto base_tmp = clone(base); base_tmp.has_value()) [[likely]]
							base_hnd = std::move(*base_tmp);
						else
							return base_tmp.error();
					}
					else if (!base_path.empty())
					{
						if (auto base_tmp = path_handle::open(base, base_path); base_tmp.has_value()) [[likely]]
							base_hnd = std::move(*base_tmp);
						else
							return base_tmp.error();
					}

					auto anon_hnd = do_open_anonymous(base, flags, caching, perm);
					if (anon_hnd.has_error()) [[unlikely]]
						return anon_hnd.error();

					auto res = relink(*anon_hnd, base_hnd, path.filename());
					if (res.has_error()) [[unlikely]]
						return unlink(*anon_hnd), res.error();
					else
						return anon_hnd;
				}
				return std::error_code(err, std::system_category());
			}

			if (bool(caching & (file_caching::avoid_precache | file_caching::force_precache)))
			{
#ifdef POSIX_FADV_SEQUENTIAL
				const auto adv = bool(caching & file_caching::avoid_precache) ? POSIX_FADV_RANDOM : (POSIX_FADV_SEQUENTIAL | POSIX_FADV_WILLNEED);
				if (::posix_fadvise(hnd, 0, 0, adv) < 0) [[unlikely]]
					return ::close(hnd), std::error_code(errno, std::system_category());
#elif __APPLE__
				const auto adv = bool(caching & file_caching::avoid_precache) ? 0 : 1;
				if (::fcntl(hnd, F_RDAHEAD, adv) < 0) [[unlikely]]
					return ::close(hnd), std::error_code(errno, std::system_category());
#endif
			}
			if (bool(caching & file_caching::sanity_barriers))
				::fsync(hnd);

			return file_handle(hnd, flags, caching);
		}
		catch (...) { return _detail::current_error(); }
	}
	result<file_handle> file_handle::do_open_anonymous(const path_handle &base, file_flags flags, file_caching caching, file_perm perm) noexcept
	{
		if (!bool(caching & (file_caching::read | file_caching::write)) && bool(flags & file_flags::append)) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		if (bool(flags & file_flags::case_sensitive)) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);

		flags = (flags & ~file_flags::unlink_stat_check) | file_flags::unlink_stat_check;
		const auto base_fd = base.is_open() ? base.native_handle().value : AT_FDCWD;
		auto fd_flags = _unix::make_fd_flags(flags, open_mode::create, caching);
		auto fd_mode = _unix::mode_from_perm(perm);

#ifdef O_TMPFILE
		{
			auto old_fd_flags = std::exchange(fd_flags, (fd_flags & ~(O_EXCL | O_CREAT)) | O_TMPFILE);
			auto hnd = ::openat(base_fd, "", fd_flags, fd_mode);
			if (hnd >= 0) [[likely]]
				return file_handle(hnd, flags, caching);

			if (errno == ENOENT) /* ::openat(O_TMPFILE) might not be supported. */
			{
				auto base_path = base.is_open() ? to_native_path(base) : current_path();
				if (base_path.has_value()) [[likely]]
				{
					hnd = ::open(base_path->c_str(), fd_flags, fd_mode);
					if (hnd >= 0) [[likely]]
						return file_handle(hnd, flags, caching);
				}
			}
			/* O_TMPFILE may not be supported at all. */
			fd_flags = old_fd_flags;
		}
#endif

		for (;;)
		{
			auto name = _handle::generate_unique_name();
			if (name.has_error()) [[unlikely]]
				return name.error();

			if (auto hnd = ::openat(base_fd, name->c_str(), fd_flags, fd_mode); hnd < 0) [[unlikely]]
			{
				if (const auto err = errno; err != EEXIST)
					return ::close(hnd), std::error_code(err, std::system_category());
				else
					continue;
			}
			else if (::unlinkat(base_fd, name->c_str(), 0) < 0) [[unlikely]]
				return ::close(hnd), std::error_code(errno, std::system_category());
			else
				return file_handle(hnd, flags, caching);
		}
	}
	result<file_handle> file_handle::do_reopen(const file_handle &other, file_flags flags, file_caching caching) noexcept
	{
		if (!bool(caching & (file_caching::read | file_caching::write)) && bool(flags & file_flags::append))
			return std::make_error_code(std::errc::not_supported);
		if (bool(flags & file_flags::case_sensitive)) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		if (flags == other.flags() && caching == other.caching())
			return clone(other);

		for (const auto timeout = file_timeout::fallback.absolute();;)
		{
			const auto other_path = to_native_path(other);
			if (other_path.has_error()) [[unlikely]]
				return other_path.error();
			if (other_path->empty()) [[unlikely]]
				return std::make_error_code(std::errc::no_such_file_or_directory);

			if (auto hnd = open({}, *other_path, flags, open_mode::existing, caching); hnd.has_value()) [[likely]]
			{
				constexpr auto query = stat::query::ino | stat::query::dev;
				auto st_other = stat{}, st_clone = stat{};
				if (get_stat(st_other, other, query).value_or({}) == query && get_stat(st_clone, *hnd, query).value_or({}) == query &&
					st_other.dev == st_clone.dev && st_other.ino == st_clone.ino)
					return hnd;
			}
			else if (hnd.error() != std::errc::no_such_file_or_directory) [[unlikely]]
				return hnd.error();
			if (const auto now = file_clock::now(); now >= timeout) [[unlikely]]
				return std::make_error_code(std::errc::timed_out);
		}
	}

	result<> file_handle::do_link(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		const auto abs_timeout = to.is_infinite() ? to : to.absolute();
		if (const auto res = _unix::link_file(native_handle(), base, path, replace, bool(flags() & file_flags::unlink_stat_check), abs_timeout); res.has_value()) [[likely]]
			return release(*res), result<>();
		else
			return res.error();
	}
	result<> file_handle::do_relink(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		const auto abs_timeout = to.is_infinite() ? to : to.absolute();
		if (const auto res = _unix::relink_file(native_handle(), base, path, replace, bool(flags() & file_flags::unlink_stat_check), abs_timeout); res.has_value()) [[likely]]
			return release(*res), result<>();
		else
			return res.error();
	}
	result<> file_handle::do_unlink(const file_timeout &to) noexcept
	{
		const auto abs_timeout = to.is_infinite() ? to : to.absolute();
		if (const auto res = _unix::unlink_file(native_handle(), false, bool(flags() & file_flags::unlink_stat_check), abs_timeout); res.has_value()) [[likely]]
			return release(*res), result<>();
		else
			return res.error();
	}

	result<void, io_status_code> file_handle::do_sync(sync_mode mode) noexcept
	{
		if (mode == sync_mode::none)
			return {};

#if !defined(__FreeBSD__) && !defined(__APPLE__)
		if (!bool(mode & sync_mode::metadata))
		{
			if (::fdatasync(native_handle()) < 0) [[unlikely]]
				return std::error_code(errno, std::system_category());
			else
				return {};
		}
#endif
#ifdef __APPLE__
		if (bool(mode & sync_mode::metadata))
		{
			if (::fcntl(native_handle(), F_FULLFSYNC) < 0) [[unlikely]]
				return std::error_code(errno, std::system_category());
			else
				return {};
		}
#endif
		if (::fsync(native_handle()) < 0) [[unlikely]]
			return std::error_code(errno, std::system_category());
		else
			return {};
	}
	result<void, io_status_code> file_handle::do_sync_at(extent_pair ext, sync_mode mode) noexcept
	{
		if (mode == sync_mode::none)
			return {};

#ifdef __linux__
		if (!bool(mode & sync_mode::metadata))
		{
			constexpr auto flags = SYNC_FILE_RANGE_WRITE | SYNC_FILE_RANGE_WAIT_BEFORE | SYNC_FILE_RANGE_WAIT_AFTER;
			if (::sync_file_range(native_handle(), ext.first, ext.second, flags) >= 0) [[likely]]
				return {};
		}
#endif
		return do_sync(mode);
	}

	template<auto IoFunc, typename Op>
	typename file_handle::io_result<Op> file_handle::invoke_io_func(io_request<Op> req, const file_timeout &to) noexcept
	{
#ifdef __cpp_lib_is_layout_compatible
		static_assert(std::is_layout_compatible_v<byte_buffer, ::iovec> && std::is_layout_compatible_v<const_byte_buffer, ::iovec>);
#else
		static_assert(sizeof(byte_buffer) == sizeof(::iovec) && sizeof(const_byte_buffer) == sizeof(::iovec));
#endif

		if (!bool(flags() & file_flags::non_blocking) && !to.is_infinite())
			return std::make_error_code(std::errc::not_supported);
#ifdef __linux__
		if (req.off > extent_type(std::numeric_limits<::off64_t>::max())) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
#else
		if (req.off > extent_type(std::numeric_limits<::off_t>::max())) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
#endif

		const auto abs_timeout = to.is_infinite() ? to : to.absolute();
		auto *iov_buff = reinterpret_cast<::iovec *>(req.buffs.data());
		auto bytes_done = extent_type(0);

		for (std::size_t i = 0; i < req.buffs.size(); ++i)
		{
			if (const auto res = IoFunc(native_handle(), iov_buff + i, req.buffs.size() - i, req.off, abs_timeout); res >= 0) [[likely]]
			{
				req.off += extent_type(res);
				if (std::exchange(bytes_done, bytes_done + extent_type(res)) < bytes_done)
					continue;
				else
					break;
			}

			auto err = std::error_code(errno, std::system_category());
			if (err.value() == EAGAIN || err.value() == EWOULDBLOCK)
			{
				if (to.is_infinite())
					continue;
				if (const auto now = file_clock::now(); now > abs_timeout.absolute(now))
					continue;

				err = std::make_error_code(std::errc::timed_out);
			}
			return io_status_code(err, bytes_done);
		}

		for (std::size_t i = 0; i < req.buffs.size(); ++i)
		{
			if (bytes_done == 0)
			{
				req.buffs = std::span(req.buffs.begin(), i);
				break;
			}

			const auto chunk_size = std::min(bytes_done, req.buffs[i].size());
			req.buffs[i] = std::span(req.buffs[i].begin(), chunk_size);
			bytes_done -= chunk_size;
		}
		return std::move(req.buffs);
	}

	read_some_at_result_t<file_handle> file_handle::do_read_some_at(io_request<read_some_at_t> req, const file_timeout &to) noexcept
	{
		constexpr auto io_func = +[](native_handle_type hnd, const iovec *iov, [[maybe_unused]] std::size_t n, extent_type off, const file_timeout &to) noexcept
		{
			for (;;)
			{
#ifdef __linux__
#ifndef ROD_HAS_PIOV
				auto res = ::pread64(hnd, iov->iov_base, iov->iov_len, ::off64_t(off));
#else
				auto res = ::preadv64(hnd, iov, n, ::off64_t(off));
#endif
#else
#ifndef ROD_HAS_PIOV
				auto res = ::pread(hnd, iov->iov_base, iov->iov_len, ::off_t(off));
#else
				auto res = ::preadv(hnd, iov, n, ::off_t(off));
#endif
#endif

				if (res < 0) [[unlikely]]
				{
					const auto err = errno;
					if ((err == EAGAIN || err == EWOULDBLOCK) && bool(file_flags(hnd.flags) & file_flags::non_blocking))
					{
						const std::int64_t msec = to.is_infinite() ? -1 : std::chrono::duration_cast<std::chrono::milliseconds>(to.relative()).count();
						auto pfd = ::pollfd{.fd = hnd, .events = POLLIN | POLLERR, .revents = 0};

						if ((res = ::poll(&pfd, 1, msec)) >= 0) [[likely]]
							continue;
					}
				}
				return res;
			}
		};
		return invoke_io_func<io_func>(std::forward<decltype(req)>(req), to);
	}
	write_some_at_result_t<file_handle> file_handle::do_write_some_at(io_request<write_some_at_t> req, const file_timeout &to) noexcept
	{
		constexpr auto io_func = +[](native_handle_type hnd, const iovec *iov, [[maybe_unused]] std::size_t n, extent_type off, const file_timeout &to) noexcept
		{
			for (;;)
			{
#ifdef __linux__
#ifndef ROD_HAS_PIOV
				auto res = ::pwrite64(hnd, iov->iov_base, iov->iov_len, ::off64_t(off));
#else
				auto res = ::pwritev64(hnd, iov, n, ::off64_t(off));
#endif
#else
#ifndef ROD_HAS_PIOV
				auto res = ::pwrite(hnd, iov->iov_base, iov->iov_len, ::off_t(off));
#else
				auto res = ::pwritev(hnd, iov, n, ::off_t(off));
#endif
#endif

				if (res < 0) [[unlikely]]
				{
					const auto err = errno;
					if ((err == EAGAIN || err == EWOULDBLOCK) && bool(file_flags(hnd.flags) & file_flags::non_blocking))
					{
						const std::int64_t msec = to.is_infinite() ? -1 : std::chrono::duration_cast<std::chrono::milliseconds>(to.relative()).count();
						auto pfd = ::pollfd{.fd = hnd, .events = POLLOUT | POLLERR, .revents = 0};

						if ((res = ::poll(&pfd, 1, msec)) >= 0) [[likely]]
							continue;
					}
				}
				return res;
			}
		};
		return invoke_io_func<io_func>(std::forward<decltype(req)>(req), to);
	}

	result<_handle::extent_type> file_handle::do_truncate(extent_type endp) noexcept
	{
#ifdef __linux__
		if (endp > extent_type(std::numeric_limits<::off64_t>::max())) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);
		if (::ftruncate64(native_handle(), ::off64_t(endp)) < 0) [[unlikely]]
			return std::error_code(errno, std::system_category());
#else
		if (endp > extent_type(std::numeric_limits<::off_t>::max())) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);
		if (::ftruncate(native_handle(), ::off_t(endp)) < 0) [[unlikely]]
			return std::error_code(errno, std::system_category());
#endif
		if (bool(caching() & file_caching::sanity_barriers))
			::fsync(native_handle());
		return endp;
	}

	enum operation_type
	{
		clone_extents,
		erase_extents,
		clone_bytes,
		erase_bytes,
	};
	struct queue_item
	{
		extent_pair extent;
		operation_type op;
		bool is_new = {};
	};

	io_result_t<file_handle, zero_extents_t> file_handle::do_zero_extents(io_request<zero_extents_t> req, const file_timeout &to) noexcept
	{
		if (req.extent.first + req.extent.second < req.extent.first) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);

		const auto abs_timeout = to.is_infinite() ? to : to.absolute();
#if defined(__linux__)
		if (::fallocate(native_handle(), FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, req.extent.first, req.extent.second) < 0) [[unlikely]]
		{
			if (const auto err = errno; err != EOPNOTSUPP)
				return std::error_code(err, std::system_category());
		}
#endif
		if (req.extent.second < get_page_size())
		{
			auto buff_mem = ::alloca(std::size_t(req.extent.second));
			std::memset(buff_mem, 0, std::size_t(req.extent.second));

			auto buff = as_bytes(static_cast<const std::byte *>(buff_mem), static_cast<const std::byte *>(buff_mem) + req.extent.second);
			if (auto res = do_write_some_at({.buffs = {&buff, 1}, .off = req.extent.first}, abs_timeout); res.has_value()) [[likely]]
				return extent_pair(req.extent.first, (*res)[0].size());
			else
				return res.error();
		}

		const auto buff_size = get_file_buff_size();
		const auto buff_mem = make_malloc_ptr<std::byte[]>(buff_size);
		if (buff_mem.get() == nullptr) [[unlikely]]
			return std::make_error_code(std::errc::not_enough_memory);

		auto bytes_done = extent_type(0);
		while (req.extent.second > 0)
		{
			auto buff = as_bytes(static_cast<const std::byte *>(buff_mem.get()), buff_mem.get() + std::min(req.extent.second, buff_size));
			if (auto res = do_write_some_at({.buffs = {&buff, 1}, .off = req.extent.first}, abs_timeout); res.has_error()) [[unlikely]]
				return io_status_code(static_cast<std::error_code>(res.error()), bytes_done);

			req.extent.second -= buff.size();
			req.extent.first += buff.size();
			bytes_done += buff.size();
		}
		return req.extent;
	}
	io_result_t<file_handle, list_extents_t> file_handle::do_list_extents(io_request<list_extents_t> req, const file_timeout &to) const noexcept
	{
		try
		{
			const auto abs_timeout = to.is_infinite() ? to : to.absolute();
			req.buff.clear();

			for (extent_type start = 0, end = 0;;)
			{
#if !(defined(SEEK_DATA) && defined(SEEK_HOLE))
				return io_status_code(std::make_error_code(std::errc::not_supported), endpos(*this).value_or(0));
#elif defined(__linux__)
				if (::off64_t(start = extent_type(::lseek64(native_handle(), end, SEEK_DATA))) < 0) [[unlikely]]
					break;
				if (::off64_t(end = extent_type(::lseek64(native_handle(), start, SEEK_HOLE))) < 0) [[unlikely]]
					break;
#else
				if (::off_t(start = extent_type(::lseek(native_handle(), end, SEEK_DATA))) < 0) [[unlikely]]
					break;
				if (::off_t(end = extent_type(::lseek(native_handle(), start, SEEK_HOLE))) < 0) [[unlikely]]
					break;
#endif
				if (end > start) [[likely]]
					req.buff.emplace_back(start, end - start);
				if (abs_timeout.is_infinite())
					continue;
				if (const auto now = file_clock::now(); abs_timeout.absolute(now) > now) [[unlikely]]
					return std::make_error_code(std::errc::timed_out);
			}
			if (const auto err = errno; err != ENXIO) [[unlikely]]
				return std::error_code(err, std::system_category());
			else
				return std::move(req.buff);
		}
		catch (...) { return _detail::current_error(); }
	}
	io_result_t<file_handle, clone_extents_to_t> file_handle::do_clone_extents_to(io_request<clone_extents_to_t> req, const file_timeout &to) noexcept
	{
#ifdef __linux__
		constexpr auto zero_extent = [](int fd, ::off64_t off, std::size_t len) -> int
#else
		constexpr auto zero_extent = [](int fd, ::off_t off, std::size_t len) -> int
#endif
		{
#if defined(__linux__)
			return ::fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, off, len);
#else
			(void) fd, (void) off, (void) len;
			errno = ENOSYS;
			return -1;
#endif
		};
#ifdef __linux__
		constexpr auto copy_extent = [](int src_fd, ::off64_t *src_off, int dst_fd, ::off64_t *dst_off, std::size_t len, unsigned flags) -> ::ssize_t
#else
		constexpr auto copy_extent = [](int src_fd, ::off_t *src_off, int dst_fd, ::off_t *dst_off, std::size_t len, unsigned flags) -> ::ssize_t
#endif
		{
#if defined(__linux__) ||  defined(__FreeBSD__)
			return ::copy_file_range(src_fd, src_off, dst_fd, dst_off, len, flags);
#else
			(void) src_fd, (void) src_off, (void) dst_fd, (void) dst_off, (void) len, (void) flags;
			errno = ENOSYS;
			return -1;
#endif
		};

		constexpr auto stat_mask = stat::query::size | stat::query::ino | stat::query::dev;
		if (!bool((flags() | req.dst.flags()) & file_flags::non_blocking) && !to.is_infinite()) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		if (!bool(req.dst.flags() & file_flags::data_write) || !bool(flags() & file_flags::data_read)) [[unlikely]]
			return std::make_error_code(std::errc::invalid_seek);

		const auto abs_timeout = to.is_infinite() ? to : to.absolute();
		auto src_stat = stat(nullptr), dst_stat = stat(nullptr);
		if (auto res = get_stat(src_stat, *this, stat_mask); res.has_error()) [[unlikely]]
			return res.error();
		else if ((*res & stat_mask) != stat_mask) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		if (auto res = get_stat(dst_stat, req.dst, stat_mask); res.has_error()) [[unlikely]]
			return res.error();
		else if ((*res & stat_mask) != stat_mask) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);

		/* {-1, -1} is special case indicating entire file. */
		if (const auto npos = extent_type(-1); req.extent.first == npos && req.extent.second == npos)
			req.extent = extent_pair(0, extent_type(src_stat.size));
		if (req.extent.first + req.extent.second < req.extent.first || req.off + req.extent.second < req.off) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);

		/* Clamp extent to source file size. */
		if (req.extent.first + req.extent.second > extent_type(src_stat.size))
			req.extent.second = extent_type(src_stat.size) - req.extent.first;
		if (req.extent.first >= extent_type(src_stat.size))
			req.extent.second = 0;
		if (req.extent.second == 0)
			return req.extent;

		const auto buff_size = get_file_buff_size();
		const auto buff_mem = make_malloc_ptr<std::byte[]>(buff_size);
		if (buff_mem.get() == nullptr) [[unlikely]]
			return std::make_error_code(std::errc::not_enough_memory);

		auto queue_cap = extent_type(64);
		auto queue_len = extent_type(0);
		auto queue_mem = make_malloc_ptr<queue_item[]>(queue_cap);
		if (queue_mem.get() == nullptr) [[unlikely]]
			return std::make_error_code(std::errc::not_enough_memory);

		const auto insert_queue = [&](std::size_t pos, auto... args) noexcept -> result<void>
		{
			if (queue_len == queue_cap) [[unlikely]]
			{
				void *old_mem = queue_mem.release(), *new_mem = std::realloc(old_mem, queue_cap *= 2);
				if (new_mem == nullptr) [[unlikely]]
					return std::free(old_mem), std::make_error_code(std::errc::not_enough_memory);
				else
					queue_mem.reset(static_cast<queue_item *>(new_mem));
			}
			auto begin = queue_mem.get(), end = begin + queue_len++;
			std::move_backward(begin + pos, end, end + 1);
			new (begin + pos) queue_item{args...};
			return result<void>();
		};
		const auto push_queue = [&](auto... args) noexcept { return insert_queue(queue_len, args...); };

		const auto req_end = req.extent.first + req.extent.second;
#if defined(SEEK_DATA) && !defined(__APPLE__)
		for (auto start = extent_type(0), end = extent_type(0);;)
		{
#ifdef ROD_ZFS_SEEK_DATA_FIX
			const auto zfx_fix_buff_len = (caching() & file_caching::all) < file_caching::meta ? 4096 : 1;
			alignas(4096) std::byte zfs_fix_buff_mem[4096];
#ifdef __linux__
			if (::pread64(native_handle(), zfs_fix_buff_mem, zfx_fix_buff_len, ::off64_t(end)) < 0) [[unlikely]]
				return std::error_code(errno, std::system_category());
#else
			if (::pread(native_handle(), zfs_fix_buff_mem, zfx_fix_buff_len, ::off_t(end)) < 0) [[unlikely]]
				return std::error_code(errno, std::system_category());
#endif
#endif

#ifdef __linux__
			if (::off64_t(start = extent_type(::lseek64(native_handle(), end, SEEK_DATA))) < 0) [[unlikely]]
#else
			if (::off_t(start = extent_type(::lseek(native_handle(), end, SEEK_DATA))) < 0) [[unlikely]]
#endif
			{
				if (const auto err = errno; err == ENXIO)
					start = src_stat.size;
				else if (start == 0)
				{
					if (const auto res = push_queue(extent_pair(0, src_stat.size), operation_type::clone_extents); res.has_error()) [[unlikely]]
						return res.error();
					else
						break;
				}
			}

			if (end >= req_end)
				break;
			if ((end <= req.extent.first && start >= req_end) || (end >= req.extent.first && end < req_end) || (start > req.extent.first && start <= req_end))
			{
				const auto ext_off = std::max(end, req.extent.first);
				const auto ext_end = std::min(start, req_end);
				if (const auto res = push_queue(extent_pair(ext_off, ext_end - ext_off), operation_type::erase_extents); res.has_error()) [[unlikely]]
					return res.error();
			}
			if (start >= req_end)
				break;

#ifdef __linux__
			if (::off64_t(end = extent_type(::lseek64(native_handle(), start, SEEK_HOLE))) < 0) [[unlikely]]
#else
			if (::off_t(end = extent_type(::lseek(native_handle(), start, SEEK_HOLE))) < 0) [[unlikely]]
#endif
			{
				if (const auto err = errno; err == ENXIO)
					end = src_stat.size;
				else if (start == 0)
				{
					if (const auto res = push_queue(extent_pair(0, src_stat.size), operation_type::clone_extents); res.has_error()) [[unlikely]]
						return res.error();
					else
						break;
				}
			}

			if ((start <= req.extent.first && end >= req_end) || (start >= req.extent.first && start < req_end) || (end > req.extent.first && end <= req_end))
			{
				const auto ext_off = std::max(start, req.extent.first);
				const auto ext_end = std::min(end, req_end);
				if (const auto res = push_queue(extent_pair(ext_off, ext_end - ext_off), operation_type::clone_extents); res.has_error()) [[unlikely]]
					return res.error();
			}
		}
#else
		if (const auto res = push_queue(req.extent, operation_type::clone_extents); res.has_error()) [[unlikely]]
			return res.error();
#endif

		if (queue_len == 0)
		{
			const auto res = push_queue(req.extent, operation_type::erase_extents);
			if (res.has_error()) [[unlikely]]
				return res.error();
		}

		/* Reverse operation order if source and destination extents overlap. */
		if (src_stat.dev == dst_stat.dev && src_stat.ino == dst_stat.ino)
		{
			using signed_extent = std::make_signed_t<extent_type>;
			if (extent_type(std::abs(signed_extent(req.off - req.extent.first))) < buff_size)
				return std::make_error_code(std::errc::invalid_argument);
			if (req.off > req.extent.first)
				std::reverse(queue_mem.get(), queue_mem.get() + queue_len);
		}

		const auto dst_diff = req.off - req.extent.first;
		if (req.off + req.extent.second > dst_stat.size)
		{
			/* Skip extents that overlap existing file data. */
			std::size_t i = 0;
			while (i < queue_len && queue_mem[i].extent.first + queue_mem[i].extent.second + dst_diff <= dst_stat.size)
				++i;
			if (i < queue_len && queue_mem[i].extent.first + dst_diff < dst_stat.size)
				++i;
			/* The rest will be newly-allocated. */
			for (; i < queue_len; ++i)
				queue_mem[i].is_new = true;
		}

		/* Truncate the destination file if needed. */
		if (dst_stat.size < req.off + req.extent.second)
		{
			auto res = truncate(req.dst, req.off + req.extent.second);
			if (res.has_error()) [[unlikely]]
				return res.error();
		}

		auto restore_guard = defer_invoke([&]() { truncate(req.dst, dst_stat.size); });
		bool do_clone_extents = !req.force_copy, do_erase_extents = true;
		auto result = extent_pair(req.extent.first, 0);

		for (const auto &item : std::span(queue_mem.get(), queue_len))
		{
			/* Process the extent operation block-by-block. */
			for (extent_type src_off = 0; src_off < item.extent.second;)
			{
				auto src_len = std::min(buff_size, item.extent.second - src_off);
				if (item.op == clone_extents && do_clone_extents)
				{
#ifdef __linux__
					auto op_off_src = ::off64_t(item.extent.first + src_off), op_off_dst = ::off64_t(extent_type(op_off_src) + dst_diff);
#else
					auto op_off_src = ::off_t(item.extent.first + src_off), op_off_dst = ::off_t(extent_type(op_off_src) + dst_diff);
#endif
					if (auto op_res = copy_extent(native_handle(), &op_off_src, req.dst.native_handle(), &op_off_dst, src_len, 0); op_res <= 0)
					{
						if (op_res < 0)
						{
							const auto err = errno;
							if ((err != EXDEV && err != EOPNOTSUPP && err != ENOSYS && err != EINVAL) || !req.emulate)
								return std::error_code(err, std::system_category());
						}
						do_clone_extents = false;
					}
					else
					{
						src_len = extent_type(op_res);
						goto item_complete;
					}
				}
				if (item.op == erase_extents && do_erase_extents && !item.is_new)
				{
#ifdef __linux__
					auto op_off = ::off64_t(item.extent.first + src_off + dst_diff);
#else
					auto op_off = ::off_t(item.extent.first + src_off + dst_diff);
#endif
					if (auto op_res = zero_extent(req.dst.native_handle(), op_off, src_len); op_res < 0)
					{
						const auto err = errno;
						if ((err != EOPNOTSUPP && err != ENOSYS) || !req.emulate)
							return std::error_code(err, std::system_category());
						else
							do_erase_extents = false;
					}
					else
						goto item_complete;
				}

				/* Fall back to byte-wise clone & erase. */
				if (item.op == clone_bytes || (item.op == clone_extents && !do_clone_extents))
				{
					auto src_buff = byte_buffer(buff_mem.get(), src_len);
					auto read_res = read_some_at(*this, {.buffs = {&src_buff, 1}, .off = item.extent.first + src_off}, abs_timeout);
					if (read_res.has_error()) [[unlikely]]
						return read_res.error();
					if (read_res->front().size() < src_len) [[unlikely]]
						return std::make_error_code(std::errc::resource_unavailable_try_again);

					auto dst_buff = const_byte_buffer(read_res->front().data(), src_len);
					auto write_res = write_some_at(req.dst, {.buffs = {&dst_buff, 1}, .off = item.extent.first + src_off + dst_diff}, abs_timeout);
					if (write_res.has_error()) [[unlikely]]
						return write_res.error();
					if (write_res->front().size() < src_len) [[unlikely]]
						return std::make_error_code(std::errc::resource_unavailable_try_again);

					goto item_complete;
				}
				if ((item.op == erase_bytes || (item.op == erase_extents && !do_erase_extents)) && !item.is_new)
				{
					auto dst_buff = const_byte_buffer(buff_mem.get(), src_len);
					std::memset(buff_mem.get(), 0, std::size_t(src_len));

					auto write_res = write_some_at(req.dst, {.buffs = {&dst_buff, 1}, .off = item.extent.first + src_off + dst_diff}, abs_timeout);
					if (write_res.has_error()) [[unlikely]]
						return write_res.error();
					if (write_res->front().size() < src_len) [[unlikely]]
						return std::make_error_code(std::errc::resource_unavailable_try_again);
				}

			item_complete:
				result.second += (src_off += src_len);
				restore_guard.release();
			}
		}
		return result;
	}
}

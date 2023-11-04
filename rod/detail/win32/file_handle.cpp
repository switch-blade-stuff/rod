/*
 * Created by switchblade on 2023-09-15.
 */

#include "file_handle.hpp"

#include <numeric>

#ifndef FSCTL_DUPLICATE_EXTENTS_TO_FILE
#define FSCTL_DUPLICATE_EXTENTS_TO_FILE CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 209, METHOD_BUFFERED, FILE_WRITE_DATA)
#endif

namespace rod::_file
{
	using namespace _win32;

	inline constexpr auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

	result<file_handle> file_handle::do_open(const path_handle &base, path_view path, file_flags flags, open_mode mode, file_caching caching, file_perm perm) noexcept
	{
		if (!bool(caching & (file_caching::read | file_caching::write)) && bool(flags & file_flags::append)) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		if (bool(flags & file_flags::case_sensitive)) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = render_as_wchar<true>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto upath = make_ustring(rpath->as_span());
		auto guard = ntapi->dos_path_to_nt_path(upath, base.is_open());
		if (guard.has_error()) [[unlikely]]
			return std::make_error_code(std::errc::no_such_file_or_directory);

		auto obj_attrib = object_attributes();
		auto opts = make_handle_opts(flags, caching) | 0x40; /*FILE_NON_DIRECTORY_FILE*/
		auto attr = make_handle_attr(flags, caching, perm);
		auto access = flags_to_access(flags);
		auto disp = mode_to_disp(mode);
		auto iosb = io_status_block();

		obj_attrib.root_dir = base.is_open() ? base.native_handle() : nullptr;
		obj_attrib.length = sizeof(object_attributes);
		obj_attrib.name = &upath;

		auto hnd = ntapi->create_file(obj_attrib, &iosb, access, attr, share, disp, opts);
		if (hnd.has_error()) [[unlikely]]
		{
			/* Map known error codes. */
			if (auto err = hnd.error(); is_error_file_not_found(err))
				return std::make_error_code(std::errc::no_such_file_or_directory);
			else if (is_error_file_exists(err) && mode == open_mode::create)
				return std::make_error_code(std::errc::file_exists);
			else
				return err;
		}

		/* Make sparse if created a new file. */
		if (!bool(flags & file_flags::no_sparse_files) && (mode == open_mode::truncate || mode == open_mode::supersede || iosb.info == 2 /*FILE_CREATED*/))
		{
			DWORD written = 0;
			auto buffer = FILE_SET_SPARSE_BUFFER{.SetSparse = 1};
			::DeviceIoControl(*hnd, FSCTL_SET_SPARSE, &buffer, sizeof(buffer), nullptr, 0, &written, nullptr);
		}
		/* Flush the file if additional sanity barriers are requested. */
		if (mode == open_mode::truncate && bool(caching & file_caching::sanity_barriers))
			::FlushFileBuffers(*hnd);

		return file_handle(*hnd, flags, caching);
	}
	result<file_handle> file_handle::do_reopen(const file_handle &other, file_flags flags, file_caching caching) noexcept
	{
		if (!bool(caching & (file_caching::read | file_caching::write)) && bool(flags & file_flags::append))
			return std::make_error_code(std::errc::not_supported);
		if (bool(flags & file_flags::case_sensitive)) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		if (flags == other.flags() && caching == other.caching())
			return clone(other);

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto opts = make_handle_opts(flags, caching);
		auto access = flags_to_access(flags);
		auto iosb = io_status_block();

		auto hnd = ntapi->reopen_file(other.native_handle(), &iosb, access, share, opts);
		if (hnd.has_value()) [[likely]]
			return file_handle(*hnd, flags, caching);
		else
			return hnd.error();
	}

	result<> file_handle::do_link(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		if (!bool(flags() & file_flags::non_blocking) && to != file_timeout())
			return std::make_error_code(std::errc::not_supported);

		const auto abs_timeout = to != file_timeout() ? to.absolute() : file_timeout();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = render_as_wchar<false>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto upath = make_ustring(rpath->as_span());
		auto iosb = io_status_block();
		if (auto status = ntapi->link_file(native_handle(), &iosb, base.native_handle(), upath, replace, abs_timeout); is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return {};
	}
	result<> file_handle::do_relink(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		if (!bool(flags() & file_flags::non_blocking) && to != file_timeout())
			return std::make_error_code(std::errc::not_supported);

		const auto abs_timeout = to != file_timeout() ? to.absolute() : file_timeout();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = render_as_wchar<false>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto upath = make_ustring(rpath->as_span());
		auto iosb = io_status_block();
		if (auto status = ntapi->relink_file(native_handle(), &iosb, base.native_handle(), upath, replace, abs_timeout); is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return {};
	}
	result<> file_handle::do_unlink(const file_timeout &to) noexcept
	{
		if (!bool(flags() & file_flags::non_blocking) && to != file_timeout())
			return std::make_error_code(std::errc::not_supported);

		const auto abs_timeout = to != file_timeout() ? to.absolute() : file_timeout();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto iosb = io_status_block();
		if (auto status = ntapi->unlink_file(native_handle(), &iosb, !bool(flags() & file_flags::unlink_on_close), abs_timeout); is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return {};
	}

	result<void, io_status_code> file_handle::do_sync(sync_mode mode) noexcept
	{
		/* WinNT does not have sparse flush, so do_sync and do_sync_at are identical. */
		return do_sync_at({.mode = mode});
	}
	io_result<sync_at_t> file_handle::do_sync_at(io_request<sync_at_t> req) noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		ULONG mode = 0;
		if ((req.mode & sync_mode::metadata) == sync_mode::none)
			mode |= 1; /*FLUSH_FLAGS_FILE_DATA_ONLY*/
		if (bool(flags() & file_flags::non_blocking))
			mode |= 2; /*FLUSH_FLAGS_NO_SYNC*/

		auto iosb = io_status_block();
		auto status = ntstatus();

		/* NtFlushBuffersFileEx may not be supported. */
		if (!ntapi->NtFlushBuffersFileEx)
			status = ntapi->NtFlushBuffersFileEx(native_handle(), mode, nullptr, 0, &iosb);
		else
			status = ntapi->NtFlushBuffersFile(native_handle(), &iosb);

		if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return req.buffs;
	}

	template<auto IoFunc, typename Op>
	typename file_handle::io_result<Op> file_handle::invoke_io_func(io_request<Op> req, const file_timeout &to) noexcept
	{
		if (!bool(flags() & file_flags::non_blocking) && to != file_timeout())
			return std::make_error_code(std::errc::not_supported);

		const file_timeout abs_timeout = to == file_timeout() ? file_timeout() : to.absolute();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		const auto buff_size = req.buffs.size() * sizeof(io_status_block);
		auto iosb_buff = ROD_MAKE_BUFFER(io_status_block, buff_size);
		if (iosb_buff.get() == nullptr) [[unlikely]]
			return std::make_error_code(std::errc::not_enough_memory);

		std::memset(iosb_buff.get(), 0, buff_size);
		extent_type pending = 0, bytes_done = 0;
		std::error_code result_status = {};

		/* Start IO operations. */
		for (std::size_t i = 0; i < req.buffs.size(); ++i)
		{
			auto &iosb = iosb_buff.get()[i];
			auto &data = req.buffs[i];

			LARGE_INTEGER offset = {.QuadPart = LONGLONG(req.off)};
			if (offset.QuadPart < 0) [[unlikely]]
			{
				result_status = std::make_error_code(std::errc::value_too_large);
				break;
			}

			((*ntapi).*IoFunc)(native_handle(), nullptr, nullptr, 0, &iosb, const_cast<std::byte *>(data.data()), ULONG(data.size()), &offset, nullptr);
			if (iosb.status == STATUS_PENDING)
			{
				req.off += data.size();
				pending += 1;
			}
			else if (!is_status_failure(iosb.status))
				req.off += iosb.info;
		}

		/* Wait until all pending operations complete or timeout is reached. */
		for (std::size_t i = 0; i < req.buffs.size() && pending; ++i)
		{
			auto &iosb = iosb_buff.get()[i];

			/* Already finished. */
			if (iosb.status == STATUS_PENDING)
				--pending;
			else
				continue;

			/* Stop iterating once timeout is reached, remaining operations will be cancelled by the cleanup loop. */
			ntapi->wait_io(native_handle(), &iosb, abs_timeout);
			if (iosb.status == STATUS_TIMEOUT)
				break;
		}

		/* If all buffers finished without an error or timeout, return success. Otherwise return the first error & terminate. */
		for (std::size_t i = 0; i < req.buffs.size(); ++i)
		{
			auto &iosb = iosb_buff.get()[i];
			auto &data = req.buffs[i];

			/* At this stage, STATUS_PENDING must be terminated. */
			if (iosb.status == STATUS_PENDING) [[unlikely]]
				ntapi->cancel_io(native_handle(), &iosb);

			/* Truncate the buffer to the actual amount of bytes transferred. */
			if (iosb.status == STATUS_TIMEOUT || is_status_failure(iosb.status)) [[unlikely]]
			{
				result_status = result_status ? result_status : status_error_code(iosb.status);
				data = io_buffer<Op>(data.data(), 0);
			}
			else
			{
				data = io_buffer<Op>(data.data(), std::size_t(iosb.info));
				bytes_done += data.size();
			}
		}

		/* Map known status codes. */
		if (result_status.value() == 0xc0000120 /*STATUS_CANCELLED*/) [[unlikely]]
			result_status = std::make_error_code(std::errc::operation_canceled);
		if (result_status.value() == STATUS_TIMEOUT) [[unlikely]]
			result_status = std::make_error_code(std::errc::timed_out);

		if (result_status) [[unlikely]]
			return io_status_code(result_status, bytes_done);
		else
			return req.buffs;
	}

	read_some_at_result_t<file_handle> file_handle::do_read_some_at(io_request<read_some_at_t> req, const fs::file_timeout &to) noexcept
	{
		return invoke_io_func<&ntapi::NtReadFile>(std::forward<decltype(req)>(req), to);
	}
	write_some_at_result_t<file_handle> file_handle::do_write_some_at(io_request<write_some_at_t> req, const fs::file_timeout &to) noexcept
	{
		return invoke_io_func<&ntapi::NtWriteFile>(std::forward<decltype(req)>(req), to);
	}

	static_assert(sizeof(extent_pair) == sizeof(FILE_ALLOCATED_RANGE_BUFFER));

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
	struct clone_info
	{
		void *handle;
		LONGLONG src_off;
		LONGLONG dst_off;
		LONGLONG length;
	};

	result<_handle::extent_type> file_handle::do_truncate(extent_type endp) noexcept
	{
		auto info = FILE_END_OF_FILE_INFO{.EndOfFile = {.QuadPart = LONGLONG(endp)}};
		if (::SetFileInformationByHandle(native_handle(), FileEndOfFileInfo, &info, sizeof(info)) == 0)
			return dos_error_code(::GetLastError());
		if (bool(caching() & file_caching::sanity_barriers))
			::FlushFileBuffers(native_handle());
		return endp;
	}
	io_result<zero_extents_t> file_handle::do_zero_extents(io_request<zero_extents_t> req, const file_timeout &to) noexcept
	{
		if (req.extent.first + req.extent.second < req.extent.first) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);

		const auto abs_timeout = to != file_timeout() ? to.absolute() : file_timeout();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto ol = OVERLAPPED{.Internal = ULONG_PTR(-1)};
		auto info = FILE_ZERO_DATA_INFORMATION();
		info.BeyondFinalZero.QuadPart = req.extent.first + req.extent.second;
		info.FileOffset.QuadPart = req.extent.first;

		if (DWORD written = 0; ::DeviceIoControl(native_handle(), FSCTL_SET_ZERO_DATA, &info, sizeof(info), nullptr, 0, &written, &ol) == 0)
		{
			if (auto err = ::GetLastError(); err == ERROR_IO_PENDING)
			{
				auto status = ntapi->wait_io(native_handle(), reinterpret_cast<io_status_block *>(&ol), abs_timeout);
				if (is_status_failure(status)) [[unlikely]]
					return status_error_code(status);
			}
			else if (err != ERROR_SUCCESS && !req.emulate)
				return dos_error_code(err);

			const auto block_size = get_block_size();
			const auto buff_size = page_align(block_size);
			auto buff = make_malloc_ptr_for_overwrite<std::byte[]>(buff_size);
			if (buff.get() == nullptr) [[unlikely]]
				return std::make_error_code(std::errc::not_enough_memory);
			else
				std::memset(buff.get(), 0, std::size_t(buff_size));

			/* Fall back to a regular 0-fill. */
			auto result = extent_pair(req.extent.first, 0);
			for (extent_type src_off = 0; src_off < req.extent.second; src_off += block_size)
			{
				const auto src_length = std::min(block_size, req.extent.second - src_off);
				auto src_buff = const_byte_buffer(buff.get() + src_off, src_length);
				if (auto write_res = write_some_at(*this, {.buffs = {&src_buff, 1}, .off = req.extent.first + src_off}, abs_timeout); write_res.has_error()) [[unlikely]]
					return write_res.error();
				else if (write_res->front().size() < src_length) [[unlikely]]
					return std::make_error_code(std::errc::resource_unavailable_try_again);
				else
					result.second += src_length;
			}
			return result;
		}
		return req.extent;
	}
	io_result<list_extents_t> file_handle::do_list_extents(io_request<list_extents_t> req, const file_timeout &to) const noexcept
	{
		auto buff = FILE_ALLOCATED_RANGE_BUFFER{.Length = {.QuadPart = (extent_type(1) << 63) - 1}};
		auto ol = OVERLAPPED{.Internal = ULONG_PTR(-1)};
		DWORD written = 0;

		const auto abs_timeout = to != file_timeout() ? to.absolute() : file_timeout();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		try
		{
			req.buffs.resize(64);
			while (::DeviceIoControl(native_handle(), FSCTL_QUERY_ALLOCATED_RANGES, &buff, sizeof(buff), req.buffs.data(), DWORD(req.buffs.size() * sizeof(FILE_ALLOCATED_RANGE_BUFFER)), &written, &ol) == 0)
			{
				if (const auto err = ::GetLastError(); err == ERROR_IO_PENDING)
				{
					auto status = ntapi->wait_io(native_handle(), reinterpret_cast<io_status_block *>(&ol), abs_timeout);
					if (is_status_failure(status)) [[unlikely]]
						return status_error_code(status);
				}
				else if (err == ERROR_INSUFFICIENT_BUFFER || err == ERROR_MORE_DATA)
					req.buffs.resize(req.buffs.size() * 2);
				else if (err != ERROR_SUCCESS) [[unlikely]]
					return dos_error_code(err);
			}

			req.buffs.resize(written / sizeof(FILE_ALLOCATED_RANGE_BUFFER));
			return req.buffs;
		}
		catch (...) { return _detail::current_error(); }
	}
	io_result<clone_extents_to_t> file_handle::do_clone_extents_to(io_request<clone_extents_to_t> req, const file_timeout &to) noexcept
	{
		constexpr auto stat_mask = stat::query::size | stat::query::ino | stat::query::dev;

		if (!bool((flags() | req.dst.flags()) & file_flags::non_blocking) && to != file_timeout()) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		if ((req.dst.flags() & file_flags::write) != file_flags::write) [[unlikely]]
			return std::make_error_code(std::errc::invalid_seek);

		const auto abs_timeout = to != file_timeout() ? to.absolute() : file_timeout();
		const auto block_size = get_block_size();
		const auto page_size = get_page_size();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

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

		const auto request_end = req.extent.first + req.extent.second;
		extent_type bytes_done = 0, queue_size = 0, queue_cap = 64;
		extent_pair extent_buff[64];

		/* Use a malloc buffer instead of std::vector to enable realloc. */
		auto queue = make_malloc_ptr_for_overwrite<queue_item[]>(queue_cap);
		if (queue.get() == nullptr) [[unlikely]]
			return std::make_error_code(std::errc::not_enough_memory);

		const auto insert_queue = [&](std::size_t pos, auto... args) noexcept -> result<void>
		{
			if (queue_size == queue_cap) [[unlikely]]
			{
				void *old_mem = queue.release(), *new_mem = std::realloc(old_mem, queue_cap *= 2);
				if (new_mem == nullptr) [[unlikely]]
					return (std::free(old_mem), std::make_error_code(std::errc::not_enough_memory));
				else
					queue.reset(static_cast<queue_item *>(new_mem));
			}
			auto begin = queue.get(), end = begin + queue_size++;
			std::move_backward(begin + pos, end, end + 1);
			new (begin + pos) queue_item{args...};
			return result<void>();
		};
		const auto push_queue = [&](auto... args) noexcept -> result<void>
		{
			return insert_queue(queue_size, args...);
		};

		/* Fill the queue with extent operations. */
		for (auto curr_offset = req.extent.first; curr_offset < request_end;)
		{
			auto range_buff = FILE_ALLOCATED_RANGE_BUFFER();
			auto ol = OVERLAPPED{.Internal = ULONG_PTR(-1)};
			DWORD written = 0;

			range_buff.Length.QuadPart = LONGLONG(request_end - curr_offset);
			range_buff.FileOffset.QuadPart = LONGLONG(curr_offset);

			if (::DeviceIoControl(native_handle(), FSCTL_QUERY_ALLOCATED_RANGES, &range_buff, sizeof(range_buff), extent_buff, sizeof(extent_buff), &written, &ol) == 0)
			{
				if (const auto err = ::GetLastError(); err == ERROR_IO_PENDING)
				{
					auto status = ntapi->wait_io(native_handle(), reinterpret_cast<io_status_block *>(&ol), abs_timeout);
					if (is_status_failure(status)) [[unlikely]]
						return status_error_code(status);
				}
				else if (err != ERROR_SUCCESS && err != ERROR_MORE_DATA) [[unlikely]]
					return dos_error_code(err);
			}
			if (written == 0) /* EOF */
				break;

			for (std::size_t i = 0; i < written / sizeof(extent_pair); ++i)
			{
				/* If the queue is not empty, insert an erase operation if we are not covering the full extent. */
				if (queue_size != 0)
				{
					const auto [last_pos, last_len] = queue[queue_size - 1].extent;
					const auto last_end = last_pos + last_len;
					if (last_end != extent_buff[i].first)
					{
						const auto to_erase = extent_pair{last_end, extent_buff[i].first - last_end};
						if (auto res = push_queue(to_erase, erase_extents); res.has_error()) [[unlikely]]
							return res.error();
					}
				}
				if (auto res = push_queue(extent_buff[i], clone_extents); res.has_value()) [[likely]]
					curr_offset = extent_buff[i].first + extent_buff[i].second;
				else
					return res.error();
			}
		}

		if (queue_size != 0)
		{
			auto &front = queue[0], &back = queue[queue_size - 1];
			auto &[front_pos, front_len] = front.extent;
			auto &[back_pos, back_len] = back.extent;

			/* Truncate or add padding for the first extent. */
			if (front_pos < req.extent.first)
			{
				auto diff = (req.extent.first - front_pos + page_size - 1) & ~(page_size - 1);
				front.extent = extent_pair(front_pos - diff, front_len + diff);

				if (front_pos != req.extent.first)
				{
					const auto to_fill = extent_pair(req.extent.first, front_pos - req.extent.first);
					const auto op = front.op == clone_extents ? clone_bytes : erase_bytes;
					if (auto res = insert_queue(0, to_fill, op); res.has_error()) [[unlikely]]
						return res.error();
				}
			}
			else if (front_pos > req.extent.first)
			{
				auto to_erase = extent_pair(req.extent.first, front_pos - req.extent.first);
				if (auto res = insert_queue(0, to_erase, erase_extents); res.has_error()) [[unlikely]]
					return res.error();
			}

			/* Truncate or add padding for the last extent. */
			if (back_pos + back_len > request_end)
			{
				back_len -= (back_pos + back_len - request_end + page_size - 1) & ~(page_size - 1);
				if (back_pos + back_len != request_end)
				{
					const auto to_fill = extent_pair(back_pos + back_len, request_end - (back_pos + back_len));
					const auto op = queue[queue_size - 1].op == clone_extents ? clone_bytes : erase_bytes;
					if (auto res = push_queue(to_fill, op); res.has_error()) [[unlikely]]
						return res.error();
				}
			}
			else if (back_pos + back_len < request_end)
			{
				auto to_erase = extent_pair(back_pos + back_len, request_end - (back_pos + back_len));
				if (auto res = push_queue(to_erase, erase_extents); res.has_error()) [[unlikely]]
					return res.error();
			}
		}
		if (queue_size == 0)
		{
			auto res = push_queue(req.extent, erase_extents);
			if (res.has_error()) [[unlikely]]
				return res.error();
		}

		/* Reverse operation order if source and destination extents overlap. */
		if (src_stat.dev == dst_stat.dev && src_stat.ino == dst_stat.ino)
		{
			using signed_extent = std::make_signed_t<extent_type>;
			if (extent_type(std::abs(signed_extent(req.off - req.extent.first))) < block_size)
				return std::make_error_code(std::errc::invalid_argument);
			if (req.off > req.extent.first)
				std::reverse(queue.get(), queue.get() + queue_size);
		}

		const auto dst_diff = req.off - req.extent.first;
		if (req.off + req.extent.second > dst_stat.size)
		{
			/* Skip extents that overlap existing file data. */
			std::size_t i = 0;
			while (i < queue_size && queue[i].extent.first + queue[i].extent.second + dst_diff <= dst_stat.size)
				++i;
			if (i < queue_size && queue[i].extent.first + dst_diff < dst_stat.size)
				++i;
			/* The rest will be newly-allocated. */
			for (; i < queue_size; ++i)
				queue[i].is_new = true;
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
		malloc_ptr<std::byte[]> data_buff;
		std::size_t buff_size = 0;

		for (const auto &item : std::span(queue.get(), queue_size))
		{
			/* Process the extent operation block-by-block. */
			for (extent_type src_off = 0; src_off < item.extent.second; src_off += block_size)
			{
				const auto src_length = std::min(block_size, item.extent.second - src_off);
				if (item.op == clone_extents && do_clone_extents)
				{
					auto ol = OVERLAPPED{.Internal = ULONG_PTR(-1)};
					auto info = clone_info
					{
						.handle = native_handle(),
						.src_off = LONGLONG(item.extent.first + src_off),
						.dst_off = LONGLONG(item.extent.first + src_off + dst_diff),
						.length = LONGLONG(src_length),
					};

					if (DWORD written = 0; ::DeviceIoControl(req.dst.native_handle(), FSCTL_DUPLICATE_EXTENTS_TO_FILE, &info, sizeof(info), nullptr, 0, &written, &ol) == 0)
					{
						if (auto err = ::GetLastError(); err == ERROR_IO_PENDING)
						{
							auto status = ntapi->wait_io(req.dst.native_handle(), reinterpret_cast<io_status_block *>(&ol), abs_timeout);
							if (is_status_failure(status)) [[unlikely]]
								return status_error_code(status);
						}
						else if (err != ERROR_SUCCESS && !req.emulate)
							return dos_error_code(err);
						else
							do_clone_extents = false;
					}
					else
						goto item_complete;
				}
				if (item.op == erase_extents && do_erase_extents && !item.is_new)
				{
					auto info = FILE_ZERO_DATA_INFORMATION();
					info.FileOffset.QuadPart = LONGLONG(item.extent.first + src_off + dst_diff);
					info.BeyondFinalZero.QuadPart = info.FileOffset.QuadPart + src_length;
					auto ol = OVERLAPPED{.Internal = ULONG_PTR(-1)};

					if (DWORD written = 0; ::DeviceIoControl(req.dst.native_handle(), FSCTL_SET_ZERO_DATA, &info, sizeof(info), nullptr, 0, &written, &ol) == 0)
					{
						if (auto err = ::GetLastError(); err == ERROR_IO_PENDING)
						{
							auto status = ntapi->wait_io(req.dst.native_handle(), reinterpret_cast<io_status_block *>(&ol), abs_timeout);
							if (is_status_failure(status)) [[unlikely]]
								return status_error_code(status);
						}
						else if (err != ERROR_SUCCESS && !req.emulate)
							return dos_error_code(err);
						else
							do_erase_extents = false;
					}
					else
						goto item_complete;
				}

				/* (Re)allocate the buffer needed for byte-wise operations. */
				if (const auto aligned_len = page_align(std::max(src_length, extent_type(4096))); buff_size < aligned_len)
				{
					void *old_mem = data_buff.release(), *new_mem;
					if (old_mem != nullptr)
						new_mem = std::realloc(old_mem, buff_size = aligned_len);
					else
						new_mem = std::malloc(buff_size = aligned_len);

					if (new_mem == nullptr) [[unlikely]]
						return (std::free(old_mem), std::make_error_code(std::errc::not_enough_memory));
					else
						data_buff.reset(static_cast<std::byte *>(new_mem));
				}

				/* Fall back to byte-wise clone & erase if FSCTL failed or if force_copy is set. */
				if (item.op == clone_bytes || (item.op == clone_extents && !do_clone_extents))
				{
					auto src_buff = byte_buffer(data_buff.get(), src_length);
					auto read_res = read_some_at(*this, {.buffs = {&src_buff, 1}, .off = item.extent.first + src_off}, abs_timeout);
					if (read_res.has_error()) [[unlikely]]
						return read_res.error();
					if (read_res->front().size() < src_length) [[unlikely]]
						return std::make_error_code(std::errc::resource_unavailable_try_again);

					auto dst_buff = const_byte_buffer(read_res->front().data(), src_length);
					auto write_res = write_some_at(req.dst, {.buffs = {&dst_buff, 1}, .off = item.extent.first + src_off + dst_diff}, abs_timeout);
					if (write_res.has_error()) [[unlikely]]
						return write_res.error();
					if (write_res->front().size() < src_length) [[unlikely]]
						return std::make_error_code(std::errc::resource_unavailable_try_again);

					goto item_complete;
				}
				if ((item.op == erase_bytes || (item.op == erase_extents && !do_erase_extents)) && !item.is_new)
				{
					auto dst_buff = const_byte_buffer(data_buff.get(), src_length);
					std::memset(data_buff.get(), 0, std::size_t(src_length));

					auto write_res = write_some_at(req.dst, {.buffs = {&dst_buff, 1}, .off = item.extent.first + src_off + dst_diff}, abs_timeout);
					if (write_res.has_error()) [[unlikely]]
						return write_res.error();
					if (write_res->front().size() < src_length) [[unlikely]]
						return std::make_error_code(std::errc::resource_unavailable_try_again);
				}

			item_complete:
				result.second += src_off + src_length;
				restore_guard.release();
			}
		}
		return result;
	}
}

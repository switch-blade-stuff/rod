/*
 * Created by switchblade on 2023-10-29.
 */

#include "link_handle.hpp"

#include <numeric>

namespace rod::_link
{
	using namespace _win32;

	inline constexpr auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

	result<link_handle> link_handle::open(const path_handle &base, path_view path, file_flags flags, open_mode mode) noexcept
	{
		if (bool(flags & (file_flags::append | file_flags::no_sparse_files | file_flags::non_blocking | file_flags::case_sensitive))) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		if (mode == open_mode::truncate || mode == open_mode::supersede) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = render_as_ustring<true>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto &upath = rpath->first;
		auto guard = ntapi->dos_path_to_nt_path(upath, base.is_open());
		if (guard.has_error()) [[unlikely]]
			return std::make_error_code(std::errc::no_such_file_or_directory);

		auto obj_attrib = object_attributes();
		auto opts = make_handle_opts(flags, file_caching::all) | 0x4000 | 0x200000; /* FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT */
		auto attr = make_handle_attr(flags, file_caching::all, file_perm::all);
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
		return link_handle(*hnd, flags);
	}
	result<link_handle> link_handle::reopen(const link_handle &other, file_flags flags) noexcept
	{
		if (bool(flags & (file_flags::append | file_flags::no_sparse_files | file_flags::non_blocking | file_flags::case_sensitive))) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		/* Try to clone if possible. */
		if (flags == other.flags())
			return clone(other);

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto opts = flags_to_opts(flags) | 0x4000 | 0x200000; /*FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT*/
		auto access = flags_to_access(flags);
		auto iosb = io_status_block();

		auto hnd = ntapi->reopen_file(other.native_handle(), &iosb, access, share, opts);
		if (hnd.has_value()) [[likely]]
			return link_handle(*hnd, flags);
		else
			return hnd.error();
	}

	result<> link_handle::do_link(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = render_as_ustring<false>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto iosb = io_status_block();
		if (auto status = ntapi->link_file(native_handle(), &iosb, base.native_handle(), rpath->first, replace, to); is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return {};
	}
	result<> link_handle::do_relink(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = render_as_ustring<false>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto iosb = io_status_block();
		if (auto status = ntapi->relink_file(native_handle(), &iosb, base.native_handle(), rpath->first, replace, to); is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return {};
	}
	result<> link_handle::do_unlink(const file_timeout &to) noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto iosb = io_status_block();
		if (auto status = ntapi->unlink_file(native_handle(), &iosb, !bool(flags() & file_flags::unlink_on_close), to); is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return {};
	}

	read_some_result_t<link_handle> link_handle::do_read_some(io_request<read_some_t> &&req, const file_timeout &) noexcept
	{
		constexpr auto min_buff_size = sizeof(reparse_data_buffer) + MAX_PATH * sizeof(wchar_t);
		auto buff_data = reinterpret_cast<reparse_data_buffer *>(req.buffs.front().data());
		auto buff_size = req.buffs.front().size() * sizeof(wchar_t);

		/* Reserve a buffer if there is not enough space in the fist user-provided buffer. */
		const auto resize_buff = [&](std::size_t new_size) noexcept -> bool
		{
			if (new_size <= req.buffs._buff_max)
				return true;

			void *old_mem = req.buffs._buff.release(), *new_mem;
			if (old_mem != nullptr)
				new_mem = std::realloc(old_mem, new_size);
			else
				new_mem = std::malloc(new_size);
			if (new_mem == nullptr) [[unlikely]]
				return (std::free(old_mem), false);

			req.buffs._buff.reset(static_cast<wchar_t *>(new_mem));
			req.buffs._buff_max = new_size / sizeof(wchar_t);

			buff_data = static_cast<reparse_data_buffer *>(new_mem);
			buff_size = new_size;
			return true;
		};
		if (buff_size < min_buff_size && !resize_buff(min_buff_size)) [[unlikely]]
			return std::make_error_code(std::errc::not_enough_memory);

		for (;;)
		{
			if (DWORD written = 0; !::DeviceIoControl(native_handle(), FSCTL_GET_REPARSE_POINT, nullptr, 0, buff_data, DWORD(buff_size), &written, nullptr))
			{
				const auto err = ::GetLastError();
				if (err == ERROR_INSUFFICIENT_BUFFER || err == ERROR_MORE_DATA)
				{
					if (!resize_buff(buff_size * 2)) [[unlikely]]
						return std::make_error_code(std::errc::not_enough_memory);
					else
						continue;
				}
				return dos_error_code(err);
			}

			std::span<wchar_t> path_data;
			switch (buff_data->reparse_tag)
			{
			default: [[unlikely]]
				return std::make_error_code(std::errc::not_supported);
			case IO_REPARSE_TAG_SYMLINK:
				path_data = std::span(buff_data->symlink.path + buff_data->symlink.subst_name_off / sizeof(wchar_t), buff_data->symlink.subst_name_len / sizeof(wchar_t));
				req.buffs._type = link_type::symbolic;
				break;
			case IO_REPARSE_TAG_MOUNT_POINT:
				path_data = std::span(buff_data->mount_point.path + buff_data->mount_point.subst_name_off / sizeof(wchar_t), buff_data->mount_point.subst_name_len / sizeof(wchar_t));
				req.buffs._type = link_type::junction;
				break;
			}

			/* At this point, path_data contains the full path. Copy it across into the output buffers. */
			for (auto &dst : req.buffs)
			{
				/* If the path buffer is empty, clear the destination buffer. */
				if (path_data.empty())
				{
					/* Terminate the buffer before truncation to make path_view more efficient. */
					if ((dst._is_terminated = !dst._buff.empty()))
						dst._buff.front() = '\0';

					dst._buff = dst._buff.subspan(0, 0);
					continue;
				}

				/* Use the internal buffer if the destination buffer is empty. */
				auto dst_buff = dst._buff.empty() ? std::span(req.buffs._buff.get(), buff_size / sizeof(wchar_t)) : dst._buff;
				auto dst_size = std::min(dst_buff.size(), path_data.size());

				/* Copy at most dst_size chars to the destination buffer. */
				std::copy_n(path_data.data(), dst_size, dst_buff.data());
				/* Set the null terminator if there is enough space. */
				if ((dst._is_terminated = dst_buff.size() > dst_size))
					dst_buff[dst_size] = '\0';

				/* Advance the path buffer & truncate the destination buffer. */
				dst._buff = dst_buff.subspan(0, dst_size);
				path_data = path_data.subspan(dst_size);
			}
			return std::move(req.buffs);
		}
	}
	write_some_result_t<link_handle> link_handle::do_write_some(io_request<write_some_t> &&req, const file_timeout &) noexcept
	{
		if (req.buffs._type == link_type::unknown) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);

		/* Find the total size of the requested path components. */
		auto path_len = std::accumulate(req.buffs.begin(), req.buffs.end(), std::size_t(0), [](auto i, auto &b) { return i + b.size(); });
		if (path_len > std::numeric_limits<USHORT>::max()) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);

		/* Take into account custom NT prefix. */
		std::size_t front_off = 0, name_len = path_len;
		if (auto &front = req.buffs.front(); front._buff[0] == '\\' && front._buff[1] == '!' && front._buff[2] == '!' && front._buff[3] == '\\')
		{
			front_off = 3;
			path_len -= 3;
		}

		/* Reserve a buffer large enough for all path components and reparse_data_buffer. */
		if (const auto bytes = sizeof(reparse_data_buffer) + (path_len + name_len + 2) * sizeof(wchar_t); req.buffs._buff_max < bytes)
		{
			void *old_mem = req.buffs._buff.release(), *new_mem;
			if (old_mem != nullptr)
				new_mem = std::realloc(old_mem, bytes);
			else
				new_mem = std::malloc(bytes);
			if (new_mem == nullptr) [[unlikely]]
				return (std::free(old_mem), std::make_error_code(std::errc::not_enough_memory));

			req.buffs._buff.reset(static_cast<wchar_t *>(new_mem));
			req.buffs._buff_max = bytes / sizeof(wchar_t);
		}
		const auto buff_data = reinterpret_cast<reparse_data_buffer *>(req.buffs._buff.get());
		const auto buff_size = req.buffs._buff_max * sizeof(wchar_t);
		*buff_data = reparse_data_buffer();

		/* Select the correct reparse_data_buffer member. */
		wchar_t *dst_path, *dst_name;
		if (req.buffs._type == link_type::symbolic)
		{
			buff_data->data_size = buff_size - offsetof(reparse_data_buffer, symlink);
			buff_data->reparse_tag = IO_REPARSE_TAG_SYMLINK;

			buff_data->symlink.subst_name_off = 0;
			buff_data->symlink.print_name_off = USHORT(path_len + 1) * sizeof(wchar_t);
			buff_data->symlink.subst_name_len = USHORT(path_len) * sizeof(wchar_t);
			buff_data->symlink.print_name_len = USHORT(name_len) * sizeof(wchar_t);

			dst_name = buff_data->symlink.path + buff_data->symlink.print_name_off / sizeof(wchar_t);
			dst_path = buff_data->symlink.path;
		}
		else
		{
			buff_data->data_size = buff_size - offsetof(reparse_data_buffer, mount_point);
			buff_data->reparse_tag = IO_REPARSE_TAG_MOUNT_POINT;

			buff_data->mount_point.subst_name_off = 0;
			buff_data->mount_point.print_name_off = USHORT(path_len + 1) * sizeof(wchar_t);
			buff_data->mount_point.subst_name_len = USHORT(path_len) * sizeof(wchar_t);
			buff_data->mount_point.print_name_len = USHORT(name_len) * sizeof(wchar_t);

			dst_name = buff_data->mount_point.path + buff_data->mount_point.print_name_off / sizeof(wchar_t);
			dst_path = buff_data->mount_point.path;
		}


		/* Fill the reparse_data_buffer with contents of the request. */
		for (std::size_t i = 0; i < req.buffs.size(); ++i)
		{
			const auto &src = req.buffs[i];
			const auto src_path_data = src.data() + front_off;
			const auto src_path_size = src.size() - front_off;
			const auto src_name_data = src.data();
			const auto src_name_size = src.size();

			dst_path = std::copy_n(src_path_data, src_path_size, dst_path);
			dst_name = std::copy_n(src_name_data, src_name_size, dst_name);
			front_off = 0;
		}
		*dst_path = '\0';
		*dst_name = '\0';

		if (DWORD written = 0; !::DeviceIoControl(native_handle(), FSCTL_SET_REPARSE_POINT, buff_data, buff_size, nullptr, 0, &written, nullptr)) [[unlikely]]
			return dos_error_code(::GetLastError());
		else
			return std::move(req.buffs);
	}
}

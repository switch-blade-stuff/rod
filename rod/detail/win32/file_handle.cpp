/*
 * Created by switch_blade on 2023-09-15.
 */

#include "file_handle.hpp"

namespace rod::_file
{
	using namespace _win32;

	inline constexpr auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

	result<file_handle> file_handle::do_open(const path_handle &base, path_view path, file_flags flags, open_mode mode, file_caching caching, file_perm perm) noexcept
	{
		/* NtCreateFile does not support append access without IO buffering. */
		if (!bool(caching & (file_caching::read | file_caching::write)) && bool(flags & file_flags::append))
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
			else if (is_error_not_a_directory(err))
				return std::make_error_code(std::errc::not_a_directory);
			else if (is_error_file_exists(err) && mode == open_mode::create)
				return std::make_error_code(std::errc::file_exists);
			else
				return err;
		}

		/* Make sparse if created a new file. */
		if (!bool(flags & file_flags::no_sparse_files) && (mode == open_mode::truncate || mode == open_mode::supersede || iosb.info == 2 /*FILE_CREATED*/))
		{
			DWORD bytes = 0;
			auto buffer = FILE_SET_SPARSE_BUFFER{.SetSparse = 1};
			::DeviceIoControl(*hnd, FSCTL_SET_SPARSE, &buffer, sizeof(buffer), nullptr, 0, &bytes, nullptr);
		}
		/* Flush the file if additional sanity barriers are requested. */
		if (mode == open_mode::truncate && bool(caching & file_caching::sanity_barriers))
			::FlushFileBuffers(*hnd);

		return file_handle(*hnd, flags, caching);
	}

	result<> file_handle::do_link(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		if (!bool(flags() & file_flags::non_blocking) && to != timeout_type())
			return std::make_error_code(std::errc::not_supported);

		const auto abs_timeout = to.absolute();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = render_as_ustring<false>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto iosb = io_status_block();
		if (auto status = ntapi->link_file(native_handle(), &iosb, base.native_handle(), rpath->first, replace, abs_timeout); is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return {};
	}
	result<> file_handle::do_relink(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
	{
		if (!bool(flags() & file_flags::non_blocking) && to != timeout_type())
			return std::make_error_code(std::errc::not_supported);

		const auto abs_timeout = to.absolute();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = render_as_ustring<false>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto iosb = io_status_block();
		if (auto status = ntapi->relink_file(native_handle(), &iosb, base.native_handle(), rpath->first, replace, abs_timeout); is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return {};
	}
	result<> file_handle::do_unlink(const file_timeout &to) noexcept
	{
		if (!bool(flags() & file_flags::non_blocking) && to != timeout_type())
			return std::make_error_code(std::errc::not_supported);

		const auto abs_timeout = to.absolute();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto iosb = io_status_block();
		if (auto status = ntapi->unlink_file(native_handle(), &iosb, !bool(flags() & file_flags::unlink_on_close), abs_timeout); is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return {};
	}

	template<auto IoFunc, typename Op>
	typename file_handle::io_result<Op> file_handle::invoke_io_func_at(io_request<Op> req, std::uint64_t pos, const file_timeout &to) noexcept
	{
		if (!bool(flags() & file_flags::non_blocking) && to != timeout_type())
			return std::make_error_code(std::errc::not_supported);

		const auto abs_timeout = to.absolute();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		const auto buff_size = req.buffs.size() * sizeof(io_status_block);
		auto iosb_buff = ROD_MAKE_BUFFER(io_status_block, buff_size);
		if (iosb_buff.get() == nullptr) [[unlikely]]
			return std::make_error_code(std::errc::not_enough_memory);

		std::memset(iosb_buff.get(), 0, buff_size);
		std::size_t pending = 0, bytes_done = 0;
		std::error_code result_status = {};

		/* Start IO operations. */
		for (std::size_t i = 0; i < req.buffs.size(); ++i)
		{
			auto &iosb = iosb_buff.get()[i];
			auto &data = req.buffs[i];

			LARGE_INTEGER offset = {.QuadPart = LONGLONG(pos)};
			if (offset.QuadPart < 0) [[unlikely]]
			{
				result_status = std::make_error_code(std::errc::value_too_large);
				break;
			}

			((*ntapi).*IoFunc)(native_handle(), nullptr, nullptr, 0, &iosb, const_cast<std::byte *>(data.data()), ULONG(data.size()), &offset, nullptr);
			if (iosb.status == STATUS_PENDING)
			{
				pos += data.size();
				pending += 1;
			}
			else if (!is_status_failure(iosb.status))
				pos += iosb.info;
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

	read_some_at_result_t<file_handle> file_handle::do_read_some_at(io_request<read_some_at_t> req, std::uint64_t pos, const file_timeout &to) noexcept
	{
		return invoke_io_func_at<&ntapi::NtReadFile>(std::forward<decltype(req)>(req), pos, to);
	}
	write_some_at_result_t<file_handle> file_handle::do_write_some_at(io_request<write_some_at_t> req, std::uint64_t pos, const file_timeout &to) noexcept
	{
		return invoke_io_func_at<&ntapi::NtWriteFile>(std::forward<decltype(req)>(req), pos, to);
	}
}

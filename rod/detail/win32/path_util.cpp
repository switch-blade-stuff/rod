/*
 * Created by switchblade on 2023-08-31.
 */

#include "path_util.hpp"

namespace rod::fs
{
	using namespace _win32;

	result<path> current_path() noexcept
	{
		try
		{
			std::wstring path;
			if (const auto n = ::GetCurrentDirectoryW(0, nullptr); !n) [[unlikely]]
				return dos_error_code(::GetLastError());
			else
				path.resize(n - 1);
			if (!::GetCurrentDirectoryW(DWORD(path.size() + 1), path.data())) [[unlikely]]
				return dos_error_code(::GetLastError());
			else
				return std::move(path);
		}
		catch (...) { return _detail::current_error(); }
	}
	result<void> current_path(path_view path) noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = path.render_unterminated();
		auto upath = unicode_string();
		upath.max = (upath.size = USHORT(rpath.size() * sizeof(wchar_t))) + sizeof(wchar_t);
		upath.buff = const_cast<wchar_t *>(rpath.data());

		auto guard = ntapi->canonize_win32_path(upath, false);
		if (guard.has_error()) [[unlikely]]
			return guard.error();

		if (!::SetCurrentDirectoryW(upath.buff))
			return dos_error_code(::GetLastError());
		else
			return {};
	}

	result<bool> exists(path_view path) noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return {in_place_error, ntapi.error()};

		auto rpath = render_as_wchar<true>(path);
		if (rpath.has_error()) [[unlikely]]
			return {in_place_error, rpath.error()};

		auto upath = make_ustring(rpath->as_span());
		auto guard = ntapi->dos_path_to_nt_path(upath, false);
		if (guard.has_error())
			return false;

		auto basic_info = file_basic_information();
		auto obj_attrib = object_attributes();
		obj_attrib.length = sizeof(object_attributes);
		obj_attrib.name = &upath;

		/* Try to get file attributes to see if the file exists. */
		if (auto status = ntapi->NtQueryAttributesFile(&obj_attrib, &basic_info); is_status_failure(status)) [[unlikely]]
		{
			if (status == 0xc000003a /*STATUS_OBJECT_PATH_NOT_FOUND*/)
				return false;

			/* NtQueryAttributesFile may fail in case the path is a DOS device name. */
			return ntapi->RtlIsDosDeviceName_U(upath.buff);
		}
		return true;
	}
	result<path> absolute(path_view path) noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = render_as_wchar<true>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto upath = make_ustring(rpath->as_span());
		auto guard = ntapi->canonize_win32_path(upath, false);
		if (guard.has_error()) [[unlikely]]
			return guard.error();

		try
		{
			std::wstring res;
			if (const auto n = ::GetFullPathNameW(upath.buff, 0, nullptr, nullptr); !n) [[unlikely]]
				return dos_error_code(::GetLastError());
			else
				res.resize(n);
			if (!::GetFullPathNameW(upath.buff, DWORD(res.size() + 1), res.data(), nullptr)) [[unlikely]]
				return dos_error_code(::GetLastError());
			else
				return std::move(res);
		}
		catch (...) { return _detail::current_error(); }
	}

	inline static result<path> do_canonical(const ntapi &ntapi, unicode_string &upath) noexcept
	{
		constexpr auto opts = 0x20 | 0x4000 | 0x200000 /*FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT*/;
		constexpr auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
		constexpr auto access = SYNCHRONIZE | FILE_READ_ATTRIBUTES;

		auto guard = ntapi.dos_path_to_nt_path(upath, false);
		if (guard.has_error()) [[unlikely]]
			return std::make_error_code(std::errc::no_such_file_or_directory);

		auto obj_attrib = object_attributes();
		auto iosb = io_status_block();

		obj_attrib.length = sizeof(object_attributes);
		obj_attrib.name = &upath;

		/* Open the target with backup semantics (i.e. do not care if directory or file). */
		auto hnd = ntapi.open_file(obj_attrib, &iosb, access, share, opts).transform_value([](auto hnd) { return path_handle(hnd); });
		if (hnd.has_error() && is_error_file_not_found(hnd.error())) [[unlikely]]
			return std::make_error_code(std::errc::no_such_file_or_directory);

		/* Use DOS native path instead of to_object_path since to_object_path returns an NT path. */
		return to_native_path(*hnd, native_path_format::generic);
	}

	result<path> canonical(path_view path) noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = render_as_wchar<true>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto upath = make_ustring(rpath->as_span());
		return do_canonical(*ntapi, upath);
	}
	result<path> weakly_canonical(path_view path) noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = render_as_wchar<true>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto upath = make_ustring(rpath->as_span());
		auto res = do_canonical(*ntapi, upath);
		if (res.has_value() || !is_error_file_not_found(res.error()))
			return res;

		try
		{
			const auto norm = fs::path(path).lexically_normal();
			const auto rel = path_view(norm).relative_path();
			res.emplace_value(norm.root_path());
			bool canonize = true;

			/* Convert as many valid components as possible. */
			for (auto &comp : rel)
			{
				res->append(comp);
				if (!canonize)
					continue;

				upath = make_ustring(res->native());
				auto tmp = do_canonical(*ntapi, upath);

				if (tmp.has_value()) [[likely]]
					res = std::move(tmp);
				else if (is_error_file_not_found(tmp.error()))
					canonize = false;
				else
					return tmp;
			}
			return res;
		}
		catch (...) { return _detail::current_error(); }
	}

	inline static result<basic_handle> open_deletable_handle(const ntapi &ntapi, io_status_block *iosb, const path_handle &base, unicode_string &upath, const file_timeout &to) noexcept
	{
		constexpr auto opts = 0x20 | 0x4000 /*FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT*/;
		constexpr auto access = SYNCHRONIZE | DELETE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES;
		constexpr auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

		auto obj_attrib = object_attributes();
		obj_attrib.root_dir = base.is_open() ? base.native_handle() : nullptr;
		obj_attrib.length = sizeof(object_attributes);
		obj_attrib.name = &upath;

		/* Open the target with backup semantics. */
		auto hnd = ntapi.open_file(obj_attrib, iosb, access, share, opts, to);
		/* Re-try without attribute permissions. */
		if (auto err = hnd.error_or({}); err.category() == status_category() && err.value() == 0xc0000022 /*STATUS_ACCESS_DENIED*/) [[unlikely]]
			hnd = ntapi.open_file(obj_attrib, iosb, DELETE | SYNCHRONIZE, share, opts, to);

		if (hnd.has_value()) [[likely]]
			return basic_handle(*hnd);
		else
			return hnd.error();
	}
	inline static result<directory_handle> open_readable_dir(const ntapi &ntapi, io_status_block *iosb, const path_handle &base, unicode_string &upath, const file_timeout &to) noexcept
	{
		constexpr auto opts = 0x20 | 1 /*FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE*/;
		constexpr auto access = SYNCHRONIZE | DELETE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | FILE_LIST_DIRECTORY;
		constexpr auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

		auto obj_attrib = object_attributes();
		obj_attrib.root_dir = base.is_open() ? base.native_handle() : nullptr;
		obj_attrib.length = sizeof(object_attributes);
		obj_attrib.name = &upath;

		/* Open the target with backup semantics. */
		auto hnd = ntapi.open_file(obj_attrib, iosb, access, share, opts, to);
		/* Re-try without attribute permissions. */
		if (auto err = hnd.error_or({}); err.category() == status_category() && err.value() == 0xc0000022 /*STATUS_ACCESS_DENIED*/) [[unlikely]]
			hnd = ntapi.open_file(obj_attrib, iosb, DELETE | SYNCHRONIZE, share, opts, to);

		if (hnd.has_value()) [[likely]]
			return directory_handle(*hnd, file_flags::attr_read | file_flags::write);
		else
			return hnd.error();
	}

	inline static result<std::size_t> do_remove(const ntapi &ntapi, io_status_block *iosb, typename basic_handle::native_handle_type hnd, const file_timeout &to) noexcept
	{
		/* Try to unlink with POSIX semantics. */
		auto disp_info_ex = file_disposition_information_ex{.flags = 0x1 | 0x2 | 0x10}; /*FILE_DISPOSITION_DELETE | FILE_DISPOSITION_POSIX_SEMANTICS | FILE_DISPOSITION_IGNORE_READONLY_ATTRIBUTE*/
		auto status = ntapi.set_file_info(hnd, iosb, &disp_info_ex, FileBasicInformation, FileDispositionInformationEx, to);
		if (!is_status_failure(status))
			return 1;

		/* Fallback to Win10 API. */
		auto disp_info = file_disposition_information{.del = true};
		status = ntapi.set_file_info(hnd, iosb, &disp_info, FileDispositionInformation, to);
		if (is_status_failure(status) && status != 0xc0000022 /*STATUS_ACCESS_DENIED*/)
			return status_error_code(status);

		/* If returned STATUS_ACCESS_DENIED, try to clear the readonly flag. */
		auto basic_info = file_basic_information();
		status = ntapi.get_file_info(hnd, iosb, &basic_info, FileBasicInformation, to);
		if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);

		/* If FILE_ATTRIBUTE_READONLY is not set, STATUS_ACCESS_DENIED was caused by something else. */
		if (!(basic_info.attributes & FILE_ATTRIBUTE_READONLY)) [[unlikely]]
			return std::make_error_code(std::errc::permission_denied);

		basic_info.attributes ^= FILE_ATTRIBUTE_READONLY;
		status = ntapi.set_file_info(hnd, iosb, &basic_info, FileBasicInformation, to);
		if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);

		/* Re-try to set delete flag. If failed again, restore the readonly flag and report error. */
		status = ntapi.set_file_info(hnd, iosb, &disp_info, FileDispositionInformation, to);
		if (is_status_failure(status)) [[unlikely]]
		{
			basic_info.attributes |= FILE_ATTRIBUTE_READONLY;
			auto old_status = status;

			status = ntapi.set_file_info(hnd, iosb, &basic_info, FileBasicInformation, to);
			if (!is_status_failure(status)) [[likely]]
				return status_error_code(old_status);
			else
				return status_error_code(status);
		}
		return 1;
	}
 	inline static result<std::size_t> do_remove(const ntapi &ntapi, io_status_block *iosb, const path_handle &base, unicode_string &upath, const file_timeout &to) noexcept
	{
		auto hnd = open_deletable_handle(ntapi, iosb, base, upath, to);
		if (hnd.has_value()) [[likely]]
			return do_remove(ntapi, iosb, hnd->native_handle(), to);

		auto err = hnd.error();
		if (err.category() != std::generic_category() || err.value() != int(std::errc::no_such_file_or_directory))
			return err;
		else
			return 0;
	}
	inline static result<std::size_t> do_remove_all(const ntapi &ntapi, io_status_block *iosb, const path_handle &base, unicode_string &upath, const file_timeout &to) noexcept
	{
		auto hnd = open_readable_dir(ntapi, iosb, base, upath, to);
		if (hnd.has_error()) [[unlikely]]
		{
			if (auto err = hnd.error(); err != std::make_error_condition(std::errc::no_such_file_or_directory))
				return err;
			else
				return 0;
		}

		auto ent = std::array{directory_handle::io_buffer<read_some_t>()};
		auto seq = directory_handle::io_buffer_sequence<read_some_t>(ent);
		std::size_t removed = 0;

		for (;;)
		{
			auto read_res = read_some(*hnd, {.buffs = std::move(seq),  .reset = true}, to);
			if (read_res.has_error()) [[unlikely]]
				return read_res.error();
			else if (!read_res->first.empty())
				seq = std::move(read_res->first);
			else
				break;

			auto [st, mask] = seq.front().st();
			if (!bool(mask & stat::query::type)) [[unlikely]]
				continue; /* Should never happen. */

			auto uleaf = unicode_string();
			uleaf.size = uleaf.max = USHORT(seq.front().size() * sizeof(wchar_t));
			uleaf.buff = seq.front().data();

			/* Recursively call remove on the subdirectory. */
			result<std::size_t> remove_res;
			if (st.type == file_type::directory)
				remove_res = do_remove_all(ntapi, iosb, *hnd, uleaf, to);
			else
				remove_res = do_remove(ntapi, iosb, *hnd, uleaf, to);

			if (remove_res.has_value()) [[likely]]
				removed += *remove_res;
			else
				return remove_res;
			if (read_res->second)
				break;
		}

		/* Finally, try to remove the directory itself. */
		constexpr std::size_t spin_max = 10;
		for (std::size_t i = 0;;)
		{
			auto res = do_remove(ntapi, iosb, hnd->native_handle(), to);
			if (res.has_value()) [[likely]]
				return removed + *res;

			/* Mirror MSVC STL behavior and retry in the following cases:
			 * STATUS_DIRECTORY_NOT_EMPTY: 0xc0000101 - directory entries might not be deleted yet.
			 * STATUS_ACCESS_DENIED: 0xc0000022 - directory might be marked for deletion. */
			if (const auto now = file_clock::now(); now >= to.absolute(now)) [[unlikely]]
				return std::make_error_code(std::errc::timed_out);
			if (auto err = res.error(); err.category() != status_category() || (err.value() != 0xc0000101 && err.value() != 0xc0000022))
				return err;
			if (i > spin_max) [[unlikely]]
				YieldProcessor();
		}
	}

	result<std::size_t> remove(const path_handle &base, path_view path, const file_timeout &to) noexcept
	{
		const auto abs_timeout = to.is_infinite() ? to : to.absolute();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = render_as_wchar<true>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto upath = make_ustring(rpath->as_span());
		auto guard = ntapi->dos_path_to_nt_path(upath, base.is_open());
		if (guard.has_error())
			return 0;

		auto iosb = io_status_block();
		auto res = do_remove(*ntapi, &iosb, base, upath, abs_timeout);
		if (res.has_value()) [[likely]]
			return res;

		/* Map STATUS_DIRECTORY_NOT_EMPTY to std::errc::directory_not_empty. */
		if (auto err = res.error(); is_error_not_empty(err)) [[likely]]
			return std::make_error_code(std::errc::directory_not_empty);
		else
			return res;
	}
	result<std::size_t> remove_all(const path_handle &base, path_view path, const file_timeout &to) noexcept
	{
		const auto abs_timeout = to.is_infinite() ? to : to.absolute();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = render_as_wchar<true>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto upath = make_ustring(rpath->as_span());
		auto guard = ntapi->dos_path_to_nt_path(upath, base.is_open());
		if (guard.has_error())
			return 0;

		auto iosb = io_status_block();
		auto res = do_remove(*ntapi, &iosb, base, upath, abs_timeout);
		if (res.has_value()) [[likely]]
			return res;

		/* If returned STATUS_DIRECTORY_NOT_EMPTY, recursively iterate through the directory. */
		if (auto err = res.error(); is_error_not_empty(err)) [[likely]]
			return do_remove_all(*ntapi, &iosb, base, upath, abs_timeout);
		else
			return res;
	}
}

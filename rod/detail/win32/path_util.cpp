/*
 * Created by switch_blade on 2023-08-31.
 */

#include "../path_algo.hpp"
#include "ntapi.hpp"

namespace rod
{
	using namespace _win32;

	auto current_path() noexcept -> result<path>
	{
		try
		{
			std::wstring path;
			if (const auto n = ::GetCurrentDirectoryW(0, nullptr); !n) [[unlikely]]
				return dos_error_code(::GetLastError());
			else
				path.resize(n);
			if (!::GetCurrentDirectoryW(DWORD(path.size() + 1), path.data())) [[unlikely]]
				return dos_error_code(::GetLastError());
			else
				return std::move(path);
		}
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
		catch (const std::system_error &e) { return e.code(); }
	}
	auto current_path(path_view path) noexcept -> result<>
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

		auto rend = render_as_ustring<true>(path);
		if (rend.has_error()) [[unlikely]]
			return {in_place_error, rend.error()};

		auto &[upath, rpath] = *rend;
		auto guard = ntapi->dos_path_to_nt_path(upath, false);
		if (auto err = guard.error_or({}); err && err.value() != ERROR_PATH_NOT_FOUND) [[unlikely]]
			return {in_place_error, guard.error()};
		else if (err.value() == ERROR_PATH_NOT_FOUND)
			return false;

		auto basic_info = file_basic_information();
		auto obj_attrib = object_attributes();
		obj_attrib.length = sizeof(object_attributes);
		obj_attrib.name = &upath;

		/* If possible, try to get all requested data using `NtQueryAttributesFile` to avoid opening a handle altogether. */
		if (auto status = ntapi->NtQueryAttributesFile(&obj_attrib, &basic_info); is_status_failure(status)) [[unlikely]]
		{
			if (status == 0xc000003a /*STATUS_OBJECT_PATH_NOT_FOUND*/)
				return false;

			/* NtQueryAttributesFile may fail in case the path is a DOS device name. */
			if (!ntapi->RtlIsDosDeviceName_Ustr(&upath)) [[unlikely]]
				return {in_place_error, status_error_code(status)};
			else
				return false;
		}
		return true;
	}
	result<bool> equivalent(path_view a, path_view b) noexcept
	{
		dev_t dev_a, dev_b;
		ino_t ino_a, ino_b;
		stat st;

		if (auto res = get_stat(st, {}, a, stat::query::dev | stat::query::ino); res.has_error()) [[unlikely]]
			return {in_place_error, res.error()};
		dev_a = st.dev;
		ino_a = st.ino;

		if (auto res = get_stat(st, {}, b, stat::query::dev | stat::query::ino); res.has_error()) [[unlikely]]
			return {in_place_error, res.error()};
		dev_b = st.dev;
		ino_b = st.ino;

		return dev_a == dev_b && ino_a == ino_b;
	}

	inline static result<void *> open_readable(const ntapi &ntapi, io_status_block *iosb, const path_handle &base, unicode_string *upath, const file_timeout &to) noexcept
	{
		constexpr auto opts = 0x20 | 1 /*FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE*/;
		constexpr auto access = SYNCHRONIZE | FILE_READ_ATTRIBUTES | FILE_LIST_DIRECTORY;
		constexpr auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

		auto obj_attrib = object_attributes();
		obj_attrib.root_dir = base.is_open() ? base.native_handle() : nullptr;
		obj_attrib.length = sizeof(object_attributes);
		obj_attrib.name = upath;

		/* Open the target with backup semantics (i.e. do not care if directory or file). */
		auto hnd = ntapi.open_file(obj_attrib, iosb, access, share, opts, to);
		/* Re-try without attribute permissions. */
		if (auto err = hnd.error_or({}); err.category() == status_category() && err.value() == 0xc0000022 /*STATUS_ACCESS_DENIED*/) [[unlikely]]
			hnd = ntapi.open_file(obj_attrib, iosb, DELETE | SYNCHRONIZE, share, opts, to);
		return hnd;
	}
	inline static result<void *> open_deletable(const ntapi &ntapi, io_status_block *iosb, const path_handle &base, unicode_string *upath, const file_timeout &to) noexcept
	{
		constexpr auto opts = 0x20 | 0x4000 | 0x200000 /*FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT*/;
		constexpr auto access = SYNCHRONIZE | DELETE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES;
		constexpr auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

		auto obj_attrib = object_attributes();
		obj_attrib.root_dir = base.is_open() ? base.native_handle() : nullptr;
		obj_attrib.length = sizeof(object_attributes);
		obj_attrib.name = upath;

		/* Open the target with backup semantics (i.e. do not care if directory or file). */
		auto hnd = ntapi.open_file(obj_attrib, iosb, access, share, opts, to);
		/* Re-try without attribute permissions. */
		if (auto err = hnd.error_or({}); err.category() == status_category() && err.value() == 0xc0000022 /*STATUS_ACCESS_DENIED*/) [[unlikely]]
			hnd = ntapi.open_file(obj_attrib, iosb, DELETE | SYNCHRONIZE, share, opts, to);
		return hnd;
	}

 	inline static result<std::size_t> do_remove(const ntapi &ntapi, io_status_block *iosb, const path_handle &base, unicode_string *upath, const file_timeout &to) noexcept
	{
		auto res = open_deletable(ntapi, iosb, base, upath, to);
		auto hnd = basic_handle();

		if (auto err = res.error_or({}); err.category() == std::generic_category() && err.value() == int(std::errc::no_such_file_or_directory))
			return 0;
		else if (!err) [[likely]]
			hnd.release(*res);
		else
			return err;

		/* Try to unlink with POSIX semantics via the Win10 API. */
		auto disp_info_ex = file_disposition_information_ex{.flags = 0x1 | 0x2 | 0x10}; /*FILE_DISPOSITION_DELETE | FILE_DISPOSITION_POSIX_SEMANTICS | FILE_DISPOSITION_IGNORE_READONLY_ATTRIBUTE*/
		auto status = ntapi.set_file_info(hnd.native_handle(), iosb, &disp_info_ex, FileBasicInformation, FileDispositionInformationEx, to);
		if (!is_status_failure(status))
			return 1;

		/* Fallback to Win10 API. */
		auto disp_info = file_disposition_information{.del = true};
		status = ntapi.set_file_info(hnd.native_handle(), iosb, &disp_info, FileDispositionInformation, to);
		if (is_status_failure(status) && status != 0xc0000022 /*STATUS_ACCESS_DENIED*/)
			return status_error_code(status);

		/* If returned STATUS_ACCESS_DENIED, try to clear the readonly flag. */
		auto basic_info = file_basic_information();
		status = ntapi.get_file_info(hnd.native_handle(), iosb, &basic_info, FileBasicInformation, to);
		if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);

		/* If FILE_ATTRIBUTE_READONLY is not set, STATUS_ACCESS_DENIED was caused by something else. */
		if (!(basic_info.attributes & FILE_ATTRIBUTE_READONLY)) [[unlikely]]
			return std::make_error_code(std::errc::permission_denied); /* EPERM, not STATUS_ACCESS_DENIED */

		basic_info.attributes ^= FILE_ATTRIBUTE_READONLY;
		status = ntapi.set_file_info(hnd.native_handle(), iosb, &basic_info, FileBasicInformation, to);
		if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);

		/* Re-try to set delete flag. If failed again, restore the readonly flag and report error. */
		status = ntapi.set_file_info(hnd.native_handle(), iosb, &disp_info, FileDispositionInformation, to);
		if (is_status_failure(status)) [[unlikely]]
		{
			basic_info.attributes |= FILE_ATTRIBUTE_READONLY;
			auto old_status = status;

			status = ntapi.set_file_info(hnd.native_handle(), iosb, &basic_info, FileBasicInformation, to);
			if (!is_status_failure(status)) [[likely]]
				return status_error_code(old_status);
			else
				return status_error_code(status);
		}
		return 1;
	}
	inline static result<std::size_t> do_remove_all(const ntapi &ntapi, io_status_block *iosb, const path_handle &base, unicode_string *upath, const file_timeout &to) noexcept
	{
		/* Mirror MSVC STL behavior. */
		constexpr std::size_t retry_max = 10;

		auto hnd = open_readable(ntapi, iosb, base, upath, to);
		if (auto err = hnd.error_or({}); err.category() == std::generic_category() && err.value() == int(std::errc::no_such_file_or_directory))
			return 0;
		else if (err) [[unlikely]]
			return err;

		auto ent = std::array{read_some_buffer_t<directory_handle>()};
		auto seq = read_some_buffer_sequence_t<directory_handle>(ent);
		auto dir = directory_handle(*hnd);
		std::size_t removed = 0;

		for (;;)
		{
			auto req = read_some_request_t<directory_handle>
			{
				.buffs = std::move(seq),
				.resume = true,
			};
			auto io_res = read_some(dir, req, to);
			if (io_res.has_value() && io_res->empty())
				break;
			if (io_res.has_error()) [[unlikely]]
				return io_res.error();

			auto [st, mask] = io_res->front().st();
			if (!bool(mask & stat::query::type)) [[unlikely]]
				continue; /* Should never happen. */

			auto uleaf = unicode_string();
			uleaf.size = uleaf.max = io_res->front().size() * sizeof(wchar_t);
			uleaf.buff = io_res->front().data();

			/* Recursively call remove on the subdirectory. */
			result<std::size_t> res;
			if (st.type == file_type::directory)
				res = do_remove_all(ntapi, iosb, dir, &uleaf, to);
			else
				res = do_remove(ntapi, iosb, dir, &uleaf, to);

			if (res.has_value()) [[likely]]
				removed += *res;
			else
				return res;
		}

		/* Finally, try to remove the directory itself. */
		for (std::size_t i = 0;;)
		{
			auto res = do_remove(ntapi, iosb, base, upath, to);
			if (res.has_value()) [[likely]]
				return removed + *res;

			/* Retry in the following cases:
			 * STATUS_DIRECTORY_NOT_EMPTY: 0xc0000101 - directory entries might not be deleted yet.
			 * ERROR_ACCESS_DENIED: 0xc0000022 - directory might be marked for deletion. */
			if (auto err = res.error(); i++ == retry_max || err.category() != status_category() || (err.value() != 0xc0000101 && err.value() != 0xc0000022))
				return err;
		}
	}

	result<bool> remove(const path_handle &base, path_view path, const file_timeout &to) noexcept
	{
		const auto abs_timeout = to.absolute();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return {in_place_error, ntapi.error()};

		auto rend = render_as_ustring<true>(path);
		if (rend.has_error()) [[unlikely]]
			return {in_place_error, rend.error()};

		auto &[upath, rpath] = *rend;
		auto guard = ntapi->dos_path_to_nt_path(upath, false);
		if (auto err = guard.error_or({}); err && err.value() != ERROR_PATH_NOT_FOUND) [[unlikely]]
			return {in_place_error, guard.error()};
		else if (err.value() == ERROR_PATH_NOT_FOUND)
			return false;

		/* Map STATUS_DIRECTORY_NOT_EMPTY to std::errc::directory_not_empty. */
		auto iosb = io_status_block();
		auto res =  do_remove(*ntapi, &iosb, base, &upath, abs_timeout);
		if (auto err = res.error_or({}); err.category() == status_category() && err.value() == 0xc0000101/*STATUS_DIRECTORY_NOT_EMPTY*/)
			return {in_place_error, std::make_error_code(std::errc::directory_not_empty)};
		else
			return res;
	}
	result<std::size_t> remove_all(const path_handle &base, path_view path, const file_timeout &to) noexcept
	{
		const auto abs_timeout = to.absolute();
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rend = render_as_ustring<true>(path);
		if (rend.has_error()) [[unlikely]]
			return rend.error();

		auto &[upath, rpath] = *rend;
		auto guard = ntapi->dos_path_to_nt_path(upath, false);
		if (auto err = guard.error_or({}); err && err.value() != ERROR_PATH_NOT_FOUND) [[unlikely]]
			return guard.error();
		else if (err.value() == ERROR_PATH_NOT_FOUND)
			return 0;

		/* Try to remove using `do_remove`. If returned STATUS_DIRECTORY_NOT_EMPTY, recursively iterate through the directory. */
		auto iosb = io_status_block();
		auto res = do_remove(*ntapi, &iosb, base, &upath, abs_timeout);
		if (auto err = res.error_or({}); err.category() == status_category() && err.value() == 0xc0000101/*STATUS_DIRECTORY_NOT_EMPTY*/)
			return do_remove_all(*ntapi, &iosb, base, &upath, abs_timeout);
		else
			return res;
	}
}

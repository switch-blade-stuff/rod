/*
 * Created by switch_blade on 2023-08-29.
 */

#include "../handle_stat.hpp"

#include <cstring>
#include <cwchar>

#include "../handle_base.hpp"
#include "ntapi.hpp"

namespace rod::_handle
{
	using namespace _win32;

	constexpr auto basic_info_mask = stat::query::type | stat::query::atime | stat::query::mtime | stat::query::ctime | stat::query::btime | stat::query::sparse | stat::query::compressed | stat::query::reparse_point;
	constexpr auto standard_info_mask = stat::query::size | stat::query::alloc | stat::query::blocks | stat::query::nlink;
	constexpr auto internal_info_mask = stat::query::ino;

	constexpr std::size_t buff_size = 32769;

	inline static result<file_type> query_file_type(const ntapi &ntapi, void *hnd, ULONG file_attr, ULONG reparse_tag) noexcept
	{
		if ((file_attr & FILE_ATTRIBUTE_REPARSE_POINT) && !reparse_tag)
		{
			auto buff = make_info_buffer(sizeof(reparse_data_buffer) + buff_size);
			if (buff.has_error()) [[unlikely]]
				return buff.error();

			auto reparse_data = reinterpret_cast<reparse_data_buffer *>(buff->get());
			std::memset(reparse_data, 0, sizeof(reparse_data_buffer));
			DWORD written = 0;

			if (!::DeviceIoControl(hnd, FSCTL_GET_REPARSE_POINT, nullptr, 0, reparse_data, sizeof(reparse_data_buffer) + buff_size, &written, nullptr)) [[unlikely]]
				return dos_error_code(::GetLastError());
			else
				reparse_tag = reparse_data->reparse_tag;
		}
		return attr_to_type(file_attr, reparse_tag);
	}
	inline static result<stat::query> query_stat_info(const ntapi &ntapi, wchar_t *buff, file_fs_sector_size_information &sector_info, stat &st, void *hnd, stat::query q) noexcept
	{
		auto stat_info = reinterpret_cast<file_stat_information *>(buff);
		std::memset(stat_info, 0, sizeof(file_stat_information));

		auto iosb = io_status_block();
		auto done = stat::query::none;

		auto status = ntapi.NtQueryInformationFile(hnd, &iosb, stat_info, sizeof(wchar_t) * buff_size, FileStatInformation);
		if (status == STATUS_PENDING)
			status = ntapi.wait_io(hnd, &iosb);
		if (is_status_failure(status))
			return status_error_code(status);

		if (bool(q & stat::query::ino))
		{
			st.ino = stat_info->file_id.QuadPart;
			done |= stat::query::ino;
		}
		if (bool(q & stat::query::type))
		{
			if (auto res = query_file_type(ntapi, hnd, stat_info->attributes, stat_info->reparse_tag); res.has_error()) [[unlikely]]
				return res.error();
			else
				st.type = *res;
			done |= stat::query::type;
		}
		if (bool(q & stat::query::nlink))
		{
			st.nlink = static_cast<std::int16_t>(stat_info->nlink);
			done |= stat::query::nlink;
		}
		if (bool(q & stat::query::atime))
		{
			st.atime = filetime_to_tp(stat_info->atime);
			done |= stat::query::atime;
		}
		if (bool(q & stat::query::mtime))
		{
			st.mtime = filetime_to_tp(stat_info->mtime);
			done |= stat::query::mtime;
		}
		if (bool(q & stat::query::btime))
		{
			st.btime = filetime_to_tp(stat_info->btime);
			done |= stat::query::btime;
		}
		if (bool(q & stat::query::ctime))
		{
			st.ctime = filetime_to_tp(stat_info->ctime);
			done |= stat::query::ctime;
		}
		if (bool(q & stat::query::size))
		{
			st.size = stat_info->endpos.QuadPart;
			done |= stat::query::size;
		}
		if (bool(q & stat::query::alloc))
		{
			st.alloc = stat_info->allocation.QuadPart;
			done |= stat::query::alloc;
		}
		if (bool(q & stat::query::blocks))
		{
			st.blocks = stat_info->allocation.QuadPart / sector_info.physical_bytes_perf;
			done |= stat::query::blocks;
		}
		if (bool(q & stat::query::blksize))
		{
			st.blksize = std::uint16_t(sector_info.physical_bytes_perf);
			done |= stat::query::blksize;
		}
		if (bool(q & stat::query::sparse))
		{
			st.sparse = stat_info->attributes & FILE_ATTRIBUTE_SPARSE_FILE;
			done |= stat::query::sparse;
		}
		if (bool(q & stat::query::compressed))
		{
			st.compressed = stat_info->attributes & FILE_ATTRIBUTE_COMPRESSED;
			done |= stat::query::compressed;
		}
		if (bool(q & stat::query::reparse_point))
		{
			st.reparse_point = stat_info->attributes & FILE_ATTRIBUTE_REPARSE_POINT;
			done |= stat::query::reparse_point;
		}

		return done;
	}
	inline static result<stat::query> query_all_info(const ntapi &ntapi, wchar_t *buff, file_fs_sector_size_information &sector_info, stat &st, void *hnd, stat::query q) noexcept
	{
		auto all_info = reinterpret_cast<file_all_information *>(buff);
		std::memset(all_info, 0, sizeof(file_all_information));

		auto iosb = io_status_block();
		auto done = stat::query::none;

		if (bool(q & basic_info_mask) + bool(q & standard_info_mask) + bool(q & internal_info_mask) >= 2)
		{
			auto status = ntapi.NtQueryInformationFile(hnd, &iosb, all_info, sizeof(wchar_t) * buff_size, FileAllInformation);
			if (status == STATUS_PENDING)
				status = ntapi.wait_io(hnd, &iosb);

			if (!is_status_failure(status))
				goto query_success;
			if (status != STATUS_INVALID_PARAMETER) [[unlikely]]
				return status_error_code(status);
		}
		if (bool(q & basic_info_mask))
		{
			iosb = io_status_block();
			auto status = ntapi.NtQueryInformationFile(hnd, &iosb, &all_info->basic_info, sizeof(all_info->basic_info), FileBasicInformation);
			if (status == STATUS_PENDING)
				status = ntapi.wait_io(hnd, &iosb);
			if (is_status_failure(status)) [[unlikely]]
				return status_error_code(status);
		}
		if (bool(q & standard_info_mask))
		{
			iosb = io_status_block();
			auto status = ntapi.NtQueryInformationFile(hnd, &iosb, &all_info->standard_info, sizeof(all_info->standard_info), FileStandardInformation);
			if (status == STATUS_PENDING)
				status = ntapi.wait_io(hnd, &iosb);
			if (is_status_failure(status)) [[unlikely]]
				return status_error_code(status);
		}
		if (bool(q & internal_info_mask))
		{
			iosb = io_status_block();
			auto status = ntapi.NtQueryInformationFile(hnd, &iosb, &all_info->internal_info, sizeof(all_info->internal_info), FileInternalInformation);
			if (status == STATUS_PENDING)
				status = ntapi.wait_io(hnd, &iosb);
			if (is_status_failure(status)) [[unlikely]]
				return status_error_code(status);
		}

	query_success:
		if (bool(q & stat::query::ino))
		{
			st.ino = all_info->internal_info.file_id.QuadPart;
			done |= stat::query::ino;
		}
		if (bool(q & stat::query::type))
		{
			if (auto res = query_file_type(ntapi, hnd, all_info->basic_info.attributes, all_info->ea_info.reparse_tag); res.has_error()) [[unlikely]]
				return res.error();
			else
				st.type = *res;
			done |= stat::query::type;
		}
		if (bool(q & stat::query::nlink))
		{
			st.nlink = static_cast<std::int16_t>(all_info->standard_info.nlink);
			done |= stat::query::nlink;
		}
		if (bool(q & stat::query::atime))
		{
			st.atime = filetime_to_tp(all_info->basic_info.atime);
			done |= stat::query::atime;
		}
		if (bool(q & stat::query::mtime))
		{
			st.mtime = filetime_to_tp(all_info->basic_info.mtime);
			done |= stat::query::mtime;
		}
		if (bool(q & stat::query::btime))
		{
			st.btime = filetime_to_tp(all_info->basic_info.btime);
			done |= stat::query::btime;
		}
		if (bool(q & stat::query::ctime))
		{
			st.ctime = filetime_to_tp(all_info->basic_info.ctime);
			done |= stat::query::ctime;
		}
		if (bool(q & stat::query::size))
		{
			st.size = all_info->standard_info.endpos.QuadPart;
			done |= stat::query::size;
		}
		if (bool(q & stat::query::alloc))
		{
			st.alloc = all_info->standard_info.allocation.QuadPart;
			done |= stat::query::alloc;
		}
		if (bool(q & stat::query::blocks))
		{
			st.blocks = all_info->standard_info.allocation.QuadPart / sector_info.physical_bytes_perf;
			done |= stat::query::blocks;
		}
		if (bool(q & stat::query::blksize))
		{
			st.blksize = std::uint16_t(sector_info.physical_bytes_perf);
			done |= stat::query::blksize;
		}
		if (bool(q & stat::query::sparse))
		{
			st.sparse = all_info->basic_info.attributes & FILE_ATTRIBUTE_SPARSE_FILE;
			done |= stat::query::sparse;
		}
		if (bool(q & stat::query::compressed))
		{
			st.compressed = all_info->basic_info.attributes & FILE_ATTRIBUTE_COMPRESSED;
			done |= stat::query::compressed;
		}
		if (bool(q & stat::query::reparse_point))
		{
			st.reparse_point = all_info->basic_info.attributes & FILE_ATTRIBUTE_REPARSE_POINT;
			done |= stat::query::reparse_point;
		}

		return done;
	}

	result<stat::query> do_get_stat(stat &st, path_view path, stat::query q, bool nofollow) noexcept
	{
		auto done = stat::query::none;
		if (q == done) return done;

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto basic_info = file_basic_information();
		auto obj_attrib = object_attributes();
		auto iosb = io_status_block();
		bool is_dir = false;

		auto rpath = path.render_null_terminated();
		auto upath = unicode_string();
		upath.max = (upath.size = USHORT(rpath.size() * sizeof(wchar_t))) + sizeof(wchar_t);
		upath.buff = const_cast<wchar_t *>(rpath.data());

		auto guard = ntapi->dos_path_to_nt_path(upath, false);
		if (guard.has_error()) [[unlikely]]
			return guard.error();

		obj_attrib.length = sizeof(object_attributes);
#if 0 /* FIXME: Should non-handle APIs use case-insensitive paths? */
		obj_attrib.attr = 0x40; /*OBJ_CASE_INSENSITIVE*/
#endif
		obj_attrib.name = &upath;

		/* If possible, try to get all requested data using `NtQueryAttributesFile` to avoid opening a handle altogether. */
		if (auto status = ntapi->NtQueryAttributesFile(&obj_attrib, &basic_info); is_status_failure(status)) [[unlikely]]
		{
			/* NtQueryAttributesFile may fail in case the path is a DOS device name. */
			if (!ntapi->RtlIsDosDeviceName_Ustr(&upath))
				return status_error_code(status);
		}

		/* Data is only useful if the target is not a symlink or we don't want to follow symlinks. */
		is_dir = basic_info.attributes & FILE_ATTRIBUTE_DIRECTORY;
		if (nofollow || !(basic_info.attributes & FILE_ATTRIBUTE_REPARSE_POINT))
		{
			if (bool(q & stat::query::atime))
			{
				st.atime = filetime_to_tp(basic_info.atime);
				done |= stat::query::atime;
			}
			if (bool(q & stat::query::mtime))
			{
				st.mtime = filetime_to_tp(basic_info.mtime);
				done |= stat::query::mtime;
			}
			if (bool(q & stat::query::ctime))
			{
				st.ctime = filetime_to_tp(basic_info.ctime);
				done |= stat::query::ctime;
			}
			if (bool(q & stat::query::btime))
			{
				st.btime = filetime_to_tp(basic_info.btime);
				done |= stat::query::btime;
			}
			if (bool(q & stat::query::reparse_point))
			{
				st.reparse_point = basic_info.attributes & FILE_ATTRIBUTE_REPARSE_POINT;
				done |= stat::query::reparse_point;
			}
			if (bool(q & stat::query::type) && is_dir)
			{
				st.type = file_type::directory;
				done |= stat::query::type;
			}
			if (bool(q & stat::query::perm))
			{
				st.perm = (basic_info.attributes & FILE_ATTRIBUTE_READONLY) ? (file_perm::all & ~file_perm::write) : file_perm::all;
				done |= stat::query::perm;
			}

			if ((q ^= done) == stat::query::none)
				return done;
		}

		/* If we still need more data then do the slow thing and open the handle. */
		const auto flags = 0x20 | 0x4000 /*FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT*/ | (is_dir ? 1 /*FILE_DIRECTORY_FILE*/ : 0) | (nofollow ? 0x20'0000 /*FILE_OPEN_REPARSE_POINT*/ : 0);
		const auto share = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;

		auto handle = INVALID_HANDLE_VALUE;
		auto status = ntapi->NtCreateFile(&handle, SYNCHRONIZE | FILE_READ_ATTRIBUTES, &obj_attrib, &iosb, nullptr, 0, share, file_open, flags, nullptr, 0);
		if (status == STATUS_PENDING) [[unlikely]]
			status = ntapi->wait_io(handle, &iosb);
		if (iosb.info == 5 /*FILE_DOES_NOT_EXIST*/) [[unlikely]]
			return std::make_error_code(std::errc::no_such_file_or_directory);
		else if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);

		if (auto res = get_stat(st, basic_handle(handle), q); res.has_value()) [[likely]]
			return *res | done;
		else
			return res;
	}

	result<stat::query> basic_handle::do_get_stat(stat &st, stat::query q) const noexcept
	{
		auto done = stat::query::none;
		if (q == done) return done;

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto buff = make_info_buffer(buff_size);
		if (buff.has_error()) [[unlikely]]
			return buff.error();

		auto sector_info = file_fs_sector_size_information();
		if (bool(q & (stat::query::blksize | stat::query::blocks)))
		{
			auto iosb = io_status_block();
			auto status = ntapi->NtQueryVolumeInformationFile(_hnd, &iosb, &sector_info, sizeof(sector_info), FileFsSectorSizeInformation);
			if (status == STATUS_PENDING)
				status = ntapi->wait_io(_hnd, &iosb);
			if (is_status_failure(status)) [[unlikely]]
				return status_error_code(status);
		}

		/* Failure could mean we are not on Win10 1709 or above and FileStatInformation may not be available. */
		if (auto res = query_stat_info(*ntapi, buff->get(), sector_info, st, _hnd, q); res.has_value())
			done = *res;
		else if (res = query_all_info(*ntapi, buff->get(), sector_info, st, _hnd, q); res.has_value())
			done = *res;
		else
			return res;

		/* Try to get device & file IDs from the filesystem with better uniqueness than FileStatInformation or FileAllInformation.
		 * This is only possible if FileIdInformation or FileObjectIdInformation is available. */
		if (bool(q & (stat::query::dev | stat::query::ino)))
		{
			auto id_info = reinterpret_cast<file_id_information *>(buff->get());
			std::memset(id_info, 0, sizeof(file_id_information));
			auto iosb = io_status_block();

			auto status = ntapi->NtQueryInformationFile(_hnd, &iosb, &id_info, sizeof(wchar_t) * buff_size, FileIdInformation);
			if (status == STATUS_PENDING)
				status = ntapi->wait_io(_hnd, &iosb);

			/* Failure might mean we do not have the FileIdInformation which is Win10 and above. */
			if (!is_status_failure(status))
			{
				union { std::uint8_t bytes[8]; std::uint64_t ino = 0; };
				st.dev = id_info->volume_id;
				done |= stat::query::dev;
				q &= ~stat::query::dev;

				/* XOR upper & lower words of 128 bit file_id. This should provide sufficient uniqueness. */
				for (std::size_t n = 0; n < 16; n++)
					if (id_info->file_id[n] != 0)
					{
						bytes[n & 7] ^= id_info->file_id[n];
						done |= stat::query::ino;
						q &= ~stat::query::ino;
					}
				if (!bool(q & stat::query::ino))
					st.ino = ino;
			}
		}
		if (bool(q & stat::query::dev))
		{
			if (const auto len = ::GetFinalPathNameByHandleW(_hnd, buff->get(), buff_size, VOLUME_NAME_NT); !len || len >= buff_size) [[unlikely]]
				return dos_error_code(::GetLastError());
			else
				buff->get()[len] = 0;

			const bool is_hdd = !std::memcmp(buff->get(), L"\\Device\\HarddiskVolume", 44);
			const bool is_unc = !std::memcmp(buff->get(), L"\\Device\\Mup", 22);
			if (!is_hdd && !is_unc) [[unlikely]]
				return std::make_error_code(std::errc::invalid_argument);
			if (is_hdd)
			{
				st.dev = _wtoi(buff->get() + 22);
				done |= stat::query::dev;
			}
		}
		if (bool(q & stat::query::ino))
		{
			auto objid_info = reinterpret_cast<file_objectid_information *>(buff->get());
			auto iosb = io_status_block();

			auto status = ntapi->NtQueryInformationFile(_hnd, &iosb, &objid_info, buff_size * sizeof(wchar_t), FileObjectIdInformation);
			if (status == STATUS_PENDING)
				status = ntapi->wait_io(_hnd, &iosb);
			if (!is_status_failure(status))
			{
				st.ino = objid_info->file_ref;
				done |= stat::query::ino;
			}
		}

		return done;
	}
	result<stat::query> basic_handle::do_set_stat(const stat &st, stat::query q) noexcept
	{
		/* perm, uid & gid is POSIX-only. */
		if ((q &= (stat::query::atime | stat::query::mtime | stat::query::btime)) == stat::query::none)
			return q;

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto info = file_basic_information();
		auto iosb = io_status_block();

		auto status = ntapi->NtQueryInformationFile(_hnd, &iosb, &info, sizeof(info), FileBasicInformation);
		if (status == STATUS_PENDING) [[unlikely]]
			status = ntapi->wait_io(_hnd, &iosb);
		if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);

		/* Set requested fields or keep them as change_time. */
		info.btime = bool(q & stat::query::btime) ? tp_to_filetime(st.btime) : info.ctime;
		info.atime = bool(q & stat::query::atime) ? tp_to_filetime(st.atime) : info.ctime;
		info.mtime = bool(q & stat::query::mtime) ? tp_to_filetime(st.mtime) : info.ctime;
		info.ctime = {};
		iosb.status = -1;

		status = ntapi->NtSetInformationFile(_hnd, &iosb, &info, sizeof(info), FileBasicInformation);
		if (status == STATUS_PENDING) [[unlikely]]
			status = ntapi->wait_io(_hnd, &iosb);
		if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return q;
	}
}
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

	constexpr auto buff_align = std::align_val_t(8);
	constexpr auto buff_size = std::size_t(32769);

	inline static result<std::unique_ptr<wchar_t[]>> make_buffer(std::size_t size) noexcept
	{
		try { return std::unique_ptr<wchar_t[]>(new(buff_align) wchar_t[buff_size]); }
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
	}

	inline static result<file_type> query_file_type(const ntapi &ntapi, const basic_handle &hnd, ULONG file_attr, ULONG reparse_tag) noexcept
	{
		if ((file_attr & FILE_ATTRIBUTE_REPARSE_POINT) && !reparse_tag)
		{
			auto buff = make_buffer(sizeof(reparse_data_buffer) + buff_size);
			if (buff.has_error()) [[unlikely]]
				return buff.error();

			auto reparse_data = reinterpret_cast<reparse_data_buffer *>(buff->get());
			std::memset(reparse_data, 0, sizeof(reparse_data_buffer));
			DWORD written = 0;

			if (!::DeviceIoControl(hnd.native_handle(), FSCTL_GET_REPARSE_POINT, nullptr, 0, reparse_data, sizeof(reparse_data_buffer) + buff_size, &written, nullptr)) [[unlikely]]
				return dos_error_code(::GetLastError());
			else
				reparse_tag = reparse_data->reparse_tag;
		}
		return attr_to_type(file_attr, reparse_tag);
	}
	inline static result<stat::query> query_stat_info(const ntapi &ntapi, wchar_t *buff, file_fs_sector_size_information &sector_info, stat &st, const basic_handle &hnd, stat::query q) noexcept
	{
		auto stat_info = reinterpret_cast<file_stat_information *>(buff);
		std::memset(stat_info, 0, sizeof(file_stat_information));

		auto iosb = io_status_block();
		auto done = stat::query::none;

		auto status = ntapi.NtQueryInformationFile(hnd.native_handle(), &iosb, stat_info, sizeof(wchar_t) * buff_size, FileStatInformation);
		if (status == STATUS_PENDING)
			status = ntapi.wait_io(hnd.native_handle(), &iosb);
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
	}
	inline static result<stat::query> query_all_info(const ntapi &ntapi, wchar_t *buff, file_fs_sector_size_information &sector_info, stat &st, const basic_handle &hnd, stat::query q) noexcept
	{
		auto all_info = reinterpret_cast<file_all_information *>(buff);
		std::memset(all_info, 0, sizeof(file_all_information));

		auto iosb = io_status_block();
		auto done = stat::query::none;

		if (bool(q & basic_info_mask) + bool(q & standard_info_mask) + bool(q & internal_info_mask) >= 2)
		{
			auto status = ntapi.NtQueryInformationFile(hnd.native_handle(), &iosb, all_info, sizeof(wchar_t) * buff_size, FileAllInformation);
			if (status == STATUS_PENDING)
				status = ntapi.wait_io(hnd.native_handle(), &iosb);

			if (!is_status_failure(status))
				goto query_success;
			if (status != STATUS_INVALID_PARAMETER) [[unlikely]]
				return status_error_code(status);
		}
		if (bool(q & basic_info_mask))
		{
			iosb = io_status_block();
			auto status = ntapi.NtQueryInformationFile(hnd.native_handle(), &iosb, &all_info->basic_info, sizeof(all_info->basic_info), FileBasicInformation);
			if (status == STATUS_PENDING)
				status = ntapi.wait_io(hnd.native_handle(), &iosb);
			if (is_status_failure(status)) [[unlikely]]
				return status_error_code(status);
		}
		if (bool(q & standard_info_mask))
		{
			iosb = io_status_block();
			auto status = ntapi.NtQueryInformationFile(hnd.native_handle(), &iosb, &all_info->standard_info, sizeof(all_info->standard_info), FileStandardInformation);
			if (status == STATUS_PENDING)
				status = ntapi.wait_io(hnd.native_handle(), &iosb);
			if (is_status_failure(status)) [[unlikely]]
				return status_error_code(status);
		}
		if (bool(q & internal_info_mask))
		{
			iosb = io_status_block();
			auto status = ntapi.NtQueryInformationFile(hnd.native_handle(), &iosb, &all_info->internal_info, sizeof(all_info->internal_info), FileInternalInformation);
			if (status == STATUS_PENDING)
				status = ntapi.wait_io(hnd.native_handle(), &iosb);
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

	result<stat::query> do_get_stat(stat &st, const basic_handle &hnd, stat::query q) noexcept
	{
		auto done = stat::query::none;
		if (q == done) return done;

		auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto buff = make_buffer(buff_size);
		if (buff.has_error()) [[unlikely]]
			return buff.error();

		auto sector_info = file_fs_sector_size_information();
		if (bool(q & (stat::query::blksize | stat::query::blocks)))
		{
			auto iosb = io_status_block();
			auto status = ntapi->NtQueryVolumeInformationFile(hnd.native_handle(), &iosb, &sector_info, sizeof(sector_info), FileFsSectorSizeInformation);
			if (status == STATUS_PENDING)
				status = ntapi->wait_io(hnd.native_handle(), &iosb);
			if (is_status_failure(status)) [[unlikely]]
				return status_error_code(status);
		}

		/* Failure could mean we are not on Win10 1709 or above and FileStatInformation may not be available. */
		if (auto res = query_stat_info(*ntapi, buff->get(), sector_info, st, hnd, q); res.has_value())
			done = *res;
		else if (res = query_all_info(*ntapi, buff->get(), sector_info, st, hnd, q); res.has_value())
			done = *res;
		else
			return res;

		/* Try to get unique device & file IDs from the filesystem. */
		if (bool(q & (stat::query::dev | stat::query::ino)))
		{
			auto id_info = reinterpret_cast<file_id_information *>(buff->get());
			std::memset(id_info, 0, sizeof(file_id_information));
			auto iosb = io_status_block();

			auto status = ntapi->NtQueryInformationFile(hnd.native_handle(), &iosb, &id_info, sizeof(wchar_t) * buff_size, FileIdInformation);
			if (status == STATUS_PENDING)
				status = ntapi->wait_io(hnd.native_handle(), &iosb);

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
		/* If FileIdInformation failed, try to get device number from volume ID. */
		if (bool(q & stat::query::dev))
		{
			if (const auto len = ::GetFinalPathNameByHandleW(hnd.native_handle(), buff->get(), buff_size, VOLUME_NAME_NT); !len || len >= buff_size) [[unlikely]]
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
		/* If FileIdInformation failed, try to use Win8 FileObjectIdInformation. */
		if (bool(q & stat::query::ino))
		{
			auto objid_info = reinterpret_cast<file_objectid_information *>(buff->get());
			auto iosb = io_status_block();

			auto status = ntapi->NtQueryInformationFile(hnd.native_handle(), &iosb, &objid_info, buff_size * sizeof(wchar_t), FileObjectIdInformation);
			if (status == STATUS_PENDING)
				status = ntapi->wait_io(hnd.native_handle(), &iosb);
			if (!is_status_failure(status))
			{
				st.ino = objid_info->file_ref;
				done |= stat::query::ino;
			}
		}

		return done;
	}
	result<stat::query> do_set_stat(const stat &st, basic_handle &hnd, stat::query q) noexcept
	{
		/* perm, uid & gid is POSIX-only. */
		if ((q &= (stat::query::atime | stat::query::mtime | stat::query::btime)) == stat::query::none)
			return q;

		auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto info = file_basic_information();
		auto iosb = io_status_block();

		auto status = ntapi->NtQueryInformationFile(hnd.native_handle(), &iosb, &info, sizeof(info), FileBasicInformation);
		if (status == STATUS_PENDING) [[unlikely]]
			status = ntapi->wait_io(hnd.native_handle(), &iosb);
		if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);

		/* Set requested fields or keep them as change_time. */
		info.btime = bool(q & stat::query::btime) ? tp_to_filetime(st.btime) : info.ctime;
		info.atime = bool(q & stat::query::atime) ? tp_to_filetime(st.atime) : info.ctime;
		info.mtime = bool(q & stat::query::mtime) ? tp_to_filetime(st.mtime) : info.ctime;
		info.ctime = {};
		iosb.status = -1;

		status = ntapi->NtSetInformationFile(hnd.native_handle(), &iosb, &info, sizeof(info), FileBasicInformation);
		if (status == STATUS_PENDING) [[unlikely]]
			status = ntapi->wait_io(hnd.native_handle(), &iosb);
		if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return q;
	}

	fs_result<stat::query> do_get_stat(stat &st, path_view path, stat::query q, bool nofollow) noexcept
	{
		auto done = stat::query::none;
		if (q == done) return done;

		const auto rpath = path.render_null_terminated();
		/* If possible, try to get relevant data using `GetFileAttributesExW` or `FindFirstFileW` to avoid opening the file altogether. */
		if (bool(q & (stat::query::perm | stat::query::atime | stat::query::mtime | stat::query::btime | stat::query::size | stat::query::reparse_point)) && (nofollow || bool(q & stat::query::reparse_point)))
		{
			WIN32_FILE_ATTRIBUTE_DATA data;
			if (!::GetFileAttributesExW(rpath.c_str(), GetFileExInfoStandard, &data))
			{
				if (const auto err = ::GetLastError(); err != ERROR_SHARING_VIOLATION)
					return fs_status_code(dos_error_code(err), rpath.c_str());

				WIN32_FIND_DATAW find_data;
				if (const auto hnd = ::FindFirstFileW(rpath.c_str(), &find_data); hnd == INVALID_HANDLE_VALUE) [[unlikely]]
					return fs_status_code(dos_error_code(::GetLastError()), rpath.c_str());
				else
					::FindClose(hnd);

				data.dwFileAttributes = find_data.dwFileAttributes;
				data.ftLastWriteTime = find_data.ftLastAccessTime;
				data.ftLastWriteTime = find_data.ftLastWriteTime;
				data.ftLastWriteTime = find_data.ftCreationTime;
				data.nFileSizeHigh = find_data.nFileSizeHigh;
				data.nFileSizeLow = find_data.nFileSizeLow;
			}

			/* Data is only useful if the target is not a symlink or we don't want to follow symlinks. */
			if (nofollow || !(data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
			{
				if (bool(q & stat::query::perm))
				{
					st.perm = (data.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? (file_perm::all & ~file_perm::write) : file_perm::all;
					done |= stat::query::perm;
				}
				if (bool(q & stat::query::atime))
				{
					st.atime = filetime_to_tp(data.ftLastAccessTime);
					done |= stat::query::atime;
				}
				if (bool(q & stat::query::mtime))
				{
					st.mtime = filetime_to_tp(data.ftLastWriteTime);
					done |= stat::query::mtime;
				}
				if (bool(q & stat::query::btime))
				{
					st.mtime = filetime_to_tp(data.ftCreationTime);
					done |= stat::query::btime;
				}
				if (bool(q & stat::query::size))
				{
					union { struct { DWORD _hw; DWORD _lw; }; std::uint64_t _size; };
					_hw = data.nFileSizeHigh;
					_lw = data.nFileSizeLow;
					st.size = _size;
					done |= stat::query::atime;
				}
				if (bool(q & stat::query::reparse_point))
				{
					st.reparse_point = data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT;
					done |= stat::query::reparse_point;
				}
			}
			if ((q ^= done) == stat::query::none)
				return done;
		}

		/* If we still need more data then do the slow thing and open the handle. */
		const auto share = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;
		const auto flags = FILE_FLAG_BACKUP_SEMANTICS | (nofollow ? FILE_FLAG_OPEN_REPARSE_POINT : 0);
		if (const auto handle = basic_handle(::CreateFileW(rpath.c_str(), FILE_READ_ATTRIBUTES, share, nullptr, OPEN_EXISTING, flags, nullptr)); !handle.is_open()) [[unlikely]]
			return fs_status_code(dos_error_code(::GetLastError()), rpath.c_str());
		else if (auto res = do_get_stat(st, handle, q); res.has_error()) [[unlikely]]
			return fs_status_code(res.error(), rpath.c_str());
		else
			return *res | done;
	}
}
/*
 * Created by switch_blade on 2023-08-29.
 */

#include "../handle_stat.hpp"
#include "path_discovery.hpp"

namespace rod::_handle
{
	using namespace _win32;

	constexpr auto basic_info_mask = stat::query::type | stat::query::atime | stat::query::mtime | stat::query::ctime | stat::query::btime | stat::query::is_sparse | stat::query::is_compressed | stat::query::is_reparse_point;
	constexpr auto standard_info_mask = stat::query::size | stat::query::alloc | stat::query::blocks | stat::query::nlink;
	constexpr auto internal_info_mask = stat::query::ino;

	constexpr std::size_t buff_size = 32768;

	inline static auto query_file_type(const ntapi &ntapi, void *hnd, ULONG file_attr, ULONG reparse_tag) noexcept -> result<file_type>
	{
		if ((file_attr & FILE_ATTRIBUTE_REPARSE_POINT) && !reparse_tag)
		{
			auto buff = ROD_MAKE_BUFFER(std::byte, buff_size * sizeof(wchar_t) + sizeof(reparse_data_buffer));
			if (buff.has_error()) [[unlikely]]
				return buff.error();

			auto reparse_data = reinterpret_cast<reparse_data_buffer *>(buff->get());
			std::memset(reparse_data, 0, sizeof(reparse_data_buffer));
			DWORD written = 0;

			if (!::DeviceIoControl(hnd, FSCTL_GET_REPARSE_POINT, nullptr, 0, reparse_data, sizeof(reparse_data_buffer) + buff_size * sizeof(wchar_t), &written, nullptr)) [[unlikely]]
				return dos_error_code(::GetLastError());
			else
				reparse_tag = reparse_data->reparse_tag;
		}
		return attr_to_type(file_attr, reparse_tag);
	}
	inline static auto query_stat_info(const ntapi &ntapi, wchar_t *buff, file_fs_sector_size_information &sector_info, stat &st, void *hnd, stat::query q) noexcept -> result<stat::query>
	{
		auto stat_info = reinterpret_cast<file_stat_information *>(buff);
		std::memset(stat_info, 0, sizeof(file_stat_information));

		auto iosb = io_status_block();
		auto done = stat::query::none;

		auto status = ntapi.get_file_info(hnd, &iosb, stat_info, sizeof(wchar_t) * buff_size, FileStatInformation);
		if (is_status_failure(status))
			return status_error_code(status);

		if (bool(q & stat::query::ino))
		{
			st.ino = stat_info->file_id;
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
			st.size = stat_info->endpos;
			done |= stat::query::size;
		}
		if (bool(q & stat::query::alloc))
		{
			st.alloc = stat_info->allocation;
			done |= stat::query::alloc;
		}
		if (bool(q & stat::query::blocks))
		{
			st.blocks = stat_info->allocation / sector_info.physical_bytes_perf;
			done |= stat::query::blocks;
		}
		if (bool(q & stat::query::blksize))
		{
			st.blksize = std::uint16_t(sector_info.physical_bytes_perf);
			done |= stat::query::blksize;
		}
		if (bool(q & stat::query::is_sparse))
		{
			st.is_sparse = stat_info->attributes & FILE_ATTRIBUTE_SPARSE_FILE;
			done |= stat::query::is_sparse;
		}
		if (bool(q & stat::query::is_compressed))
		{
			st.is_compressed = stat_info->attributes & FILE_ATTRIBUTE_COMPRESSED;
			done |= stat::query::is_compressed;
		}
		if (bool(q & stat::query::is_reparse_point))
		{
			st.is_reparse_point = stat_info->attributes & FILE_ATTRIBUTE_REPARSE_POINT;
			done |= stat::query::is_reparse_point;
		}

		return done;
	}
	inline static auto query_all_info(const ntapi &ntapi, wchar_t *buff, file_fs_sector_size_information &sector_info, stat &st, void *hnd, stat::query q) noexcept -> result<stat::query>
	{
		auto all_info = reinterpret_cast<file_all_information *>(buff);
		std::memset(all_info, 0, sizeof(file_all_information));

		auto iosb = io_status_block();
		auto done = stat::query::none;

		if (bool(q & basic_info_mask) + bool(q & standard_info_mask) + bool(q & internal_info_mask) >= 2)
		{
			auto status = ntapi.get_file_info(hnd, &iosb, all_info, sizeof(wchar_t) * buff_size, FileAllInformation);
			if (!is_status_failure(status))
				goto query_success;
			if (status != STATUS_INVALID_PARAMETER) [[unlikely]]
				return status_error_code(status);
		}
		if (bool(q & basic_info_mask))
		{
			auto status = ntapi.get_file_info(hnd, &iosb, &all_info->basic_info, FileBasicInformation);
			if (is_status_failure(status)) [[unlikely]]
				return status_error_code(status);
		}
		if (bool(q & standard_info_mask))
		{
			auto status = ntapi.get_file_info(hnd, &iosb, &all_info->standard_info, FileStandardInformation);
			if (is_status_failure(status)) [[unlikely]]
				return status_error_code(status);
		}
		if (bool(q & internal_info_mask))
		{
			auto status = ntapi.get_file_info(hnd, &iosb, &all_info->internal_info, FileInternalInformation);
			if (is_status_failure(status)) [[unlikely]]
				return status_error_code(status);
		}

	query_success:
		if (bool(q & stat::query::ino))
		{
			st.ino = all_info->internal_info.file_id;
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
			st.size = all_info->standard_info.endpos;
			done |= stat::query::size;
		}
		if (bool(q & stat::query::alloc))
		{
			st.alloc = all_info->standard_info.allocation;
			done |= stat::query::alloc;
		}
		if (bool(q & stat::query::blocks))
		{
			st.blocks = all_info->standard_info.allocation / sector_info.physical_bytes_perf;
			done |= stat::query::blocks;
		}
		if (bool(q & stat::query::blksize))
		{
			st.blksize = std::uint16_t(sector_info.physical_bytes_perf);
			done |= stat::query::blksize;
		}
		if (bool(q & stat::query::is_sparse))
		{
			st.is_sparse = all_info->basic_info.attributes & FILE_ATTRIBUTE_SPARSE_FILE;
			done |= stat::query::is_sparse;
		}
		if (bool(q & stat::query::is_compressed))
		{
			st.is_compressed = all_info->basic_info.attributes & FILE_ATTRIBUTE_COMPRESSED;
			done |= stat::query::is_compressed;
		}
		if (bool(q & stat::query::is_reparse_point))
		{
			st.is_reparse_point = all_info->basic_info.attributes & FILE_ATTRIBUTE_REPARSE_POINT;
			done |= stat::query::is_reparse_point;
		}

		return done;
	}

	result<stat::query> do_get_stat(stat &st, const path_handle &base, path_view path, stat::query q, bool nofollow) noexcept
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

		auto rpath = render_as_ustring<true>(path);
		if (rpath.has_error()) [[unlikely]]
			return rpath.error();

		auto &upath = rpath->first;
		auto guard = ntapi->dos_path_to_nt_path(upath, base.is_open());
		if (guard.has_error()) [[unlikely]]
			return std::make_error_code(std::errc::no_such_file_or_directory);

		obj_attrib.root_dir = base.is_open() ? base.native_handle() : nullptr;
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
			if (bool(q & stat::query::is_reparse_point))
			{
				st.is_reparse_point = basic_info.attributes & FILE_ATTRIBUTE_REPARSE_POINT;
				done |= stat::query::is_reparse_point;
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
		const auto opts = 0x20 | 0x4000 /*FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT*/ | (is_dir ? 1 /*FILE_DIRECTORY_FILE*/ : 0) | (nofollow ? 0x20'0000 /*FILE_OPEN_REPARSE_POINT*/ : 0);
		const auto share = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;

		auto hnd = ntapi->create_file(obj_attrib, &iosb, SYNCHRONIZE | FILE_READ_ATTRIBUTES, 0, share, file_open, opts);
		if (hnd.has_error()) [[unlikely]]
			return hnd.error();

		if (auto res = get_stat(st, basic_handle(*hnd), q); res.has_value()) [[likely]]
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

		auto buff = ROD_MAKE_BUFFER(wchar_t, buff_size * sizeof(wchar_t));
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

		/* Try to get device ID using Win10 FileIdInformation. */
		if (bool(q & (stat::query::dev/* | stat::query::ino*/)))
		{
			auto id_info = reinterpret_cast<file_id_information *>(buff->get());
			std::memset(id_info, 0, sizeof(file_id_information));
			auto iosb = io_status_block();

			auto status = ntapi->get_file_info(_hnd, &iosb, &id_info, sizeof(wchar_t) * buff_size, FileIdInformation);
			if (!is_status_failure(status)) /* May fail on older windows. */
			{
				st.dev = id_info->volume_id;
				done |= stat::query::dev;
				q &= ~stat::query::dev;

#if 0 /* Don't use the 128bit ID to stay consistent with other inode sources (ex. NtQueryDirectoryFile). */
				union { std::uint8_t bytes[8]; std::uint64_t ino = 0; };
				for (std::size_t n = 0; n < 16; n++)
					if (id_info->file_id[n] != 0)
					{
						bytes[n & 7] ^= id_info->file_id[n];
						done |= stat::query::ino;
						q &= ~stat::query::ino;
					}
				if (!bool(q & stat::query::ino))
					st.ino = ino;
#endif
			}
		}
		/* Re-try to get device ID from volume name. */
		if (bool(q & stat::query::dev))
		{
			if (const auto len = ::GetFinalPathNameByHandleW(_hnd, buff->get(), buff_size, VOLUME_NAME_NT); !len || len >= buff_size) [[unlikely]]
				return dos_error_code(::GetLastError());
			else
				buff->get()[len] = 0;

			const bool is_hdd = _wcsnicmp(buff->get(), LR"(\Device\HarddiskVolume)", 22) == 0;
			const bool is_unc = _wcsnicmp(buff->get(), LR"(\Device\Mup)", 11) == 0;
			if (!is_hdd && !is_unc) [[unlikely]]
				return std::make_error_code(std::errc::invalid_argument);
			if (is_hdd)
			{
				st.dev = _wtoi(buff->get() + 22);
				done |= stat::query::dev;
			}
		}
#if 0 /* Don't use the object or file reference IDs to stay consistent with other inode sources (ex. NtQueryDirectoryFile). */
		if (bool(q & stat::query::ino))
		{
			auto objid_info = reinterpret_cast<file_objectid_information *>(buff->get());
			auto iosb = io_status_block();

			auto status = ntapi->get_file_info(_hnd, &iosb, &objid_info, buff_size * sizeof(wchar_t), FileObjectIdInformation);
			if (!is_status_failure(status))
			{
				st.ino = objid_info->file_ref;
				done |= stat::query::ino;
			}
		}
#endif

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

		auto basic_info = file_basic_information();
		auto iosb = io_status_block();

		auto status = ntapi->get_file_info(_hnd, &iosb, &basic_info, FileBasicInformation);
		if (status == STATUS_PENDING) [[unlikely]]
			status = ntapi->wait_io(_hnd, &iosb);
		if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);

		/* Set requested fields or keep them as change_time. */
		basic_info.btime = bool(q & stat::query::btime) ? tp_to_filetime(st.btime) : basic_info.ctime;
		basic_info.atime = bool(q & stat::query::atime) ? tp_to_filetime(st.atime) : basic_info.ctime;
		basic_info.mtime = bool(q & stat::query::mtime) ? tp_to_filetime(st.mtime) : basic_info.ctime;
		basic_info.ctime = {};
		iosb.status = -1;

		status = ntapi->set_file_info(_hnd, &iosb, &basic_info, FileBasicInformation);
		if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return q;
	}

	result<fs_stat::query> basic_handle::do_get_fs_stat(fs_stat &st, fs_stat::query q) const noexcept
	{
		auto done = fs_stat::query::none;
		if (q == done) return done;

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto buff = ROD_MAKE_BUFFER(wchar_t, buff_size * sizeof(wchar_t));
		if (buff.has_error()) [[unlikely]]
			return buff.error();

		/* Query filesystem type & attributes. */
		if (bool(q & (fs_stat::query::flags | fs_stat::query::filename_max | fs_stat::query::fs_type)))
		{
			auto attr_info = reinterpret_cast<file_fs_attribute_information *>(buff->get());
			auto iosb = io_status_block();

			auto status = ntapi->NtQueryVolumeInformationFile(native_handle(), &iosb, attr_info, sizeof(file_fs_attribute_information), FileFsAttributeInformation);
			if (status == STATUS_PENDING)
				status = ntapi->wait_io(native_handle(), &iosb);
			if (is_status_failure(status)) [[unlikely]]
				return status_error_code(status);

			if (bool(q & (fs_stat::query::fs_type | fs_stat::query::flags)))
			{
				try
				{
					std::string type;
					type.resize(attr_info->name_len / sizeof(wchar_t));
					for (std::size_t i = 0; i < st.fs_type.size(); ++i)
						type[i] = static_cast<char>(attr_info->name[i]);

					if (bool(q & fs_stat::query::flags) || _detail::match_network_backed(type))
						st.flags |= fs_flags::network;
					if (bool(q & fs_stat::query::fs_type))
						st.fs_type = std::move(type);
				}
				catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
				catch (const std::system_error &e) { return e.code(); }
				done |= q & (fs_stat::query::fs_type | fs_stat::query::flags);
			}
			if (bool(q & fs_stat::query::filename_max))
			{
				st.filename_max = attr_info->max_component_size;
				done |= fs_stat::query::filename_max;
			}
			if (bool(q & fs_stat::query::flags))
			{
				if (attr_info->attributes & FILE_READ_ONLY_VOLUME)
					st.flags |= fs_flags::rdonly;
				if (attr_info->attributes & FILE_SUPPORTS_SPARSE_FILES)
					st.flags |= fs_flags::sparse_files;
				if (attr_info->attributes & FILE_FILE_COMPRESSION)
					st.flags |= fs_flags::file_compression;
				if (attr_info->attributes & FILE_VOLUME_IS_COMPRESSED)
					st.flags |= fs_flags::volume_compression;
				if (attr_info->attributes & FILE_NAMED_STREAMS)
					st.flags |= fs_flags::extended_attributes;
				if (attr_info->attributes & FILE_PERSISTENT_ACLS)
					st.flags |= fs_flags::access_control_list;
			}
		}
		/* Query filesystem path & name. */
		for (std::wstring hnd_path; bool(q & (fs_stat::query::fs_name | fs_stat::query::fs_path));)
		{
			try
			{
				/* Save original path of the handle to check if it exists in the target volume. */
				if (const auto len = ::GetFinalPathNameByHandleW(native_handle(), hnd_path.data(), DWORD(hnd_path.size()), FILE_NAME_OPENED | VOLUME_NAME_NONE); !len) [[unlikely]]
					return dos_error_code(::GetLastError());
				else if (len <= hnd_path.size())
					hnd_path.resize(len);
				else
				{
					hnd_path.resize(len);
					continue;
				}

				/* Query volume name. */
				if (bool(q & fs_stat::query::fs_name))
				{
					auto len = ::GetFinalPathNameByHandleW(native_handle(), buff->get(), buff_size, FILE_NAME_OPENED | VOLUME_NAME_NT);
					if (!len || len >= buff_size) [[unlikely]]
						return dos_error_code(::GetLastError());
					else
						buff->get()[len] = 0;

					/* Make sure path to the handle did not change. */
					if (std::memcmp(hnd_path.data(), buff->get() - hnd_path.size() + len, hnd_path.size()) != 0)
						continue;

					const bool is_hdd = _wcsnicmp(buff->get(), LR"(\Device\HarddiskVolume)", 22) == 0;
					const bool is_unc = _wcsnicmp(buff->get(), LR"(\Device\Mup)", 11) == 0;
					if (!is_hdd && !is_unc) [[unlikely]]
						return std::make_error_code(std::errc::invalid_argument);

					/* Replace \Device with \!!\Device */
					if (is_hdd || is_unc)
						len -= DWORD(hnd_path.size());
					if (is_unc)
					{
						len += 1;
						for (std::size_t seps = 2; seps; len += bool(seps))
						{
							if (buff->get()[len] == '\\')
								seps -= 1;
						}
					}

					st.fs_name.resize(len + 3);
					static_cast_copy(buff->get(), buff->get() + len, st.fs_name.data());
					std::memcpy(st.fs_name.data(), "\\!!", 3 * sizeof(char));
					done |= fs_stat::query::fs_name;
					q &= ~fs_stat::query::fs_name;
				}
				/* Query volume path. */
				if (bool(q & fs_stat::query::fs_path))
				{
					const auto len = ::GetFinalPathNameByHandleW(native_handle(), buff->get(), buff_size, FILE_NAME_OPENED | VOLUME_NAME_DOS);
					if (!len || len >= buff_size) [[unlikely]]
						return dos_error_code(::GetLastError());
					else
						buff->get()[len] = 0;

					/* Make sure path to the handle did not change. */
					if (std::memcmp(hnd_path.data(), buff->get() - hnd_path.size() + len, hnd_path.size()) != 0)
						continue;

					st.fs_path = std::wstring_view(buff->get(), std::size_t(len) - hnd_path.size());
					done |= fs_stat::query::fs_path;
					q &= ~fs_stat::query::fs_path;
				}
			}
			catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
			catch (const std::system_error &e) { return e.code(); }
		}
		/* Query filesystem ID. */
		if (bool(q & fs_stat::query::fs_id))
		{
			auto obj_info = reinterpret_cast<file_fs_objectid_information *>(buff->get());
			auto iosb = io_status_block();

			auto status = ntapi->NtQueryVolumeInformationFile(native_handle(), &iosb, obj_info, sizeof(file_fs_objectid_information), FileFsObjectIdInformation);
			if (status == STATUS_PENDING)
				status = ntapi->wait_io(native_handle(), &iosb);

			/* Some filesystems may not support filesystem IDs. */
			if (!is_status_failure(status))
			{
				std::memcpy(st.fs_id, obj_info->obj_id, sizeof(st.fs_id));
				done |= fs_stat::query::fs_id;
			}
		}

		/* Query filesystem block info. */
		if (bool(q & (fs_stat::query::blk_size | fs_stat::query::blk_count | fs_stat::query::blk_avail | fs_stat::query::blk_free)))
		{
			auto size_info = reinterpret_cast<file_fs_full_size_information *>(buff->get());
			auto iosb = io_status_block();

			auto status = ntapi->NtQueryVolumeInformationFile(native_handle(), &iosb, size_info, sizeof(file_fs_full_size_information), FileFsFullSizeInformation);
			if (status == STATUS_PENDING)
				status = ntapi->wait_io(native_handle(), &iosb);
			if (is_status_failure(status)) [[unlikely]]
				return status_error_code(status);

			if (bool(q & fs_stat::query::blk_size))
			{
				st.blk_size = size_info->blk_sectors * size_info->sector_size;
				done |= fs_stat::query::blk_size;
			}
			if (bool(q & fs_stat::query::blk_count))
			{
				st.blk_size = size_info->blk_count;
				done |= fs_stat::query::blk_count;
			}
			if (bool(q & fs_stat::query::blk_avail))
			{
				st.blk_size = size_info->blk_avail;
				done |= fs_stat::query::blk_avail;
			}
			if (bool(q & fs_stat::query::blk_free))
			{
				st.blk_size = size_info->blk_free;
				done |= fs_stat::query::blk_free;
			}
		}
		/* Query filesystem IO size. */
		if (bool(q & fs_stat::query::io_size))
		{
			auto size_info = reinterpret_cast<file_fs_sector_size_information *>(buff->get());
			auto iosb = io_status_block();

			auto status = ntapi->NtQueryVolumeInformationFile(native_handle(), &iosb, size_info, sizeof(file_fs_sector_size_information), FileFsSectorSizeInformation);
			if (status == STATUS_PENDING)
				status = ntapi->wait_io(native_handle(), &iosb);
			if (is_status_failure(status)) [[unlikely]]
				return status_error_code(status);

			st.io_size = size_info->physical_bytes_perf;
			done |= fs_stat::query::io_size;
		}

		return done;
	}
}
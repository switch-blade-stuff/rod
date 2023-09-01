/*
 * Created by switch_blade on 2023-08-31.
 */

#include "../path_util.hpp"

#include "../handle_stat.hpp"
#include "ntapi.hpp"

namespace rod
{
	using namespace _win32;

	inline static result<file_id_information> query_file_id_info(const ntapi &ntapi, path_view p) noexcept
	{
		const auto share = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;
		const auto flags = FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT;
		const auto rpath = p.render_null_terminated();
		constexpr std::size_t buff_size = 32769;

		/* FIXME: replace with a NtCreateFile with proper support for NT paths. */
		auto hnd = basic_handle(::CreateFileW(rpath.c_str(), FILE_READ_ATTRIBUTES, share, nullptr, OPEN_EXISTING, flags, nullptr));
		if (!hnd.is_open()) [[unlikely]]
			return dos_error_code(::GetLastError());

		auto id_info = file_id_information();
		if (auto iosb = io_status_block(); !is_status_failure(ntapi.NtQueryInformationFile(hnd.native_handle(), &iosb, &id_info, sizeof(id_info), FileIdInformation)))
			return id_info;

		/* Try to get volume ID from volume name. */
		auto volume_name = make_info_buffer(buff_size);
		if (volume_name.has_error()) [[unlikely]]
			return volume_name.error();

		if (const auto len = ::GetFinalPathNameByHandleW(hnd.native_handle(), volume_name->get(), buff_size, VOLUME_NAME_NT); !len) [[unlikely]]
			return dos_error_code(::GetLastError());
		else
			volume_name->get()[len] = 0;

		const bool is_hdd = !std::memcmp(volume_name->get(), L"\\Device\\HarddiskVolume", 44);
		const bool is_unc = !std::memcmp(volume_name->get(), L"\\Device\\Mup", 22);
		if (!is_hdd && !is_unc) [[unlikely]]
			return std::make_error_code(std::errc::invalid_argument);
		if (is_hdd)
			id_info.volume_id = _wtoi(volume_name->get() + 22);

		auto objid_info = file_objectid_information();
		if (auto iosb = io_status_block(); !is_status_failure(ntapi.NtQueryInformationFile(hnd.native_handle(), &iosb, &objid_info, sizeof(objid_info), FileObjectIdInformation)))
		{
			std::memcpy(id_info.file_id, &objid_info.file_ref, sizeof(LONGLONG));
			return id_info;
		}

		/* At this point, if we don't have a file ID, fallback the potentially non-unique FileInternalInformation. */
		auto internal_info = file_internal_information();
		auto iosb = io_status_block();
		auto status = ntapi.NtQueryInformationFile(hnd.native_handle(), &iosb, &internal_info, sizeof(internal_info), FileInternalInformation);
		if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);

		std::memcpy(id_info.file_id, &internal_info.file_id, sizeof(LARGE_INTEGER));
		return id_info;
	}

	result<bool> equivalent(path_view a, path_view b) noexcept
	{
		auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return result(in_place_error, ntapi.error());

		auto id_info_a = query_file_id_info(*ntapi, a);
		if (id_info_a.has_error()) [[unlikely]]
			return result(in_place_error, id_info_a.error());
		auto id_info_b = query_file_id_info(*ntapi, b);
		if (id_info_b.has_error()) [[unlikely]]
			return result(in_place_error, id_info_b.error());

		return !std::memcmp(&*id_info_a, &*id_info_b, sizeof(file_id_information));
	}
}

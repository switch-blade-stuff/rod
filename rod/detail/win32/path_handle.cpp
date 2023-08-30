/*
 * Created by switch_blade on 2023-08-09.
 */

#include "../path_handle.hpp"

#include <cstdlib>

#include "../path_discovery.hpp"
#include "ntapi.hpp"

namespace rod
{
	using namespace _win32;

	inline constexpr auto heapalloc_free = [](auto *p) { ::HeapFree(::GetProcessHeap(), 0, p); };
	template<typename T>
	using heapalloc_ptr = std::unique_ptr<T, decltype(heapalloc_free)>;

	fs_result<path_handle> path_handle::open(path_view path) noexcept { return open({}, path); }
	fs_result<path_handle> path_handle::open(const path_handle &base, path_view path) noexcept
	{
		constexpr auto flags = 0x20 /* FILE_SYNCHRONOUS_IO_NONALERT */ | 1 /* FILE_DIRECTORY_FILE */;
		constexpr auto share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
		constexpr auto access = SYNCHRONIZE;

		auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto rpath = path.render_null_terminated();
		heapalloc_ptr<wchar_t> uname_guard;

		object_attributes attrib = {};
		LARGE_INTEGER alloc_size = {};
		io_status_block iosb = {};
		unicode_string uname = {};

		attrib.root_dir = base.is_open() ? base.native_handle() : nullptr;
		attrib.length = sizeof(object_attributes);
		attrib.name = &uname;

		if (const auto sv = std::wstring_view(rpath.data(), rpath.size()); base.is_open() || sv.starts_with(L"\\!!\\") || sv.starts_with(L"\\??\\"))
		{
			uname.max = (uname.size = static_cast<USHORT>(rpath.size() * sizeof(wchar_t))) + sizeof(wchar_t);
			uname.buffer = const_cast<wchar_t *>(rpath.data());

			if (sv.starts_with(L"\\!!\\"))
			{
				uname.size -= 3 * sizeof(wchar_t);
				uname.max -= 3 * sizeof(wchar_t);
				uname.buffer += 3;
			}
		}
		else if (!ntapi->RtlDosPathNameToNtPathName_U(rpath.data(), &uname, nullptr, nullptr)) [[unlikely]]
			return fs_status_code(dos_error_code(ERROR_FILE_NOT_FOUND), rpath.as_span());
		else
			uname_guard.reset(uname.buffer);

		/* Wait for completion if NtCreateFile returns STATUS_PENDING. */
		auto handle = INVALID_HANDLE_VALUE;
		auto status = ntapi->NtCreateFile(&handle, access, &attrib, &iosb, &alloc_size, 0, share, file_create, flags, nullptr, 0);
		if (status == STATUS_PENDING) [[unlikely]]
			status = ntapi->wait_io(handle, &iosb);
		if (!is_status_failure(status)) [[likely]]
			return path_handle(handle);

		/* Report error with FULL path (base + leaf) */
		const auto code = status_error_code(status);
		const auto p = std::span(uname.buffer, uname.buffer + uname.size);
		return fs_status_code(code, base.is_open() ? *base.object_path() / p : p);
	}

	fs_result<directory_handle> directory_handle::open(path_view path) noexcept { return open({}, path); }
	fs_result<directory_handle> directory_handle::open(const path_handle &base, path_view path) noexcept
	{
	}

	[[nodiscard]] inline static std::wstring generate_unique_name()
	{
		constexpr wchar_t alphabet[] = L"0123456789abcdef";

		auto str = std::wstring(64, '\0');
		auto gen = system_random(str.data(), str.size() * sizeof(wchar_t));
		for (std::size_t i = 0; i < str.size(); ++i)
		{
			/* std::rand fallback is fine since it's not used it for cryptography. */
			if (i >= gen) [[unlikely]]
				str[i] = alphabet[std::rand() % 16];
			else
				str[i] = alphabet[str[i] % 16];
		}
		return str;
	}

	fs_result<directory_handle> directory_handle::open_unique(const path_handle &base) noexcept
	{
		try { return open(base, generate_unique_name()); }
		catch (const std::system_error &e) { return e.code(); }
		catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
	}
	fs_result<directory_handle> directory_handle::open_temporary(path_view path) noexcept
	{
		if (path.empty())
			return open_unique(temporary_file_directory());
		else
			return open(temporary_file_directory(), path);
	}
}

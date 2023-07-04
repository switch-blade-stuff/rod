/*
 * Created by switch_blade on 2023-07-03.
 */

#ifdef _WIN32

#include "shm.hpp"

#define NOMINMAX
#include <windows.h>

namespace rod::detail
{
	result<system_shm, std::error_code> system_shm::open(const char *name, std::size_t size, int mode) noexcept
	{
		try
		{
			if (const auto buff_size = ::MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0); size) [[likely]]
			{
				auto buff = std::wstring(buff_size - 1, '\0');
				if (::MultiByteToWideChar(CP_UTF8, 0, name, -1, buff.data(), buff_size)) [[likely]]
					return open(buff.c_str(), size, mode);
			}
			return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
		}
		catch (const std::bad_alloc &)
		{
			return std::make_error_code(std::errc::not_enough_memory);
		}
	}
	result<system_shm, std::error_code> system_shm::open(const wchar_t *name, std::size_t size, int mode) noexcept
	{
		native_handle_type hnd;
		if (mode & openmode::nocreate)
		{
			DWORD access = FILE_MAP_READ;
			if (!(mode & openmode::readonly))
				access = FILE_MAP_WRITE;

			hnd = ::OpenFileMappingW(access, false, name);
		}
		else
		{
			DWORD prot;
			if (mode & openmode::readonly)
				prot = PAGE_READONLY;
			else
				prot = PAGE_READWRITE;

			DWORD size_h = {}, size_l;
#if SIZE_MAX >= UINT64_MAX
			size_h = static_cast<DWORD>(size >> std::numeric_limits<DWORD>::digits);
#endif
			size_l = static_cast<DWORD>(size);

			hnd = ::CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, prot, size_h, size_l, name);
		}

		if (hnd == nullptr) [[unlikely]]
			return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
		if (DWORD err; (mode & openmode::noreplace) && (err = ::GetLastError()) == ERROR_ALREADY_EXISTS)
			return std::error_code{static_cast<int>(err), std::system_category()};
		return system_shm{hnd};
	}

	result<system_mmap, std::error_code> system_shm::map(std::size_t off, std::size_t n, int mode) const noexcept
	{
		DWORD access = 0;
		if (mode & system_mmap::mapmode::copy)
			access |= FILE_MAP_COPY;
		if (mode & system_mmap::mapmode::read)
			access |= FILE_MAP_READ;
		if (mode & system_mmap::mapmode::write)
			access |= FILE_MAP_WRITE;

		/* Align the file offset to page size. */
		const auto page_off = system_mmap::pagesize_off(off);

		DWORD off_h = {}, off_l;
#if SIZE_MAX >= UINT64_MAX
		off_h = static_cast<DWORD>(page_off >> std::numeric_limits<DWORD>::digits);
#endif
		off_l = static_cast<DWORD>(page_off);

		if (const auto data = ::MapViewOfFile(native_handle(), access, off_h, off_l, n + off - page_off); !data) [[unlikely]]
			return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
		else
			return system_mmap{data, off - page_off, n + off - page_off};
	}
}
#endif
/*
 * Created by switch_blade on 2023-07-03.
 */

#ifdef _WIN32

#include "shm.hpp"

#define NOMINMAX
#include <windows.h>
#include <intrin.h>

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
				access |= FILE_MAP_WRITE;

			hnd = ::OpenFileMappingW(access, false, name);
		}
		else
		{
			size += sizeof(header_t);

			DWORD prot = PAGE_READWRITE;
			if (mode & openmode::readonly)
				prot = PAGE_READONLY;

			DWORD size_h = {}, size_l;
#if SIZE_MAX >= UINT64_MAX
			size_h = static_cast<DWORD>(size >> std::numeric_limits<DWORD>::digits);
#endif
			size_l = static_cast<DWORD>(size);

			hnd = ::CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, prot, size_h, size_l, name);
		}

		if (hnd == nullptr) [[unlikely]]
			return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};

		const auto header = static_cast<header_t *>(::MapViewOfFile(hnd, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(header_t)));
		if (header == nullptr) [[unlikely]]
			return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};

		return system_shm{hnd, header};
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

		/* Align the offset to page & header size. */
		off += sizeof(header_t);
		const auto base_off = system_mmap::pagesize_off(off);

		DWORD off_h = {}, off_l;
#if SIZE_MAX >= UINT64_MAX
		off_h = static_cast<DWORD>(base_off >> std::numeric_limits<DWORD>::digits);
#endif
		off_l = static_cast<DWORD>(base_off);
ERROR_ALREADY_EXISTS;
		if (const auto data = ::MapViewOfFile(native_handle(), access, off_h, off_l, n + off - base_off); !data) [[unlikely]]
			return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
		else
			return system_mmap{data, off - base_off, n + off - base_off};
	}

	void system_shm::lock() noexcept
	{
		for (std::size_t spin = 16; !try_lock();)
		{
			if (!spin)
				::YieldProcessor();
			else
				spin--;
		}
	}
	bool system_shm::try_lock() noexcept { return !_header->busy.test_and_set(std::memory_order_acq_rel); }
	void system_shm::unlock() noexcept { _header->busy.clear(std::memory_order_release); }

	system_shm::~system_shm() { ::UnmapViewOfFile(_header); }
}
#endif
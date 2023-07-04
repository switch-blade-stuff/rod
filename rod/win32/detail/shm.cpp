/*
 * Created by switch_blade on 2023-07-03.
 */

#ifdef _WIN32

#include "shm.hpp"

#ifdef __SSE2__
#include <emmintrin.h>
#endif

#define NOMINMAX

#include <windows.h>
#include <thread>

namespace rod::detail
{
	/* Use the same name for both mutex & shared memory and differentiate via a postfix.
	 * Postfix is used to allow for win32-esque "Global\" & "Local\" session namespaces. */
	result<system_shm, std::error_code> system_shm::open(const char *name, std::size_t size, int mode) noexcept
	{
		try
		{
			if (const auto name_size = ::MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0); size) [[likely]]
			{
				auto shm_name = std::wstring(name_size + 3, '\0');
				auto mtx_name = std::wstring(name_size + 3, '\0');
				if (::MultiByteToWideChar(CP_UTF8, 0, name, -1, shm_name.data(), name_size)) [[likely]]
				{
					mtx_name.replace(0, name_size - 1, shm_name);
					shm_name.replace(name_size - 1, 3, L":shm");
					mtx_name.replace(name_size - 1, 3, L":mtx");
					return open(shm_name.c_str(), mtx_name.c_str(), size, mode);
				}
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
		try
		{
			auto shm_name = std::wstring{name};
			auto mtx_name = std::wstring{name};
			shm_name.append(L":shm");
			mtx_name.append(L":mtx");
			return open(shm_name.c_str(), mtx_name.c_str(), size, mode);
		}
		catch (const std::bad_alloc &)
		{
			return std::make_error_code(std::errc::not_enough_memory);
		}
	}
	result<system_shm, std::error_code> system_shm::open(const wchar_t *shm_name, const wchar_t *mtx_name, std::size_t size, int mode) noexcept
	{
		native_handle_type hnd, mtx = {};
		if (mode & openmode::nocreate)
		{
			DWORD access = FILE_MAP_READ;
			if (!(mode & openmode::readonly))
				access |= FILE_MAP_WRITE;
			hnd = ::OpenFileMappingW(access, false, shm_name);

			if (hnd != nullptr) [[likely]]
			{
				access = SYNCHRONIZE;
				if (!(mode & openmode::readonly))
					access |= MUTEX_MODIFY_STATE;
				mtx = ::OpenMutexW(access, false, mtx_name);
			}
		}
		else
		{
			DWORD prot = PAGE_READWRITE;
			if (mode & openmode::readonly)
				prot = PAGE_READONLY;

			DWORD size_h = {}, size_l;
#if SIZE_MAX >= UINT64_MAX
			size_h = static_cast<DWORD>(size >> std::numeric_limits<DWORD>::digits);
#endif
			size_l = static_cast<DWORD>(size);

			if ((hnd = ::CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, prot, size_h, size_l, shm_name)) != nullptr)
				[[likely]] mtx = ::CreateMutexW(nullptr, false, mtx_name);
		}

		if (hnd && !mtx)
			::CloseHandle(hnd);
		if (!hnd || !mtx) [[unlikely]]
			return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
		else
			return system_shm{hnd, mtx};
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

	bool system_shm::try_lock() noexcept
	{
		const auto res = ::WaitForSingleObject(_mtx.native_handle(), 0);
		return res == WAIT_OBJECT_0 || res == WAIT_ABANDONED;
	}
	std::error_code system_shm::lock() noexcept
	{
		if (::WaitForSingleObject(_mtx.native_handle(), INFINITE) == WAIT_FAILED) [[unlikely]]
			return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
		else
			return {};
	}
	std::error_code system_shm::unlock() noexcept
	{
		if (!::ReleaseMutex(_mtx.native_handle())) [[unlikely]]
			return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
		else
			return {};
	}
}
#endif
/*
 * Created by switch_blade on 2023-07-03.
 */

#ifdef _WIN32

#include "shm.hpp"

#define NOMINMAX
#include <windows.h>

namespace rod::detail
{
	static std::pair<std::size_t, std::size_t> fill_name_buffer(std::wstring &buff, const wchar_t *name, std::size_t name_size) noexcept
	{
		/* Use the same name for both mutex & shared memory and differentiate via a postfix.
		 * Postfix is used to allow for win32-esque "Global\" & "Local\" session namespaces. */
		const std::size_t shm_pos = 0, mtx_pos = name_size + 5;

		buff.replace(shm_pos, name_size, name);
		buff.replace(mtx_pos, name_size, name);
		buff.replace(shm_pos + name_size, 4, L":shm");
		buff.replace(mtx_pos + name_size, 4, L":mtx");
		return {shm_pos, mtx_pos};
	}

	result<system_shm, std::error_code> system_shm::open(const char *name, std::size_t size, int mode) noexcept
	{
		try
		{
			if (auto name_size = ::MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0); size) [[likely]]
			{
				auto name_buff = std::wstring((--name_size + 4) * 2 + 1, '\0');
				if (::MultiByteToWideChar(CP_UTF8, 0, name, -1, name_buff.data(), name_size + 1)) [[likely]]
				{
					const auto [shm_pos, mtx_pos] = fill_name_buffer(name_buff, name_buff.data(), name_size);
					return open(name_buff.data() + shm_pos, name_buff.data() + mtx_pos, size, mode);
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
			const auto name_size = std::wcslen(name);
			auto name_buff = std::wstring((name_size + 4) * 2 + 1, '\0');
			const auto [shm_pos, mtx_pos] = fill_name_buffer(name_buff, name_buff.data(), name_size);
			return open(name_buff.data() + shm_pos, name_buff.data() + mtx_pos, size, mode);
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
			DWORD size_h = {}, size_l;
#if SIZE_MAX >= UINT64_MAX
			size_h = static_cast<DWORD>(size >> std::numeric_limits<DWORD>::digits);
#endif
			size_l = static_cast<DWORD>(size);

			if ((hnd = ::CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, size_h, size_l, shm_name)) != nullptr)
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

	bool system_shm::try_lock() noexcept { return ::WaitForSingleObject(_mtx.native_handle(), 0) <= WAIT_ABANDONED; }
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
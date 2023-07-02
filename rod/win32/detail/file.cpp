/*
 * Created by switchblade on 2023-05-18.
 */

#ifdef _WIN32

#include "file.hpp"

#define NOMINMAX
#include <windows.h>

namespace rod::detail
{
	constexpr DWORD share = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;

	result<system_file, std::error_code> system_file::reopen(native_handle_type hnd, int mode) noexcept
	{
		DWORD access = 0;
		if (mode & openmode::in) access |= FILE_GENERIC_READ;
		if (mode & openmode::out)
		{
			access |= FILE_GENERIC_WRITE;
			if (mode & openmode::app)
				access ^= FILE_WRITE_DATA;
			else if (!(mode & openmode::ate))
				access ^= FILE_APPEND_DATA;
		}

		DWORD flags = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_POSIX_SEMANTICS | FILE_FLAG_OVERLAPPED;
		if (mode & openmode::direct) flags |= FILE_FLAG_WRITE_THROUGH;

		if (hnd = ::ReOpenFile(hnd, access, share, flags); hnd != INVALID_HANDLE_VALUE) [[likely]]
		{
#if 0   /* TODO: Set file permissions if created a new file. */
			if (GetLastError() != ERROR_ALREADY_EXISTS)
			{
			}
#endif

#if SIZE_MAX >= UINT64_MAX
			LONG high = 0;
			if (const auto res = ::SetFilePointer(hnd, 0, &high, FILE_CURRENT); res != INVALID_SET_FILE_POINTER) [[likely]]
			{
				const auto off_h = static_cast<std::size_t>(high) << std::numeric_limits<LONG>::digits;
				const auto off_l = static_cast<std::size_t>(res);
				return system_file{hnd, off_h | off_l};
			}
#else
			if (const auto res = ::SetFilePointer(hnd, 0, nullptr, FILE_CURRENT); res != INVALID_SET_FILE_POINTER) [[likely]]
				return system_file{hnd, static_cast<std::size_t>(res)};
#endif
			::CloseHandle(hnd);
		}
		return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
	}
	result<system_file, std::error_code> system_file::open(const wchar_t *path, int mode, int) noexcept
	{
		DWORD access = 0;
		if (mode & openmode::in) access |= FILE_GENERIC_READ;
		if (mode & openmode::out)
		{
			access |= FILE_GENERIC_WRITE;
			if (mode & openmode::app)
				access ^= FILE_WRITE_DATA;
			else if (!(mode & openmode::ate))
				access ^= FILE_APPEND_DATA;
		}

		DWORD disp;
		if (mode & openmode::nocreate)
		{
			if (mode & openmode::trunc)
				disp = TRUNCATE_EXISTING;
			else
				disp = OPEN_EXISTING;
		}
		else if (mode & openmode::noreplace)
			disp = CREATE_NEW;
		else if (mode & openmode::trunc)
			disp = CREATE_ALWAYS;
		else
			disp = OPEN_ALWAYS;

		DWORD flags = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_POSIX_SEMANTICS | FILE_FLAG_OVERLAPPED;
		if (mode & openmode::direct) flags |= FILE_FLAG_WRITE_THROUGH;

		if (const auto hnd = ::CreateFileW(path, access, share, nullptr, disp, flags, nullptr); hnd != INVALID_HANDLE_VALUE) [[likely]]
		{
#if 0   /* TODO: Set file permissions if created a new file. */
			if (GetLastError() != ERROR_ALREADY_EXISTS)
			{
			}
#endif

#if SIZE_MAX >= UINT64_MAX
			LONG high = 0;
			if (const auto res = ::SetFilePointer(hnd, 0, &high, FILE_CURRENT); res != INVALID_SET_FILE_POINTER) [[likely]]
			{
				const auto off_h = static_cast<std::size_t>(high) << std::numeric_limits<LONG>::digits;
				const auto off_l = static_cast<std::size_t>(res);
				return system_file{hnd, off_h | off_l};
			}
#else
			if (const auto res = ::SetFilePointer(hnd, 0, nullptr, FILE_CURRENT); res != INVALID_SET_FILE_POINTER) [[likely]]
				return system_file{hnd, static_cast<std::size_t>(res)};
#endif
			::CloseHandle(hnd);
		}
		return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
	}
	result<system_file, std::error_code> system_file::open(const char *path, int mode, int prot) noexcept
	{
		try
		{
			if (const auto size = ::MultiByteToWideChar(CP_UTF8, 0, path, -1, nullptr, 0); size) [[likely]]
			{
				auto buff = std::wstring(size - 1, '\0');
				if (::MultiByteToWideChar(CP_UTF8, 0, path, -1, buff.data(), size)) [[likely]]
					return open(buff.c_str(), mode, prot);
			}
			return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
		}
		catch (const std::bad_alloc &)
		{
			return std::make_error_code(std::errc::not_enough_memory);
		}
	}

	result<std::size_t, std::error_code> system_file::size() const noexcept
	{
		if (LARGE_INTEGER result = {}; !::GetFileSizeEx(native_handle(), &result)) [[unlikely]]
			return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
		else
			return static_cast<std::size_t>(result.QuadPart);
	}
	result<std::size_t, std::error_code> system_file::tell() const noexcept
	{
		if (_offset != std::numeric_limits<std::size_t>::max()) [[likely]]
			return _offset;

#if SIZE_MAX >= UINT64_MAX
		LONG high = 0;
		if (const auto res = ::SetFilePointer(native_handle(), 0, &high, FILE_CURRENT); res != INVALID_SET_FILE_POINTER) [[likely]]
		{
			const auto pos_h = static_cast<std::size_t>(high) << std::numeric_limits<LONG>::digits;
			const auto pos_l = static_cast<std::size_t>(res);
			return pos_h | pos_l;
		}
#else
		if (const auto res = ::SetFilePointer(native_handle(), 0, nullptr, FILE_CURRENT); res == INVALID_SET_FILE_POINTER) [[likely]]
			return static_cast<std::size_t>(res);
#endif
		return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
	}

	result<std::size_t, std::error_code> system_file::resize(std::size_t n) noexcept
	{
		LARGE_INTEGER oldp;
		LARGE_INTEGER endp;

		if (auto pos = tell(); !pos.has_value()) [[unlikely]]
			return pos;
		else
		{
			oldp = {.QuadPart = *pos};
			endp = {.QuadPart = n};
		}

		if (!::SetFilePointerEx(native_handle(), endp, nullptr, FILE_BEGIN))
			goto fail;
		if (!::SetEndOfFile(native_handle()) || !::SetFilePointerEx(native_handle(), oldp, nullptr, FILE_BEGIN))
			goto fail;

		_offset = static_cast<std::size_t>(oldp.QuadPart);
		return static_cast<std::size_t>(endp.QuadPart);
	fail:
		return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
	}
	result<std::size_t, std::error_code> system_file::seek(std::ptrdiff_t off, int dir) noexcept
	{
		if (dir == seekdir::cur)
		{
			auto pos = tell();
			if (!pos.has_value()) [[unlikely]]
				return pos;

			off += static_cast<std::ptrdiff_t>(*pos);
			dir = seekdir::beg;
		}

#if SIZE_MAX >= UINT64_MAX
		auto high = static_cast<LONG>(off >> std::numeric_limits<LONG>::digits);
		if (const auto res = ::SetFilePointer(native_handle(), static_cast<LONG>(off), &high, dir); res != INVALID_SET_FILE_POINTER) [[likely]]
		{
			const auto off_h = static_cast<std::size_t>(high) << std::numeric_limits<LONG>::digits;
			const auto off_l = static_cast<std::size_t>(res);
			return _offset = off_h | off_l;
		}
#else
		if (const auto res = ::SetFilePointer(native_handle(), static_cast<LONG>(off), nullptr, dir); res != INVALID_SET_FILE_POINTER) [[likely]]
			return _offset = static_cast<std::size_t>(res);
#endif
		return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
	}

	result<std::filesystem::path, std::error_code> system_file::path() const
	{
		try
		{
			if (const auto size = ::GetFinalPathNameByHandleW(native_handle(), nullptr, 0, FILE_NAME_NORMALIZED); size) [[likely]]
			{
				auto buff = std::wstring(size - 1, '\0');
				if (::GetFinalPathNameByHandleW(native_handle(), buff.data(), size, FILE_NAME_NORMALIZED)) [[likely]]
					return std::filesystem::path{buff};
			}
			return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
		}
		catch (const std::bad_alloc &)
		{
			return std::make_error_code(std::errc::not_enough_memory);
		}
	}

	std::error_code system_file::sync() noexcept
	{
		if (!::FlushFileBuffers(native_handle())) [[unlikely]]
			return {static_cast<int>(::GetLastError()), std::system_category()};
		else
			return {};
	}
	result<std::size_t, std::error_code> system_file::sync_read(void *dst, std::size_t n) noexcept
	{
		if (auto pos = tell(); pos.has_value()) [[likely]]
			return sync_read_at(dst, n, *pos);
		else
			return pos;
	}
	result<std::size_t, std::error_code> system_file::sync_write(const void *src, std::size_t n) noexcept
	{
		if (auto pos = tell(); pos.has_value()) [[likely]]
			return sync_write_at(src, n, *pos);
		else
			return 0;
	}
	result<std::size_t, std::error_code> system_file::sync_read_at(void *dst, std::size_t n, std::size_t off) noexcept
	{
		std::size_t total = 0;
		for (DWORD n_done = 0, err = 0; total < n && err != ERROR_HANDLE_EOF;)
		{
			OVERLAPPED overlapped = {};
			overlapped.Offset = static_cast<DWORD>(off + total);
#if SIZE_MAX >= UINT64_MAX
			overlapped.OffsetHigh = static_cast<DWORD>((off + total) << std::numeric_limits<DWORD>::digits);
#endif

			n_done = static_cast<DWORD>(std::min<std::size_t>(n - total, std::numeric_limits<DWORD>::max()));
			if (!::ReadFile(native_handle(), static_cast<std::byte *>(dst) + total, n_done, nullptr, &overlapped) && (err = ::GetLastError()) != ERROR_IO_PENDING) [[unlikely]]
				return std::error_code{static_cast<int>(err), std::system_category()};
			if (!::GetOverlappedResultEx(native_handle(), &overlapped, &n_done, INFINITE, false) && (err = ::GetLastError()) != ERROR_HANDLE_EOF) [[unlikely]]
				return std::error_code{static_cast<int>(err), std::system_category()};

			total += n_done;
		}
		_offset = off + total;
		return total;
	}
	result<std::size_t, std::error_code> system_file::sync_write_at(const void *src, std::size_t n, std::size_t off) noexcept
	{
		std::size_t total = 0;
		for (DWORD n_done = 0, err = 0; total < n && err != ERROR_HANDLE_EOF;)
		{
			OVERLAPPED overlapped = {};
			overlapped.Offset = static_cast<DWORD>(off + total);
#if SIZE_MAX >= UINT64_MAX
			overlapped.OffsetHigh = static_cast<DWORD>((off + total) << std::numeric_limits<DWORD>::digits);
#endif

			n_done = static_cast<DWORD>(std::min<std::size_t>(n - total, std::numeric_limits<DWORD>::max()));
			if (!::WriteFile(native_handle(), static_cast<const std::byte *>(src) + total, n_done, nullptr, &overlapped) && (err = ::GetLastError()) != ERROR_IO_PENDING) [[unlikely]]
				return std::error_code{static_cast<int>(err), std::system_category()};
			if (!::GetOverlappedResultEx(native_handle(), &overlapped, &n_done, INFINITE, false) && (err = ::GetLastError()) != ERROR_HANDLE_EOF) [[unlikely]]
				return std::error_code{static_cast<int>(err), std::system_category()};

			total += n_done;
		}
		_offset = off + total;
		return total;
	}
}
#endif

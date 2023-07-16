/*
 * Created by switchblade on 2023-05-18.
 */

#include "file.hpp"

#ifdef ROD_WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace rod::_file
{
	constexpr DWORD share = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;

	std::error_code system_handle::open(const wchar_t *path, int mode, int) noexcept
	{
		if (auto err = close(); err) [[unlikely]]
			return err;

		DWORD access = 0;
		if (mode & file_base::in) access |= FILE_GENERIC_READ;
		if (mode & file_base::out)
		{
			access |= FILE_GENERIC_WRITE;
			if (mode & file_base::app)
				access ^= FILE_WRITE_DATA;
			else if (!(mode & file_base::ate))
				access ^= FILE_APPEND_DATA;
		}

		DWORD disp;
		if (mode & file_base::nocreate)
		{
			if (mode & file_base::trunc)
				disp = TRUNCATE_EXISTING;
			else
				disp = OPEN_EXISTING;
		}
		else if (mode & file_base::noreplace)
			disp = CREATE_NEW;
		else if (mode & file_base::trunc)
			disp = CREATE_ALWAYS;
		else
			disp = OPEN_ALWAYS;

		DWORD flags = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_POSIX_SEMANTICS;
		if (mode & overlapped) flags |= FILE_FLAG_OVERLAPPED;
		if (mode & file_base::direct) flags |= FILE_FLAG_WRITE_THROUGH;

		if (const auto hnd = ::CreateFileW(path, access, share, nullptr, disp, flags, nullptr); hnd != INVALID_HANDLE_VALUE) [[likely]]
		{
#if 0   /* TODO: Set file permissions if created a new file. */
			if (GetLastError() != ERROR_ALREADY_EXISTS)
			{
			}
#endif
			unique_handle::release(hnd);

			/* Get position of the file. Leave npos on failure. */
			if (mode & file_base::trunc)
			{
				setpos(0);
				return {};
			}

#if SIZE_MAX >= UINT64_MAX
			LONG high = 0;
			if (const auto res = ::SetFilePointer(native_handle(), 0, &high, FILE_CURRENT); res != INVALID_SET_FILE_POINTER) [[likely]]
			{
				const auto pos_h = static_cast<std::size_t>(high) << std::numeric_limits<LONG>::digits;
				const auto pos_l = static_cast<std::size_t>(res);
				setpos(pos_h | pos_l);
			}
#else
			if (const auto res = ::SetFilePointer(native_handle(), 0, nullptr, FILE_CURRENT); res != INVALID_SET_FILE_POINTER)
				setpos(static_cast<std::size_t>(res));
#endif
			return {};
		}
		return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
	}
	std::error_code system_handle::open(const char *path, int mode, int prot) noexcept
	{
		if (const auto size = ::MultiByteToWideChar(CP_UTF8, 0, path, -1, nullptr, 0); size) [[likely]]
		{
			if (auto buff = static_cast<wchar_t *>(std::calloc(size, sizeof(wchar_t))); !buff) [[unlikely]]
				return std::make_error_code(std::errc::not_enough_memory);
			else if (!::MultiByteToWideChar(CP_UTF8, 0, path, -1, buff, size))
				[[unlikely]] std::free(buff);
			else
			{
				auto res = open(buff, mode, prot);
				std::free(buff);
				return res;
			}
		}
		return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
	}

	std::error_code system_handle::sync() noexcept
	{
		if (!::FlushFileBuffers(native_handle())) [[unlikely]]
			return {static_cast<int>(::GetLastError()), std::system_category()};
		else
			return {};
	}

	result<std::size_t, std::error_code> system_handle::size() const noexcept
	{
		if (LARGE_INTEGER result = {}; !::GetFileSizeEx(native_handle(), &result)) [[unlikely]]
			return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
		else
			return static_cast<std::size_t>(result.QuadPart);
	}
	result<std::size_t, std::error_code> system_handle::tell() const noexcept
	{
		if (_pos != npos) [[likely]]
			return _pos;

#if SIZE_MAX >= UINT64_MAX
		LONG high = 0;
		if (const auto res = ::SetFilePointer(native_handle(), 0, &high, FILE_CURRENT); res != INVALID_SET_FILE_POINTER) [[likely]]
		{
			const auto pos_h = static_cast<std::size_t>(high) << std::numeric_limits<LONG>::digits;
			const auto pos_l = static_cast<std::size_t>(res);
			return pos_h | pos_l;
		}
#else
		if (const auto res = ::SetFilePointer(native_handle(), 0, nullptr, FILE_CURRENT); res != INVALID_SET_FILE_POINTER) [[likely]]
			return static_cast<std::size_t>(res);
#endif
		return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
	}
	result<std::filesystem::path, std::error_code> system_handle::path() const
	{
		if (const auto size = ::GetFinalPathNameByHandleW(native_handle(), nullptr, 0, FILE_NAME_NORMALIZED); size) [[likely]]
		{
			if (auto buff = static_cast<wchar_t *>(std::calloc(size, sizeof(wchar_t))); !buff) [[unlikely]]
				return std::make_error_code(std::errc::not_enough_memory);
			else if (!::GetFinalPathNameByHandleW(native_handle(), buff, size, FILE_NAME_NORMALIZED))
				[[unlikely]] std::free(buff);
			else
			{
				auto res = std::filesystem::path{buff};
				std::free(buff);
				return res;
			}
		}
		return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
	}

	result<std::size_t, std::error_code> system_handle::resize(std::size_t n) noexcept
	{
		LARGE_INTEGER oldp, endp;
		oldp = {.QuadPart = static_cast<ntapi::longlong>(_pos)};
		endp = {.QuadPart = static_cast<ntapi::longlong>(n)};
		if (::SetFilePointerEx(native_handle(), endp, nullptr, FILE_BEGIN) && ::SetEndOfFile(native_handle())) [[likely]]
		{
			if (_pos == npos || oldp.QuadPart > endp.QuadPart || ::SetFilePointerEx(native_handle(), oldp, nullptr, FILE_BEGIN)) [[likely]]
				return static_cast<std::size_t>(endp.QuadPart);
		}
		return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
	}
	result<std::size_t, std::error_code> system_handle::seek(std::ptrdiff_t off, int dir) noexcept
	{
		if (dir == file_base::beg)
			return setpos(static_cast<std::size_t>(off));
		if (dir == file_base::cur)
		{
			if (auto pos = tell(); pos.has_value()) [[likely]]
				return setpos(*pos + static_cast<std::size_t>(off));
			else
				return pos;
		}
		if (dir == file_base::end)
		{
#if SIZE_MAX >= UINT64_MAX
			auto high = static_cast<LONG>(off >> std::numeric_limits<LONG>::digits);
			if (const auto res = ::SetFilePointer(native_handle(), static_cast<LONG>(off), &high, dir); res != INVALID_SET_FILE_POINTER) [[likely]]
			{
				const auto off_h = static_cast<std::size_t>(high) << std::numeric_limits<LONG>::digits;
				const auto off_l = static_cast<std::size_t>(res);
				return setpos(off_h | off_l);
			}
#else
			if (const auto res = ::SetFilePointer(native_handle(), static_cast<LONG>(off), nullptr, dir); res != INVALID_SET_FILE_POINTER) [[unkely]]
				return setpos(static_cast<std::size_t>(res));
#endif
			return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
		}
		return std::make_error_code(std::errc::invalid_argument);
	}

	result<ntapi::ulong, std::error_code> system_handle::read_chunk(void *buff, ntapi::ulong size, std::size_t pos) noexcept
	{
		const auto &ntapi = ntapi::instance;
		ntapi::io_status_block iosb;
		ntapi::large_integer offset;

		offset = {.quad = static_cast<ntapi::longlong>(pos)};
		if (const auto status = ntapi.NtReadFile(native_handle(), nullptr, nullptr, 0, &iosb, buff, size, &offset, nullptr); status < 0) [[unlikely]]
			return std::error_code{static_cast<int>(ntapi.RtlNtStatusToDosError(status)), std::system_category()};
		else if (std::error_code err; status != STATUS_PENDING || !(err = wait())) [[unlikely]]
			return static_cast<ntapi::ulong>(iosb.info);
		else
			return err;
	}
	result<ntapi::ulong, std::error_code> system_handle::write_chunk(const void *buff, ntapi::ulong size, std::size_t pos) noexcept
	{
		const auto &ntapi = ntapi::instance;
		ntapi::io_status_block iosb;
		ntapi::large_integer offset;

		offset = {.quad = static_cast<ntapi::longlong>(pos)};
		if (const auto status = ntapi.NtWriteFile(native_handle(), nullptr, nullptr, 0, &iosb, const_cast<void *>(buff), size, &offset, nullptr); status < 0) [[unlikely]]
			return std::error_code{static_cast<int>(ntapi.RtlNtStatusToDosError(status)), std::system_category()};
		else if (std::error_code err; status != STATUS_PENDING || !(err = wait())) [[unlikely]]
			return static_cast<ntapi::ulong>(iosb.info);
		else
			return err;
	}

	result<std::size_t, std::error_code> system_handle::read_some(std::span<std::byte> buff) noexcept
	{
		if (auto pos = tell(); pos.has_value()) [[likely]]
			return read_some_at(*pos, buff);
		else
			return pos;
	}
	result<std::size_t, std::error_code> system_handle::write_some(std::span<const std::byte> buff) noexcept
	{
		if (auto pos = tell(); pos.has_value()) [[likely]]
			return write_some_at(*pos, buff);
		else
			return pos;
	}
	result<std::size_t, std::error_code> system_handle::read_some_at(std::size_t pos, std::span<std::byte> buff) noexcept
	{
		for (std::size_t total = 0;;)
		{
			const auto chunk = static_cast<ntapi::ulong>(buff.size() - total);
			auto result = read_chunk(buff.data() + total, chunk, pos + total);
			if (result.has_value())
				[[likely]] total += *result;
			else
				return result;

			if (total >= buff.size() || *result < chunk)
			{
				setpos(pos + total);
				return total;
			}
		}
	}
	result<std::size_t, std::error_code> system_handle::write_some_at(std::size_t pos, std::span<const std::byte> buff) noexcept
	{
		for (std::size_t total = 0;;)
		{
			const auto chunk = static_cast<ntapi::ulong>(buff.size() - total);
			auto result = write_chunk(buff.data() + total, chunk, pos + total);
			if (result.has_value())
				[[likely]] total += *result;
			else
				return result;

			if (total >= buff.size() || *result < chunk)
			{
				setpos(pos + total);
				return total;
			}
		}
	}
}
#endif

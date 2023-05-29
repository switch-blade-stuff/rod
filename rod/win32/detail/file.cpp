/*
 * Created by switchblade on 2023-05-18.
 */

#include <Windows.h>

#include "file.hpp"
#include "rod/unix/detail/file.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	native_file native_file::open(const char *path, int mode, std::error_code &err) noexcept
	{
		::MultiByteToWideChar();
	}
	native_file native_file::open(const wchar_t *path, int mode, std::error_code &err) noexcept
	{
	}

	std::ptrdiff_t native_file::tell(std::error_code &err) const noexcept
	{
#if PTRDIFF_MAX >= INT64_MAX
		LONG high = 0;
		if (const auto res = ::SetFilePointer(native_handle(), 0, &high, FILE_CURRENT); res != INVALID_SET_FILE_POINTER) [[likely]]
			return (err = {}, static_cast<std::ptrdiff_t>(res) | (static_cast<std::ptrdiff_t>(high) << std::numeric_limits<LONG>::digits));
		else
			return (err = {static_cast<int>(::GetLastError()), std::system_category()}, -1);
#else
		if (const auto res = ::SetFilePointer(native_handle(), 0, nullptr, FILE_CURRENT); res == INVALID_SET_FILE_POINTER) [[unlikely]]
			return (err = {static_cast<int>(::GetLastError()), std::system_category()}, -1);
		else
			return (err = {}, static_cast<std::ptrdiff_t>(res));
#endif
	}
	std::ptrdiff_t native_file::seek(std::ptrdiff_t off, int dir, std::error_code &err) noexcept
	{
#if PTRDIFF_MAX >= INT64_MAX
		auto high = static_cast<LONG>(off >> std::numeric_limits<LONG>::digits);
		if (const auto res = ::SetFilePointer(native_handle(), static_cast<LONG>(off), &high, dir); res != INVALID_SET_FILE_POINTER) [[likely]]
			return (err = {}, static_cast<std::ptrdiff_t>(res) | (static_cast<std::ptrdiff_t>(high) << std::numeric_limits<LONG>::digits));
		else
			return (err = {static_cast<int>(::GetLastError()), std::system_category()}, -1);
#else
		if (const auto res = ::SetFilePointer(native_handle(), static_cast<LONG>(off), nullptr, dir); res == INVALID_SET_FILE_POINTER) [[unlikely]]
			return (err = {static_cast<int>(::GetLastError()), std::system_category()}, -1);
		else
			return (err = {}, static_cast<std::ptrdiff_t>(res));
#endif
	}

	std::error_code native_file::flush() noexcept
	{
		if (!::FlushFileBuffers(native_handle())) [[unlikely]]
			return {static_cast<int>(::GetLastError()), std::system_category()};
		else
			return {};
	}
	std::size_t native_file::sync_read(void *dst, std::size_t n, std::error_code &err) noexcept
	{
		return 0;
	}
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

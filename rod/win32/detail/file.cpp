/*
 * Created by switchblade on 2023-05-18.
 */

#include <Windows.h>

#include "file.hpp"
#include "rod/unix/detail/file.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	system_file system_file::open(const char *path, int mode, std::error_code &err) noexcept
	{
		::MultiByteToWideChar();
	}
	system_file system_file::open(const wchar_t *path, int mode, std::error_code &err) noexcept
	{
	}

	std::size_t system_file::tell(std::error_code &err) const noexcept
	{
#if SIZE_MAX >= UINT64_MAX
		LONG high = 0;
		if (const auto res = ::SetFilePointer(native_handle(), 0, &high, FILE_CURRENT); res != INVALID_SET_FILE_POINTER) [[likely]]
			return (err = {}, static_cast<std::size_t>(res) | (static_cast<std::size_t>(high) << std::numeric_limits<LONG>::digits));
		else
			return (err = {static_cast<int>(::GetLastError()), std::system_category()}, 0);
#else
		if (const auto res = ::SetFilePointer(native_handle(), 0, nullptr, FILE_CURRENT); res == INVALID_SET_FILE_POINTER) [[unlikely]]
			return (err = {static_cast<int>(::GetLastError()), std::system_category()}, 0);
		else
			return (err = {}, static_cast<std::size_t>(res));
#endif
	}
	std::size_t system_file::seek(std::ptrdiff_t off, int dir, std::error_code &err) noexcept
	{
#if SIZE_MAX >= UINT64_MAX
		auto high = static_cast<LONG>(off >> std::numeric_limits<LONG>::digits);
		if (const auto res = ::SetFilePointer(native_handle(), static_cast<LONG>(off), &high, dir); res != INVALID_SET_FILE_POINTER) [[likely]]
			return (err = {}, static_cast<std::size_t>(res) | (static_cast<std::size_t>(high) << std::numeric_limits<LONG>::digits));
		else
			return (err = {static_cast<int>(::GetLastError()), std::system_category()}, 0);
#else
		if (const auto res = ::SetFilePointer(native_handle(), static_cast<LONG>(off), nullptr, dir); res == INVALID_SET_FILE_POINTER) [[unlikely]]
			return (err = {static_cast<int>(::GetLastError()), std::system_category()}, 0);
		else
			return (err = {}, static_cast<std::size_t>(res));
#endif
	}

	std::error_code system_file::flush() noexcept
	{
		if (!::FlushFileBuffers(native_handle())) [[unlikely]]
			return {static_cast<int>(::GetLastError()), std::system_category()};
		else
			return {};
	}
	std::size_t system_file::sync_read(void *dst, std::size_t n, std::error_code &err) noexcept
	{
		return 0;
	}
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

/*
 * Created by switchblade on 2023-05-18.
 */

#include "file.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	constexpr DWORD share = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;

	system_file system_file::open(const char *path, int mode, int prot, std::error_code &err) noexcept
	{
		const auto size = ::MultiByteToWideChar(CP_UTF8, 0, path, -1, nullptr, 0);
		if (!size) [[unlikely]] return (err = {static_cast<int>(::GetLastError()), std::system_category()}, system_file{});

		auto buff = std::wstring(static_cast<std::size_t>(size), '\0');
		if (!::MultiByteToWideChar(CP_UTF8, 0, path, -1, buff.data(), buff.size())) [[unlikely]]
			return (err = {static_cast<int>(::GetLastError()), std::system_category()}, system_file{});
		else
			return open(buff.c_str(), mode, prot, err);
	}
	system_file system_file::open(const wchar_t *path, int mode, int, std::error_code &err) noexcept
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

		const auto hnd = ::CreateFileW(path, access, share, nullptr, disp, flags, nullptr);
		if (hnd == INVALID_HANDLE_VALUE) [[unlikely]] return (err = {static_cast<int>(::GetLastError()), std::system_category()}, system_file{});

#if 0   /* TODO: Set file permissions if created a new file. */
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
		}
#endif

		auto off = std::numeric_limits<std::size_t>::max();
#if SIZE_MAX >= UINT64_MAX
		LONG high = 0;
		if (const auto res = ::SetFilePointer(hnd, 0, &high, FILE_CURRENT); res != INVALID_SET_FILE_POINTER)
			[[likely]] off = (static_cast<std::size_t>(res) | (static_cast<std::size_t>(high) << std::numeric_limits<LONG>::digits));
		else
			err = {static_cast<int>(::GetLastError()), std::system_category()};
#else
		if (const auto res = ::SetFilePointer(hnd, 0, nullptr, FILE_CURRENT); res == INVALID_SET_FILE_POINTER)
			[[unlikely]] err = {static_cast<int>(::GetLastError()), std::system_category()};
		else
			off = static_cast<std::size_t>(res);
#endif
		return system_file{hnd, off};
	}

	inline std::size_t system_file::tell_or_getptr(std::error_code &err) const noexcept
	{
		if (m_offset == std::numeric_limits<std::size_t>::max()) [[unlikely]]
		{
#if SIZE_MAX >= UINT64_MAX
			LONG high = 0;
			if (const auto res = ::SetFilePointer(native_handle(), 0, &high, FILE_CURRENT); res != INVALID_SET_FILE_POINTER) [[likely]]
				return (err = {}, static_cast<std::size_t>(res) | (static_cast<std::size_t>(high) << std::numeric_limits<LONG>::digits));
			else
				return (err = {static_cast<int>(::GetLastError()), std::system_category()}, m_offset);
#else
			if (const auto res = ::SetFilePointer(native_handle(), 0, nullptr, FILE_CURRENT); res == INVALID_SET_FILE_POINTER) [[unlikely]]
				return (err = {static_cast<int>(::GetLastError()), std::system_category()}, m_offset);
			else
				return (err = {}, static_cast<std::size_t>(res));
#endif
		}
		return m_offset;
	}

	std::size_t system_file::tell(std::error_code &err) noexcept
	{
		return m_offset = tell_or_getptr(err);
	}
	std::size_t system_file::seek(std::ptrdiff_t off, int dir, std::error_code &err) noexcept
	{
#if SIZE_MAX >= UINT64_MAX
		auto high = static_cast<LONG>(off >> std::numeric_limits<LONG>::digits);
		if (const auto res = ::SetFilePointer(native_handle(), static_cast<LONG>(off), &high, dir); res != INVALID_SET_FILE_POINTER) [[likely]]
			return (err = {}, m_offset = (static_cast<std::size_t>(res) | (static_cast<std::size_t>(high) << std::numeric_limits<LONG>::digits)));
		else
			return (err = {static_cast<int>(::GetLastError()), std::system_category()}, 0);
#else
		if (const auto res = ::SetFilePointer(native_handle(), static_cast<LONG>(off), nullptr, dir); res == INVALID_SET_FILE_POINTER) [[unlikely]]
			return (err = {static_cast<int>(::GetLastError()), std::system_category()}, 0);
		else
			return (err = {}, m_offset = static_cast<std::size_t>(res));
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
		if (const auto pos = tell(err); !err) [[likely]]
			return sync_read_at(dst, n, pos, err);
		else
			return 0;
	}
	std::size_t system_file::sync_read_at(void *dst, std::size_t n, std::size_t off, std::error_code &err) noexcept
	{
		return 0;
	}
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

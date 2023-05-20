/*
 * Created by switchblade on 2023-05-18.
 */

#include "file.hpp"

#ifdef __unix__

#include <unistd.h>
#include <fcntl.h>

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::io::detail
{
	descriptor_handle::~descriptor_handle() { ::close(m_fd); }
	std::error_code descriptor_handle::close() noexcept
	{
		if (::close(std::exchange(m_fd, -1))) [[unlikely]]
			return std::make_error_code(static_cast<std::errc>(errno));
		else
			return {};
	}

	std::size_t descriptor_handle::read(void *dst, std::size_t n, std::error_code &err) noexcept
	{
		if (const auto res = ::read(m_fd, dst, n); res < 0) [[unlikely]]
			return (err = std::make_error_code(static_cast<std::errc>(errno)), 0);
		else
			return (err = {}, static_cast<std::size_t>(res));
	}
	std::size_t descriptor_handle::write(const void *src, std::size_t n, std::error_code &err) noexcept
	{
		if (const auto res = ::write(m_fd, src, n); res < 0) [[likely]]
			return (err = std::make_error_code(static_cast<std::errc>(errno)), 0);
		else
			return (err = {}, static_cast<std::size_t>(res));
	}

	std::size_t descriptor_handle::seek(std::ptrdiff_t off, int dir, std::error_code &err) noexcept
	{
#if PTRDIFF_MAX >= INT64_MAX || SIZE_MAX >= UINT64_MAX
		const auto res = ::lseek64(m_fd, static_cast<off64_t>(off), dir);
#else
		const auto res = ::lseek(m_fd, static_cast<off_t>(off), dir);
#endif
		if (res < 0) [[likely]]
			return (err = std::make_error_code(static_cast<std::errc>(errno)), 0);
		else
			return (err = {}, static_cast<std::size_t>(res));
	}
	std::size_t descriptor_handle::tell(std::error_code &err) const noexcept
	{
#if SIZE_MAX >= UINT64_MAX
		const auto res = ::lseek64(m_fd, 0, SEEK_CUR);
#else
		const auto res = ::lseek(m_fd, 0, SEEK_CUR);
#endif
		if (res < 0) [[likely]]
			return (err = std::make_error_code(static_cast<std::errc>(errno)), 0);
		else
			return (err = {}, static_cast<std::size_t>(res));
	}

	static inline int native_flags(int flags) noexcept
	{
		int result = O_CREAT | O_CLOEXEC;
		switch (flags & (openmode::in | openmode::out))
		{
			case openmode::out | openmode::in: result |= O_RDWR; break;
			case openmode::out: result |= O_WRONLY; break;
			default: result |= O_RDONLY; break;
		}

		if (flags & openmode::app) result |= O_APPEND;
		if (flags & openmode::trunc) result |= O_TRUNC;
		if (flags & openmode::direct) result |= O_DIRECT;
		if (flags & openmode::noreplace) result |= O_EXCL;
		/* openmode::binary & openmode::ate has no use here. */
		return result;
	}

	native_file native_file::open(const char *path, int mode, std::error_code &err) noexcept
	{
		constexpr auto perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
		const auto fd = ::open(path, native_flags(mode), perms);
		if (fd < 0) [[unlikely]] goto fail;

		if ((mode & openmode::ate) && ::lseek(fd, 0, SEEK_END) < 0) [[unlikely]]
		{
			::close(fd);
			goto fail;
		}

		return (err = {}, native_file{fd});
	fail:
		return (err = std::make_error_code(static_cast<std::errc>(errno)), native_file{});
	}
	native_file native_file::reopen(native_handle_type other_fd, int mode, std::error_code &err) noexcept
	{
		const auto new_fd = ::fcntl(other_fd, F_DUPFD, 0);
		if (new_fd < 0) [[unlikely]] goto fail;

		/* Make sure to close the file if we fail to update flags. */
		if (::fcntl(new_fd, F_SETFD, native_flags(mode)) < 0) [[unlikely]]
		{
			::close(new_fd);
			goto fail;
		}
		if ((mode & openmode::ate) && ::lseek(new_fd, 0, SEEK_END) < 0) [[unlikely]]
		{
			::close(new_fd);
			goto fail;
		}
		return (err = {}, native_file{new_fd});
	fail:
		return (err = std::make_error_code(static_cast<std::errc>(errno)), native_file{});
	}
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

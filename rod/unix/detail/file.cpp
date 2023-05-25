/*
 * Created by switchblade on 2023-05-18.
 */

#include "file.hpp"

#include <fcntl.h>

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	static inline int native_flags(int flags) noexcept
	{
		int result = O_CREAT | O_CLOEXEC;
		switch (flags & (openmode::in | openmode::out))
		{
			case openmode::out | openmode::in: result |= O_RDWR;
				break;
			case openmode::out: result |= O_WRONLY;
				break;
			default: result |= O_RDONLY;
				break;
		}

		if (flags & openmode::app) result |= O_APPEND;
		if (flags & openmode::trunc) result |= O_TRUNC;
		if (flags & openmode::direct) result |= O_DIRECT;
		if (flags & openmode::noreplace) result |= O_EXCL;
		/* Async files must use O_NONBLOCK. */
		if (flags & openmode::_async) result |= O_NONBLOCK;
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
		return (err = errno_code(), native_file{});
	}
	native_file native_file::reopen(native_handle_type other_fd, int mode, std::error_code &err) noexcept
	{
		const auto new_fd = ::fcntl(other_fd, F_DUPFD, 0);
		if (new_fd < 0) [[unlikely]] goto fail;

		/* Make sure to close the file if we fail to update flags. */
		if (::fcntl(new_fd, F_SETFD, O_CLOEXEC)) [[unlikely]]
		{
			::close(new_fd);
			goto fail;
		}
		if (::fcntl(new_fd, F_SETFL, native_flags(mode))) [[unlikely]]
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
		return (err = errno_code(), native_file{});
	}

	std::error_code native_file::flush() noexcept
	{
		if (::fsync(native_handle())) [[unlikely]]
			return errno_code();
		else
			return {};
	}
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

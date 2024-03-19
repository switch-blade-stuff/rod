/*
 * Created by switchblade on 2023-05-20.
 */

#include "handle_base.hpp"

namespace rod
{
	namespace _unix
	{
		result<native_handle_type> link_file(native_handle_type hnd, const path_handle &base, path_view path, bool replace, bool check_stat, const file_timeout &to) noexcept
		{
			const auto new_base = base.native_handle() ? base.native_handle() : basic_handle::native_handle_type(AT_FDCWD);
			const auto new_leaf = path.render_null_terminated();
			int last_err;

#ifdef AT_EMPTY_PATH
			if (::linkat(hnd, "", new_base, new_leaf.c_str(), AT_EMPTY_PATH) < 0) [[unlikely]]
				last_err = errno;
			else
				return hnd;
#ifdef __linux__
			if (last_err == ENOENT) /* Missing CAP_DAC_READ_SEARCH will cause ENOENT with AT_EMPTY_PATH. */
			{
				char link[64];
				::snprintf(link, 64, "/proc/self/fd/%d", hnd.value);

				if (::linkat(AT_FDCWD, link, new_base, new_leaf.c_str(), AT_SYMLINK_FOLLOW) < 0) [[unlikely]]
					last_err = errno;
				else
					return hnd;
			}
#endif
			if (last_err == EEXIST && !replace) [[unlikely]]
				return std::error_code(last_err, std::system_category());
#endif

			auto old_leaf = std::string();
			auto old_base = get_fd_base(hnd, &old_leaf, check_stat, to);
			if (old_base.has_error()) [[unlikely]]
				return old_base.error();

			if (::linkat(old_base->native_handle(), old_leaf.c_str(), new_base, new_leaf.c_str(), 0) < 0) [[unlikely]]
				last_err = errno;
			else
				return hnd;

			/* Handle cases where we need to replace an existing entry. */
			if (struct ::stat st = {}; replace && last_err == EEXIST && ::fstatat(new_base, new_leaf.c_str(), &st, AT_SYMLINK_NOFOLLOW) >= 0)
			{
				/* If the file is a directory, use AT_REMOVEDIR. otherwise, use unlinkat. */
				if (::unlinkat(new_base, new_leaf.c_str(), (st.st_mode & S_IFMT) == S_IFDIR ? AT_REMOVEDIR : 0) < 0) [[unlikely]]
					last_err = errno;
				else
					return hnd;
			}
			return std::error_code(last_err, std::system_category());
		}
		result<native_handle_type> relink_file(native_handle_type hnd, const path_handle &base, path_view path, bool replace, bool check_stat, const file_timeout &to) noexcept
		{
			const auto new_base = base.native_handle() ? base.native_handle() : basic_handle::native_handle_type(AT_FDCWD);
			const auto new_leaf = path.render_null_terminated();

			auto old_leaf = std::string();
			auto old_base = get_fd_base(hnd, &old_leaf, check_stat, to);
			if (old_base.has_error()) [[unlikely]]
				return old_base.error();

			if (!replace)
			{
#ifdef __linux__
				if (::renameat2(old_base->native_handle(), old_leaf.c_str(), new_base, new_leaf.c_str(), RENAME_NOREPLACE) >= 0) [[likely]]
					return hnd;
				if (const auto err = errno; err == EEXIST) [[unlikely]]
					return std::error_code(err, std::system_category());
#endif
				if (::linkat(old_base->native_handle(), old_leaf.c_str(), new_base, new_leaf.c_str(), 0) < 0) [[unlikely]]
					return std::error_code(errno, std::system_category());
#if defined(__APPLE__) && defined(__MACH__)
				if (::unlinkat(old_base->native_handle(), old_leaf.c_str(), 0) < 0) [[unlikely]]
					return std::error_code(errno, std::system_category());
#endif

				auto last_err = 0;
				if (const auto old_attr = ::fcntl(hnd, F_GETFL); old_attr >= 0) [[likely]]
				{
					if (const auto new_hnd = ::openat(new_base, new_leaf.c_str(), old_attr); new_hnd >= 0) [[likely]]
						::close(std::exchange(hnd, {new_hnd, hnd.flags}));
					last_err = errno;
				}
#if !(defined(__APPLE__) && defined(__MACH__))
				if (::unlinkat(old_base->native_handle(), old_leaf.c_str(), 0) < 0) [[unlikely]]
					last_err = errno;
#endif
				if (last_err != 0) [[unlikely]]
					return std::error_code(last_err, std::system_category());
				else
					return hnd;
			}
			if (::renameat(old_base->native_handle(), old_leaf.c_str(), new_base, new_leaf.c_str()) < 0) [[unlikely]]
				return std::error_code(errno, std::system_category());
			else
				return hnd;
		}
		result<native_handle_type> unlink_file(native_handle_type hnd, bool rmdir, bool check_stat, const file_timeout &to) noexcept
		{
			auto old_leaf = std::string();
			auto old_base = get_fd_base(hnd, &old_leaf, check_stat, to);
			if (old_base.has_error()) [[unlikely]]
				return old_base.error();

			if (::unlinkat(old_base->native_handle(), old_leaf.c_str(), rmdir ? AT_REMOVEDIR : 0) < 0) [[unlikely]]
				return std::error_code(errno, std::system_category());
			else
				return hnd;
		}
	}

	namespace _handle
	{
		auto basic_handle::do_close() noexcept -> result<>
		{
			if (is_open() && ::close(release()) != 0) [[unlikely]]
				return std::error_code(errno, std::system_category());
			else
				return {};
		}
		auto basic_handle::do_clone() const noexcept -> result<basic_handle>
		{
			if (!is_open()) [[unlikely]]
				return {};

#ifdef F_DUPFD_CLOEXEC
			if (const auto hnd = ::fcntl(_hnd.value, F_DUPFD_CLOEXEC, 0); hnd >= 0) [[likely]]
				return hnd;
			else if (const auto err = errno; err != EBADF) [[unlikely]]
				return std::error_code(err, std::system_category());
#endif

			if (const auto hnd = ::fcntl(_hnd.value, F_DUPFD, 0); hnd >= 0) [[likely]]
			{
				const auto flags = ::fcntl(_hnd.value, F_GETFL);
				if (flags != -1 && ::fcntl(_hnd.value, F_SETFL, flags | FD_CLOEXEC) != -1) [[likely]]
					return hnd;
			}
			return std::error_code(errno, std::system_category());
		}
	}
}

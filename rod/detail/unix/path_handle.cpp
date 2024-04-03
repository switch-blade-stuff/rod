/*
 * Created by switchblade on 2023-08-09.
 */

#include "path_handle.hpp"

namespace rod::_path
{
	result<path_handle> path_handle::open(const path_handle &base, path_view path) noexcept
	{
		try
		{
			const auto base_hnd = base.is_open() ? base.native_handle() : native_handle_type(AT_FDCWD);
			const auto rpath = (path.empty() && base.is_open() ? "." : path).render_null_terminated();
			auto fd_flags = O_CLOEXEC | O_RDONLY;
#ifdef O_DIRECTORY
			fd_flags |= O_DIRECTORY;
#endif
#ifdef O_PATH
			fd_flags |= O_PATH;
#endif

			if (const auto hnd = ::openat(base_hnd, rpath.c_str(), fd_flags); hnd < 0) [[unlikely]]
				return std::error_code(errno, std::system_category());
			else
				return path_handle(hnd);
		}
		catch (...) { return _detail::current_error(); }
	}
}

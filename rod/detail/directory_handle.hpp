/*
 * Created by switch_blade on 2023-09-01.
 */

#pragma once

#include "fs_handle_base.hpp"
#include "path_handle.hpp"

namespace rod
{
	namespace _path
	{
		/** @brief Handle to a unique directory within the filesystem.
		 *
		 * `directory_handle` works as an extended version of `path_handle` (and is a subclass of it),
		 * providing additional support for directory creation, enumeration, and filesystem queries. */
		class directory_handle : public fs_handle_adaptor<directory_handle, path_handle>
		{
			friend fs_handle_adaptor<directory_handle, path_handle>;
			friend handle_adaptor<directory_handle, path_handle>;

			using adaptor_base = fs_handle_adaptor<directory_handle, path_handle>;

		public:
			using native_handle_type = typename adaptor_base::native_handle_type;

			/** Integer type used for handle offsets. */
			using extent_type = typename adaptor_base::extent_type;
			/** Integer type used for handle buffers. */
			using size_type = typename adaptor_base::size_type;

			/** Opens or creates a directory specified by \a path.
			 *
			 * @note The following values of \a flags are not supported:
			 * <ul>
			 * <li>`unlink_on_close`</li>
			 * <li>`no_sparse_files`</li>
			 * </ul>
			 * @note The following values of \a mode are not supported:
			 * <ul>
			 * <li>`truncate`</li>
			 * <li>`supersede`</li>
			 * </ul>
			 *
			 * @param path Path to the new directory.
			 * @param flags Handle flags to open the handle with.
			 * @param mode Mode to use when opening or creating the handle.
			 * @return Handle to the directory or a status code on failure. */
			[[nodiscard]] static result<directory_handle> open(path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::existing) noexcept { return open({}, path, flags, mode); }
			/**  Opens or creates a directory specified by \a path relative to parent location \a base.
			 *
			 * @note The following values of \a flags are not supported:
			 * <ul>
			 * <li>`unlink_on_close`</li>
			 * <li>`no_sparse_files`</li>
			 * </ul>
			 * @note The following values of \a mode are not supported:
			 * <ul>
			 * <li>`truncate`</li>
			 * <li>`supersede`</li>
			 * </ul>
			 *
			 * @param base Handle to the parent location.
			 * @param path Path to the new directory relative to \a base.
			 * @param flags Handle flags to open the handle with.
			 * @param mode Mode to use when opening or creating the handle.
			 * @return Handle to the directory or a status code on failure. */
			[[nodiscard]] static ROD_API_PUBLIC result<directory_handle> open(const path_handle &base, path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::existing) noexcept;

			/** Creates a new uniquely-named directory relative to parent location \a base.
			 *
			 * @note The following values of \a flags are not supported:
			 * <ul>
			 * <li>`unlink_on_close`</li>
			 * <li>`no_sparse_files`</li>
			 * </ul>
			 *
			 * @param base Handle to the parent location.
			 * @param flags Handle flags to open the handle with.
			 * @return Handle to the directory or a status code on failure. */
			[[nodiscard]] static ROD_API_PUBLIC result<directory_handle> open_unique(const path_handle &base, file_flags flags = file_flags::read | file_flags::write) noexcept;
			/** Opens or creates a directory specified by \a path relative to the system's temporary file directory.
			 *
			 * @note If \a path is empty, generates an implementation-defined unique name.
			 * @note The following values of \a flags are not supported:
			 * <ul>
			 * <li>`unlink_on_close`</li>
			 * <li>`no_sparse_files`</li>
			 * </ul>
			 * @note The following values of \a mode are not supported:
			 * <ul>
			 * <li>`truncate`</li>
			 * <li>`supersede`</li>
			 * </ul>
			 *
			 * @param flags Handle flags to open the handle with.
			 * @param mode Mode to use when opening or creating the handle.
			 * @return Handle to the directory or a status code on failure. */
			[[nodiscard]] static ROD_API_PUBLIC result<directory_handle> open_temporary(path_view path = {}, file_flags flags = file_flags::read | file_flags::write, open_mode mode = open_mode::always) noexcept;

		private:
			directory_handle(path_handle &&other, file_flags flags) noexcept : adaptor_base(std::forward<path_handle>(other))
			{
				auto hnd = adaptor_base::release();
				hnd.flags = std::uint32_t(flags);
				adaptor_base::release(hnd);
			}

		public:
			directory_handle(const directory_handle &) = delete;
			directory_handle &operator=(const directory_handle &) = delete;

			/** Initializes a closed directory handle. */
			directory_handle() noexcept = default;
			directory_handle(directory_handle &&) noexcept = default;
			directory_handle &operator=(directory_handle &&) noexcept = default;

			/** Initializes directory handle from a native handle. */
			explicit directory_handle(native_handle_type hnd) noexcept : adaptor_base(hnd) {}
			/** Initializes directory handle from a native handle and explicit device & inode IDs. */
			explicit directory_handle(native_handle_type hnd, dev_t dev, ino_t ino) noexcept : adaptor_base(hnd, dev, ino) {}

			[[nodiscard]] constexpr operator const path_handle &() const & noexcept { return adaptor_base::base(); }
			[[nodiscard]] constexpr operator const path_handle &&() const && noexcept { return std::move(adaptor_base::base()); }

			/** Returns the flags of the directory handle. */
			[[nodiscard]] file_flags flags() const noexcept { return static_cast<file_flags>(adaptor_base::native_handle().flags); }

			constexpr void swap(directory_handle &other) noexcept { adaptor_base::swap(other); }
			friend constexpr void swap(directory_handle &a, directory_handle &b) noexcept { a.swap(b); }

		private:
			result<directory_handle> do_clone() const noexcept
			{
				if (auto res = clone(base()); res.has_value())
					return directory_handle(std::move(*res), flags());
				else
					return res.error();
			}

#ifdef ROD_WIN32
			/* Win32 directory handles are not created with DELETE permission and need to be re-opened for link operations to work. */
			ROD_API_PUBLIC result<basic_handle> reopen_as_deletable() const noexcept;

			result<> do_link(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
			{
				if (auto hnd = reopen_as_deletable(); hnd.has_value()) [[likely]]
					return _handle::do_link(hnd->native_handle(), base, path, replace, to);
				else
					return hnd.error();
			}
			result<> do_relink(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept
			{
				if (auto hnd = reopen_as_deletable(); hnd.has_value()) [[likely]]
					return _handle::do_relink(hnd->native_handle(), base, path, replace, to);
				else
					return hnd.error();
			}
			result<> do_unlink(const file_timeout &to) noexcept
			{
				if (auto hnd = reopen_as_deletable(); hnd.has_value()) [[likely]]
					return _handle::do_unlink(hnd->native_handle(), to, flags());
				else
					return hnd.error();
			}
#else
			result<> do_link(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept { return _handle::do_link(adaptor_base::native_handle(), base, path, replace, to); }
			result<> do_relink(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept { return _handle::do_relink(adaptor_base::native_handle(), base, path, replace, to); }
			result<> do_unlink(const file_timeout &to) noexcept { return _handle::do_unlink(adaptor_base::native_handle(), to, flags()); }
#endif

		};

		static_assert(std::convertible_to<const directory_handle &, const path_handle &>);
		static_assert(fs_handle<directory_handle>);
	}

	using _path::directory_handle;
}

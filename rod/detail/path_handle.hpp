/*
 * Created by switch_blade on 2023-08-08.
 */

#pragma once

#include "fs_status_code.hpp"
#include "path_view.hpp"

#if defined(ROD_WIN32)
#include "../win32/detail/path.hpp"
#else
#error Path handle not implemented
#endif

namespace rod::fs
{
	namespace _path
	{
		template<typename BaseHandle>
		class basic_path_handle;
	}

#if defined(ROD_WIN32)
	/** Lightweight handle to a unique directory within the filesystem. */
	using path_handle = _path::basic_path_handle<_win32::path_handle>;
#endif

	/** @overload Opens a path handle for an existing leaf directory specified by \a path. */
	[[nodiscard]] inline static result<path_handle> open_path(path_view path) noexcept;
	/** @overload Opens a path handle for an existing leaf directory specified by \a path relative to parent location referenced by \a base. */
	[[nodiscard]] inline static result<path_handle> open_path(const path_handle &base, path_view path) noexcept;

	namespace _path
	{
		template<typename BaseHandle>
		class basic_path_handle : BaseHandle
		{
			friend inline result<path_handle> fs::open_path(path_view) noexcept;
			friend inline result<path_handle> fs::open_path(const path_handle &, path_view) noexcept;

		public:
			using native_handle_type = typename BaseHandle::native_handle_type;

		private:
			[[nodiscard]] static result<basic_path_handle> open(path_view path) noexcept { return result<basic_path_handle>(BaseHandle::open(path)); }
			[[nodiscard]] static result<basic_path_handle> open(const basic_path_handle &base, path_view path) noexcept { return result<basic_path_handle>(BaseHandle::open(base, path)); }

		public:
			basic_path_handle(const basic_path_handle &) = delete;
			basic_path_handle &operator=(const basic_path_handle &) = delete;

			/** Initializes a closed path handle. */
			basic_path_handle() noexcept = default;
			basic_path_handle(basic_path_handle &&) noexcept = default;
			basic_path_handle &operator=(basic_path_handle &&) noexcept = default;

			/** Initializes path handle from a native handle. */
			constexpr explicit basic_path_handle(native_handle_type handle) noexcept : BaseHandle(handle) {}
			constexpr explicit basic_path_handle(BaseHandle &&base) noexcept : BaseHandle(std::forward<BaseHandle>(base)) {}

			/** Closes the path handle. */
			result<> close() noexcept { return BaseHandle::close(); }

			/** Returns path of the location referenced to by the handle.
			 * @note Underlying location may not have a valid path, in which case an empty path is returned. */
			[[nodiscard]] result<fs::path> path() const noexcept { return BaseHandle::path(); }
			/** Duplicates path handle. This function is used instead of a copy constructor because cloning a system handle is an expensive operation and may fail. */
			[[nodiscard]] result<basic_path_handle> clone() const noexcept { return result<basic_path_handle>(BaseHandle::clone()); }

			/** Checks if the path handle is open (references a valid filesystem location). */
			[[nodiscard]] bool is_open() const noexcept { return BaseHandle::is_open(); }
			/** Returns the underlying native handle. */
			[[nodiscard]] native_handle_type native_handle() const noexcept { return BaseHandle::native_handle(); }

			/** Releases the underlying native handle. */
			[[nodiscard]] native_handle_type release() noexcept { return BaseHandle::release(); }
			/** Releases the underlying native handle and replaces it with \a handle. */
			[[nodiscard]] native_handle_type release(native_handle_type handle) noexcept { return BaseHandle::release(handle); }

			constexpr void swap(basic_path_handle &other) noexcept { BaseHandle::swap(other); }
			friend constexpr void swap(basic_path_handle &a, basic_path_handle &b) noexcept { a.swap(b); }

			[[nodiscard]] friend bool operator==(const basic_path_handle &, const basic_path_handle &b) noexcept = default;
			[[nodiscard]] friend bool operator!=(const basic_path_handle &, const basic_path_handle &b) noexcept = default;
		};
	}

	result<path_handle> open_path(path_view path) noexcept { return path_handle::open(path); }
	result<path_handle> open_path(const path_handle &base, path_view path) noexcept { return path_handle::open(base, path); }
}

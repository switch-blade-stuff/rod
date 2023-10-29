/*
 * Created by switchblade on 2023-08-08.
 */

#pragma once

#include "handle_base.hpp"

namespace rod
{
	namespace _path
	{
		/** Lightweight handle to a unique directory within the filesystem. */
		class path_handle : public handle_adaptor<path_handle>
		{
			using adp_base = handle_adaptor<path_handle>;
			friend adp_base;

		public:
			/** Opens a path handle for an existing directory specified by \a path relative to parent location \a base.
			 * @param base Handle to the parent location. If set to an invalid handle, \a path must be a fully-qualified path.
			 * @param path Path to the target directory relative to \a base if it is a valid handle, otherwise a fully-qualified path.
			 * @return Handle to the directory or a status code on failure. */
			[[nodiscard]] static ROD_API_PUBLIC result<path_handle> open(const path_handle &base, path_view path) noexcept;

		public:
			path_handle(const path_handle &) = delete;
			path_handle &operator=(const path_handle &) = delete;

			/** Initializes a closed path handle. */
			path_handle() noexcept = default;
			path_handle(path_handle &&) noexcept = default;
			path_handle &operator=(path_handle &&) noexcept = default;

			/** Initializes path handle from a native handle. */
			explicit path_handle(typename adp_base::native_handle_type hnd) noexcept : adp_base(hnd) {}
			/** Initializes path handle from a basic handle. */
			explicit path_handle(basic_handle &&other) noexcept : adp_base(std::forward<basic_handle>(other)) {}

			constexpr void swap(path_handle &other) noexcept { adp_base::swap(other); }
			friend constexpr void swap(path_handle &a, path_handle &b) noexcept { a.swap(b); }

			friend constexpr bool operator==(const path_handle &, const path_handle &) noexcept = default;
			friend constexpr bool operator!=(const path_handle &, const path_handle &) noexcept = default;
		};

		static_assert(handle<path_handle>);
	}

	namespace fs { using _path::path_handle; }
}

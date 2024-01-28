/*
 * Created by switchblade on 2023-08-08.
 */

#pragma once

#include "handle_stat.hpp"

namespace rod
{
	namespace _path
	{
		/** Lightweight handle to a unique directory within the filesystem. */
		class path_handle : public handle_adaptor<path_handle>
		{
			using adaptor = handle_adaptor<path_handle>;
			friend adaptor;

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
			explicit path_handle(typename adaptor::native_handle_type hnd) noexcept : adaptor(hnd) {}
			/** Initializes path handle from a basic handle. */
			explicit path_handle(basic_handle &&other) noexcept : adaptor(std::forward<basic_handle>(other)) {}

			constexpr void swap(path_handle &other) noexcept { adaptor::swap(other); }
			friend constexpr void swap(path_handle &a, path_handle &b) noexcept { a.swap(b); }

			friend constexpr bool operator==(const path_handle &, const path_handle &) noexcept = default;
			friend constexpr bool operator!=(const path_handle &, const path_handle &) noexcept = default;

		public:
			template<std::same_as<get_stat_t> T>
			friend auto tag_invoke(T, stat &st, const path_handle &hnd, stat::query q) noexcept { return _handle::do_get_stat(st, hnd.native_handle(), q); }
			template<std::same_as<set_stat_t> T>
			friend auto tag_invoke(T, const stat &st, path_handle &hnd, stat::query q) noexcept { return _handle::do_set_stat(st, hnd.native_handle(), q); }
			template<std::same_as<fs::get_fs_stat_t> T>
			friend auto tag_invoke(T, fs::fs_stat &st, const path_handle &hnd, fs::fs_stat::query q) noexcept { return _handle::do_get_fs_stat(st, hnd.native_handle(), q); }

			template<std::same_as<fs::to_object_path_t> T>
			friend auto tag_invoke(T, const path_handle &hnd) noexcept { return _path::do_to_object_path(hnd.native_handle()); }
			template<std::same_as<fs::to_native_path_t> T>
			friend auto tag_invoke(T, const path_handle &hnd, fs::native_path_format fmt, fs::dev_t dev, fs::ino_t ino) noexcept { return _path::do_to_native_path(hnd.native_handle(), fmt, dev, ino); }
		};

		static_assert(handle<path_handle>);
	}

	namespace fs { using _path::path_handle; }
}

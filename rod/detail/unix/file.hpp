/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#ifdef __unix__

#include <cstdio>

#include "../file_fwd.hpp"
#include "descriptor.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	class native_file : public descriptor_handle
	{
	public:
		using native_handle_type = int;

		static ROD_PUBLIC native_file open(const char *path, int mode, std::error_code &err) noexcept;
		static ROD_PUBLIC native_file reopen(native_handle_type other_fd, int mode, std::error_code &err) noexcept;

		constexpr native_file() = default;

		constexpr explicit native_file(native_handle_type fd) noexcept : descriptor_handle(fd) {}
		constexpr explicit native_file(descriptor_handle &&fd) noexcept : descriptor_handle(std::move(fd)) {}

		ROD_PUBLIC std::error_code flush() noexcept;

		constexpr void swap(native_file &other) noexcept { descriptor_handle::swap(other); }
		friend constexpr void swap(native_file &a, native_file &b) noexcept { a.swap(b); }
	};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

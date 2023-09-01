/*
 * Created by switch_blade on 2023-07-30.
 */

#include "../path_base.hpp"
#include "ntapi.hpp"

namespace rod::_path
{
	result<std::size_t> wide_to_multibyte_with_default(std::uint32_t codepage, const wchar_t *src, std::size_t n_src, char *dst, std::size_t n_dst) noexcept
	{
		if (n_src > INT_MAX || n_dst > INT_MAX) [[unlikely]]
			return std::make_error_code(std::errc::invalid_argument);

		auto len = ::WideCharToMultiByte(codepage, WC_NO_BEST_FIT_CHARS, src, int(n_src), dst, int(n_dst), nullptr, nullptr);
		if (len == 0 && (len = ::WideCharToMultiByte(codepage, 0, src, int(n_src), dst, int(n_dst), nullptr, nullptr)) == 0) [[unlikely]]
			return std::error_code(int(::GetLastError()), std::system_category());
		else
			return len;
	}
	result<std::size_t> wide_to_multibyte(std::uint32_t codepage, const wchar_t *src, std::size_t n_src, char *dst, std::size_t n_dst) noexcept
	{
		if (n_src > INT_MAX || n_dst > INT_MAX) [[unlikely]]
			return std::make_error_code(std::errc::invalid_argument);

		std::error_code err;
		std::size_t dst_len;
		if (codepage == codepage_utf8 || codepage == codepage_gb18030)
			dst_len = ::WideCharToMultiByte(codepage, WC_ERR_INVALID_CHARS, src, int(n_src), dst, int(n_dst), nullptr, nullptr);
		else
		{
			BOOL invalid = false;
			dst_len = ::WideCharToMultiByte(codepage, WC_NO_BEST_FIT_CHARS, src, int(n_src), dst, int(n_dst), nullptr, &invalid);
			if (invalid) [[unlikely]]
				err = std::error_code(ERROR_NO_UNICODE_TRANSLATION, std::system_category());
		}

		if (!dst_len && !err && (err = std::error_code(int(::GetLastError()), std::system_category())).value() == ERROR_INVALID_FLAGS)
		{
			dst_len = ::WideCharToMultiByte(codepage, 0, src, int(n_src), dst, int(n_dst), nullptr, nullptr);
			if (dst_len == 0) [[unlikely]]
				err = std::error_code(int(::GetLastError()), std::system_category());
			else
				err = {};
		}
		if (!err) [[likely]]
			return dst_len;
		else
			return err;
	}
	result<std::size_t> multibyte_to_wide(std::uint32_t codepage, const char *src, std::size_t n_src, wchar_t *dst, std::size_t n_dst) noexcept
	{
		if (n_src > INT_MAX || n_dst > INT_MAX) [[unlikely]]
			return std::make_error_code(std::errc::invalid_argument);

		if (const auto dst_len = ::MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS, src, int(n_src), dst, int(n_dst)); dst_len == 0) [[unlikely]]
			return std::error_code(int(::GetLastError()), std::system_category());
		else
			return static_cast<std::size_t>(dst_len);
	}
	std::uint32_t system_codepage() noexcept { return ::GetACP(); }

	path from_binary(std::span<const std::byte> data)
	{
		if (data.size() == sizeof(GUID))
		{
			wchar_t buff[39];
			const auto n = ::StringFromGUID2(*reinterpret_cast<const GUID *>(data.data()), buff, 39);
			return path(std::wstring_view(buff, n - 1), path::binary_format);
		}

		/* Assume data is a native wide string unless it's size is not a multiple of wchar_t. */
		if (data.size() % sizeof(wchar_t))
			return from_multibyte(data);
		else
			return from_wide(data);
	}
}

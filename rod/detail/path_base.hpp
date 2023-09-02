/*
 * Created by switch_blade on 2023-07-30.
 */

#pragma once

#ifdef _MSC_VER
#define _SILENCE_CXX20_CODECVT_FACETS_DEPRECATION_WARNING
#endif

#include <string_view>
#include <algorithm>
#include <iomanip>
#include <iosfwd>
#include <locale>
#include <array>
#include <span>

#include "../result.hpp"
#include "../hash.hpp"

namespace rod
{
	namespace _path
	{
		class path;
		class path_view;
		class path_view_component;
		struct path_view_like;

		template<typename T>
		concept accepted_char = one_of<std::decay_t<T>, char, wchar_t, char8_t, char16_t, char32_t>;

		template<typename T>
		struct source_string;
		template<typename T> requires std::is_pointer_v<T>
		struct source_string<T> { using type = std::remove_pointer_t<T>; };
		template<typename T> requires std::ranges::forward_range<T>
		struct source_string<T> { using type = std::ranges::range_value_t<T>; };

		template<typename T>
		using source_string_t = typename source_string<T>::type;

		template<typename T>
		struct is_accepted_source : std::false_type {};
		template<typename T> requires std::is_pointer_v<T> || std::ranges::forward_range<T>
		struct is_accepted_source<T> : std::bool_constant<accepted_char<source_string_t<T>>> {};

		template<typename T>
		concept accepted_source = !decays_to_same<T, path> && is_accepted_source<std::decay_t<T>>::value;

		template<typename F, typename T>
		concept resize_func =  std::invocable<F, T *, std::size_t> && std::assignable_from<T *&, std::invoke_result_t<F, T *, std::size_t>>;
		template<typename F, typename T>
		concept noexcept_resize_func = std::is_nothrow_invocable_v<F, T *, std::size_t>;

		template<typename SrcChar, typename DstChar, resize_func<DstChar> Resize> requires(sizeof(SrcChar) == sizeof(DstChar))
		inline static result<std::size_t> cvt(const SrcChar *src, std::size_t n_src, DstChar *dst, std::size_t n_dst, Resize resize) noexcept(noexcept_resize_func<Resize, DstChar>)
		{
			static_cast_copy(src, src + n_src, resize(dst, n_src));
			return n_src;
		}

		struct cvt_status
		{
			std::size_t n_src = 0;
			std::size_t n_dst = 0;
			bool done = false;
		};

		ROD_API_PUBLIC result<cvt_status> utf16_to_utf32(const char16_t *src_first, const char16_t *src_last, char32_t *dst_first, char32_t *dst_last) noexcept;
		ROD_API_PUBLIC result<cvt_status> utf32_to_utf16(const char32_t *src_first, const char32_t *src_last, char16_t *dst_first, char16_t *dst_last) noexcept;

		template<std::same_as<char16_t> SrcChar, std::same_as<char32_t> DstChar, resize_func<DstChar> Resize>
		inline static result<std::size_t> cvt(const SrcChar *src, std::size_t n_src, DstChar *dst, std::size_t n_dst, Resize resize) noexcept(noexcept_resize_func<Resize, DstChar>)
		{
			if (n_dst < n_src)
			{
				dst = resize(dst, n_src);
				n_dst = n_src;
			}

			auto src_pos = src;
			auto dst_pos = dst;
			for (;;)
			{
				const auto src_first = src_pos, src_last = src + n_src;
				const auto dst_first = dst_pos, dst_last = dst + n_dst;
				auto status = utf16_to_utf32(src_first, src_last, dst_first, dst_last);
				if (status.has_error()) [[unlikely]]
					return status.error();
				else if (status->done) [[likely]]
					return status->n_dst;

				src_pos += status->n_src;
				dst_pos += status->n_dst;
				if (dst_pos == dst_last)
				{
					const auto pos = dst - dst_pos;
					dst = resize(dst, n_dst * 2);
					dst_pos = dst + pos;
				}
			}
		}
		template<std::same_as<char32_t> SrcChar, std::same_as<char16_t> DstChar, resize_func<DstChar> Resize>
		inline static result<std::size_t> cvt(const SrcChar *src, std::size_t n_src, DstChar *dst, std::size_t n_dst, Resize resize) noexcept(noexcept_resize_func<Resize, DstChar>)
		{
			if (n_dst < n_src)
			{
				dst = resize(dst, n_src);
				n_dst = n_src;
			}

			auto src_pos = src;
			auto dst_pos = dst;
			for (;;)
			{
				const auto src_first = src_pos, src_last = src + n_src;
				const auto dst_first = dst_pos, dst_last = dst + n_dst;
				auto status = utf32_to_utf16(src_first, src_last, dst_first, dst_last);
				if (status.has_error()) [[unlikely]]
					return status.error();
				else if (status->done) [[likely]]
					return status->n_dst;

				src_pos += status->n_src;
				dst_pos += status->n_dst;
				if (dst_pos == dst_last)
				{
					const auto pos = dst - dst_pos;
					dst = resize(dst, n_dst * 2);
					dst_pos = dst + pos;
				}
			}
		}


#ifdef ROD_WIN32
		static constexpr std::uint32_t codepage_gb18030 = 54936;
		static constexpr std::uint32_t codepage_utf8 = 65001;

		ROD_API_PUBLIC result<std::size_t> wide_to_multibyte_with_default(std::uint32_t codepage, const wchar_t *src, std::size_t n_src, char *dst, std::size_t n_dst) noexcept;
		ROD_API_PUBLIC result<std::size_t> wide_to_multibyte(std::uint32_t codepage, const wchar_t *src, std::size_t n_src, char *dst, std::size_t n_dst) noexcept;
		ROD_API_PUBLIC result<std::size_t> multibyte_to_wide(std::uint32_t codepage, const char *src, std::size_t n_src, wchar_t *dst, std::size_t n_dst) noexcept;
		ROD_API_PUBLIC std::uint32_t system_codepage() noexcept;

		template<one_of<char, char8_t> SrcChar, one_of<wchar_t, char16_t> DstChar, resize_func<DstChar> Resize>
		inline static result<std::size_t> cvt(const SrcChar *src, std::size_t n_src, DstChar *dst, std::size_t n_dst, Resize resize) noexcept(noexcept_resize_func<Resize, DstChar>)
		{
			std::uint32_t codepage;
			if constexpr (!std::same_as<SrcChar, char8_t>)
				codepage = system_codepage();
			else
				codepage = codepage_utf8;

			if (auto len = multibyte_to_wide(codepage, reinterpret_cast<const char *>(src), n_src, reinterpret_cast<wchar_t *>(dst), n_dst); len.has_value()) [[likely]]
				dst = resize(dst, n_dst = *len);
			else
				return len;
			return multibyte_to_wide(codepage, reinterpret_cast<const char *>(src), n_src, reinterpret_cast<wchar_t *>(dst), n_dst);
		}
		template<one_of<wchar_t, char16_t> SrcChar, one_of<char, char8_t> DstChar, resize_func<DstChar> Resize>
		inline static result<std::size_t> cvt(const SrcChar *src, std::size_t n_src, DstChar *dst, std::size_t n_dst, Resize resize) noexcept(noexcept_resize_func<Resize, DstChar>)
		{
			std::uint32_t codepage;
			if constexpr (!std::same_as<SrcChar, char8_t>)
				codepage = system_codepage();
			else
				codepage = codepage_utf8;

			if (auto len = wide_to_multibyte(codepage, reinterpret_cast<const wchar_t *>(src), n_src, reinterpret_cast<char *>(dst), n_dst); len.has_value()) [[likely]]
				dst = resize(dst, n_dst = *len);
			else
				return len;
			return wide_to_multibyte(codepage, reinterpret_cast<const wchar_t *>(src), n_src, reinterpret_cast<char *>(dst), n_dst);
		}
#endif

#if WCHAR_MAX < INT32_MAX
		template<std::same_as<wchar_t> SrcChar, std::same_as<char32_t> DstChar, resize_func<DstChar> Resize>
		inline static result<std::size_t> cvt(const SrcChar *src, std::size_t n_src, DstChar *dst, std::size_t n_dst, Resize resize) noexcept(noexcept_resize_func<Resize, DstChar>)
		{
			return cvt(reinterpret_cast<const char16_t *>(src), n_src, dst, n_dst, resize);
		}
		template<std::same_as<char32_t> SrcChar, std::same_as<wchar_t> DstChar, resize_func<DstChar> Resize>
		inline static result<std::size_t> cvt(const SrcChar *src, std::size_t n_src, DstChar *dst, std::size_t n_dst, Resize resize) noexcept(noexcept_resize_func<Resize, DstChar>)
		{
			return cvt(src, n_src, reinterpret_cast<char16_t *>(dst), n_dst, [=](auto...args) { return reinterpret_cast<char16_t *>(resize(args...)); });
		}
#else
		template<std::same_as<char16_t> SrcChar, std::same_as<wchar_t> DstChar, resize_func<DstChar> Resize>
		inline static result<std::size_t> cvt(const SrcChar *src, std::size_t n_src, DstChar *dst, std::size_t n_dst, Resize resize) noexcept(noexcept_resize_func<Resize, DstChar>)
		{
			return cvt(src, n_src, reinterpret_cast<char32_t *>(dst), n_dst, [=](auto...args) { return reinterpret_cast<char32_t *>(resize(args...)); });
		}
		template<std::same_as<wchar_t> SrcChar, std::same_as<char16_t> DstChar, resize_func<DstChar> Resize>
		inline static result<std::size_t> cvt(const SrcChar *src, std::size_t n_src, DstChar *dst, std::size_t n_dst, Resize resize) noexcept(noexcept_resize_func<Resize, DstChar>)
		{
			return cvt(reinterpret_cast<const char32_t *>(src), n_src, dst, n_dst, resize);
		}
#endif

		template<typename SrcChar, typename DstChar, typename State, resize_func<DstChar> Resize>
		inline static result<std::size_t> cvt(const std::codecvt<SrcChar, DstChar, State> &cvt, const SrcChar *src, std::size_t n_src, DstChar *dst, std::size_t n_dst, Resize resize) noexcept(noexcept_resize_func<Resize, DstChar>)
		{
			if (const auto n = cvt.encoding(); n > 0)
				dst = resize(dst, n_dst = n_src * n);
			else if (n_dst == 0)
				dst = resize(dst, n_dst = 1);

			State state = State();
			auto src_next = src;
			auto dst_next = dst;
			for (;;)
			{
				const auto src_first = src_next, src_last = src + n_src;
				const auto dst_first = dst_next, dst_last = dst + n_dst;
				const auto status = cvt.out(state, src_first, src_last, src_next, dst_first, dst_last, dst_next);
				if (status == std::codecvt_base::ok)
					return dst_next - dst_first;
				else if (status == std::codecvt_base::noconv)
				{
					static_cast_copy(src, src + n_src, resize(dst, n_src));
					return n_src;
				}
				else if (status == std::codecvt_base::partial)
				{
					if ((src_next == src_last && dst_next != dst_last)) [[unlikely]]
						return std::make_error_code(std::errc::invalid_argument);

					const auto pos = dst_next - dst;
					dst = resize(dst, n_dst * 2);
					dst_next = dst + pos;
					continue;
				}
				return std::make_error_code(std::errc::invalid_argument);
			}
		}
		template<typename SrcChar, typename DstChar, typename State, resize_func<DstChar> Resize>
		inline static result<std::size_t> cvt(const std::codecvt<DstChar, SrcChar, State> &cvt, const SrcChar *src, std::size_t n_src, DstChar *dst, std::size_t n_dst, Resize resize) noexcept(noexcept_resize_func<Resize, DstChar>)
		{
			if (const auto n = cvt.encoding(); n > 0)
				dst = resize(dst, n_dst = n_src / n);
			else if (n_dst == 0)
				dst = resize(dst, n_dst = 1);

			State state = State();
			auto src_next = src;
			auto dst_next = dst;
			for (;;)
			{
				const auto src_first = src_next, src_last = src + n_src;
				const auto dst_first = dst_next, dst_last = dst + n_dst;
				const auto status = cvt.in(state, src_first, src_last, src_next, dst_first, dst_last, dst_next);
				if (status == std::codecvt_base::ok)
					return dst_next - dst_first;
				else if (status == std::codecvt_base::noconv)
				{
					static_cast_copy(src, src + n_src, resize(dst, n_src));
					return n_src;
				}
				else if (status == std::codecvt_base::partial)
				{
					if ((src_next == src_last && dst_next != dst_last)) [[unlikely]]
						return std::make_error_code(std::errc::invalid_argument);

					const auto pos = dst_next - dst;
					dst = resize(dst, n_dst * 2);
					dst_next = dst + pos;
					continue;
				}
				return std::make_error_code(std::errc::invalid_argument);
			}
		}

		template<one_of<char, wchar_t, char16_t, char32_t> SrcChar, one_of<char, char8_t> DstChar>
		inline static auto &use_codecvt(const std::locale &loc) { return use_facet<std::codecvt<SrcChar, DstChar, std::mbstate_t>>(loc); }
		template<one_of<char, char8_t> SrcChar, one_of<char, wchar_t, char16_t, char32_t> DstChar>
		inline static auto &use_codecvt(const std::locale &loc) { return use_facet<std::codecvt<DstChar, SrcChar, std::mbstate_t>>(loc); }

		template<typename SrcChar, typename DstChar, resize_func<DstChar> Resize>
		inline static result<std::size_t> cvt(const std::locale &loc, const SrcChar *src, std::size_t n_src, DstChar *dst, std::size_t n_dst, Resize resize) noexcept(noexcept_resize_func<Resize, DstChar>)
		{
			if constexpr (requires { use_codecvt<SrcChar, DstChar>(loc); })
				return cvt(use_codecvt<SrcChar, DstChar>(loc), src, n_src, dst, n_dst, resize);
			else
				return cvt(src, n_src, dst, n_dst, resize);
		}
		template<typename SrcChar, typename DstChar, resize_func<DstChar> Resize>
		inline static result<std::size_t> cvt(const SrcChar *src, std::size_t n_src, DstChar *dst, std::size_t n_dst, Resize resize) noexcept(noexcept_resize_func<Resize, DstChar>)
		{
			return cvt(std::locale(), src, n_src, dst, n_dst, resize);
		}

#ifdef ROD_WIN32
		using value_type = wchar_t;
#else
		using value_type = char;
#endif

		using string_view_type = std::basic_string_view<value_type>;
		using string_type = std::basic_string<value_type>;
		using size_type = typename string_type::size_type;

		template<typename Src, typename C = std::decay_t<source_string_t<std::decay_t<Src>>>>
		inline constexpr auto to_string_buffer(Src &&src) noexcept((std::is_pointer_v<std::decay_t<Src>> || std::ranges::contiguous_range<Src>) && std::same_as<C, value_type>)
		{
			if constexpr (std::is_pointer_v<std::decay_t<Src>>)
				return std::basic_string_view<C>(src, rod::_detail::strlen(src));
			else if constexpr (std::ranges::contiguous_range<std::decay_t<Src>>)
				return std::basic_string_view<C>(std::ranges::begin(src), std::ranges::end(src));
			else
			{
				std::basic_string<C> buff;
				if constexpr (std::ranges::random_access_range<std::decay_t<Src>>)
					buff.reserve(std::ranges::size(src));
				for (auto pos = std::ranges::begin(src); pos != std::ranges::end(src) && *pos != C{}; ++pos)
					buff.push_back(*pos);

				return buff;
			}
		}
		template<typename C = value_type, typename Traits = std::char_traits<C>, typename Alloc = std::allocator<C>, typename Src>
		inline static std::basic_string<C, Traits, Alloc> to_native_string(Src &&src, const std::locale &loc, const Alloc &alloc = Alloc())
		{
			if constexpr (decays_to_same<Src, std::basic_string<C, Traits, Alloc>> || decays_to_same<Src, std::basic_string_view<C, Traits>>)
				return std::basic_string<C, Traits, Alloc>(std::forward<Src>(src));
			else
			{
				auto dst_buff = std::basic_string<C, Traits, Alloc>(alloc);
				if constexpr (!std::is_pointer_v<std::decay_t<Src>>)
					if (std::ranges::empty(src)) return dst_buff;

				auto src_buff = to_string_buffer(std::forward<Src>(src));
				if (src_buff.empty()) return dst_buff;

				const auto n = cvt(loc, src_buff.data(), src_buff.size(), dst_buff.data(), 0, [&](auto, std::size_t n)
				{
					dst_buff.resize(n, '\0');
					return dst_buff.data();
				});
				dst_buff.resize(n.value());
				return dst_buff;
			}
		}
		template<typename C = value_type, typename Traits = std::char_traits<C>, typename Alloc = std::allocator<C>, typename Src>
		inline static std::basic_string<C, Traits, Alloc> to_native_string(Src &&src, const Alloc &alloc = Alloc())
		{
			if constexpr (decays_to_same<Src, std::basic_string<C, Traits, Alloc>> || decays_to_same<Src, std::basic_string_view<C, Traits>>)
				return std::basic_string<C, Traits, Alloc>(std::forward<Src>(src));
			else
			{
				auto dst_buff = std::basic_string<C, Traits, Alloc>(alloc);
				if constexpr (!std::is_pointer_v<std::decay_t<Src>>)
					if (std::ranges::empty(src)) return dst_buff;

				auto src_buff = to_string_buffer(std::forward<Src>(src));
				if (src_buff.empty()) return dst_buff;

				const auto n = cvt(src_buff.data(), src_buff.size(), dst_buff.data(), 0, [&](auto, std::size_t n)
				{
					dst_buff.resize(n, '\0');
					return dst_buff.data();
				});
				dst_buff.resize(n.value());
				return dst_buff;
			}
		}

		enum class format_type : std::uint8_t
		{
			auto_format = 0,
			native_format = 1,
			binary_format = 2,
			generic_format = 3,
			unknown_format = 4,
		};

		struct constants
		{
			/** Automatic path formatting mode, path format will be deduced at runtime. */
			static constexpr auto auto_format = format_type::auto_format;
			/** Native path formatting mode, path manipulation functions and queries will use native parsing rules. */
			static constexpr auto native_format = format_type::native_format;
			/** Binary path formatting mode, path manipulation functions and queries will be treated as no-ops. */
			static constexpr auto binary_format = format_type::binary_format;
			/** Generic path formatting mode, path manipulation functions and queries will use generic parsing rules. */
			static constexpr auto generic_format = format_type::generic_format;
			/** Unknown path formatting mode, path manipulation functions and queries will use implementation-defined rules (usually equivalent to `auto_format`). */
			static constexpr auto unknown_format = format_type::unknown_format;

#ifdef ROD_WIN32
			/** Preferred separator character of the current system. */
			static constexpr value_type preferred_separator = L'\\';
#else
			/** Preferred separator character of the current system. */
			static constexpr value_type preferred_separator = '/';
#endif

#if defined(FILENAME_MAX)
			/** Default size of the in-place rendered path buffer. */
			static constexpr size_type default_buffer_size = std::max(1024, FILENAME_MAX);
#elif defined(PATH_MAX)
			/** Default size of the in-place rendered path buffer. */
			static constexpr size_type default_buffer_size = std::max(1024, PATH_MAX);
#elif defined(MAX_PATH)
			/** Default size of the in-place rendered path buffer. */
			static constexpr size_type default_buffer_size = std::max(1024, MAX_PATH);
#else
			/** Default size of the in-place rendered path buffer. */
			static constexpr size_type default_buffer_size = 1024;
#endif
		};

		template<typename C = value_type>
		inline constexpr std::size_t lfind_separator(std::basic_string_view<C> path, format_type fmt, std::size_t idx = 0) noexcept
		{
			switch (fmt)
			{
			case constants::binary_format:
				return std::basic_string_view<C>::npos;
			default:
				return path.find_first_of('/', idx);
#ifdef ROD_WIN32
			case constants::generic_format:
				return path.find_first_of('/', idx);
			case constants::native_format:
				return path.find_first_of('\\', idx);
			case constants::unknown_format:
			case constants::auto_format:
				return path.find_first_of(std::basic_string_view(std::array<C, 2>{'\\', '/'}.data(), 2), idx);
#endif
			}
		}
		template<typename C = value_type>
		inline constexpr std::size_t rfind_separator(std::basic_string_view<C> path, format_type fmt, std::size_t idx = std::basic_string_view<C>::npos) noexcept
		{
			switch (fmt)
			{
			case constants::binary_format:
				return std::basic_string_view<C>::npos;
			default:
				return path.find_last_of('/', idx);
#ifdef ROD_WIN32
			case constants::generic_format:
				return path.find_last_of('/', idx);
			case constants::native_format:
				return path.find_last_of('\\', idx);
			case constants::unknown_format:
			case constants::auto_format:
				return path.find_last_of(std::basic_string_view(std::array<C, 2>{'\\', '/'}.data(), 2), idx);
#endif
			}
		}
		template<typename C = value_type>
		inline constexpr bool is_separator(C ch, format_type fmt) noexcept
		{
			switch (fmt)
			{
			default:
				return ch == '/';
			case constants::binary_format:
				return false;
#ifdef ROD_WIN32
			case constants::native_format:
				return ch == '\\';
			case constants::unknown_format:
			case constants::auto_format:
				return ch == '/' || ch == '\\';
#endif
			}
		}

#ifdef ROD_WIN32
		template<typename C = value_type>
		inline constexpr bool is_drive_letter(C ch) noexcept
		{
			constexpr C case_mask = ' ';
			constexpr C drive_max = 'Z';
			return (ch & ~case_mask) <= drive_max;
		}
		template<typename C = value_type>
		inline constexpr bool is_unc_drive_path(std::basic_string_view<C> path, format_type fmt)
		{
			return path.size() >= 6 && is_separator(path[0], fmt) && is_separator(path[1], fmt) && path[2] == '?' && is_separator(path[3], fmt) && path[5] == ':' && is_drive_letter(path[4]);
		}

		[[nodiscard]] inline static bool has_illegal_path_sequences(std::wstring_view sv) noexcept
		{
			constexpr std::wstring_view reserved_names[] = {L"CON", L"PRN", L"AUX", L"NUL"};
			constexpr std::wstring_view reserved_chars = L"\"*/:<>?|";

			if (sv.size() > 260)
				return true;
			for (std::size_t i = 7; i < sv.size(); ++i)
			{
				/* Test for control characters. */
				if (sv[i] >= 1 && sv[i] <= 31)
					return true;
				/* Test for reserved characters. */
				for (std::size_t j = 0; j < sizeof(reserved_chars); ++j)
					if (sv[i] == reserved_chars[j]) return true;
			}

			/* Special case for COM# */
			if (sv.find(L"COM") != std::wstring_view::npos)
				return true;
			/* Special case for LPT# */
			if (sv.find(L"LPT") != std::wstring_view::npos)
				return true;
			/* Test for other reserved names. */
			return std::ranges::any_of(reserved_names, [&](const auto &name)
			{
				if (sv.find(name) != std::wstring_view::npos)
					return true;
				if (sv.ends_with(name.substr(0, name.size() - 1)))
					return true;
				return false;
			});
		}

		template<typename C = value_type>
		inline constexpr std::size_t namespace_prefix_size(std::basic_string_view<C> path, format_type fmt) noexcept
		{
			if (path.size() >= 4 && is_separator(path[3], fmt) && (path.size() == 4 || !is_separator(path[4], fmt)))
			{
				/* \\.\, \\?\, \??\, \!!\ */
				if (is_separator(path[1], fmt) && (path[2] == '.' || path[2] == '?') ||
					(path[1] == '?' && path[2] == '?') ||
					(path[1] == '!' && path[2] == '!'))
					return 4;
			}
			return 0;
		}
		template<typename C = value_type>
		inline constexpr std::size_t hostname_prefix_size(std::basic_string_view<C> path, format_type fmt) noexcept
		{
			if (path.size() >= 3 && is_separator(path[1], fmt) && !is_separator(path[2], fmt))
			{
				const auto pos = lfind_separator(path, fmt, 3);
				if (pos == std::basic_string_view<C>::npos)
					return path.size();
				else
					return pos;
			}
			return 0;
		}
		template<typename C = value_type>
		inline constexpr std::size_t root_name_size(std::basic_string_view<C> path, format_type fmt) noexcept
		{
			if (fmt != constants::binary_format && path.size() >= 2)
			{
				/* Drive absolute or relative path. */
				if (path[1] == ':' && is_drive_letter(path[0]))
					return 2;

				/* Paths starting with a separator. */
				if (is_separator(path[0], fmt))
				{
					/* Win32 & NT namespace prefixes. */
					if (const auto size = namespace_prefix_size(path, fmt); size)
						return size;
					/* \\hostname path */
					if (const auto size = hostname_prefix_size(path, fmt); size)
						return size;
				}
			}
			return 0;
		}
#else
		template<typename C = value_type>
		inline constexpr std::size_t namespace_prefix_size(std::basic_string_view<C>, format_type) noexcept { return 0; }
		template<typename C = value_type>
		inline constexpr std::size_t root_name_size(std::basic_string_view<C>, format_type) noexcept { return 0; }
#endif

		template<typename Value, typename BaseIter, typename NextFunc, typename PrevFunc>
		class component_iterator
		{
		public:
			using value_type = Value;
			using pointer = const value_type *;
			using reference = const value_type &;

			using difference_type = typename std::iterator_traits<BaseIter>::difference_type;
			using iterator_category = std::input_iterator_tag;

		private:
			static constexpr bool nothrow_next = std::is_nothrow_invocable_v<NextFunc, reference, BaseIter, value_type &>;
			static constexpr bool nothrow_prev = std::is_nothrow_invocable_v<PrevFunc, reference, BaseIter, value_type &>;

		public:
			constexpr component_iterator() noexcept(std::is_nothrow_default_constructible_v<value_type>) = default;

			constexpr explicit component_iterator(const value_type &comp, pointer base, BaseIter pos) noexcept(std::is_nothrow_copy_constructible_v<value_type>) : _comp(comp), _base(base), _pos(pos) {}
			constexpr explicit component_iterator(value_type &&comp, pointer  base, BaseIter pos) noexcept(std::is_nothrow_move_constructible_v<value_type>) : _comp(std::move(comp)), _base(base), _pos(pos) {}

			constexpr component_iterator &operator++() noexcept(nothrow_next)
			{
				NextFunc{}(_comp, *_base, _pos);
				return *this;
			}
			constexpr component_iterator operator++(int) noexcept(nothrow_next)
			{
				auto tmp = *this;
				operator++();
				return tmp;
			}

			constexpr component_iterator &operator--() noexcept(nothrow_prev)
			{
				PrevFunc{}(_comp, *_base, _pos);
				return *this;
			}
			constexpr component_iterator operator--(int) noexcept(nothrow_prev)
			{
				auto tmp = *this;
				operator--();
				return tmp;
			}

			[[nodiscard]] constexpr pointer operator->() const noexcept { return &_comp; }
			[[nodiscard]] constexpr reference operator*() const noexcept { return _comp; }

			[[nodiscard]] friend constexpr bool operator==(const component_iterator &a, const component_iterator &b) noexcept { return a._pos == b._pos; }

			constexpr void swap(component_iterator &other) noexcept
			{
				adl_swap(_comp, other._comp);
				adl_swap(_base, other._base);
				adl_swap(_pos, other._pos);
			}
			friend constexpr void swap(component_iterator &a, component_iterator &b) noexcept { a.swap(b); }

		private:
			value_type _comp = {};
			pointer _base = {};
			BaseIter _pos = {};
		};

		template<typename C = value_type>
		inline constexpr std::basic_string_view<C> iter_next(std::basic_string_view<C> comp, std::basic_string_view<C> base, const C *&pos, format_type fmt) noexcept
		{
			if (pos += comp.size(); pos == base.data())
			{
				const auto pred = [&](auto ch) { return is_separator(ch, fmt); };
				const auto root_name_end = base.data() + root_name_size(base, fmt);
				const auto root_path_end = std::find_if_not(root_name_end, base.data() + base.size(), pred);

				if (root_name_end != base.data() && root_name_end != root_path_end)
					return {root_name_end, root_path_end};
			}
			if (is_separator(*pos, fmt) && comp.empty())
			{
				pos += 1;
				return {};
			}

			if (pos == base.data() + base.size())
				return {};

			for (; is_separator(*pos, fmt); ++pos)
				if (pos + 1 == base.data() + base.size())
				{
					pos += 1;
					return {};
				}

			if (const auto end = lfind_separator(base, fmt, pos - base.data()); end == std::basic_string_view<C>::npos)
				return {pos, base.size() - (pos - base.data())};
			else
				return {pos, base.data() + end};
		}
		template<typename C = value_type>
		inline constexpr std::basic_string_view<C> iter_prev(std::basic_string_view<C> comp, std::basic_string_view<C> base, const C *&pos, format_type fmt) noexcept
		{
			const auto pred = [&](auto ch) { return is_separator(ch, fmt); };
			const auto root_name_end = base.data() + root_name_size(base, fmt);
			const auto root_path_end = std::find_if_not(root_name_end, base.data() + base.size(), pred);

			/* Root directory component. */
			if (root_name_end != root_path_end && pos == root_path_end)
				return {pos = root_name_end, root_path_end};
			/* Root name component. */
			if (root_name_end != base.data() && pos == root_name_end)
				return {pos = base.data(), root_name_end};

			/* Relative path component. */
			if (pos == base.data() + base.size() && is_separator(pos[-1], fmt))
			{
				pos -= 1;
				return {};
			}

			while (pos != root_path_end && is_separator(pos[-1], fmt))
				--pos;
			const auto end = pos;
			while (pos != root_path_end && !is_separator(pos[-1], fmt))
				--pos;

			return {pos, end};
		}
		template<typename C = value_type>
		inline constexpr std::size_t iter_begin(std::basic_string_view<C> base, format_type fmt) noexcept
		{
			const auto root_name_end = base.data() + root_name_size(base, fmt);
			if (base.data() == root_name_end)
			{
				const auto pred = [&](auto ch) { return is_separator(ch, fmt); };
				const auto root_path_end = std::find_if_not(root_name_end, base.data() + base.size(), pred);

				if (base.data() == root_path_end)
					return std::find_if(root_path_end, base.data() + base.size(), pred) - base.data();
				else
					return root_path_end - base.data();
			}
			return root_name_end - base.data();
		}

		template<typename C = value_type>
		inline constexpr int compare(std::basic_string_view<C> a_str, format_type a_fmt, std::basic_string_view<C> b_str, format_type b_fmt) noexcept
		{
			const auto a_root_name_end = a_str.begin() + root_name_size(a_str, a_fmt);
			const auto b_root_name_end = b_str.begin() + root_name_size(b_str, b_fmt);

			/* End comparison if name strings compare unequal. */
			const auto a_root_name = std::basic_string_view<C>(a_str.begin(), a_root_name_end);
			const auto b_root_name = std::basic_string_view<C>(b_str.begin(), b_root_name_end);
			if (const auto cmp = a_root_name.compare(b_root_name); cmp != 0)
				return cmp;

			const auto a_pred = [&](auto ch) { return is_separator(ch, a_fmt); };
			const auto b_pred = [&](auto ch) { return is_separator(ch, b_fmt); };
			const auto a_root_path_end = std::find_if_not(a_root_name_end, a_str.end(), a_pred);
			const auto b_root_path_end = std::find_if_not(b_root_name_end, b_str.end(), b_pred);

			/* End comparison if root directory separators are unequal. */
			const auto a_root_dir = std::basic_string_view<C>(a_root_name_end, a_root_path_end);
			const auto b_root_dir = std::basic_string_view<C>(b_root_name_end, b_root_path_end);
			if (const auto a_empty = a_root_dir.empty(), b_empty = b_root_dir.empty(); a_empty != b_empty)
				return a_empty - b_empty;

			/* Compare element-wise. */
			for (auto a_pos = a_root_path_end, b_pos = b_root_path_end;;)
			{
				/* End comparison if path string lengths are unequal. */
				const auto a_at_end = a_pos == a_str.end();
				const auto b_at_end = b_pos == b_str.end();
				if (a_at_end || b_at_end) return a_at_end - b_at_end;

				/* End comparison if component string lengths are unequal. */
				const auto a_is_sep = is_separator(*a_pos, a_fmt);
				const auto b_is_sep = is_separator(*b_pos, b_fmt);
				if (a_is_sep != b_is_sep) return a_is_sep - b_is_sep;

				/* Compare directory separators or component characters. */
				if (a_is_sep)
				{
					a_pos = std::find_if_not(std::next(a_pos), a_str.end(), a_pred);
					b_pos = std::find_if_not(std::next(b_pos), b_str.end(), b_pred);
				}
				else if (const auto cmp = *a_pos - *b_pos; cmp != 0)
					return cmp;
				else
				{
					a_pos = std::next(a_pos);
					b_pos = std::next(b_pos);
				}
			}
		}
		inline constexpr int compare_bytes(std::span<const std::uint8_t> a_bytes, std::span<const std::uint8_t> b_bytes) noexcept
		{
			if (const auto cmp = static_cast<std::ptrdiff_t>(a_bytes.size() - b_bytes.size()); cmp)
				return int(cmp);

			if (!std::is_constant_evaluated())
				return std::memcmp(a_bytes.data(), b_bytes.data(), a_bytes.size());
			else
			{
				for (std::size_t i = 0; i < a_bytes.size(); ++i)
				{
					const auto cmp = static_cast<std::int8_t>(a_bytes[i] - b_bytes[i]);
					if (cmp) return cmp;
				}
				return 0;
			}
		}

		template<typename C = value_type>
		inline constexpr std::size_t hash_string(std::basic_string_view<C> path, format_type fmt) noexcept
		{
			fnv1a_builder builder;
			const auto root_name_end = path.begin() + root_name_size(path, fmt);
			builder.write(path.data(), root_name_end - path.begin());

			bool skip_separator = false;
			for (auto pos = root_name_end; pos != path.end(); ++pos)
			{
				if (!is_separator(*pos, fmt))
				{
					skip_separator = false;
					builder.write(*pos);
				}
				else if (!skip_separator)
				{
					builder.write(constants::preferred_separator);
					skip_separator = true;
				}
			}
			return builder.finish();
		}

		template<typename C = value_type>
		inline constexpr std::size_t find_relative_path(std::basic_string_view<C> path, format_type fmt) noexcept
		{
			const auto pred = [&](auto ch) { return is_separator(ch, fmt); };
			const auto root_name_end = path.begin() + root_name_size(path, fmt);
			const auto root_path_end = std::find_if_not(root_name_end, path.end(), pred);
			return static_cast<std::size_t>(std::distance(path.begin(), root_path_end));
		}
		template<typename C = value_type>
		inline constexpr std::size_t parent_path_size(std::basic_string_view<C> path, format_type fmt) noexcept
		{
			const auto min = find_relative_path(path, fmt);
			std::size_t i = path.size();
			/* Remove filename & last directory separators. */
			while (i != min && !is_separator(path[i - 1], fmt))
				--i;
			/* Remove trailing separators. */
			while (i != min && is_separator(path[i - 1], fmt))
				--i;
			return i;
		}

		template<typename C = value_type>
		inline constexpr bool is_absolute(std::basic_string_view<C> path, format_type fmt) noexcept
		{
			/* Always treat binary format as absolute. */
			if (fmt == constants::binary_format)
				return true;

#ifdef ROD_WIN32
			/* Check if the path is an absolute drive path (i.e. C:\path instead of C:path). */
			if (path.size() >= 2 && path[1] == ':' && is_drive_letter(path[0]))
				return path.size() > 2 && is_separator(path[2], fmt);
#endif
			/* All absolute paths start with a separator (i.e. /dir, //hostname/dir, etc.). */
			return !path.empty() && is_separator(path[0], fmt);
		}
		template<typename C = value_type>
		inline constexpr bool contains_root(std::basic_string_view<C> path, format_type fmt) noexcept
		{
			for (auto pos = find_relative_path(path, fmt); pos < path.size();)
			{
				auto end = lfind_separator(path, fmt, pos);
				if (root_name_size(path.substr(pos, end), fmt))
					return true;

				while (end < path.size() && is_separator(path[end], fmt))
					++end;
				pos = end;
			}
			return false;
		}

		template<typename C = value_type>
		inline constexpr std::basic_string_view<C> root_namespace_substr(std::basic_string_view<C> path, format_type fmt) noexcept
		{
			return path.substr(0, namespace_prefix_size(path, fmt));
		}
		template<typename C = value_type>
		inline constexpr std::basic_string_view<C> root_directory_substr(std::basic_string_view<C> path, format_type fmt) noexcept
		{
			const auto pred = [&](auto ch) { return is_separator(ch, fmt); };
			const auto root_name_end = path.begin() + root_name_size(path, fmt);
			const auto root_path_end = std::find_if_not(root_name_end, path.end(), pred);
			return std::basic_string_view<C>(root_name_end, root_path_end);
		}
		template<typename C = value_type>
		inline constexpr std::basic_string_view<C> root_path_substr(std::basic_string_view<C> path, format_type fmt) noexcept
		{
			return path.substr(0, find_relative_path(path, fmt));
		}
		template<typename C = value_type>
		inline constexpr std::basic_string_view<C> root_name_substr(std::basic_string_view<C> path, format_type fmt) noexcept
		{
			return path.substr(0, root_name_size(path, fmt));
		}

		template<typename C = value_type>
		inline constexpr std::basic_string_view<C> relative_path_substr(std::basic_string_view<C> path, format_type fmt) noexcept
		{
			return path.substr(find_relative_path(path, fmt));
		}
		template<typename C = value_type>
		inline constexpr std::basic_string_view<C> parent_path_substr(std::basic_string_view<C> path, format_type fmt) noexcept
		{
			return path.substr(0, parent_path_size(path, fmt));
		}

		template<typename C = value_type>
		inline constexpr std::size_t find_file_name(std::basic_string_view<C> path, format_type fmt) noexcept
		{
			const auto sep = rfind_separator(path, fmt);
#ifdef ROD_WIN32
			if (sep == 2 && path.size() == 3)
			{
				if (path[1] == ':') /* Drive letter */
					return 2;
				else
					return 0;
			}
#endif
			if (sep != std::basic_string_view<C>::npos && (sep != 0 || path.size() > 1))
				return sep + 1;
			else
				return 0;
		}
		template<typename C = value_type>
		inline constexpr std::size_t file_stem_size(std::basic_string_view<C> path) noexcept
		{
#ifdef ROD_WIN32
			const auto ads_pos = path.find_first_of(':');
			if (ads_pos != std::basic_string_view<C>::npos)
				path = path.substr(0, ads_pos);
#endif

			if (const auto dot = path.rfind('.'); dot == std::basic_string_view<C>::npos || dot == 0 || (dot == 1 && path[0] != '.'))
				return path.size();
			else
				return dot;
		}

		template<typename C = value_type>
		inline constexpr std::basic_string_view<C> file_name_substr(std::basic_string_view<C> path, format_type fmt) noexcept { return path.substr(find_file_name(path, fmt)); }
		template<typename C = value_type>
		inline constexpr std::basic_string_view<C> file_stem_substr(std::basic_string_view<C> path) noexcept { return path.substr(0, file_stem_size(path)); }
		template<typename C = value_type>
		inline constexpr std::basic_string_view<C> file_ext_substr(std::basic_string_view<C> path) noexcept { return path.substr(file_stem_size(path)); }

		/** Container of a filesystem-specific path representation. */
		class path : public constants
		{
		public:
			using string_view_type = typename _path::string_view_type;
			using string_type = typename _path::string_type;
			using format_type = typename _path::format_type;
			using value_type = typename _path::value_type;
			using size_type = typename _path::size_type;

		private:
			void iter_next(const path &base, const value_type *&pos) { assign(_path::iter_next<value_type>(_string, base._string, pos, base.format())); }
			void iter_prev(const path &base, const value_type *&pos) { assign(_path::iter_prev<value_type>(_string, base._string, pos, base.format())); }

			template<void (path::*Func)(const path &, const value_type *&)>
			struct iter_func { void operator()(path &comp, const path &base, const value_type *&pos) const { (comp.*Func)(base, pos); } };

		public:
			using iterator = component_iterator<path, const value_type *, iter_func<&path::iter_next>, iter_func<&path::iter_prev>>;
			using const_iterator = iterator;
			using reverse_iterator = std::reverse_iterator<iterator>;
			using const_reverse_iterator = reverse_iterator;

		public:
			constexpr path(const path &) = default;
			constexpr path &operator=(const path &) = default;
			constexpr path(path &&) noexcept = default;
			constexpr path &operator=(path &&) noexcept = default;

			/** Initializes an empty path. */
			constexpr path() noexcept : path(auto_format) {}
			/** Initializes an empty path with format \a fmt. */
			constexpr path(format_type fmt) noexcept : _string(), _format(fmt) {}

			/** Initializes a path from C-style string \a str and format \a fmt. */
			constexpr path(const value_type *str, format_type fmt = auto_format) : _string(str), _format(fmt) {}
			/** Initializes a path from a move-constructed string \a str and format \a fmt. */
			constexpr path(string_type &&str, format_type fmt = auto_format) noexcept : _string(std::forward<string_type>(str)), _format(fmt) {}

			/** Initializes a path from a character range \a src and format \a fmt. */
			template<accepted_source Src>
			path(Src &&src, format_type fmt = auto_format) : path(to_native_string(std::forward<Src>(src)), fmt) {}
			/** Initializes a path from a character range \a src using locale \a loc for encoding conversion, and format \a fmt. */
			template<accepted_source Src>
			path(Src &&src, const std::locale &loc, format_type fmt = auto_format) : path(to_native_string(std::forward<Src>(src), loc), fmt) {}

			/** Initializes a path from characters in range [\a first, \a last) and format \a fmt. */
			template<std::forward_iterator I, std::sentinel_for<I> S> requires accepted_char<std::iter_value_t<I>>
			path(I first, S last, format_type fmt = auto_format) : path(to_native_string(std::ranges::subrange(first, last)), fmt) {}
			/** Initializes a path from characters in range [\a first, \a last) using locale \a loc for encoding conversion, and format \a fmt. */
			template<std::forward_iterator I, std::sentinel_for<I> S> requires accepted_char<std::iter_value_t<I>>
			path(I first, S last, const std::locale &loc, format_type fmt = auto_format) : path(to_native_string(std::ranges::subrange(first, last), loc), fmt) {}

			/** Initializes a path from an exposition-only path-view-like object \a p and format \a fmt. */
			inline explicit path(path_view_like p, format_type fmt = auto_format);
			/** Initializes a path from an exposition-only path-view-like object \a p using locale \a loc for encoding conversion, and format \a fmt. */
			inline path(path_view_like p, const std::locale &loc, format_type fmt = auto_format);

		public:
			/** Assigns path contents from C-style string \a str. */
			constexpr path &operator=(const value_type *str) noexcept { return assign(str); }
			/** Assigns path contents from C-style string \a str. */
			constexpr path &assign(const value_type *str) noexcept { return (_string = str, *this); }

			/** Move-assigns the path from string \a str. */
			constexpr path &operator=(string_type &&str) noexcept { return assign(std::forward<string_type>(str)); }
			/** Assigns path contents from string \a str. */
			constexpr path &assign(string_type &&str) noexcept { return (_string = std::forward<string_type>(str), *this); }

			/** Assigns path contents from a character range \a src. */
			template<accepted_source Src>
			constexpr path &operator=(Src &&src) { return assign(std::forward<Src>(src)); }
			/** Assigns path contents from a character range \a src. */
			template<accepted_source Src>
			constexpr path &assign(Src &&src) { return assign_native(std::forward<Src>(src)); }
			/** Assigns path contents from a character range \a src using locale \a loc for encoding conversion. */
			template<accepted_source Src>
			constexpr path &assign(Src &&src, const std::locale &loc) { return assign_native(std::forward<Src>(src), loc); }

			/** Assigns path contents from characters in range [\a first, \a last) \a src. */
			template<std::forward_iterator I, std::sentinel_for<I> S> requires accepted_char<std::iter_value_t<I>>
			constexpr path &assign(I first, S last) { return assign_native(std::ranges::subrange(first, last)); }
			/** Assigns path contents from characters in range [\a first, \a last) \a src using locale \a loc for encoding conversion. */
			template<std::forward_iterator I, std::sentinel_for<I> S> requires accepted_char<std::iter_value_t<I>>
			constexpr path &assign(I first, S last, const std::locale &loc) { return assign_native(std::ranges::subrange(first, last), loc); }

			/** Clears contents of the path. */
			constexpr void clear() noexcept { _string.clear(); }

		private:
			template<typename Src>
			constexpr path &assign_native(Src &&src)
			{
				if constexpr (!decays_to_same<source_string_t<std::decay_t<Src>>, value_type>)
					_string.assign(to_native_string(std::forward<Src>(src)));
				else
					_string.assign(to_string_buffer(std::forward<Src>(src)));
				return *this;
			}
			template<typename Src>
			constexpr path &assign_native(Src &&src, const std::locale &loc)
			{
				if constexpr (!decays_to_same<source_string_t<std::decay_t<Src>>, value_type>)
					_string.assign(to_native_string(std::forward<Src>(src), loc));
				else
					_string.assign(to_string_buffer(std::forward<Src>(src)));
				return *this;
			}

		public:
			/** Appends string representation of \a other to string representation of `this`. */
			path &operator+=(const path &other) { return operator+=(other._string); }

			/** Appends string \a str to string representation of `this`. */
			path &operator+=(const string_type &str) { return concat_native(str); }
			/** Appends string \a str to string representation of `this`. */
			path &operator+=(string_view_type str) { return concat_native(str); }

			/** Appends character \a ch to string representation of `this`. */
			path &operator+=(value_type ch) { return concat_native(string_view_type(&ch, 1)); }
			/** Appends C-style string \a str to string representation of `this`. */
			path &operator+=(const value_type *str) { return concat_native(string_view_type(str)); }

			/** Appends character range \a src to string representation of `this`. */
			template<accepted_source Src>
			path &operator+=(Src &&src) { return concat(std::forward<Src>(src)); }
			/** Appends character range \a src to string representation of `this`. */
			template<accepted_source Src>
			path &concat(Src &&src) { return concat_native(std::forward<Src>(src)); }
			/** Appends character range \a src to string representation of `this` using locale \a loc for encoding conversion. */
			template<accepted_source Src>
			path &concat(Src &&src, const std::locale &loc) { return concat_native(std::forward<Src>(src), loc); }

			/** Appends characters in range [\a first, \a last) to string representation of `this`. */
			template<std::forward_iterator I, std::sentinel_for<I> S> requires accepted_char<std::iter_value_t<I>>
			path &concat(I first, S last) { return concat_native(std::ranges::subrange(first, last)); }
			/** Appends characters in range [\a first, \a last) to string representation of `this` using locale \a loc for encoding conversion. */
			template<std::forward_iterator I, std::sentinel_for<I> S> requires accepted_char<std::iter_value_t<I>>
			path &concat(I first, S last, const std::locale &loc) { return concat_native(std::ranges::subrange(first, last), loc); }

		private:
			template<typename Src>
			path &concat_native(Src &&src)
			{
				if constexpr (!decays_to_same<source_string_t<std::decay_t<Src>>, value_type>)
					_string.append(to_native_string(std::forward<Src>(src)));
				else
					_string.append(to_string_buffer(std::forward<Src>(src)));
				return *this;
			}
			template<typename Src>
			path &concat_native(Src &&src, const std::locale &loc)
			{
				if constexpr (!decays_to_same<source_string_t<std::decay_t<Src>>, value_type>)
					_string.append(to_native_string(std::forward<Src>(src), loc));
				else
					_string.append(to_string_buffer(std::forward<Src>(src)));
				return *this;
			}

		public:
			/** Appends path \a other to `this`. */
			path &operator/=(const path &other) { return append_string(other._string, other.format()); }
			/** Appends path \a other to `this`. */
			path &append(const path &other) { return append_string(other._string, other.format()); }

			/** Appends character range \a src to `this`. */
			template<accepted_source Src>
			path &operator/=(Src &&src) { return append(std::forward<Src>(src)); }
			/** Appends character range \a src to `this`. */
			template<accepted_source Src>
			path &append(Src &&src) { return append_native(std::forward<Src>(src)); }
			/** Appends character range \a src to `this` using locale \a loc for encoding conversion. */
			template<accepted_source Src>
			path &append(Src &&src, const std::locale &loc) { return append_native(std::forward<Src>(src), loc); }

			/** Appends characters in range [\a first, \a last) to `this`. */
			template<std::forward_iterator I, std::sentinel_for<I> S> requires accepted_char<std::iter_value_t<I>>
			path &append(I first, S last) { return append_native(std::ranges::subrange(first, last)); }
			/** Appends characters in range [\a first, \a last) to `this` using locale \a loc for encoding conversion. */
			template<std::forward_iterator I, std::sentinel_for<I> S> requires accepted_char<std::iter_value_t<I>>
			path &append(I first, S last, const std::locale &loc) { return append_native(std::ranges::subrange(first, last), loc); }

		private:
			template<typename Str>
			path &append_string(Str &&str, format_type fmt = auto_format)
			{
				if (_path::is_absolute<value_type>(str, fmt))
					return operator=(std::forward<Str>(str));

				const auto this_root_name = root_name_substr<value_type>(_string, format());
				const auto other_root_name = root_name_substr<value_type>(str, fmt);
				if (!other_root_name.empty() && this_root_name != other_root_name)
					return operator=(std::forward<Str>(str));

				if (other_root_name.size() && other_root_name.size() != str.size() && is_separator(str[other_root_name.size()], fmt))
					_string.erase(this_root_name.size());
				else if (this_root_name.size() == _string.size())
				{
#ifdef ROD_WIN32
					/* Ignore drive-relative paths under windows. */
					if (this_root_name.size() > 2)
						_string.push_back(preferred_separator);
#else
					_string.push_back(preferred_separator);
#endif
				}
				else if (!is_separator(_string.back(), format()))
					_string.push_back(preferred_separator);

				/* _string now contains a valid parent path with a separator. */
				_string.append(str.data() + other_root_name.size(), str.data() + str.size());
				return *this;
			}

			template<typename Src>
			path &append_native(Src &&src)
			{
				if constexpr (!decays_to_same<source_string_t<std::decay_t<Src>>, value_type>)
					return append_string(to_native_string(std::forward<Src>(src)));
				else
					return append_string(to_string_buffer(std::forward<Src>(src)));
			}
			template<typename Src>
			path &append_native(Src &&src, const std::locale &loc)
			{
				if constexpr (!decays_to_same<source_string_t<std::decay_t<Src>>, value_type>)
					return append_string(to_native_string(std::forward<Src>(src), loc));
				else
					return append_string(to_string_buffer(std::forward<Src>(src)));
			}

		public:
			/** Returns iterator to the first sub-component of the path.
			 * @note Path iterators allocate memory for each sub-component of the path, consider using `path_view` to avoid memory allocation. */
			[[nodiscard]] iterator begin() const { return iterator(path(c_str(), c_str() + iter_begin<value_type>(_string, format()), format()), this, c_str()); }
			/** @copydoc begin */
			[[nodiscard]] iterator cbegin() const { return begin(); }

			/** Returns iterator one past the last sub-component of the path.
			 * @note Path iterators allocate memory for each sub-component of the path, consider using `path_view` to avoid memory allocation. */
			[[nodiscard]] iterator end() const { return iterator(path(format()), this, c_str() + native_size()); }
			/** @copydoc end */
			[[nodiscard]] iterator cend() const { return end(); }

			/** Returns reverse iterator to the last sub-component of the path.
			 * @note Path iterators allocate memory for each sub-component of the path, consider using `path_view` to avoid memory allocation. */
			[[nodiscard]] reverse_iterator rbegin() const { return reverse_iterator(end()); }
			/** @copydoc rbegin */
			[[nodiscard]] reverse_iterator crbegin() const { return reverse_iterator(cend()); }

			/** Returns reverse iterator one past the last sub-component of the path.
			 * @note Path iterators allocate memory for each sub-component of the path, consider using `path_view` to avoid memory allocation. */
			[[nodiscard]] reverse_iterator rend() const { return reverse_iterator(begin()); }
			/** @copydoc rend */
			[[nodiscard]] reverse_iterator crend() const { return reverse_iterator(cbegin()); }

		public:
			/** Returns the path's formatting type. */
			[[nodiscard]] constexpr format_type format() const noexcept { return _format; }

			/** Returns reference to the underlying native path string. */
			[[nodiscard]] constexpr const string_type &native() const & noexcept { return _string; }
			/** @copydoc native */
			[[nodiscard]] constexpr string_type &&native() && noexcept { return std::move(_string); }

			[[nodiscard]] operator string_type() const & { return _string; }
			[[nodiscard]] operator string_type() && { return std::move(_string); }

			/** Returns pointer to the native C-style string. Equivalent to `native().c_str()`. */
			[[nodiscard]] constexpr const value_type *c_str() const noexcept { return native().c_str(); }

			/** Checks if the path string is empty. */
			[[nodiscard]] constexpr bool empty() const noexcept  { return native_size() == 0; }
			/** Returns size of the native path string. */
			[[nodiscard]] constexpr size_type native_size() const noexcept  { return _string.size(); }

		public:
			/** Returns string representation of the path encoded in the specified character encoding.
			 * @param alloc Allocator used to allocate the resulting string. */
			template<typename Char, typename Traits = std::char_traits<Char>, typename Alloc = std::allocator<Char>>
			[[nodiscard]] std::basic_string<Char, Traits, Alloc> string(const Alloc &alloc = Alloc{}) const { return to_native_string<Char, Traits>(_string, alloc); }
			/** Returns string representation of the path encoded in the specified character encoding using locale \a loc.
			 * @param loc Locale used for conversion to the target character encoding.
			 * @param alloc Allocator used to allocate the resulting string. */
			template<typename Char, typename Traits = std::char_traits<Char>, typename Alloc = std::allocator<Char>>
			[[nodiscard]] std::basic_string<Char, Traits, Alloc> string(const std::locale &loc, const Alloc &alloc = Alloc{}) const { return to_native_string<Char, Traits>(_string, loc, alloc); }

			/** Returns string representation of the path to a multi-byte `char` string. */
			[[nodiscard]] std::string string() const { return string<char>(); }
			/** Returns string representation of the path to a multi-byte `char` string using locale \a loc. */
			[[nodiscard]] std::string string(const std::locale &loc) const { return string<char>(loc); }

			/** Returns string representation of the path as a `wchar_t` string. */
			[[nodiscard]] std::wstring wstring() const { return string<wchar_t>(); }
			/** Returns string representation of the path as a `wchar_t` string using locale \a loc. */
			[[nodiscard]] std::wstring wstring(const std::locale &loc) const { return string<wchar_t>(loc); }

			/** Returns string representation of the path as a multi-byte `char8_t` string. */
			[[nodiscard]] std::u8string u8string() const { return string<char8_t>(); }
			/** Returns string representation of the path as a multi-byte `char8_t` string using locale \a loc. */
			[[nodiscard]] std::u8string u8string(const std::locale &loc) const { return string<char8_t>(loc); }

			/** Returns string representation of the path as a `char16_t` string. */
			[[nodiscard]] std::u16string u16string() const { return string<char16_t>(); }
			/** Returns string representation of the path as a `char16_t` string using locale \a loc. */
			[[nodiscard]] std::u16string u16string(const std::locale &loc) const { return string<char16_t>(loc); }

			/** Returns string representation of the path as a `char32_t` string. */
			[[nodiscard]] std::u32string u32string() const { return string<char32_t>(); }
			/** Returns string representation of the path as a `char32_t` string using locale \a loc. */
			[[nodiscard]] std::u32string u32string(const std::locale &loc) const { return string<char32_t>(loc); }

			/** Returns generic string representation of the path encoded in the specified character encoding.
			 * @param alloc Allocator used to allocate the resulting string. */
			template<typename Char, typename Traits = std::char_traits<Char>, typename Alloc = std::allocator<Char>>
			[[nodiscard]] std::basic_string<Char, Traits, Alloc> generic_string(const Alloc &alloc = Alloc{}) const
			{
				auto str = string<Char, Traits, Alloc>(alloc);
#ifdef ROD_WIN32
				if (format() != binary_format)
					std::ranges::replace(str, '\\', '/');
#endif
				return str;
			}
			/** Returns generic string representation of the path encoded in the specified character encoding using locale \a loc.
			 * @param loc Locale used for conversion to the target character encoding.
			 * @param alloc Allocator used to allocate the resulting string. */
			template<typename Char, typename Traits = std::char_traits<Char>, typename Alloc = std::allocator<Char>>
			[[nodiscard]] std::basic_string<Char, Traits, Alloc> generic_string(const std::locale &loc, const Alloc &alloc = Alloc{}) const
			{
				auto str = string<Char, Traits, Alloc>(loc, alloc);
#ifdef ROD_WIN32
				if (format() != binary_format)
					std::ranges::replace(str, '\\', '/');
#endif
				return str;
			}

			/** Returns generic string representation of the path as a multi-byte `char` string. */
			[[nodiscard]] std::string generic_string() const { return generic_string<char>(); }
			/** Returns generic string representation of the path as a multi-byte `char` string using locale \a loc. */
			[[nodiscard]] std::string generic_string(const std::locale &loc) const { return generic_string<char>(loc); }

			/** Returns generic string representation of the path as a `wchar_t` string. */
			[[nodiscard]] std::wstring generic_wstring() const { return generic_string<wchar_t>(); }
			/** Returns generic string representation of the path as a `wchar_t` string using locale \a loc. */
			[[nodiscard]] std::wstring generic_wstring(const std::locale &loc) const { return generic_string<wchar_t>(loc); }

			/** Returns generic string representation of the path as a multi-byte `char8_t` string. */
			[[nodiscard]] std::u8string generic_u8string() const { return generic_string<char8_t>(); }
			/** Returns generic string representation of the path as a multi-byte `char8_t` string using locale \a loc. */
			[[nodiscard]] std::u8string generic_u8string(const std::locale &loc) const { return generic_string<char8_t>(loc); }

			/** Returns generic string representation of the path as a `char16_t` string. */
			[[nodiscard]] std::u16string generic_u16string() const { return generic_string<char16_t>(); }
			/** Returns generic string representation of the path as a `char16_t` string using locale \a loc. */
			[[nodiscard]] std::u16string generic_u16string(const std::locale &loc) const { return generic_string<char16_t>(loc); }

			/** Returns generic string representation of the path as a `char32_t` string. */
			[[nodiscard]] std::u32string generic_u32string() const { return generic_string<char32_t>(); }
			/** Returns generic string representation of the path as a `char32_t` string using locale \a loc. */
			[[nodiscard]] std::u32string generic_u32string(const std::locale &loc) const { return generic_string<char32_t>(loc); }

		public:
			/** Checks if the path is absolute. */
			[[nodiscard]] constexpr bool is_absolute() const noexcept { return _path::is_absolute<value_type>(_string, format()); }
			/** Checks if the path is relative (not absolute). */
			[[nodiscard]] constexpr bool is_relative() const noexcept { return !is_absolute(); }

			/** Returns a path containing the root namespace component of this path. Namespaces are only supported on
			 * Windows and follow the Win32 path format specification with the addition of a special `\!!\` namespace
			 * used to identify native NT paths. */
			[[nodiscard]] path root_namespace() const { return {root_namespace_substr<value_type>(_string, format()), format()}; }
			/** Checks if the path has a non-empty root namespace. */
			[[nodiscard]] constexpr bool has_root_namespace() const noexcept { return !root_namespace_substr<value_type>(_string, format()).empty(); }

			/** Returns a path containing the root directory component of this path. */
			[[nodiscard]] path root_directory() const { return {root_directory_substr<value_type>(_string, format()), format()}; }
			/** Checks if the path has a non-empty root directory. */
			[[nodiscard]] constexpr bool has_root_directory() const noexcept { return !root_directory_substr<value_type>(_string, format()).empty(); }

			/** Returns a path containing the root path component of this path. */
			[[nodiscard]] path root_path() const { return {root_path_substr<value_type>(_string, format()), format()}; }
			/** Checks if the path has a non-empty root path. */
			[[nodiscard]] constexpr bool has_root_path() const noexcept { return !root_path_substr<value_type>(_string, format()).empty(); }

			/** Returns a path containing the root name component of this path. */
			[[nodiscard]] path root_name() const { return {root_name_substr<value_type>(_string, format()), format()}; }
			/** Checks if the path has a non-empty root name. */
			[[nodiscard]] constexpr bool has_root_name() const noexcept { return !root_name_substr<value_type>(_string, format()).empty(); }

			/** Returns a path containing the parent path component of this path. */
			[[nodiscard]] path parent_path() const { return {parent_path_substr<value_type>(_string, format()), format()}; }
			/** Checks if the path has a non-empty parent path component. */
			[[nodiscard]] constexpr bool has_parent_path() const noexcept { return !parent_path_substr<value_type>(_string, format()).empty(); }

			/** Returns a path containing the relative path component of this path. */
			[[nodiscard]] path relative_path() const { return {relative_path_substr<value_type>(_string, format()), format()}; }
			/** Checks if the path has a non-empty relative path component. */
			[[nodiscard]] constexpr bool has_relative_path() const noexcept { return !relative_path_substr<value_type>(_string, format()).empty(); }

			/** Returns a path containing the filename component of this path. */
			[[nodiscard]] path filename() const { return {file_name_substr<value_type>(_string, format()), format()}; }
			/** Checks if the path has a non-empty filename. */
			[[nodiscard]] constexpr bool has_filename() const noexcept { return !file_name_substr<value_type>(_string, format()).empty(); }

			/** Returns a path containing the stem component of this path. */
			[[nodiscard]] path stem() const { return {file_stem_substr(file_name_substr<value_type>(_string, format())), format()}; }
			/** Checks if the path has a non-empty stem. */
			[[nodiscard]] constexpr bool has_stem() const noexcept { return !file_stem_substr(file_name_substr<value_type>(_string, format())).empty(); }

			/** Returns a path containing the extension component of this path. */
			[[nodiscard]] path extension() const { return {file_ext_substr(file_name_substr<value_type>(_string, format())), format()}; }
			/** Checks if the path has a non-empty extension. */
			[[nodiscard]] constexpr bool has_extension() const noexcept { return !file_ext_substr(file_name_substr<value_type>(_string, format())).empty(); }

		private:
			[[nodiscard]] static inline path make_relative(const path &, const path &);

		public:
			/** Returns a normalized version of the path. */
			[[nodiscard]] ROD_API_PUBLIC path lexically_normal() const;
			/** Returns a version of the path relative to \a base. */
			[[nodiscard]] ROD_API_PUBLIC path lexically_relative(const path &base) const;
			/** Returns a version of the path proximate to \a base. */
			[[nodiscard]] path lexically_proximate(const path &base) const
			{
				auto result = lexically_relative(base);
				if (result.empty())
					result = *this;
				return result;
			}

		public:
			/** Converts path to preferred-separator form.
			 * @note On POSIX-compliant systems where `preferred_separator` is `/`, results in a no-op. */
			constexpr path &make_preferred() noexcept
			{
#ifdef ROD_WIN32
				if (format() != binary_format)
					std::ranges::replace(_string, '/', '\\');
#endif
				return *this;
			}

			/** Removes the file name component of the path. */
			constexpr path &remove_filename() noexcept
			{
				_string.erase(find_file_name<value_type>(_string, format()));
				return *this;
			}
			/** Replaces file name component of the path with \a other. */
			path &replace_filename(const path &other)
			{
				remove_filename();
				return operator/=(other);
			}

			/** Removes the file stem component of the path. */
			constexpr path &remove_stem() noexcept
			{
				const auto name_start = find_file_name<value_type>(_string, format());
				auto name_view = string_view_type(_string.data() + name_start, _string.size() - name_start);
				_string.erase(name_start, file_stem_size<value_type>(name_view));
				return *this;
			}
			/** Removes the extension component of the path. */
			constexpr path &remove_extension() noexcept
			{
				const auto name_start = find_file_name<value_type>(_string, format());
				auto name_view = string_view_type(_string.data() + name_start, _string.size() - name_start);
				_string.erase(name_start + file_stem_size<value_type>(name_view));
				return *this;
			}

			/** Replaces file stem component of the path with \a other. */
			path &replace_stem(const path &other = path())
			{
				const auto name_start = find_file_name<value_type>(_string, format());
				auto name_view = string_view_type(_string.data() + name_start, _string.size() - name_start);
				_string.replace(name_start, file_stem_size<value_type>(name_view), other._string);
				return *this;
			}
			/** Replaces extension component of the path with \a other. */
			path &replace_extension(const path &other = path())
			{
				/* Remove extension & append a dot it `other` does not have one. */
				remove_extension();
				if (!other.empty())
				{
					if (other._string[0] != '.')
						_string.push_back('.');
					operator+=(other);
				}
				return *this;
			}

		public:
			/** Swaps contents of `this` with \a other. */
			constexpr void swap(path &other) noexcept
			{
				std::swap(_string, other._string);
				std::swap(_format, other._format);
			}
			friend constexpr void swap(path &a, path &b) noexcept { a.swap(b); }

			/** Lexicographically compares `this` with \a other. */
			[[nodiscard]] constexpr int compare(const path &other) const noexcept { return compare_native(other._string, other.format()); }
			/** Lexicographically compares `this` with path equivalent to \a src. */
			template<accepted_source Src>
			[[nodiscard]] constexpr int compare(const Src &src) const noexcept(noexcept(compare_native(src, auto_format))) { return compare_native(src, auto_format); }

		private:
			template<typename Src, typename C = source_string_t<std::decay_t<Src>>>
			[[nodiscard]] constexpr int compare_native(const Src &src, format_type fmt) const noexcept(decays_to_same<C, value_type> && noexcept(to_string_buffer(src)))
			{
				if constexpr (!decays_to_same<C, value_type>)
					return _path::compare<value_type>(_string, format(), to_native_string(src), fmt);
				else
					return _path::compare<value_type>(_string, format(), to_string_buffer(src), fmt);
			}

		public:
			[[nodiscard]] friend path operator/(const path &a, const path &b) { return path(a) /= b; }
			template<accepted_source Src>
			[[nodiscard]] friend path operator/(const path &a, Src &&b) { return path(a) /= std::forward<Src>(b); }
			template<accepted_source Src>
			[[nodiscard]] friend path operator/(Src &&a, const path &b) { return path(std::forward<Src>(a)) /= b; }

			[[nodiscard]] friend path operator+(const path &a, const path &b) { return path(a) += b; }
			template<accepted_source Src>
			[[nodiscard]] friend path operator+(const path &a, Src &&b) { return path(a) += std::forward<Src>(b); }

			template<typename C, typename T>
			friend std::basic_ostream<C, T> &operator<<(std::basic_ostream<C, T> &os, const path &p)
			{
				os << std::quoted(p.string<C, T>());
				return os;
			}
			template<typename C, typename T>
			friend std::basic_istream<C, T> &operator>>(std::basic_istream<C, T> &is, path &p)
			{
				std::basic_string<C, T> str;
				is >> std::quoted(str);
				p = str;
				return is;
			}

		private:
			string_type _string;
			format_type _format;
		};

		/** Preforms a component-wise lexicographical equality comparison between \a a and \a b. Equivalent to `a.compare(b) == 0`. */
		[[nodiscard]] inline constexpr bool operator==(const path &a, const path &b) noexcept { return a.compare(b) == 0; }
		/** Preforms a component-wise lexicographical three-way comparison between \a a and \a b. Equivalent to `a.compare(b) <=> 0`. */
		[[nodiscard]] inline constexpr auto operator<=>(const path &a, const path &b) noexcept { return a.compare(b) <=> 0; }

		[[maybe_unused]] static path from_wide(std::span<const std::byte> data) { return {std::wstring_view{reinterpret_cast<const wchar_t *>(data.data()), data.size()}}; }
		[[maybe_unused]] static path from_multibyte(std::span<const std::byte> data) { return {std::string_view{reinterpret_cast<const char *>(data.data()), data.size()}}; }

#ifdef ROD_WIN32
		ROD_API_PUBLIC path from_binary(std::span<const std::byte> data);
#else
		path from_binary(std::span<const std::byte> data) { return from_multibyte(data); }
#endif

		template<typename P>
		struct compare_equal
		{
			static_assert(one_of<std::decay_t<P>, path, path_view, path_view_component>, "can only compare path types");
			[[nodiscard]] constexpr bool operator()(const P &a, const P &b) const noexcept { return a.compare(b) == 0; }
		};
		template<typename P>
		struct compare_three_way
		{
			static_assert(one_of<std::decay_t<P>, path, path_view, path_view_component>, "can only compare path types");
			[[nodiscard]] constexpr auto operator()(const P &a, const P &b) const noexcept { return a.compare(b) <=> 0; }
		};

		template<typename P>
		struct compare_not_equal
		{
			static_assert(one_of<std::decay_t<P>, path, path_view, path_view_component>, "can only compare path types");
			[[nodiscard]] constexpr bool operator()(const P &a, const P &b) const noexcept { return a.compare(b) != 0; }
		};
		template<typename P>
		struct compare_less_than
		{
			static_assert(one_of<std::decay_t<P>, path, path_view, path_view_component>, "can only compare path types");
			[[nodiscard]] constexpr bool operator()(const P &a, const P &b) const noexcept { return a.compare(b) < 0; }
		};
		template<typename P>
		struct compare_less_equal
		{
			static_assert(one_of<std::decay_t<P>, path, path_view, path_view_component>, "can only compare path types");
			[[nodiscard]] constexpr bool operator()(const P &a, const P &b) const noexcept { return a.compare(b) <= 0; }
		};
		template<typename P>
		struct compare_greater_than
		{
			static_assert(one_of<std::decay_t<P>, path, path_view, path_view_component>, "can only compare path types");
			[[nodiscard]] constexpr bool operator()(const P &a, const P &b) const noexcept { return a.compare(b) > 0; }
		};
		template<typename P>
		struct compare_greater_equal
		{
			static_assert(one_of<std::decay_t<P>, path, path_view, path_view_component>, "can only compare path types");
			[[nodiscard]] constexpr bool operator()(const P &a, const P &b) const noexcept { return a.compare(b) >= 0; }
		};
	}

	using _path::path;
	using _path::operator==;
	using _path::operator<=>;

	/** Functor used for lexicographical equality comparison of paths. */
	template<typename P>
	using path_compare_equal = _path::compare_equal<P>;
	/** Functor used for lexicographical three-way comparison of paths. */
	template<typename P>
	using path_compare_three_way = _path::compare_three_way<P>;

	/** Functor used for lexicographical inequality comparison of paths. */
	template<typename P>
	using path_compare_not_equal = _path::compare_not_equal<P>;
	/** Functor used for lexicographical less-than comparison of paths. */
	template<typename P>
	using path_compare_less_than = _path::compare_less_than<P>;
	/** Functor used for lexicographical less-equal comparison of paths. */
	template<typename P>
	using path_compare_less_equal = _path::compare_less_equal<P>;
	/** Functor used for lexicographical greater-than comparison of paths. */
	template<typename P>
	using path_compare_greater_than = _path::compare_greater_than<P>;
	/** Functor used for lexicographical greater-equal comparison of paths. */
	template<typename P>
	using path_compare_greater_equal = _path::compare_greater_equal<P>;
}

template<>
struct std::hash<rod::path> { [[nodiscard]] constexpr std::size_t operator()(const rod::path &p) const noexcept { return rod::_path::hash_string<typename rod::path::value_type>(p.native(), p.format()); } };

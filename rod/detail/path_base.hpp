/*
 * Created by switch_blade on 2023-07-30.
 */

#pragma once

#include <string_view>
#include <algorithm>
#include <iomanip>
#include <iosfwd>
#include <locale>
#include <array>
#include <span>

#include "../result.hpp"

namespace rod::fs
{
	class path;
	class path_view;
	class path_view_component;

	namespace detail
	{
		struct path_view_like;

		template<typename T>
		concept path_char = one_of<std::decay_t<T>, char, wchar_t, char8_t, char16_t, char32_t>;

		template<typename T>
		struct path_source_value;
		template<typename T> requires std::is_pointer_v<T>
		struct path_source_value<T> { using type = std::remove_pointer_t<T>; };
		template<typename T> requires std::ranges::forward_range<T>
		struct path_source_value<T> { using type = std::ranges::range_value_t<T>; };

		template<typename T>
		using path_source_value_t = typename path_source_value<T>::type;

		template<typename T>
		struct is_path_source : std::false_type {};
		template<typename T> requires std::is_pointer_v<T> || std::ranges::forward_range<T>
		struct is_path_source<T> : std::bool_constant<path_char<path_source_value_t<T>>> {};

		template<typename T>
		concept path_source = !decays_to<T, path> && is_path_source<std::decay_t<T>>::value;

		struct path_base
		{
			using encoding = std::uint8_t;

			static constexpr encoding byte_encoding = 0;
			static constexpr encoding char_encoding = 1;
			static constexpr encoding wchar_encoding = 2;
			static constexpr encoding char8_encoding = 3;
			static constexpr encoding char16_encoding = 4;

			using format = std::uint8_t;

			/** Automatic path formatting mode, path format will be deduced at runtime. */
			static constexpr format auto_format = 0;
			/** Native path formatting mode, path manipulation functions and queries will use native parsing rules. */
			static constexpr format native_format = 1;
			/** Binary path formatting mode, path manipulation functions and queries will be treated as no-ops. */
			static constexpr format binary_format = 2;
			/** Generic path formatting mode, path manipulation functions and queries will use generic parsing rules. */
			static constexpr format generic_format = 3;
			/** Unknown path formatting mode, path manipulation functions and queries will use implementation-defined rules (usually equivalent to `auto_format`). */
			static constexpr format unknown_format = 4;

#ifdef ROD_WIN32
			using value_type = wchar_t;
#else
			using value_type = char;
#endif

			using string_type = std::basic_string<value_type, std::char_traits<value_type>>;
			using string_view_type = std::basic_string_view<value_type>;
			using size_type = typename string_type::size_type;

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

			static ROD_API_PUBLIC result<std::size_t> utf16_to_utf32(std::span<const char16_t> src, void (*put_func)(void *, char32_t), void *data);
			static ROD_API_PUBLIC result<std::size_t> utf32_to_utf16(std::span<const char32_t> src, void (*put_func)(void *, char16_t), void *data);

			template<typename SrcChar, typename DstChar, typename Traits = std::char_traits<DstChar>, typename Alloc = std::allocator<DstChar>> requires(sizeof(SrcChar) == sizeof(char32_t) && sizeof(DstChar) == sizeof(char16_t))
			static result<std::basic_string<DstChar, Traits, Alloc>> cvt_value(std::basic_string_view<SrcChar> src, const std::locale &, const Alloc &alloc = Alloc{})
			{
				return cvt_value<SrcChar, DstChar, Traits, Alloc>(src, alloc);
			}
			template<typename SrcChar, typename DstChar, typename Traits = std::char_traits<DstChar>, typename Alloc = std::allocator<DstChar>> requires(sizeof(SrcChar) == sizeof(char16_t) && sizeof(DstChar) == sizeof(char32_t))
			static result<std::basic_string<DstChar, Traits, Alloc>> cvt_value(std::basic_string_view<SrcChar> src, const std::locale &, const Alloc &alloc = Alloc{})
			{
				return cvt_value<SrcChar, DstChar, Traits, Alloc>(src, alloc);
			}
			template<typename SrcChar, typename DstChar, typename Traits = std::char_traits<DstChar>, typename Alloc = std::allocator<DstChar>> requires(sizeof(SrcChar) == sizeof(char32_t) && sizeof(DstChar) == sizeof(char16_t))
			static result<std::basic_string<DstChar, Traits, Alloc>> cvt_value(std::basic_string_view<SrcChar> src, const Alloc &alloc = Alloc{})
			{
				result<std::basic_string<DstChar, Traits, Alloc>> result = {in_place_value, alloc};
				result->reserve(src.size());

				if (!src.empty()) [[likely]]
				{
					auto len = utf32_to_utf16({reinterpret_cast<const char32_t *>(src.data()), src.size()}, [](void *p, char16_t c) { static_cast<std::basic_string<DstChar, Traits, Alloc> *>(p)->push_back(static_cast<DstChar>(c)); }, &*result);
					if (len.has_error()) [[unlikely]]
						result = len.error();
				}
				return result;
			}
			template<typename SrcChar, typename DstChar, typename Traits = std::char_traits<DstChar>, typename Alloc = std::allocator<DstChar>> requires(sizeof(SrcChar) == sizeof(char16_t) && sizeof(DstChar) == sizeof(char32_t))
			static result<std::basic_string<DstChar, Traits, Alloc>> cvt_value(std::basic_string_view<SrcChar> src, const Alloc &alloc = Alloc{})
			{
				result<std::basic_string<DstChar, Traits, Alloc>> result = {in_place_value, alloc};
				result->reserve(src.size());

				if (!src.empty()) [[likely]]
				{
					auto len = utf16_to_utf32({reinterpret_cast<const char16_t *>(src.data()), src.size()}, [](void *p, char32_t c) { static_cast<std::basic_string<DstChar, Traits, Alloc> *>(p)->push_back(static_cast<DstChar>(c)); }, &*result);
					if (len.has_error()) [[unlikely]]
						result = len.error();
				}
				return result;
			}

			/* `codecvt` is directional, as such we have to use either `codecvt::in` or `codecvt::out` depending on which side `char` (or `char8_t`) is on. */
			template<typename SrcChar, typename DstChar, typename Traits = std::char_traits<DstChar>, typename Alloc = std::allocator<DstChar>, typename State = std::mbstate_t>
			static result<std::basic_string<DstChar, Traits, Alloc>> cvt_value(std::basic_string_view<SrcChar> src, const Alloc &alloc, const std::codecvt<SrcChar, DstChar, State> &cvt)
			{
				for (auto dst = std::basic_string<DstChar, Traits, Alloc>(alloc);;)
				{
					auto *src_first = src.data(), *src_last = src_first + src.size();
					auto *dst_first = dst.data(), *dst_last = dst_first + dst.size();
					const SrcChar *src_next = {};
					DstChar *dst_next = {};
					State state = {};

					const auto status = cvt.out(state, src_first, src_last, src_next, dst_first, dst_last, dst_next);
					if (src_next < src_first || src_next > src_last || dst_next < dst_first || dst_next > dst_last) [[unlikely]]
						return std::make_error_code(std::errc::invalid_argument);
					if (status == std::codecvt_base::ok)
					{
						dst.resize(dst_next - dst_first);
						return std::move(dst);
					}
					if (status == std::codecvt_base::partial)
					{
						if ((src_next == src_first && dst_next != dst_last) || dst.size() > dst.max_size() / 2) [[unlikely]]
							return std::make_error_code(std::errc::invalid_argument);
						else
							dst.resize(dst.size() * 2, '\0');
						continue;
					}
					return std::make_error_code(std::errc::invalid_argument);
				}
			}
			template<typename SrcChar, typename DstChar, typename Traits = std::char_traits<DstChar>, typename Alloc = std::allocator<DstChar>, typename State = std::mbstate_t>
			static result<std::basic_string<DstChar, Traits, Alloc>> cvt_value(std::basic_string_view<SrcChar> src, const Alloc &alloc, const std::codecvt<DstChar, SrcChar, State> &cvt)
			{
				for (auto dst = std::basic_string<DstChar, Traits, Alloc>(alloc);;)
				{
					auto *src_first = src.data(), *src_last = src_first + src.size();
					auto *dst_first = dst.data(), *dst_last = dst_first + dst.size();
					const SrcChar *src_next = {};
					DstChar *dst_next = {};
					State state = {};

					const auto status = cvt.in(state, src_first, src_last, src_next, dst_first, dst_last, dst_next);
					if (src_next < src_first || src_next > src_last || dst_next < dst_first || dst_next > dst_last) [[unlikely]]
						return std::make_error_code(std::errc::invalid_argument);
					if (status == std::codecvt_base::ok)
					{
						dst.resize(dst_next - dst_first);
						return std::move(dst);
					}
					if (status == std::codecvt_base::partial)
					{
						if ((src_next == src_first && dst_next != dst_last) || dst.size() > dst.max_size() / 2) [[unlikely]]
							return std::make_error_code(std::errc::invalid_argument);
						else
							dst.resize(dst.size() * 2, '\0');
						continue;
					}
					return std::make_error_code(std::errc::invalid_argument);
				}
			}

			template<one_of<char, char8_t> SrcChar, typename DstChar, typename Traits = std::char_traits<DstChar>, typename Alloc = std::allocator<DstChar>>
			static result<std::basic_string<DstChar, Traits, Alloc>> cvt_value(std::basic_string_view<SrcChar> src, const std::locale &loc, const Alloc &alloc = Alloc{})
			{
				auto &cvt = use_facet<std::codecvt<DstChar, SrcChar, std::mbstate_t>>(loc);
				return cvt_value<SrcChar, DstChar>(src, alloc, cvt);
			}
			template<typename SrcChar, one_of<char, char8_t> DstChar, typename Traits = std::char_traits<DstChar>, typename Alloc = std::allocator<DstChar>>
			static result<std::basic_string<DstChar, Traits, Alloc>> cvt_value(std::basic_string_view<SrcChar> src, const std::locale &loc, const Alloc &alloc = Alloc{})
			{
				auto &cvt = use_facet<std::codecvt<SrcChar, DstChar, std::mbstate_t>>(loc);
				return cvt_value<SrcChar, DstChar>(src, alloc, cvt);
			}

#ifdef ROD_WIN32
			static constexpr std::uint32_t codepage_gb18030 = 54936;
			static constexpr std::uint32_t codepage_utf8 = 65001;

			static ROD_API_PUBLIC result<std::size_t> wide_to_multibyte_with_default(std::uint32_t codepage, std::span<const wchar_t> src, std::span<char> dst) noexcept;
			static ROD_API_PUBLIC result<std::size_t> wide_to_multibyte(std::uint32_t codepage, std::span<const wchar_t> src, std::span<char> dst) noexcept;
			static ROD_API_PUBLIC result<std::size_t> multibyte_to_wide(std::uint32_t codepage, std::span<const char> src, std::span<wchar_t> dst) noexcept;
			static ROD_API_PUBLIC std::uint32_t system_codepage() noexcept;

			static result<std::string> cvt_pretty_value(std::wstring_view src)
			{
				if (src.empty()) [[unlikely]]
					return in_place_value;

				const auto codepage = system_codepage();
				std::string result;

				if (const auto len = wide_to_multibyte_with_default(codepage, src, {}); len.has_error()) [[likely]]
					result.resize(*len, '\0');
				else
					return len.error();
				if (const auto len = wide_to_multibyte_with_default(codepage, src, result); len.has_error()) [[likely]]
					result.resize(*len, '\0');
				else
					return len.error();

				return result;
			}

			template<one_of<char, char8_t> SrcChar, std::same_as<char32_t> DstChar, typename Traits = std::char_traits<DstChar>, typename Alloc = std::allocator<DstChar>>
			static result<std::basic_string<DstChar, Traits, Alloc>> cvt_value(std::basic_string_view<SrcChar> src, const Alloc &alloc = Alloc{})
			{
				return cvt_value<SrcChar, DstChar>(src, std::locale{}, alloc);
			}
			template<std::same_as<char32_t> SrcChar, one_of<char, char8_t> DstChar, typename Traits = std::char_traits<DstChar>, typename Alloc = std::allocator<DstChar>>
			static result<std::basic_string<DstChar, Traits, Alloc>> cvt_value(std::basic_string_view<SrcChar> src, const Alloc &alloc = Alloc{})
			{
				return cvt_value<SrcChar, DstChar>(src, std::locale{}, alloc);
			}

			template<one_of<char, char8_t> SrcChar, one_of<wchar_t, char16_t> DstChar, typename Traits = std::char_traits<DstChar>, typename Alloc = std::allocator<DstChar>>
			static result<std::basic_string<DstChar, Traits, Alloc>> cvt_value(std::basic_string_view<SrcChar> src, const Alloc &alloc = Alloc{})
			{
				auto dst = std::basic_string<DstChar, Traits, Alloc>(alloc);
				if (!src.empty()) [[likely]]
				{
					std::uint32_t codepage;
					if constexpr (!std::same_as<SrcChar, char8_t>)
						codepage = system_codepage();
					else
						codepage = codepage_utf8;

					if (auto len = multibyte_to_wide(codepage, {reinterpret_cast<const char *>(src.data()), src.size()}, {}); len.has_value()) [[likely]]
						dst.resize(*len, '\0');
					else
						return len.error();

					if (auto len = multibyte_to_wide(codepage, {reinterpret_cast<const char *>(src.data()), src.size()}, {reinterpret_cast<wchar_t *>(dst.data()), dst.size()}); len.has_value()) [[likely]]
						dst.resize(*len, '\0');
					else
						return len.error();
				}
				return std::move(dst);
			}
			template<one_of<wchar_t, char16_t> SrcChar, one_of<char, char8_t> DstChar, typename Traits = std::char_traits<DstChar>, typename Alloc = std::allocator<DstChar>>
			static result<std::basic_string<DstChar, Traits, Alloc>> cvt_value(std::basic_string_view<SrcChar> src, const Alloc &alloc = Alloc{})
			{
				auto dst = std::basic_string<DstChar, Traits, Alloc>(alloc);
				if (!src.empty()) [[likely]]
				{
					std::uint32_t codepage;
					if constexpr (!std::same_as<SrcChar, char8_t>)
						codepage = system_codepage();
					else
						codepage = codepage_utf8;

					if (auto len = wide_to_multibyte(codepage, {reinterpret_cast<const wchar_t *>(src.data()), src.size()}, {}); len.has_value()) [[likely]]
						dst.resize(*len, '\0');
					else
						return len.error();

					if (auto len = wide_to_multibyte(codepage, {reinterpret_cast<const wchar_t *>(src.data()), src.size()}, {reinterpret_cast<char *>(dst.data()), dst.size()}); len.has_value()) [[likely]]
						dst.resize(*len, '\0');
					else
						return len.error();
				}
				return std::move(dst);
			}
#else
			template<one_of<char, char8_t> SrcChar, typename DstChar, typename Traits = std::char_traits<DstChar>, typename Alloc = std::allocator<DstChar>>
			static result<std::basic_string<DstChar, Traits, Alloc>> cvt_value(std::basic_string_view<SrcChar> src, const Alloc &alloc = Alloc{})
			{
				return cvt_value<SrcChar, DstChar>(src, std::locale{}, alloc);
			}
			template<typename SrcChar, one_of<char, char8_t> DstChar, typename Traits = std::char_traits<DstChar>, typename Alloc = std::allocator<DstChar>>
			static result<std::basic_string<DstChar, Traits, Alloc>> cvt_value(std::basic_string_view<SrcChar> src, const Alloc &alloc = Alloc{})
			{
				return cvt_value<SrcChar, DstChar>(src, std::locale{}, alloc);
			}
#endif

			template<typename SrcChar, typename DstChar, typename Traits = std::char_traits<DstChar>, typename Alloc = std::allocator<DstChar>> requires(sizeof(SrcChar) == sizeof(DstChar))
			static result<std::basic_string<DstChar, Traits, Alloc>> cvt_value(std::basic_string_view<SrcChar> src, const std::locale &, const Alloc &alloc = Alloc{})
			{
				result<std::basic_string<DstChar, Traits, Alloc>> result = {in_place_value, src.size(), '\0', alloc};
				std::memcpy(result->data(), src.data(), src.size() * sizeof(SrcChar));
				return result;
			}
			template<typename SrcChar, typename DstChar, typename Traits = std::char_traits<DstChar>, typename Alloc = std::allocator<DstChar>> requires(sizeof(SrcChar) == sizeof(DstChar))
			static result<std::basic_string<DstChar, Traits, Alloc>> cvt_value(std::basic_string_view<SrcChar> src, const Alloc &alloc = Alloc{})
			{
				result<std::basic_string<DstChar, Traits, Alloc>> result = {in_place_value, src.size(), '\0', alloc};
				std::memcpy(result->data(), src.data(), src.size() * sizeof(SrcChar));
				return result;
			}

			template<typename I, typename S, typename C = std::decay_t<std::iter_value_t<I>>> requires (std::contiguous_iterator<I>)
			[[nodiscard]] static constexpr auto make_string_buff(I first, S last) noexcept
			{
				const auto ptr_first = std::to_address(first), ptr_last = std::to_address(last);
				return std::basic_string_view<C>(ptr_first, rod::detail::strlen(ptr_first, ptr_last));
			}
			template<typename I, typename S, typename C = std::decay_t<std::iter_value_t<I>>> requires (!std::contiguous_iterator<I>)
			[[nodiscard]] static constexpr auto make_string_buff(I first, S last)
			{
				std::basic_string<C> buff;
				if constexpr (std::random_access_iterator<I>)
					buff.reserve(std::distance(first, last));
				for (; first != last && *first != C{}; ++first)
					buff.push_back(*first);

				return buff;
			}
			template<typename Src, typename C = std::decay_t<path_source_value_t<std::decay_t<Src>>>>
			[[nodiscard]] static constexpr auto make_string_buff(Src &&src) noexcept((std::is_pointer_v<std::decay_t<Src>> || std::ranges::contiguous_range<Src>) && std::same_as<C, value_type>)
			{
				if constexpr (decays_to<Src, string_type> || decays_to<Src, string_view_type>)
					return std::forward<Src>(src);
				else if constexpr (std::is_pointer_v<std::decay_t<Src>>)
					return std::basic_string_view<C>(src, rod::detail::strlen(src));
				else
					return make_string_buff(std::ranges::cbegin(src), std::ranges::cend(src));
			}

			template<typename I, typename S> requires(!decays_to<std::iter_value_t<I>, value_type>)
			[[nodiscard]] static string_type make_native_string(I first, S last, const std::locale &loc) { return cvt_value<std::iter_value_t<I>, value_type>(make_string_buff(first, last), loc).value(); }
			template<typename I, typename S> requires(!decays_to<std::iter_value_t<I>, value_type>)
			[[nodiscard]] static string_type make_native_string(I first, S last) { return cvt_value<std::iter_value_t<I>, value_type>(make_string_buff(first, last)).value(); }

			template<typename I, typename S> requires decays_to<std::iter_value_t<I>, value_type>
			[[nodiscard]] static string_type make_native_string(I first, S last, const std::locale &) { return string_type(make_string_buff(first, last)); }
			template<typename I, typename S> requires decays_to<std::iter_value_t<I>, value_type>
			[[nodiscard]] static string_type make_native_string(I first, S last) { return string_type(make_string_buff(first, last)); }

			template<typename Src>
			[[nodiscard]] static string_type make_native_string(Src &&src, const std::locale &loc)
			{
				if constexpr (decays_to<Src, string_type> || decays_to<Src, string_view_type>)
					return string_type(std::forward<Src>(src));
				else if constexpr (std::is_pointer_v<std::decay_t<Src>>)
					return make_native_string(src, src + rod::detail::strlen(src), loc);
				else
					return make_native_string(std::ranges::cbegin(src), std::ranges::cend(src), loc);
			}
			template<typename Src>
			[[nodiscard]] static string_type make_native_string(Src &&src)
			{
				if constexpr (decays_to<Src, string_type> || decays_to<Src, string_view_type>)
					return string_type(std::forward<Src>(src));
				else if constexpr (std::is_pointer_v<std::decay_t<Src>>)
					return make_native_string(src, src + rod::detail::strlen(src));
				else
					return make_native_string(std::ranges::cbegin(src), std::ranges::cend(src));
			}

			template<typename C = value_type>
			static constexpr std::size_t lfind_separator(std::basic_string_view<C> path, format fmt, std::size_t idx = 0) noexcept
			{
				switch (fmt)
				{
				case binary_format:
					return std::basic_string_view<C>::npos;
				default:
					return path.find_first_of('/', idx);
#ifdef ROD_WIN32
				case generic_format:
					return path.find_first_of('/', idx);
				case native_format:
					return path.find_first_of('\\', idx);
				case unknown_format:
				case auto_format:
				{
					const auto seps = std::array<C, 2>{'\\', '/'};
					return path.find_first_of(std::basic_string_view<C>{seps.data(), 2}, idx);
				}
#endif
				}
			}
			template<typename C = value_type>
			static constexpr std::size_t rfind_separator(std::basic_string_view<C> path, format fmt, std::size_t idx = std::basic_string_view<C>::npos) noexcept
			{
				switch (fmt)
				{
				case binary_format:
					return std::basic_string_view<C>::npos;
				default:
					return path.find_last_of('/', idx);
#ifdef ROD_WIN32
				case generic_format:
					return path.find_last_of('/', idx);
				case native_format:
					return path.find_last_of('\\', idx);
				case unknown_format:
				case auto_format:
				{
					const auto seps = std::array<C, 2>{'\\', '/'};
					return path.find_last_of(std::basic_string_view<C>{seps.data(), 2}, idx);
				}
#endif
				}
			}
			template<typename C = value_type>
			static constexpr bool is_separator(C ch, format fmt) noexcept
			{
				switch (fmt)
				{
				default:
					return ch == '/';
				case binary_format:
					return false;
#ifdef ROD_WIN32
				case native_format:
					return ch == '\\';
				case unknown_format:
				case auto_format:
					return ch == '/' || ch == '\\';
#endif
				}
			}

#ifdef ROD_WIN32
			template<typename C = value_type>
			static constexpr bool is_drive_letter(C ch) noexcept
			{
				constexpr C case_mask = ' ';
				constexpr C drive_max = 'Z';
				return (ch & ~case_mask) <= drive_max;
			}
			template<typename C = value_type>
			static constexpr bool is_unc_drive_path(std::basic_string_view<C> path, format fmt)
			{
				return path.size() >= 6 && is_separator(path[0], fmt) && is_separator(path[1], fmt) && path[2] == '?' && is_separator(path[3], fmt) && path[5] == ':' && is_drive_letter(path[4]);
			}

			template<typename C = value_type>
			static constexpr std::size_t namespace_prefix_size(std::basic_string_view<C> path, format fmt) noexcept
			{
				if (path.size() >= 4 && is_separator(path[3], fmt) && (path.size() == 4 || !is_separator(path[4], fmt)))
				{
					if (is_separator(path[1], fmt) && (path[2] == '.' || path[2] == '?') ||
							(path[1] == '?' && path[2] == '?') ||
							(path[1] == '!' && path[2] == '!'))
						return 4;
				}
				return 0;
			}
			template<typename C = value_type>
			static constexpr std::size_t hostname_prefix_size(std::basic_string_view<C> path, format fmt) noexcept
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
			static constexpr std::size_t root_name_size(std::basic_string_view<C> path, format fmt) noexcept
			{
				if (fmt != binary_format && path.size() >= 2)
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
			static constexpr size_type root_name_size(std::basic_string_view<C> path, format fmt) noexcept
			{
			}
#endif

			template<typename C = value_type>
			static constexpr std::size_t find_relative_path(std::basic_string_view<C> path, format fmt) noexcept
			{
				const auto pred = [&](auto ch) { return is_separator(ch, fmt); };
				const auto root_name_end = path.begin() + root_name_size(path, fmt);
				const auto root_path_end = std::find_if_not(root_name_end, path.end(), pred);
				return static_cast<std::size_t>(std::distance(path.begin(), root_path_end));
			}
			template<typename C = value_type>
			static constexpr std::size_t parent_path_size(std::basic_string_view<C> path, format fmt) noexcept
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
			static constexpr bool is_absolute(std::basic_string_view<C> path, format fmt) noexcept
			{
				/* Always treat binary format as absolute. */
				if (fmt == binary_format)
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
			static constexpr bool contains_root(std::basic_string_view<C> path, format fmt) noexcept
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
			static constexpr std::basic_string_view<C> root_path_substr(std::basic_string_view<C> path, format fmt) noexcept
			{
				return path.substr(0, find_relative_path(path, fmt));
			}
			template<typename C = value_type>
			static constexpr std::basic_string_view<C> root_name_substr(std::basic_string_view<C> path, format fmt) noexcept
			{
				return path.substr(0, root_name_size(path, fmt));
			}
			template<typename C = value_type>
			static constexpr std::basic_string_view<C> root_dir_substr(std::basic_string_view<C> path, format fmt) noexcept
			{
				const auto pred = [&](auto ch) { return is_separator(ch, fmt); };
				const auto root_name_end = path.begin() + root_name_size(path, fmt);
				const auto root_path_end = std::find_if_not(root_name_end, path.end(), pred);
				return std::basic_string_view<C>(root_name_end, root_path_end);
			}

			template<typename C = value_type>
			static constexpr std::basic_string_view<C> relative_path_substr(std::basic_string_view<C> path, format fmt) noexcept
			{
				return path.substr(find_relative_path(path, fmt));
			}
			template<typename C = value_type>
			static constexpr std::basic_string_view<C> parent_path_substr(std::basic_string_view<C> path, format fmt) noexcept
			{
				return path.substr(0, parent_path_size(path, fmt));
			}

			template<typename C = value_type>
			static constexpr std::size_t find_file_name(std::basic_string_view<C> path, format fmt) noexcept
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
			static constexpr std::size_t file_stem_size(std::basic_string_view<C> path) noexcept
			{
				if (const auto dot = path.rfind('.'); dot == std::basic_string_view<C>::npos || dot == 0 || (dot == 1 && path[0] != '.'))
					return path.size();
				else
					return dot;
			}

			template<typename C = value_type>
			static constexpr std::basic_string_view<C> file_name_substr(std::basic_string_view<C> path, format fmt) noexcept
			{
				return path.substr(find_file_name(path, fmt));
			}

#ifdef ROD_WIN32
			/* Account for alternate data streams under windows. */
			template<typename C = value_type>
			static constexpr std::size_t find_file_ads(std::basic_string_view<C> path) noexcept
			{
				const auto ads_pos = path.find_first_of(':');
				if (ads_pos == std::basic_string_view<C>::npos)
					return path.size();
				else
					return ads_pos;
			}

			template<typename C = value_type>
			static constexpr std::basic_string_view<C> file_stem_substr(std::basic_string_view<C> path) noexcept
			{
				path = path.substr(0, find_file_ads(path));
				return path.substr(0, file_stem_size(path));
			}
			template<typename C = value_type>
			static constexpr std::basic_string_view<C> file_ext_substr(std::basic_string_view<C> path) noexcept
			{
				path = path.substr(0, find_file_ads(path));
				return path.substr(file_stem_size(path));
			}
#else
			template<typename C = value_type>
			static constexpr std::basic_string_view<C> file_stem_substr(std::basic_string_view<C> path) noexcept
			{
				return path.substr(0, file_stem_size(path));
			}
			template<typename C = value_type>
			static constexpr std::basic_string_view<C> file_ext_substr(std::basic_string_view<C> path) noexcept
			{
				return path.substr(file_stem_size(path));
			}
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
			static constexpr std::size_t iter_begin(std::basic_string_view<C> base, format fmt) noexcept
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
			static constexpr std::basic_string_view<C> iter_next(std::basic_string_view<C> comp, std::basic_string_view<C> base, const C *&pos, format fmt) noexcept
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
			static constexpr std::basic_string_view<C> iter_prev(std::basic_string_view<C> comp, std::basic_string_view<C> base, const C *&pos, format fmt) noexcept
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
			[[nodiscard]] static constexpr int compare(std::basic_string_view<C> a_str, format a_fmt, std::basic_string_view<C> b_str, format b_fmt) noexcept
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
		};

		ROD_API_PUBLIC path path_from_binary(std::span<const std::byte> data);
	}

	/** Container of a filesystem-specific path representation. */
	class path : detail::path_base
	{
		friend class path_view_component;

		using path_base = detail::path_base;

	public:
		using size_type = typename path_base::size_type;
		using value_type = typename path_base::value_type;

		using string_type = typename path_base::string_type;
		using string_view_type = typename path_base::string_view_type;

		using format = typename path_base::format;

		using path_base::auto_format;
		using path_base::native_format;
		using path_base::binary_format;
		using path_base::generic_format;
		using path_base::unknown_format;
		
		using path_base::default_buffer_size;
		using path_base::preferred_separator;

	private:
		void iter_next(const path &base, const value_type *&pos) { assign(path_base::iter_next<value_type>(_value, base._value, pos, base.formatting())); }
		void iter_prev(const path &base, const value_type *&pos) { assign(path_base::iter_prev<value_type>(_value, base._value, pos, base.formatting())); }

		template<void (path::*Func)(const path &, const value_type *&)>
		struct iter_func { void operator()(path &comp, const path &base, const value_type *&pos) const { (comp.*Func)(base, pos); } };

	public:
		using iterator = typename path_base::component_iterator<path, const value_type *, iter_func<&path::iter_next>, iter_func<&path::iter_prev>>;
		using const_iterator = iterator;

	public:
		constexpr path(const path &) = default;
		constexpr path &operator=(const path &) = default;
		constexpr path(path &&) noexcept = default;
		constexpr path &operator=(path &&) noexcept = default;

		/** Initializes an empty path. */
		constexpr path() noexcept : path(auto_format) {}
		/** Initializes an empty path with format \a fmt. */
		constexpr path(format fmt) noexcept : _value(), _formatting(fmt) {}

		/** Initializes a path from C-style string \a str and format \a fmt. */
		constexpr path(const value_type *str, format fmt = auto_format) noexcept : _value(str), _formatting(fmt) {}
		/** Initializes a path from a move-constructed string \a str and format \a fmt. */
		constexpr path(string_type &&str, format fmt = auto_format) noexcept : _value(std::forward<string_type>(str)), _formatting(fmt) {}

		/** Initializes a path from a character range \a src and format \a fmt. */
		template<detail::path_source Src>
		path(Src &&src, format fmt = auto_format) : path(make_native_string(std::forward<Src>(src)), fmt) {}
		/** Initializes a path from a character range \a src using locale \a loc for encoding conversion, and format \a fmt. */
		template<detail::path_source Src>
		path(Src &&src, const std::locale &loc, format fmt = auto_format) : path(make_native_string(std::forward<Src>(src), loc), fmt) {}

		/** Initializes a path from characters in range [\a first, \a last) and format \a fmt. */
		template<std::forward_iterator I, std::sentinel_for<I> S> requires detail::path_char<std::iter_value_t<I>>
		path(I first, S last, format fmt = auto_format) : path(make_native_string(first, last), fmt) {}
		/** Initializes a path from characters in range [\a first, \a last) using locale \a loc for encoding conversion, and format \a fmt. */
		template<std::forward_iterator I, std::sentinel_for<I> S> requires detail::path_char<std::iter_value_t<I>>
		path(I first, S last, const std::locale &loc, format fmt = auto_format) : path(make_native_string(first, last, loc), fmt) {}

		/** Initializes a path from an exposition-only path-view-like object \a p and format \a fmt. */
		inline explicit path(detail::path_view_like p, format fmt = auto_format);
		/** Initializes a path from an exposition-only path-view-like object \a p using locale \a loc for encoding conversion, and format \a fmt. */
		inline path(detail::path_view_like p, const std::locale &loc, format fmt = auto_format);

	public:
		/** Assigns path contents from C-style string \a str. */
		constexpr path &operator=(const value_type *str) noexcept { return assign(str); }
		/** Assigns path contents from C-style string \a str. */
		constexpr path &assign(const value_type *str) noexcept { return (_value = str, *this); }

		/** Move-assigns the path from string \a str. */
		constexpr path &operator=(string_type &&str) noexcept { return assign(std::forward<string_type>(str)); }
		/** Assigns path contents from string \a str. */
		constexpr path &assign(string_type &&str) noexcept { return (_value = std::forward<string_type>(str), *this); }

		/** Assigns path contents from a character range \a src. */
		template<detail::path_source Src>
		constexpr path &operator=(Src &&src) { return assign(std::forward<Src>(src)); }
		/** Assigns path contents from a character range \a src. */
		template<detail::path_source Src>
		constexpr path &assign(Src &&src) { return assign_native(std::forward<Src>(src)); }
		/** Assigns path contents from a character range \a src using locale \a loc for encoding conversion. */
		template<detail::path_source Src>
		constexpr path &assign(Src &&src, const std::locale &loc) { return assign_native(std::forward<Src>(src), loc); }

		/** Assigns path contents from characters in range [\a first, \a last) \a src. */
		template<std::forward_iterator I, std::sentinel_for<I> S> requires detail::path_char<std::iter_value_t<I>>
		constexpr path &assign(I first, S last) { return assign_native(make_string_buff(first, last)); }
		/** Assigns path contents from characters in range [\a first, \a last) \a src using locale \a loc for encoding conversion. */
		template<std::forward_iterator I, std::sentinel_for<I> S> requires detail::path_char<std::iter_value_t<I>>
		constexpr path &assign(I first, S last, const std::locale &loc) { return assign_native(make_string_buff(first, last), loc); }

		/** Clears contents of the path. */
		constexpr void clear() noexcept { _value.clear(); }

	private:
		template<typename Src>
		constexpr path &assign_native(Src &&src, const std::locale &loc)
		{
			if constexpr (!decays_to<detail::path_source_value_t<std::decay_t<Src>>, value_type>)
				_value.assign(make_native_string(std::forward<Src>(src), loc));
			else
				_value.assign(make_string_buff(std::forward<Src>(src), loc));
			return *this;
		}
		template<typename Src>
		constexpr path &assign_native(Src &&src)
		{
			if constexpr (!decays_to<detail::path_source_value_t<std::decay_t<Src>>, value_type>)
				_value.assign(make_native_string(std::forward<Src>(src)));
			else
				_value.assign(make_string_buff(std::forward<Src>(src)));
			return *this;
		}

	public:
		/** Appends string representation of \a other to string representation of `this`. */
		path &operator+=(const path &other) { return operator+=(other._value); }

		/** Appends string \a str to string representation of `this`. */
        path &operator+=(const string_type &str) { return concat_native(str); }
		/** Appends string \a str to string representation of `this`. */
		path &operator+=(string_view_type str) { return concat_native(str); }

		/** Appends character \a ch to string representation of `this`. */
		path &operator+=(value_type ch) { return concat_native(string_view_type(&ch, 1)); }
		/** Appends C-style string \a str to string representation of `this`. */
        path &operator+=(const value_type *str) { return concat_native(string_view_type(str)); }

		/** Appends character range \a src to string representation of `this`. */
		template<detail::path_source Src>
		path &operator+=(Src &&src) { return concat(std::forward<Src>(src)); }
		/** Appends character range \a src to string representation of `this`. */
		template<detail::path_source Src>
		path &concat(Src &&src) { return concat_native(std::forward<Src>(src)); }
		/** Appends characters in range [\a first, \a last) to string representation of `this`. */
		template<std::forward_iterator I, std::sentinel_for<I> S> requires detail::path_char<std::iter_value_t<I>>
		path &concat(I first, S last) { return concat_native(make_string_buff(first, last)); }

	private:
		template<typename Src>
		path &concat_native(Src &&src)
		{
			if constexpr (!decays_to<detail::path_source_value_t<std::decay_t<Src>>, value_type>)
				_value.append(make_native_string(std::forward<Src>(src)));
			else
				_value.append(make_string_buff(std::forward<Src>(src)));
			return *this;
		}

	public:
		/** Appends path \a other to `this`. */
		path &operator/=(const path &other) { return append_string(other._value, other.formatting()); }
		/** Appends path \a other to `this`. */
		path &append(const path &other) { return append_string(other._value, other.formatting()); }

		/** Appends character range \a src to `this`. */
		template<detail::path_source Src>
		path &operator/=(Src &&src) { return append(std::forward<Src>(src)); }
		/** Appends character range \a src to `this`. */
		template<detail::path_source Src>
		path &append(Src &&src) { return append_native(std::forward<Src>(src)); }
		/** Appends character range \a src to `this` using locale \a loc for encoding conversion. */
		template<detail::path_source Src>
		path &append(Src &&src, const std::locale &loc) { return append_native(std::forward<Src>(src), loc); }

		/** Appends characters in range [\a first, \a last) to `this`. */
		template<std::forward_iterator I, std::sentinel_for<I> S> requires detail::path_char<std::iter_value_t<I>>
		path &append(I first, S last) { return append_native(make_string_buff(first, last)); }
		/** Appends characters in range [\a first, \a last) to `this` using locale \a loc for encoding conversion. */
		template<std::forward_iterator I, std::sentinel_for<I> S> requires detail::path_char<std::iter_value_t<I>>
		path &append(I first, S last, const std::locale &loc) { return append_native(make_string_buff(first, last, loc)); }

	private:
		template<typename Str>
		path &append_string(Str &&str, format fmt = auto_format)
		{
			if (path_base::is_absolute<value_type>(str, fmt))
				return operator=(std::forward<Str>(str));

			const auto this_root_name = root_name_substr<value_type>(_value, formatting());
			const auto other_root_name = root_name_substr<value_type>(str, fmt);
			if (!other_root_name.empty() && this_root_name != other_root_name)
				return operator=(std::forward<Str>(str));

			if (other_root_name.size() && other_root_name.size() != str.size() && is_separator(other_root_name.back(), fmt))
				_value.erase(this_root_name.size());
			else if (this_root_name.size() == _value.size())
			{
#ifdef ROD_WIN32
				/* Ignore drive-relative paths under windows. */
				if (this_root_name.size() > 2)
					_value.push_back(preferred_separator);
#else
				_value.push_back(preferred_separator);
#endif
			}
			else if (!is_separator(_value.back(), formatting()))
				_value.push_back(preferred_separator);

			/* _value now contains a valid parent path with a separator. */
			_value.append(str.data() + other_root_name.size(), str.data() + str.size());
			return *this;
		}

		template<typename Src>
		path &append_native(Src &&src, const std::locale &loc)
		{
			if constexpr (!decays_to<detail::path_source_value_t<std::decay_t<Src>>, value_type>)
				return append_string(make_native_string(std::forward<Src>(src), loc));
			else
				return append_string(make_string_buff(std::forward<Src>(src), loc));
		}
		template<typename Src>
		path &append_native(Src &&src)
		{
			if constexpr (!decays_to<detail::path_source_value_t<std::decay_t<Src>>, value_type>)
				return append_string(make_native_string(std::forward<Src>(src)));
			else
				return append_string(make_string_buff(std::forward<Src>(src)));
		}

	public:
		/** Returns iterator to the first sub-component of the path.
		 * @note Path iterators allocate memory for each sub-component of the path, consider using `path_view` to avoid memory allocation. */
		[[nodiscard]] iterator begin() const { return iterator(path(c_str(), c_str() + iter_begin<value_type>(_value, formatting()), formatting()), this, c_str()); }
		/** @copydoc begin */
		[[nodiscard]] iterator cbegin() const { return begin(); }

		/** Returns iterator one past the last sub-component of the path.
		 * @note Path iterators allocate memory for each sub-component of the path, consider using `path_view` to avoid memory allocation. */
		[[nodiscard]] iterator end() const { return iterator(path(formatting()), this, c_str() + native_size()); }
		/** @copydoc end */
		[[nodiscard]] iterator cend() const { return end(); }

	public:
		/** Returns the path's formatting type. */
		[[nodiscard]] constexpr format formatting() const noexcept { return _formatting; }

		/** Returns reference to the underlying native path string. */
		[[nodiscard]] constexpr const string_type &native() const & noexcept { return _value; }
		/** @copydoc native */
		[[nodiscard]] constexpr string_type &&native() && noexcept { return std::move(_value); }

		[[nodiscard]] operator string_type() const & { return _value; }
		[[nodiscard]] operator string_type() && { return std::move(_value); }

		/** Returns pointer to the native C-style string. Equivalent to `native().c_str()`. */
		[[nodiscard]] constexpr const value_type *c_str() const noexcept { return native().c_str(); }

		/** Checks if the path string is empty. */
		[[nodiscard]] constexpr bool empty() const noexcept  { return native_size() == 0; }
		/** Returns size of the native path string. */
		[[nodiscard]] constexpr size_type native_size() const noexcept  { return _value.size(); }

	public:
		/** Returns string representation of the path encoded in the specified character encoding.
		 * @param alloc Allocator used to allocate the resulting string. */
		template<typename Char, typename Traits = std::char_traits<Char>, typename Alloc = std::allocator<Char>>
		[[nodiscard]] std::basic_string<Char, Traits, Alloc> string(const Alloc &alloc = Alloc{}) const
		{
			return cvt_value<value_type, Char, Traits, Alloc>(_value, alloc).value();
		}
		/** Returns string representation of the path encoded in the specified character encoding using locale \a loc.
		 * @param loc Locale used for conversion to the target character encoding.
		 * @param alloc Allocator used to allocate the resulting string. */
		template<typename Char, typename Traits = std::char_traits<Char>, typename Alloc = std::allocator<Char>>
		[[nodiscard]] std::basic_string<Char, Traits, Alloc> string(const std::locale &loc, const Alloc &alloc = Alloc{}) const
		{
			return cvt_value<value_type, Char, Traits, Alloc>(_value, loc, alloc).value();
		}

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
			if (formatting() != binary_format)
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
			if (formatting() != binary_format)
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
		[[nodiscard]] constexpr bool is_absolute() const noexcept { return path_base::is_absolute<value_type>(_value, formatting()); }
		/** Checks if the path is relative (not absolute). */
		[[nodiscard]] constexpr bool is_relative() const noexcept { return !is_absolute(); }

		/** Returns a path containing the root path component of this path. */
		[[nodiscard]] path root_path() const { return {root_path_substr<value_type>(_value, formatting()), formatting()}; }
		/** Checks if the path has a non-empty root path. */
		[[nodiscard]] constexpr bool has_root_path() const noexcept { return !root_path_substr<value_type>(_value, formatting()).empty(); }

		/** Returns a path containing the root name component of this path. */
		[[nodiscard]] path root_name() const { return {root_name_substr<value_type>(_value, formatting()), formatting()}; }
		/** Checks if the path has a non-empty root name. */
		[[nodiscard]] constexpr bool has_root_name() const noexcept { return !root_name_substr<value_type>(_value, formatting()).empty(); }

		/** Returns a path containing the root directory component of this path. */
		[[nodiscard]] path root_directory() const { return {root_dir_substr<value_type>(_value, formatting()), formatting()}; }
		/** Checks if the path has a non-empty root directory. */
		[[nodiscard]] constexpr bool has_root_directory() const noexcept { return !root_dir_substr<value_type>(_value, formatting()).empty(); }

		/** Returns a path containing the parent path component of this path. */
		[[nodiscard]] path parent_path() const { return {parent_path_substr<value_type>(_value, formatting()), formatting()}; }
		/** Checks if the path has a non-empty parent path component. */
		[[nodiscard]] constexpr bool has_parent_path() const noexcept { return !parent_path_substr<value_type>(_value, formatting()).empty(); }

		/** Returns a path containing the relative path component of this path. */
		[[nodiscard]] path relative_path() const { return {relative_path_substr<value_type>(_value, formatting()), formatting()}; }
		/** Checks if the path has a non-empty relative path component. */
		[[nodiscard]] constexpr bool has_relative_path() const noexcept { return !relative_path_substr<value_type>(_value, formatting()).empty(); }

		/** Returns a path containing the filename component of this path. */
		[[nodiscard]] path filename() const { return {file_name_substr<value_type>(_value, formatting()), formatting()}; }
		/** Checks if the path has a non-empty filename. */
		[[nodiscard]] constexpr bool has_filename() const noexcept { return !file_name_substr<value_type>(_value, formatting()).empty(); }

		/** Returns a path containing the stem component of this path. */
		[[nodiscard]] path stem() const { return {file_stem_substr(file_name_substr<value_type>(_value, formatting())), formatting()}; }
		/** Checks if the path has a non-empty stem. */
		[[nodiscard]] constexpr bool has_stem() const noexcept { return !file_stem_substr(file_name_substr<value_type>(_value, formatting())).empty(); }

		/** Returns a path containing the extension component of this path. */
		[[nodiscard]] path extension() const { return {file_ext_substr(file_name_substr<value_type>(_value, formatting())), formatting()}; }
		/** Checks if the path has a non-empty extension. */
		[[nodiscard]] constexpr bool has_extension() const noexcept { return !file_ext_substr(file_name_substr<value_type>(_value, formatting())).empty(); }

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
			if (formatting() != binary_format)
				std::ranges::replace(_value, '/', '\\');
#endif
            return *this;
        }

		/** Removes the file name component of the path. */
		constexpr path &remove_filename() noexcept
		{
            _value.erase(find_file_name<value_type>(_value, formatting()));
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
			const auto name_start = find_file_name<value_type>(_value, formatting());
			auto name_view = string_view_type(_value.data() + name_start, _value.size() - name_start);
#ifdef ROD_WIN32
			name_view = name_view.substr(0, find_file_ads<value_type>(name_view));
#endif
			_value.erase(name_start, file_stem_size<value_type>(name_view));
			return *this;
		}
		/** Removes the extension component of the path. */
		constexpr path &remove_extension() noexcept
		{
			const auto name_start = find_file_name<value_type>(_value, formatting());
			auto name_view = string_view_type(_value.data() + name_start, _value.size() - name_start);
#ifdef ROD_WIN32
			name_view = name_view.substr(0, find_file_ads<value_type>(name_view));
#endif
			_value.erase(name_start + file_stem_size<value_type>(name_view));
			return *this;
		}

		/** Replaces file stem component of the path with \a other. */
		path &replace_stem(const path &other = path())
		{
			const auto name_start = find_file_name<value_type>(_value, formatting());
			auto name_view = string_view_type(_value.data() + name_start, _value.size() - name_start);
#ifdef ROD_WIN32
			name_view = name_view.substr(0, find_file_ads<value_type>(name_view));
#endif
			_value.replace(name_start, file_stem_size<value_type>(name_view), other._value);
			return *this;
		}
		/** Replaces extension component of the path with \a other. */
		path &replace_extension(const path &other = path()) noexcept
		{
			/* Remove extension & append a dot it `other` does not have one. */
			remove_extension();
			if (!other.empty())
			{
				if (other._value[0] != '.')
					_value.push_back('.');
				operator+=(other);
			}
			return *this;
        }

	public:
		/** Swaps contents of `this` with \a other. */
		constexpr void swap(path &other) noexcept
		{
			std::swap(_value, other._value);
			std::swap(_formatting, other._formatting);
		}

		/** Lexicographically compares `this` with \a other. */
		[[nodiscard]] constexpr int compare(const path &other) const noexcept { return compare_native(other._value, other.formatting()); }
		/** Lexicographically compares `this` with path equivalent to \a src. */
		template<detail::path_source Src>
		[[nodiscard]] constexpr int compare(const Src &src) const noexcept(noexcept(compare_native(src, auto_format))) { return compare_native(src, auto_format); }

	private:
		template<typename Src, typename C = detail::path_source_value_t<std::decay_t<Src>>>
		[[nodiscard]] constexpr int compare_native(const Src &src, format fmt) const noexcept((std::is_pointer_v<Src> || std::ranges::contiguous_range<Src>) && decays_to<C, value_type>)
		{
			if constexpr (!decays_to<C, value_type>)
				return path_base::compare<value_type>(_value, formatting(), make_native_string(src), fmt);
			else
				return path_base::compare<value_type>(_value, formatting(), make_string_buff(src), fmt);
		}

	public:
		[[nodiscard]] friend path operator/(const path &a, const path &b) { return path(a) /= b; }
		template<detail::path_source Src>
		[[nodiscard]] friend path operator/(const path &a, Src &&b) { return path(a) /= std::forward<Src>(b); }
		template<detail::path_source Src>
		[[nodiscard]] friend path operator/(Src &&a, const path &b) { return path(std::forward<Src>(a)) /= b; }

		[[nodiscard]] friend path operator+(const path &a, const path &b) { return path(a) += b; }
		template<detail::path_source Src>
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

		[[nodiscard]] friend constexpr bool operator==(const path &a, const path &b) noexcept { return a.compare(b) == 0; }
		[[nodiscard]] friend constexpr auto operator<=>(const path &a, const path &b) noexcept { return a.compare(b) <=> 0; }

		template<detail::path_source Src>
		[[nodiscard]] friend constexpr bool operator==(const path &a, const Src &b) noexcept { return a.compare(b) == 0; }
		template<detail::path_source Src>
		[[nodiscard]] friend constexpr bool operator==(const Src &a, const path &b) noexcept { return b.compare(a) == 0; }
		template<detail::path_source Src>
		[[nodiscard]] friend constexpr auto operator<=>(const path &a, const Src &b) noexcept { return a.compare(b) <=> 0; }
		template<detail::path_source Src>
		[[nodiscard]] friend constexpr auto operator<=>(const Src &a, const path &b) noexcept { return -b.compare(a) <=> 0; }

	private:
		string_type _value;
		format _formatting;
	};

	/** Swaps contents of \a a and \a b. */
	constexpr void swap(path &a, path &b) noexcept { a.swap(b); }
}

template<>
struct std::hash<rod::fs::path> {  };
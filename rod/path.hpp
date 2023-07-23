/*
 * Created by switch_blade on 2023-07-20.
 */

#pragma once

#include <string_view>
#include <array>
#include <span>

#include "utility.hpp"

namespace rod::fs
{
	namespace detail
	{
		struct path_base
		{
			using encoding = std::uint8_t;

			static constexpr encoding byte_encoding = 0;
			static constexpr encoding char_encoding = 1;
			static constexpr encoding wchar_encoding = 2;
			static constexpr encoding char8_encoding = 3;
			static constexpr encoding char16_encoding = 4;

			using format = std::uint8_t;

			static constexpr format unknown_format = 0;
			static constexpr format generic_format = 1;
			static constexpr format binary_format = 2;
			static constexpr format native_format = 3;
			static constexpr format auto_format = 4;

#ifdef ROD_WIN32
			using value_type = wchar_t;
#else
			using value_type = char;
#endif

			using string_type = std::basic_string_view<value_type>;
			using size_type = typename string_type::size_type;

#ifdef ROD_WIN32
			static constexpr value_type preferred_separator = L'\\';
#else
			static constexpr value_type preferred_separator = '/';
#endif

#if defined(FILENAME_MAX)
			static constexpr size_type default_buffer_size = std::max(1024, FILENAME_MAX);
#elif defined(PATH_MAX)
			static constexpr size_type default_buffer_size = std::max(1024, PATH_MAX);
#elif defined(MAX_PATH)
			static constexpr size_type default_buffer_size = std::max(1024, MAX_PATH);
#else
			static constexpr size_type default_buffer_size = 1024;
#endif

			template<typename C>
			static constexpr size_type lfind_separator(std::basic_string_view<C> path, format fmt, size_type idx = std::basic_string_view<C>::npos) noexcept
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
			template<typename C>
			static constexpr size_type rfind_separator(std::basic_string_view<C> path, format fmt, size_type idx = std::basic_string_view<C>::npos) noexcept
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

			template<typename C>
			static constexpr std::basic_string_view<C> find_file_name(std::basic_string_view<C> path, format fmt) noexcept
			{
				const auto sep = rfind_separator(path, fmt);
#ifdef ROD_WIN32
				if (sep == 2 && path.size() == 3)
				{
					if (path[1] == ':') /* Drive letter */
						return std::basic_string_view<C>(path.data() + 2, 1);
					else
						return path;
				}
#endif
				if (sep != std::basic_string_view<C>::npos && (sep != 0 || path.size() > 1))
					return std::basic_string_view<C>(path.data() + sep + 1, path.size() - sep - 1);
				else
					return path;
			}
			template<typename C>
			static constexpr std::basic_string_view<C> find_file_stem(std::basic_string_view<C> path) noexcept
			{
				if (const auto dot = path.rfind('.'); dot != std::basic_string_view<C>::npos && dot > 0 && (dot > 1 || path[0] != '.'))
					return std::basic_string_view<C>(path.data(), dot);
				else
					return path;
			}
			template<typename C>
			static constexpr std::basic_string_view<C> find_file_ext(std::basic_string_view<C> path) noexcept
			{
				if (const auto dot = path.rfind('.'); dot != std::basic_string_view<C>::npos && dot > 0 && (dot > 1 || path[0] != '.'))
					return std::basic_string_view<C>(path.data() + dot, path.size() - dot);
				else
					return std::basic_string_view<C>{};
			}
		};
	}

	class path : detail::path_base
	{
		using base_t = detail::path_base;

	public:
		using size_type = typename base_t::size_type;
		using value_type = typename base_t::value_type;
		using string_type = typename base_t::string_type;

		using const_iterator = typename string_type::const_iterator;
		using iterator = const_iterator;

		using format = typename base_t::format;

		using base_t::auto_format;
		using base_t::native_format;
		using base_t::binary_format;
		using base_t::generic_format;
		using base_t::unknown_format;

		using base_t::default_buffer_size;
		using base_t::preferred_separator;

	public:
		/** Returns the path's formatting type. */
		[[nodiscard]] constexpr format formatting() const noexcept  { return static_cast<format>(_formatting); }

		[[nodiscard]] path root_path() const;
		[[nodiscard]] path root_name() const;
		[[nodiscard]] path root_directory() const;

		[[nodiscard]] path parent_path() const;
		[[nodiscard]] path relative_path() const;

		/** Returns a path containing the filename component of this path. */
		[[nodiscard]] path filename() const { return {find_file_name(_string, formatting()), formatting()}; }
		/** Returns a path containing the stem component of this path. */
		[[nodiscard]] path stem() const { return {find_file_stem(find_file_name(_string, formatting())), formatting()}; }
		/** Returns a path containing the extension component of this path. */
		[[nodiscard]] path extension() const { return {find_file_ext(find_file_name(_string, formatting())), formatting()}; }

		/** Checks if the path has a non-empty filename. */
		[[nodiscard]] constexpr bool has_filename() const noexcept { return !find_file_name(_string, formatting()).empty(); }
		/** Checks if the path has a non-empty stem. */
		[[nodiscard]] constexpr bool has_stem() const noexcept { return !find_file_stem(find_file_name(_string, formatting())).empty(); }
		/** Checks if the path has a non-empty extension. */
		[[nodiscard]] constexpr bool has_extension() const noexcept { return !find_file_ext(find_file_name(_string, formatting())).empty(); }

	private:
		string_type _string;
		std::uint8_t _formatting = {};
	};

	namespace detail
	{
		template<typename F>
		concept path_view_visitor = std::invocable<F, std::span<const std::byte>> &&
		                            std::invocable<F, std::basic_string_view<char>> &&
		                            std::invocable<F, std::basic_string_view<wchar_t>> &&
		                            std::invocable<F, std::basic_string_view<char8_t>> &&
		                            std::invocable<F, std::basic_string_view<char16_t>>;
		template<typename F>
		using nothrow_view_visitor = std::conjunction<
				std::is_nothrow_invocable<F, std::span<const std::byte>>,
				std::is_nothrow_invocable<F, std::basic_string_view<char>>,
				std::is_nothrow_invocable<F, std::basic_string_view<wchar_t>>,
				std::is_nothrow_invocable<F, std::basic_string_view<char8_t>>,
				std::is_nothrow_invocable<F, std::basic_string_view<char16_t>>>;
	}

	class path_view_component : detail::path_base
	{
		using base_t = detail::path_base;

	public:
		using size_type = typename base_t::size_type;
		using format = typename base_t::format;

		using base_t::auto_format;
		using base_t::native_format;
		using base_t::binary_format;
		using base_t::generic_format;
		using base_t::unknown_format;

		using base_t::default_buffer_size;
		using base_t::preferred_separator;

	public:
		/** Initializes an empty path component of unknown format. */
		constexpr path_view_component() noexcept = default;

		/** Copy-initializes path component from \a other. */
		constexpr path_view_component(const path_view_component &other) noexcept = default;
		/** Copy-initializes path component from \a other with format \a fmt. */
		constexpr path_view_component(const path_view_component &other, format fmt) noexcept : path_view_component(other) { _formatting = fmt; }

		/** Move-initializes path component from \a other. */
		constexpr path_view_component(path_view_component &&other) noexcept = default;
		/** Move-initializes path component from \a other with format \a fmt. */
		constexpr path_view_component(path_view_component &&other, format fmt) noexcept : path_view_component(std::move(other)) { _formatting = fmt; }

		/* TODO: */
		/** Initializes path component from path \a p and format \a fmt. */
		constexpr path_view_component(const path &p, format fmt = auto_format) noexcept;

		/** Initializes a path component from a null-terminated `char` string \a str of length \a len and format \a fmt. */
		constexpr path_view_component(const char *str, format fmt = binary_format) noexcept : path_view_component(str, rod::detail::strlen(str), true, fmt) {}
		/** Initializes a path component from a `char` string \a str of length \a len, null-termination flag \a term and format \a fmt. */
		constexpr path_view_component(const char *str, size_type len, bool term, format fmt = binary_format) noexcept : _data(str), _size(len), _is_null_terminated(term), _formatting(fmt), _encoding(char_encoding) {}

		/** Initializes a path component from a `wchar_t` string \a str of length \a len, null-termination flag \a term and format \a fmt. */
		constexpr path_view_component(const wchar_t *str, format fmt = binary_format) noexcept : path_view_component(str, rod::detail::strlen(str), true, fmt) {}
		/** Initializes a path component from a null-terminated `wchar_t` string \a str of length \a len and format \a fmt. */
		constexpr path_view_component(const wchar_t *str, size_type len, bool term, format fmt = binary_format) noexcept : _data(str), _size(len), _is_null_terminated(term), _formatting(fmt), _encoding(wchar_encoding) {}

		/** Initializes a path component from a `char8_t` string \a str of length \a len, null-termination flag \a term and format \a fmt. */
		constexpr path_view_component(const char8_t *str, format fmt = binary_format) noexcept : path_view_component(str, rod::detail::strlen(str), true, fmt) {}
		/** Initializes a path component from a null-terminated `char8_t` string \a str of length \a len and format \a fmt. */
		constexpr path_view_component(const char8_t *str, size_type len, bool term, format fmt = binary_format) noexcept : _data(str), _size(len), _is_null_terminated(term), _formatting(fmt), _encoding(char8_encoding) {}

		/** Initializes a path component from a `char16_t` string \a str of length \a len, null-termination flag \a term and format \a fmt. */
		constexpr path_view_component(const char16_t *str, format fmt = binary_format) noexcept : path_view_component(str, rod::detail::strlen(str), true, fmt) {}
		/** Initializes a path component from a null-terminated `char16_t` string \a str of length \a len and format \a fmt. */
		constexpr path_view_component(const char16_t *str, size_type len, bool term, format fmt = binary_format) noexcept : _data(str), _size(len), _is_null_terminated(term), _formatting(fmt), _encoding(char16_encoding) {}

		/** Initializes a path component from a null-terminated byte array \a data of length \a len and format \a fmt. */
		constexpr path_view_component(const std::byte *data, format fmt = binary_format) noexcept : path_view_component(data, rod::detail::strlen(data), true, fmt) {}
		/** Initializes a path component from a byte array \a data of length \a len, null-termination flag \a term and format \a fmt. */
		constexpr path_view_component(const std::byte *data, size_type len, bool term, format fmt = binary_format) noexcept : _data(data), _size(len), _is_null_terminated(term), _formatting(fmt), _encoding(byte_encoding) {}

		/** Initializes a path component from string \a str and format \a fmt. */
		template<typename C = base_t::value_type, typename T = std::char_traits<C>, typename Alloc = std::allocator<C>>
		constexpr path_view_component(const std::basic_string<C, T, Alloc> &str, format fmt = binary_format) noexcept : path_view_component(str.data(), str.size(), true, fmt) {}
		/** Initializes a path component from string view \a str, null-termination flag \a term and format \a fmt. */
		template<typename C = base_t::value_type, typename T = std::char_traits<C>>
		constexpr path_view_component(std::basic_string_view<C, T> str, bool term, format fmt = binary_format) noexcept : path_view_component(str.data(), str.size(), term, fmt) {}

		/** Returns the native size (in codepoints or bytes) of the path component. */
		[[nodiscard]] constexpr size_type native_size() const noexcept { return _size; }
		/** Returns the path component's formatting type. */
		[[nodiscard]] constexpr format formatting() const noexcept  { return static_cast<format>(_formatting); }

		/** Checks if the path component is empty. */
		[[nodiscard]] constexpr bool empty() const noexcept { return native_size() == 0; }
		/** Checks if the path component has a null terminator. */
		[[nodiscard]] constexpr bool is_null_terminated() const noexcept { return _is_null_terminated; }

	private:
		template<typename F>
		constexpr auto visit(F &&f) const noexcept
		{
			switch (_encoding)
			{
			case byte_encoding:
				return std::invoke(std::forward<F>(f), std::span{static_cast<const std::byte *>(_data), _size});
			case char_encoding:
				return std::invoke(std::forward<F>(f), std::basic_string_view{static_cast<const char *>(_data), _size});
			case wchar_encoding:
				return std::invoke(std::forward<F>(f), std::basic_string_view{static_cast<const wchar_t *>(_data), _size});
			case char8_encoding:
				return std::invoke(std::forward<F>(f), std::basic_string_view{static_cast<const char8_t *>(_data), _size});
			case char16_encoding:
				return std::invoke(std::forward<F>(f), std::basic_string_view{static_cast<const char16_t *>(_data), _size});
			}
		}

	public:
		/** Returns a sub-component containing the stem of this path component. */
		[[nodiscard]] constexpr path_view_component stem() const noexcept
		{
			return visit([&]<typename Str>(Str str) -> path_view_component
			{
				if constexpr (!decays_to<std::ranges::range_value_t<Str>, std::byte>)
				{
					const auto result = find_file_stem(find_file_name(str, formatting()));
					return {result, str.ends_with(result) && is_null_terminated(), formatting()};
				}
				return *this;
			});
		}
		/** Returns a sub-component containing the extension of this path component. */
		[[nodiscard]] constexpr path_view_component extension() const noexcept
		{
			return visit([&]<typename Str>(Str str) -> path_view_component
			{
				if constexpr (!decays_to<std::ranges::range_value_t<Str>, std::byte>)
				{
					const auto result = find_file_ext(find_file_name(str, formatting()));
					return {result, str.ends_with(result) && is_null_terminated(), formatting()};
				}
				return {};
			});
		}

		/** Checks if the path component has a non-empty stem. */
		[[nodiscard]] constexpr bool has_stem() const noexcept { return !stem().empty(); }
		/** Checks if the path component has a non-empty extension. */
		[[nodiscard]] constexpr bool has_extension() const noexcept { return !extension().empty(); }

		constexpr void swap(path_view_component &other) noexcept
		{
			std::swap(_data, other._data);
			std::swap(_size, other._size);
			std::swap(_encoding, other._encoding);
			std::swap(_formatting, other._formatting);
			std::swap(_is_null_terminated, other._is_null_terminated);
		}
		friend constexpr void swap(path_view_component &a, path_view_component &b) noexcept { a.swap(b); }

		template<detail::path_view_visitor F>
		friend inline constexpr auto visit(F &&f, path_view_component p) noexcept(detail::nothrow_view_visitor<F>::value);

	private:
		const void *_data = {};
		std::size_t _size = {};
		std::uint8_t _encoding = {};
		std::uint8_t _formatting = {};
		bool _is_null_terminated = {};
	};

	/** Invokes \a f with a string view or span of bytes depending on the internal encoding of path component \a p. */
	template<detail::path_view_visitor F>
	inline constexpr auto visit(F &&f, path_view_component p) noexcept(detail::nothrow_view_visitor<F>::value) { return p.visit(std::forward<F>(f)); }
}
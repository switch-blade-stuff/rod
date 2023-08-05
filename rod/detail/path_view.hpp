/*
 * Created by switch_blade on 2023-07-30.
 */

#pragma once

#include "path_base.hpp"

namespace rod::fs
{
	namespace detail
	{
		template<typename F>
		concept path_view_visitor = std::invocable<F, std::span<const std::byte>> &&
		                            std::invocable<F, std::basic_string_view<char>> &&
		                            std::invocable<F, std::basic_string_view<wchar_t>> &&
		                            std::invocable<F, std::basic_string_view<char8_t>> &&
		                            std::invocable<F, std::basic_string_view<char16_t>>;
		template<typename F>
		using nothrow_path_view_visitor = std::conjunction<
				std::is_nothrow_invocable<F, std::span<const std::byte>>,
				std::is_nothrow_invocable<F, std::basic_string_view<char>>,
				std::is_nothrow_invocable<F, std::basic_string_view<wchar_t>>,
				std::is_nothrow_invocable<F, std::basic_string_view<char8_t>>,
				std::is_nothrow_invocable<F, std::basic_string_view<char16_t>>>;
	}

	/** Non-owning view of a portion of a filesystem path. */
	class path_view_component : detail::path_base
	{
		using path_base = detail::path_base;

	public:
		using size_type = typename path_base::size_type;
		using format = typename path_base::format;

		using path_base::auto_format;
		using path_base::native_format;
		using path_base::binary_format;
		using path_base::generic_format;
		using path_base::unknown_format;

		using path_base::default_buffer_size;
		using path_base::preferred_separator;

	public:
		/** Initializes an empty path component of unknown format. */
		constexpr path_view_component() noexcept : path_view_component(unknown_format) {}
		/** Initializes an empty path component with format \a fmt. */
		constexpr path_view_component(format fmt) noexcept : _data(nullptr), _size(0), _is_null_terminated(false), _formatting(fmt), _encoding(byte_encoding) {}

		/** Copy-initializes path component from \a other. */
		constexpr path_view_component(const path_view_component &other) noexcept = default;
		/** Copy-initializes path component from \a other with format \a fmt. */
		constexpr path_view_component(const path_view_component &other, format fmt) noexcept : path_view_component(other) { _formatting = fmt; }

		/** Move-initializes path component from \a other. */
		constexpr path_view_component(path_view_component &&other) noexcept = default;
		/** Move-initializes path component from \a other with format \a fmt. */
		constexpr path_view_component(path_view_component &&other, format fmt) noexcept : path_view_component(std::forward<path_view_component>(other)) { _formatting = fmt; }

		/** Initializes a path component from a null-terminated `char` string \a str of length \a len and format \a fmt. */
		constexpr path_view_component(const char *str, format fmt = binary_format) noexcept : path_view_component(str, rod::detail::strlen(str), true, fmt) {}
		/** Initializes a path component from a `char` string \a str of length \a len, null-termination flag \a term and format \a fmt. */
		constexpr path_view_component(const char *str, size_type len, bool term, format fmt = binary_format) noexcept : _data(str), _size(len * sizeof(char)), _is_null_terminated(term), _formatting(fmt), _encoding(char_encoding) {}

		/** Initializes a path component from a `wchar_t` string \a str of length \a len, null-termination flag \a term and format \a fmt. */
		constexpr path_view_component(const wchar_t *str, format fmt = binary_format) noexcept : path_view_component(str, rod::detail::strlen(str), true, fmt) {}
		/** Initializes a path component from a null-terminated `wchar_t` string \a str of length \a len and format \a fmt. */
		constexpr path_view_component(const wchar_t *str, size_type len, bool term, format fmt = binary_format) noexcept : _data(str), _size(len * sizeof(wchar_t)), _is_null_terminated(term), _formatting(fmt), _encoding(wchar_encoding) {}

		/** Initializes a path component from a `char8_t` string \a str of length \a len, null-termination flag \a term and format \a fmt. */
		constexpr path_view_component(const char8_t *str, format fmt = binary_format) noexcept : path_view_component(str, rod::detail::strlen(str), true, fmt) {}
		/** Initializes a path component from a null-terminated `char8_t` string \a str of length \a len and format \a fmt. */
		constexpr path_view_component(const char8_t *str, size_type len, bool term, format fmt = binary_format) noexcept : _data(str), _size(len * sizeof(char8_t)), _is_null_terminated(term), _formatting(fmt), _encoding(char8_encoding) {}

		/** Initializes a path component from a `char16_t` string \a str of length \a len, null-termination flag \a term and format \a fmt. */
		constexpr path_view_component(const char16_t *str, format fmt = binary_format) noexcept : path_view_component(str, rod::detail::strlen(str), true, fmt) {}
		/** Initializes a path component from a null-terminated `char16_t` string \a str of length \a len and format \a fmt. */
		constexpr path_view_component(const char16_t *str, size_type len, bool term, format fmt = binary_format) noexcept : _data(str), _size(len * sizeof(char16_t)), _is_null_terminated(term), _formatting(fmt), _encoding(char16_encoding) {}

		/** Initializes a path component from a null-terminated byte array \a data of length \a len and format \a fmt. */
		constexpr path_view_component(const std::byte *data, format fmt = binary_format) noexcept : path_view_component(data, rod::detail::strlen(data), true, fmt) {}
		/** Initializes a path component from a byte array \a data of length \a len, null-termination flag \a term and format \a fmt. */
		constexpr path_view_component(const std::byte *data, size_type len, bool term, format fmt = binary_format) noexcept : _data(data), _size(len), _is_null_terminated(term), _formatting(fmt), _encoding(byte_encoding) {}

		/** Initializes path component from path \a p and format \a fmt. */
		path_view_component(const path &p) noexcept : path_view_component(p._value.data(), p._value.size(), true, p.formatting()) {}
		/** Initializes a path component from string \a str and format \a fmt. */
		template<typename C = path_base::value_type, typename T = std::char_traits<C>, typename Alloc = std::allocator<C>> requires detail::path_char_type<C>
		constexpr path_view_component(const std::basic_string<C, T, Alloc> &str, format fmt = binary_format) noexcept : path_view_component(str.data(), str.size(), true, fmt) {}
		/** Initializes a path component from string view \a str, null-termination flag \a term and format \a fmt. */
		template<typename C = path_base::value_type, typename T = std::char_traits<C>> requires detail::path_char_type<C>
		constexpr path_view_component(std::basic_string_view<C, T> str, bool term, format fmt = binary_format) noexcept : path_view_component(str.data(), str.size(), term, fmt) {}

		constexpr path_view_component &operator=(const path_view_component &) noexcept = default;
		constexpr path_view_component &operator=(path_view_component &&) noexcept = default;

	protected:
		template<typename F>
		constexpr auto visit(F &&f) const noexcept(detail::nothrow_path_view_visitor<F>::value)
		{
			switch (_encoding)
			{
			case byte_encoding:
				return std::invoke(std::forward<F>(f), std::span{static_cast<const std::byte *>(_data), _size});
			case char_encoding:
				return std::invoke(std::forward<F>(f), std::basic_string_view{static_cast<const char *>(_data), _size / sizeof(char)});
			case wchar_encoding:
				return std::invoke(std::forward<F>(f), std::basic_string_view{static_cast<const wchar_t *>(_data), _size / sizeof(wchar_t)});
			case char8_encoding:
				return std::invoke(std::forward<F>(f), std::basic_string_view{static_cast<const char8_t *>(_data), _size / sizeof(char8_t)});
			case char16_encoding:
				return std::invoke(std::forward<F>(f), std::basic_string_view{static_cast<const char16_t *>(_data), _size / sizeof(char16_t)});
			}
		}

		void iter_next(const path_view_component &base, const std::byte *&pos) noexcept
		{
			base.visit([&]<typename V>(V base_view) noexcept
			{
				using value_type = std::ranges::range_value_t<V>;
				if constexpr (decays_to<value_type, std::byte>)
					operator=(path_view_component(pos = base_view.data() + base_view.size(), 0, base.is_null_terminated()));
				else
				{
					const auto bytes = static_cast<const std::byte *>(base._data);
					const auto comp_data = static_cast<const value_type *>(_data);
					auto comp_view = std::basic_string_view(comp_data, native_size());
					auto new_pos = base_view.data() + (pos - bytes) / sizeof(value_type);

					comp_view = path_base::iter_next<value_type>(comp_view, base_view, new_pos, base.formatting());
					const auto term = base.is_null_terminated() && comp_view.data() + comp_view.size() == base_view.data() + base_view.size();

					pos = bytes + (new_pos - base_view.data()) * sizeof(value_type);
					operator=(path_view_component(comp_view, term));
				}
			});
		}
		void iter_prev(const path_view_component &base, const std::byte *&pos) noexcept
		{
			base.visit([&]<typename V>(V base_view) noexcept
			{
				using value_type = std::ranges::range_value_t<V>;
				if constexpr (decays_to<value_type, std::byte>)
					operator=(path_view_component(pos = base_view.data(), base_view.size(), base.is_null_terminated()));
				else
				{
					const auto bytes = static_cast<const std::byte *>(base._data);
					const auto comp_data = static_cast<const value_type *>(_data);
					auto comp_view = std::basic_string_view(comp_data, native_size());
					auto new_pos = base_view.data() + (pos - bytes) / sizeof(value_type);

					comp_view = path_base::iter_prev<value_type>(comp_view, base_view, new_pos, base.formatting());
					const auto term = base.is_null_terminated() && comp_view.data() + comp_view.size() == base_view.data() + base_view.size();

					pos = bytes + (new_pos - base_view.data()) * sizeof(value_type);
					operator=(path_view_component(comp_view, term));
				}
			});
		}

		template<void (path_view_component::*Func)(const path_view_component &, const std::byte *&) noexcept>
		struct iter_func { void operator()(path_view_component &comp, const path_view_component &base, const std::byte *&pos) const noexcept { (comp.*Func)(base, pos); } };

	public:
		/** Returns the path component's formatting type. */
		[[nodiscard]] constexpr format formatting() const noexcept  { return _formatting; }
		/** Returns the native size (in codepoints or bytes) of the path component. */
		[[nodiscard]] constexpr size_type native_size() const noexcept { return visit([&]<typename V>(V sv) noexcept { return _size / sizeof(std::ranges::range_value_t<V>); }); }

		/** Checks if the path component is empty. */
		[[nodiscard]] constexpr bool empty() const noexcept { return native_size() == 0; }
		/** Checks if the path component has a null terminator. */
		[[nodiscard]] constexpr bool is_null_terminated() const noexcept { return _is_null_terminated; }

		/** Returns a sub-component containing the stem of this path component. */
		[[nodiscard]] constexpr path_view_component stem() const noexcept
		{
			return visit([&]<typename Str>(Str str) noexcept -> path_view_component
			{
				if constexpr (!decays_to<std::ranges::range_value_t<Str>, std::byte>)
				{
					const auto result = file_stem_substr(file_name_substr(str, formatting()));
					return {result, str.ends_with(result) && is_null_terminated(), formatting()};
				}
				return *this;
			});
		}
		/** Returns a sub-component containing the extension of this path component. */
		[[nodiscard]] constexpr path_view_component extension() const noexcept
		{
			return visit([&]<typename Str>(Str str) noexcept -> path_view_component
			{
				if constexpr (!decays_to<std::ranges::range_value_t<Str>, std::byte>)
				{
					const auto result = file_ext_substr(file_name_substr(str, formatting()));
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
		friend inline constexpr auto visit(F &&f, path_view_component p) noexcept(detail::nothrow_path_view_visitor<F>::value);

	protected:
		const void *_data;
		std::size_t _size;
		std::uint8_t _encoding;
		std::uint8_t _formatting;
		bool _is_null_terminated;
	};

	/** Non-owning view of a filesystem path. */
	class path_view : public path_view_component
	{
		using path_base = detail::path_base;

	public:
		using size_type = typename path_view_component::size_type;
		using format = typename path_view_component::format;

		using path_view_component::auto_format;
		using path_view_component::native_format;
		using path_view_component::binary_format;
		using path_view_component::generic_format;
		using path_view_component::unknown_format;

		using path_view_component::default_buffer_size;
		using path_view_component::preferred_separator;

	public:
		using iterator = typename path_base::component_iterator<path_view_component, const std::byte *, iter_func<&path_view::iter_next>, iter_func<&path_view::iter_prev>>;
		using const_iterator = iterator;

	public:
		using path_view_component::path_view_component;
		using path_view_component::operator=;

	public:
		/** Returns iterator to the first sub-component of the path. */
		[[nodiscard]] constexpr iterator begin() const noexcept
		{
			return path_view_component::visit([&]<typename V>(V base_view) noexcept
			{
				if constexpr (decays_to<std::ranges::range_value_t<V>, std::byte>)
					return iterator(*this, this, static_cast<const std::byte *>(_data));
				else
				{
					const auto comp_size = path_base::iter_begin(base_view, formatting());
					const auto term = is_null_terminated() && comp_size == base_view.size();
					return iterator(path_view_component(base_view.data(), comp_size, term), this, static_cast<const std::byte *>(_data));
				}
			});
		}
		/** @copydoc begin */
		[[nodiscard]] constexpr iterator cbegin() const noexcept { return begin(); }

		/** Returns iterator one past the last sub-component of the path. */
		[[nodiscard]] constexpr iterator end() const noexcept { return iterator({}, this, static_cast<const std::byte *>(_data) + _size); }
		/** @copydoc end */
		[[nodiscard]] constexpr iterator cend() const noexcept { return end(); }

	};

	/** Invokes \a f with a string view or span of bytes depending on the internal encoding of path component \a p. */
	template<detail::path_view_visitor F>
	inline constexpr auto visit(F &&f, path_view_component p) noexcept(detail::nothrow_path_view_visitor<F>::value) { return p.visit(std::forward<F>(f)); }
	/** Invokes \a f with a string view or span of bytes depending on the internal encoding of path view \a p. */
	template<detail::path_view_visitor F>
	inline constexpr auto visit(F &&f, path_view p) noexcept(detail::nothrow_path_view_visitor<F>::value) { return visit(std::forward<F>(f), static_cast<const path_view_component &>(p)); }

	namespace detail
	{
		struct path_view_like
		{
			constexpr path_view_like() noexcept = default;
			template<typename T> requires(std::is_convertible_v<T, path_view> && !std::is_convertible_v<T, path>)
			constexpr path_view_like(T &&value) noexcept(std::is_nothrow_constructible_v<path_view, T>) : view(std::forward<T>(value)) {}

			path_view view;
		};

		inline path path_from_view_like(detail::path_view_like p, path::format fmt)
		{
			return visit([&]<typename V>(V view)
			{
				if constexpr(std::same_as<V, std::span<const std::byte>>)
					return path_from_binary(view);
				else
					return path(view, fmt);
			}, p.view);
		}
		inline path path_from_view_like(detail::path_view_like p, const std::locale &loc, path::format fmt)
		{
			return visit([&]<typename V>(V view)
			{
				if constexpr(std::same_as<V, std::span<const std::byte>>)
					return path_from_binary(view);
				else
					return path(view, loc, fmt);
			}, p.view);
		}
	}

	path::path(detail::path_view_like p, path::format fmt) : path(detail::path_from_view_like(p, fmt)) {}
	path::path(detail::path_view_like p, const std::locale &loc, path::format fmt) : path(detail::path_from_view_like(p, loc, fmt)) {}
}

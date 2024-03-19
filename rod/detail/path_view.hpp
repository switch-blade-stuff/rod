/*
 * Created by switchblade on 2023-07-30.
 */

#pragma once

#include "path_base.hpp"

namespace rod
{
	namespace _path
	{
		template<typename F>
		concept visitor = std::invocable<F, std::span<const std::byte>> &&
			                        std::invocable<F, std::basic_string_view<char>> &&
		                            std::invocable<F, std::basic_string_view<wchar_t>> &&
		                            std::invocable<F, std::basic_string_view<char8_t>> &&
		                            std::invocable<F, std::basic_string_view<char16_t>>;
		template<typename F>
		using nothrow_visitor = std::conjunction<
				std::is_nothrow_invocable<F, std::span<const std::byte>>,
				std::is_nothrow_invocable<F, std::basic_string_view<char>>,
				std::is_nothrow_invocable<F, std::basic_string_view<wchar_t>>,
				std::is_nothrow_invocable<F, std::basic_string_view<char8_t>>,
				std::is_nothrow_invocable<F, std::basic_string_view<char16_t>>>;
		template<typename F>
		using visitor_result = std::common_type_t<
				std::invoke_result_t<F, std::span<const std::byte>>,
				std::invoke_result_t<F, std::basic_string_view<char>>,
				std::invoke_result_t<F, std::basic_string_view<wchar_t>>,
				std::invoke_result_t<F, std::basic_string_view<char8_t>>,
				std::invoke_result_t<F, std::basic_string_view<char16_t>>>;

		enum class encoding : std::uint8_t
		{
			byte = 0,
			cchar = 1,
			wchar = 2,
			char8 = 3,
			char16 = 4,
			char32 = 5,
		};

		/** Non-owning view of a portion of a filesystem path. */
		class path_view_component : public constants
		{
		public:
			using format_type = typename _path::format_type;
			using size_type = typename _path::size_type;

			using constants::auto_format;
			using constants::native_format;
			using constants::binary_format;
			using constants::generic_format;
			using constants::unknown_format;

			/** Tag type used to select an implementation-defined allocator for rendered paths. */
			template<typename T>
			struct default_rendered_path_allocator {};

			using constants::default_buffer_size;
			using constants::preferred_separator;

		private:
			template<typename T>
			struct alloc_alias { using allocator_type = T; };
			template<instance_of<default_rendered_path_allocator> T>
			struct alloc_alias<T> {};

		public:
			/** Structure used to represent a path (or component of a path) rendered to a specified encoding.
			 * @tparam Term String termination flag. If set to `true`, rendered path is null-terminated.
			 * @tparam T Character type of the target path encoding. Uses the implementation-defined native path character by default.
			 * @tparam Alloc Allocator type used for memory allocation when the internal buffer is not large enough. Uses implementation-defined allocator by default.
			 * @tparam BuffSize Size of the internal buffer used for in-place storage of the rendered path. Uses implementation-defined buffer size by default.  */
			template<bool Term, typename T = typename _path::value_type, typename Alloc = default_rendered_path_allocator<T>, size_type BuffSize = default_buffer_size>
			class rendered_path;

		public:
			/** Initializes an empty path component of unknown format. */
			constexpr path_view_component() noexcept : path_view_component(unknown_format) {}
			/** Initializes an empty path component with format \a fmt. */
			constexpr path_view_component(format_type fmt) noexcept : _data(nullptr), _size(0), _encoding(encoding::byte), _format(fmt), _is_null_terminated(false) {}

			/** Copy-initializes path component from \a other. */
			constexpr path_view_component(const path_view_component &other) noexcept = default;
			/** Copy-initializes path component from \a other with format \a fmt. */
			constexpr path_view_component(const path_view_component &other, format_type fmt) noexcept : path_view_component(other) { _format = fmt; }

			/** Move-initializes path component from \a other. */
			constexpr path_view_component(path_view_component &&other) noexcept = default;
			/** Move-initializes path component from \a other with format \a fmt. */
			constexpr path_view_component(path_view_component &&other, format_type fmt) noexcept : path_view_component(std::forward<path_view_component>(other)) { _format = fmt; }

			constexpr path_view_component &operator=(const path_view_component &) noexcept = default;
			constexpr path_view_component &operator=(path_view_component &&) noexcept = default;

			/** Initializes a path component from a null-terminated `char` string \a str and format \a fmt. */
			constexpr path_view_component(const char *str, format_type fmt = binary_format) noexcept : path_view_component(str, rod::_detail::strlen(str), true, fmt) {}
			/** Initializes a path component from a `char` string \a str of length \a len, null-termination flag \a term and format \a fmt. */
			constexpr path_view_component(const char *str, size_type len, bool term, format_type fmt = binary_format) noexcept : _data(str), _size(len * sizeof(char)), _encoding(encoding::cchar), _format(fmt), _is_null_terminated(term) {}

			/** Initializes a path component from a null-terminated `wchar_t` string \a str and format \a fmt. */
			constexpr path_view_component(const wchar_t *str, format_type fmt = binary_format) noexcept : path_view_component(str, rod::_detail::strlen(str), true, fmt) {}
			/** Initializes a path component from a `wchar_t` string \a str of length \a len, null-termination flag \a term and format \a fmt. */
			constexpr path_view_component(const wchar_t *str, size_type len, bool term, format_type fmt = binary_format) noexcept : _data(str), _size(len * sizeof(wchar_t)), _encoding(encoding::wchar), _format(fmt), _is_null_terminated(term) {}

			/** Initializes a path component from a null-terminated `char8_t` string \a str and format \a fmt. */
			constexpr path_view_component(const char8_t *str, format_type fmt = binary_format) noexcept : path_view_component(str, rod::_detail::strlen(str), true, fmt) {}
			/** Initializes a path component from a `char8_t` string \a str of length \a len, null-termination flag \a term and format \a fmt. */
			constexpr path_view_component(const char8_t *str, size_type len, bool term, format_type fmt = binary_format) noexcept : _data(str), _size(len * sizeof(char8_t)), _encoding(encoding::char8), _format(fmt), _is_null_terminated(term) {}

			/** Initializes a path component from a null-terminated `char16_t` string \a str and format \a fmt. */
			constexpr path_view_component(const char16_t *str, format_type fmt = binary_format) noexcept : path_view_component(str, rod::_detail::strlen(str), true, fmt) {}
			/** Initializes a path component from a `char16_t` string \a str of length \a len, null-termination flag \a term and format \a fmt. */
			constexpr path_view_component(const char16_t *str, size_type len, bool term, format_type fmt = binary_format) noexcept : _data(str), _size(len * sizeof(char16_t)), _encoding(encoding::char16), _format(fmt), _is_null_terminated(term) {}

			/** Initializes a path component from a null-terminated `char32_t` string \a str and format \a fmt. */
			constexpr path_view_component(const char32_t *str, format_type fmt = binary_format) noexcept : path_view_component(str, rod::_detail::strlen(str), true, fmt) {}
			/** Initializes a path component from a `char32_t` string \a str of length \a len, null-termination flag \a term and format \a fmt. */
			constexpr path_view_component(const char32_t *str, size_type len, bool term, format_type fmt = binary_format) noexcept : _data(str), _size(len * sizeof(char32_t)), _encoding(encoding::char32), _format(fmt), _is_null_terminated(term) {}

			/** Initializes a path component from a null-terminated byte array \a data and format \a fmt. */
			constexpr path_view_component(const std::byte *data, format_type fmt = binary_format) noexcept : path_view_component(data, rod::_detail::strlen(data), true, fmt) {}
			/** Initializes a path component from a byte array \a data of length \a len, null-termination flag \a term and format \a fmt. */
			constexpr path_view_component(const std::byte *data, size_type len, bool term, format_type fmt = binary_format) noexcept : _data(data), _size(len), _encoding(encoding::byte), _format(fmt), _is_null_terminated(term) {}

			/** Initializes path component from path \a p. */
			constexpr path_view_component(const path &p) noexcept : path_view_component(p.native().data(), p.native().size(), true, p.format()) {}
			/** Initializes a path component from string \a str and format \a fmt. */
			template<accepted_char C = value_type, typename T = std::char_traits<C>, typename Alloc = std::allocator<C>>
			constexpr path_view_component(const std::basic_string<C, T, Alloc> &str, format_type fmt = binary_format) noexcept : path_view_component(str.data(), str.size(), true, fmt) {}
			/** Initializes a path component from string view \a str, null-termination flag \a term and format \a fmt. */
			template<accepted_char C = value_type, typename T = std::char_traits<C>>
			constexpr path_view_component(std::basic_string_view<C, T> str, bool term, format_type fmt = binary_format) noexcept : path_view_component(str.data(), str.size(), term, fmt) {}

		protected:
			template<typename T>
			[[nodiscard]] constexpr auto view() const noexcept
			{
				if constexpr (!one_of<T, std::byte, std::uint8_t>)
					return std::basic_string_view(static_cast<const T *>(_data), _size / sizeof(T));
				else
					return std::span(static_cast<const T *>(_data), _size);
			}

			template<typename F>
			constexpr visitor_result<F> visit(F &&f) const noexcept(nothrow_visitor<F>::value)
			{
				switch (_encoding)
				{
				default: return std::invoke(std::forward<F>(f), view<std::byte>());
				case encoding::cchar: return std::invoke(std::forward<F>(f), view<char>());
				case encoding::wchar: return std::invoke(std::forward<F>(f), view<wchar_t>());
				case encoding::char8: return std::invoke(std::forward<F>(f), view<char8_t>());
				case encoding::char16: return std::invoke(std::forward<F>(f), view<char16_t>());
				case encoding::char32: return std::invoke(std::forward<F>(f), view<char32_t>());
				}
			}

			void iter_next(const path_view_component &base, const std::byte *&pos) noexcept
			{
				base.visit([&]<typename V>(V base_view) noexcept
				{
					using view_value = std::ranges::range_value_t<V>;
					if constexpr (decays_to_same<view_value, std::byte>)
						operator=(path_view_component(pos = base_view.data() + base_view.size(), 0, base.is_null_terminated()));
					else
					{
						const auto bytes = static_cast<const std::byte *>(base._data);
						const auto comp_data = static_cast<const view_value *>(_data);
						auto comp_view = std::basic_string_view(comp_data, native_size());
						auto new_pos = base_view.data() + (pos - bytes) / sizeof(value_type);

						comp_view = _path::iter_next<view_value>(comp_view, base_view, new_pos, base.format());
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
					using view_value = std::ranges::range_value_t<V>;
					if constexpr (decays_to_same<view_value, std::byte>)
						operator=(path_view_component(pos = base_view.data(), base_view.size(), base.is_null_terminated()));
					else
					{
						const auto bytes = static_cast<const std::byte *>(base._data);
						const auto comp_data = static_cast<const view_value *>(_data);
						auto comp_view = std::basic_string_view(comp_data, native_size());
						auto new_pos = base_view.data() + (pos - bytes) / sizeof(value_type);

						comp_view = _path::iter_prev<view_value>(comp_view, base_view, new_pos, base.format());
						const auto term = base.is_null_terminated() && comp_view.data() + comp_view.size() == base_view.data() + base_view.size();

						pos = bytes + (new_pos - base_view.data()) * sizeof(value_type);
						operator=(path_view_component(comp_view, term));
					}
				});
			}

			template<void (path_view_component::*Func)(const path_view_component &, const std::byte *&) noexcept>
			struct iter_func { void operator()(path_view_component &comp, const path_view_component &base, const std::byte *&pos) const noexcept { (comp.*Func)(base, pos); }};

		public:
			/** Returns the path component's formatting type. */
			[[nodiscard]] constexpr format_type format() const noexcept { return _format; }
			/** Returns the native size (in codepoints or bytes) of the path component. */
			[[nodiscard]] constexpr size_type native_size() const noexcept { return visit([&]<typename V>(V) noexcept { return _size / sizeof(std::ranges::range_value_t<V>); }); }

			/** Checks if the path component is empty. */
			[[nodiscard]] constexpr bool empty() const noexcept { return native_size() == 0; }
			/** Checks if the path component has a null terminator. */
			[[nodiscard]] constexpr bool is_null_terminated() const noexcept { return _is_null_terminated; }

			/** Returns a sub-component containing the stem of this path component. */
			[[nodiscard]] constexpr path_view_component stem() const noexcept
			{
				return visit([&]<typename Str>(Str str) noexcept -> path_view_component
				{
					if constexpr (!decays_to_same<std::ranges::range_value_t<Str>, std::byte>)
					{
						const auto result = file_stem_substr(file_name_substr(str, format()));
						return {result, str.ends_with(result) && is_null_terminated(), format()};
					}
					return *this;
				});
			}
			/** Returns a sub-component containing the extension of this path component. */
			[[nodiscard]] constexpr path_view_component extension() const noexcept
			{
				return visit([&]<typename Str>(Str str) noexcept -> path_view_component
				{
					if constexpr (!decays_to_same<std::ranges::range_value_t<Str>, std::byte>)
					{
						const auto result = file_ext_substr(file_name_substr(str, format()));
						return {result, str.ends_with(result) && is_null_terminated(), format()};
					}
					return {};
				});
			}

			/** Checks if the path component has a non-empty stem. */
			[[nodiscard]] constexpr bool has_stem() const noexcept { return !stem().empty(); }
			/** Checks if the path component has a non-empty extension. */
			[[nodiscard]] constexpr bool has_extension() const noexcept { return !extension().empty(); }

		public:
			/** Renders a null-terminated path component encoded into character type \a T with allocator \a alloc and buffer size \a BuffSize. */
			template<typename T = value_type, typename Alloc = default_rendered_path_allocator<T>, size_type BuffSize = default_buffer_size>
			constexpr rendered_path<true, T, Alloc, BuffSize> render_null_terminated(const Alloc &alloc = Alloc()) const;
			/** Renders a null-terminated path component encoded into character type \a T with allocator \a alloc and buffer size \a BuffSize, using locale \a loc for encoding conversion. */
			template<typename T = value_type, typename Alloc = default_rendered_path_allocator<T>, size_type BuffSize = default_buffer_size>
			constexpr rendered_path<true, T, Alloc, BuffSize> render_null_terminated(const std::locale &loc, const Alloc &alloc = Alloc()) const;

			/** Renders an unterminated path component encoded into character type \a T with allocator \a alloc and buffer size \a BuffSize. */
			template<typename T = value_type, typename Alloc = default_rendered_path_allocator<T>, size_type BuffSize = default_buffer_size>
			constexpr rendered_path<false, T, Alloc, BuffSize> render_unterminated(const Alloc &alloc = Alloc()) const;
			/** Renders an unterminated path component encoded into character type \a T with allocator \a alloc and buffer size \a BuffSize, using locale \a loc for encoding conversion. */
			template<typename T = value_type, typename Alloc = default_rendered_path_allocator<T>, size_type BuffSize = default_buffer_size>
			constexpr rendered_path<false, T, Alloc, BuffSize> render_unterminated(const std::locale &loc, const Alloc &alloc = Alloc()) const;

		public:
			/** Lexicographically compares `this` with \a other. */
			template<typename T = value_type, typename Alloc = default_rendered_path_allocator<T>, size_type BuffSize = default_buffer_size>
			[[nodiscard]] inline constexpr int compare(path_view_component other) const noexcept;

		public:
			friend inline constexpr bool operator<(path_view_component a, path_view_component b) noexcept;
			friend inline constexpr bool operator==(path_view_component a, path_view_component b) noexcept;
			friend inline constexpr auto operator<=>(path_view_component a, path_view_component b) noexcept;

			template<visitor F>
			friend inline constexpr visitor_result<F> visit(F &&f, path_view_component p) noexcept(nothrow_visitor<F>::value);

		protected:
			const void *_data;
			std::size_t _size;
			encoding _encoding;
			format_type _format;
			bool _is_null_terminated;
		};

		/** Invokes \a f with a string view or span of bytes depending on the internal encoding of path component \a p. */
		template<visitor F>
		inline constexpr visitor_result<F> visit(F &&f, path_view_component p) noexcept(nothrow_visitor<F>::value) { return p.visit(std::forward<F>(f)); }

		/** Preforms a shallow less-than comparison between \a a and \a b using a byte-wise compare equivalent to `memcmp`.
		 * @note For component-wise lexicographical comparison use `path_view_component::compare` instead. */
		inline constexpr bool operator<(path_view_component a, path_view_component b) noexcept { return _path::compare_bytes(a.view<std::uint8_t>(), b.view<std::uint8_t>()) < 0; }
		/** Preforms a shallow equality comparison between \a a and \a b using a byte-wise compare equivalent to `memcmp`.
		 * @note For component-wise lexicographical comparison use `path_view_component::compare` instead. */
		inline constexpr bool operator==(path_view_component a, path_view_component b) noexcept { return _path::compare_bytes(a.view<std::uint8_t>(), b.view<std::uint8_t>()) == 0; }
		/** Preforms a shallow three-way comparison between \a a and \a b using a byte-wise compare equivalent to `memcmp`.
		 * @note For component-wise lexicographical comparison use `path_view_component::compare` instead. */
		inline constexpr auto operator<=>(path_view_component a, path_view_component b) noexcept { return _path::compare_bytes(a.view<std::uint8_t>(), b.view<std::uint8_t>()) <=> 0; }

		template<typename T, size_type N>
		struct rendered_path_data
		{
			size_type cap = 0;
			std::span<const T> data;
			std::array<T, N> buffer;
		};
		template<typename T>
		struct rendered_path_data<T, 0>
		{
			size_type cap = 0;
			std::span<const T> data;
		};

		template<bool Term, typename T, typename Alloc, typename path_view_component::size_type BuffSize>
		class path_view_component::rendered_path : public alloc_alias<Alloc>, empty_base<Alloc>, rendered_path_data<T, BuffSize>
		{
			using data_base = rendered_path_data<T, BuffSize>;
			using alloc_base = empty_base<Alloc>;

		public:
			using value_type = std::add_const_t<T>;

			using pointer = typename std::span<value_type>::pointer;
			using reference = typename std::span<value_type>::reference;
			using const_pointer = typename std::span<value_type>::pointer;
			using const_reference = typename std::span<value_type>::reference;

			using iterator = typename std::span<value_type>::iterator;
			using const_iterator = typename std::span<value_type>::iterator;
			using reverse_iterator = typename std::span<value_type>::reverse_iterator;
			using const_reverse_iterator = typename std::span<value_type>::reverse_iterator;

			using size_type = typename std::span<value_type>::size_type;
			using difference_type = typename std::span<value_type>::difference_type;

		public:
			rendered_path(const rendered_path &) = delete;
			rendered_path &operator=(const rendered_path &) = delete;

			/** Initializes an empty rendered path. */
			constexpr rendered_path() noexcept { data_base::data = std::span(buff_data(), 0); }
			constexpr ~rendered_path() { if (data_base::cap) deallocate(data_base::data.data(), data_base::cap); }

			constexpr rendered_path(rendered_path &&other) noexcept : alloc_base(std::move(other))
			{
				if (other.cap)
				{
					std::swap(data_base::cap, other.data_base::cap);
					std::swap(data_base::data, other.data_base::data);
				}
				else if (other.references_source())
					data_base::data = other.data_base::data;
				else if constexpr (BuffSize)
				{
					data_base::data = std::span(buff_data(), other.size());
					data_base::buffer = other.data_base::buffer;
				}
			}
			constexpr rendered_path &operator=(rendered_path &&other) noexcept
			{
				if (data_base::cap)
					deallocate(data_base::data.data(), data_base::cap);
				if (other.data_base::cap)
				{
					alloc_base::operator=(std::move(other));
					data_base::cap = std::exchange(other.data_base::cap, {});
					data_base::data = std::exchange(other.data_base::data, {});
				}
				else if (other.references_source())
					data_base::data = other.data_base::data;
				else if constexpr (BuffSize)
				{
					data_base::data = std::span(buff_data(), other.size());
					data_base::buffer = other.data_base::buffer;
				}
				return *this;
			}

			/** Initializes rendered path from a path component \a p with allocator \a alloc. */
			constexpr rendered_path(path_view_component p, const Alloc &alloc = Alloc()) : alloc_base(alloc)
			{
				if (p.empty())
					return;
				p.visit([&]<typename V>(V view)
				{
					if constexpr (decays_to_same<std::ranges::range_value_t<V>, T>)
						init_direct(view, p.is_null_terminated());
					else if constexpr (std::same_as<V, std::span<const std::byte>>)
						init_direct(p.view<T>(), p.is_null_terminated());
					else if constexpr (sizeof(std::ranges::range_value_t<V>) == sizeof(T))
						init_direct(p.view<T>(), p.is_null_terminated());
					else
						init_encode(view);
				});
			}
			/** Initializes rendered path from a path component \a p with allocator \a alloc, using locale \a loc for encoding conversion. */
			constexpr rendered_path(path_view_component p, const std::locale &loc, const Alloc &alloc = Alloc()) : alloc_base(alloc)
			{
				if (p.empty())
					return;
				p.visit([&]<typename V>(V view)
				{
					if constexpr (decays_to_same<std::ranges::range_value_t<V>, T>)
						init_direct(view, p.is_null_terminated());
					else if constexpr (std::same_as<V, std::span<const std::byte>>)
						init_direct(p.view<T>(), p.is_null_terminated());
					else if constexpr (sizeof(std::ranges::range_value_t<V>) == sizeof(T))
						init_direct(p.view<T>(), p.is_null_terminated());
					else
						init_encode(view, loc);
				});
			}

			/** Returns iterator to the first element of the rendered path. */
			[[nodiscard]] constexpr iterator begin() noexcept { return as_span().begin(); }
			/** Returns iterator to the first element of the rendered path. */
			[[nodiscard]] constexpr const_iterator begin() const noexcept { return as_span().begin(); }
			/** @copydoc begin */
			[[nodiscard]] constexpr const_iterator cbegin() const noexcept { return as_span().cbegin(); }

			/** Returns iterator one past the last element of the rendered path. */
			[[nodiscard]] constexpr iterator end() noexcept { return as_span().end(); }
			/** Returns iterator one past the last element of the rendered path. */
			[[nodiscard]] constexpr const_iterator end() const noexcept { return as_span().end(); }
			/** @copydoc end */
			[[nodiscard]] constexpr const_iterator cend() const noexcept { return as_span().cend(); }

			/** Returns reverse iterator to the last element of the rendered path. */
			[[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return as_span().rbegin(); }
			/** Returns reverse iterator to the last of the rendered path. */
			[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return as_span().rbegin(); }
			/** @copydoc rbegin */
			[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return as_span().crbegin(); }

			/** Returns reverse iterator one past the first element of the rendered path. */
			[[nodiscard]] constexpr reverse_iterator rend() noexcept { return as_span().rend(); }
			/** Returns reverse iterator one past the first element of the rendered path. */
			[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return as_span().rend(); }
			/** @copydoc rend */
			[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return as_span().crend(); }

			/** Returns size the data of the rendered path in bytes or code-points. */
			[[nodiscard]] constexpr size_type size() const noexcept { return data_base::data.size(); }
			/** @copydoc size */
			[[nodiscard]] constexpr size_type length() const noexcept { return size(); }

			/** Checks if the rendered path is empty. */
			[[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }
			/** Checks if the rendered path is a non-owning reference to the path source. */
			[[nodiscard]] constexpr bool references_source() const noexcept { return data() != buff_data() && !data_base::cap; }

			/** Returns the maximum possible size the data of the rendered path in bytes or code-points. */
			[[nodiscard]] constexpr size_type max_size() const noexcept { return std::numeric_limits<size_type>::max() - static_cast<size_type>(Term); }
			/** Returns the current capacity of the rendered path in bytes or code-points. */
			[[nodiscard]] constexpr size_type capacity() const noexcept { return (data_base::cap ? data_base::cap : BuffSize) - static_cast<size_type>(Term); }

			/** Returns pointer to the data of the rendered path. */
			[[nodiscard]] constexpr const_pointer data() const noexcept { return data_base::data.data(); }
			/** Returns pointer to the null-terminated string of the rendered path.
			 * @note This function is available only if the rendered path is null-terminated. */
			[[nodiscard]] constexpr const_pointer c_str() const noexcept requires Term { return data(); }
			/** Converts rendered path to a span of elements. */
			[[nodiscard]] constexpr std::span<value_type> as_span() const noexcept { return std::span<value_type>(data(), size()); }

			/** Returns reference to the first element of the rendered path. */
			[[nodiscard]] constexpr const_reference front() const noexcept { return *data(); }
			/** Returns reference to the last element of the rendered path. */
			[[nodiscard]] constexpr const_reference back() const noexcept { return data()[size() - 1]; }
			/** Returns reference to the element at index \a i within the rendered path. */
			[[nodiscard]] constexpr const_reference operator[](size_type i) const noexcept { return data()[i]; }
			/** @copydoc operator[]
			 * @throw std::out_of_range if \a i is greater or equal to `size()`. */
			[[nodiscard]] constexpr const_reference at(size_type i) const
			{
				if (i >= size()) [[unlikely]]
					ROD_THROW(std::out_of_range("rendered_path::at"));
				else
					return operator[](i);
			}

		private:
			constexpr auto *buff_data() noexcept
			{
				if constexpr (BuffSize)
					return data_base::buffer.data();
				else
					return nullptr;
			}
			constexpr auto *buff_data() const noexcept
			{
				if constexpr (BuffSize)
					return data_base::buffer.data();
				else
					return nullptr;
			}

			[[nodiscard]] constexpr auto *allocate(std::size_t n)
			{
				if constexpr (!std::same_as<Alloc, default_rendered_path_allocator<T>>)
					return std::allocator_traits<Alloc>::allocate(alloc_base::value(), n);
				else
					return new T[n];
			}
			constexpr void deallocate(auto *ptr, std::size_t n)
			{
				if constexpr (!std::same_as<Alloc, default_rendered_path_allocator<T>>)
					return std::allocator_traits<Alloc>::deallocate(alloc_base::value(), ptr, n);
				else
					return delete[] ptr;
			}

			template<typename Src>
			constexpr void init_direct(Src src, bool term)
			{
				if (!Term || term)
					data_base::data = std::span(src.data(), src.size());
				else if (src.size() > BuffSize - static_cast<size_type>(Term))
				{
					const auto mem = allocate(src.size() + static_cast<size_type>(Term));
					data_base::data = std::span(mem, src.size());
					data_base::cap = src.size();

					std::copy_n(src.data(), src.size(), mem);
					if constexpr (Term) mem[src.size()] = {};
				}
				else if constexpr (BuffSize)
				{
					data_base::data = std::span(buff_data(), src.size());
					std::copy_n(src.data(), src.size(), buff_data());
					if constexpr (Term) buff_data()[src.size()] = {};
				}
			}

			template<typename Src>
			constexpr void init_encode(Src src)
			{
				T *data_ptr = buff_data();
				const auto n = cvt(src.data(), src.size(), buff_data(), capacity(), [&](auto, std::size_t n)
				{
					if (n <= capacity())
						return data_ptr;

					const auto new_data = allocate(n + static_cast<size_type>(Term));
					std::copy_n(data_ptr, capacity(), new_data);
					if (data_base::cap) deallocate(data_ptr, data_base::cap);
					return data_ptr = new_data;
				});

				data_base::data = std::span(data_ptr, n.value());
#ifdef ROD_WIN32
				std::replace(data_ptr, data_ptr + *n, '/', '\\');
#endif
				data_ptr[size()] = {};
			}
			template<typename Src>
			constexpr void init_encode(Src src, const std::locale &loc)
			{
				T *data_ptr = buff_data();
				const auto n = cvt(loc, src.data(), src.size(), buff_data(), capacity(), [&](auto, std::size_t n)
				{
					if (n <= capacity())
						return data_ptr;

					const auto new_data = allocate(n + static_cast<size_type>(Term));
					std::copy_n(data_ptr, capacity(), new_data);
					if (data_base::cap) deallocate(data_ptr, data_base::cap);
					return data_ptr = new_data;
				});

				data_base::data = std::span(data_ptr, n.value());
#ifdef ROD_WIN32
				std::replace(data_ptr, data_ptr + *n, '/', '\\');
#endif
				data_ptr[size()] = {};
			}
		};

		template<typename T, typename Alloc, size_type BuffSize>
		constexpr path_view_component::rendered_path<true, T, Alloc, BuffSize> path_view_component::render_null_terminated(const Alloc &alloc) const
		{
			return rendered_path<true, T, Alloc, BuffSize>(*this, alloc);
		}
		template<typename T, typename Alloc, size_type BuffSize>
		constexpr path_view_component::rendered_path<true, T, Alloc, BuffSize> path_view_component::render_null_terminated(const std::locale &loc, const Alloc &alloc) const
		{
			return rendered_path<true, T, Alloc, BuffSize>(*this, loc, alloc);
		}
		template<typename T, typename Alloc, size_type BuffSize>
		constexpr path_view_component::rendered_path<false, T, Alloc, BuffSize> path_view_component::render_unterminated(const Alloc &alloc) const
		{
			return rendered_path<false, T, Alloc, BuffSize>(*this, alloc);
		}
		template<typename T, typename Alloc, size_type BuffSize>
		constexpr path_view_component::rendered_path<false, T, Alloc, BuffSize> path_view_component::render_unterminated(const std::locale &loc, const Alloc &alloc) const
		{
			return rendered_path<false, T, Alloc, BuffSize>(*this, loc, alloc);
		}

		template<typename T, typename Alloc, typename path_view_component::size_type BuffSize>
		constexpr int path_view_component::compare(path_view_component other) const noexcept
		{
			if constexpr (std::same_as<T, std::byte>)
				return _path::compare_bytes(view<std::uint8_t>(), other.view<std::uint8_t>());
			else if (_encoding == other._encoding)
			{
				/* Direct comparison of equal encodings. */
				return visit([&]<typename V>(V v)
				{
					if constexpr (!std::same_as<V, std::span<const std::byte>>)
						return _path::compare(v, format(), other.view<std::decay_t<std::ranges::range_value_t<V>>>(), other.format());
					else
						return _path::compare_bytes(view<std::uint8_t>(), other.view<std::uint8_t>());
				});
			}
			else
			{
				/* Comparison via rendered_path. */
				auto alloc = Alloc();
				const auto a_rendered = rendered_path<false, T, Alloc, BuffSize>(*this, alloc);
				const auto b_rendered = rendered_path<false, T, Alloc, BuffSize>(other, alloc);
				return _path::compare<T>({a_rendered.data(), a_rendered.size()}, format(), {b_rendered.data(), b_rendered.size()}, other.format());
			}
		}

		/** Non-owning view of a filesystem path. */
		class path_view : public path_view_component
		{
		public:
			using format_type = typename path_view_component::format_type;
			using size_type = typename path_view_component::size_type;

		public:
			using iterator = component_iterator<path_view_component, const std::byte *, iter_func<&path_view::iter_next>, iter_func<&path_view::iter_prev>>;
			using const_iterator = iterator;
			using reverse_iterator = std::reverse_iterator<iterator>;
			using const_reverse_iterator = reverse_iterator;

		public:
			/** Initializes an path view of unknown format. */
			constexpr path_view() noexcept : path_view(unknown_format) {}
			/** Initializes an empty path view with format \a fmt. */
			constexpr path_view(format_type fmt) noexcept : path_view_component(fmt) {}

			/** Copy-initializes path view from \a other. */
			constexpr path_view(const path_view &other) noexcept = default;
			/** Copy-initializes path view from \a other with format \a fmt. */
			constexpr path_view(const path_view &other, format_type fmt) noexcept : path_view_component(other, fmt) {}

			/** Move-initializes path view from \a other. */
			constexpr path_view(path_view &&other) noexcept = default;
			/** Move-initializes path view from \a other with format \a fmt. */
			constexpr path_view(path_view &&other, format_type fmt) noexcept : path_view_component(std::forward<path_view_component>(other), fmt) {}

			constexpr path_view &operator=(const path_view &) noexcept = default;
			constexpr path_view &operator=(path_view &&) noexcept = default;

			/** Initializes a path view from a path component \a p with automatic formatting. */
			constexpr path_view(path_view_component p) noexcept : path_view_component(p, auto_format) {}
			/** Initializes a path view from a path component \a p with format \a fmt. */
			constexpr path_view(path_view_component p, format_type fmt) noexcept : path_view_component(p, fmt) {}

			/** Initializes a path view from a null-terminated character string or byte array \a data and format \a fmt. */
			template<typename C> requires one_of<std::decay_t<C>, std::byte, char, wchar_t, char8_t, char16_t, char32_t>
			constexpr path_view(const C *data, format_type fmt = auto_format) noexcept : path_view_component(data, fmt) {}
			/** Initializes a path view from a null-terminated character string or byte array \a data of length \a len, null-termination flag \a term and format \a fmt. */
			template<typename C> requires one_of<std::decay_t<C>, std::byte, char, wchar_t, char8_t, char16_t, char32_t>
			constexpr path_view(const C *data, size_type len, bool term, format_type fmt = auto_format) noexcept : path_view_component(data, len, term, fmt) {}

			/** Initializes path view from path \a p. */
			constexpr path_view(const path &p) noexcept : path_view_component(p) {}
			/** Initializes a path view from string \a str and format \a fmt. */
			template<accepted_char C = value_type, typename T = std::char_traits<C>, typename Alloc = std::allocator<C>>
			constexpr path_view(const std::basic_string<C, T, Alloc> &str, format_type fmt = auto_format) noexcept : path_view_component(str, fmt) {}
			/** Initializes a path view from string view \a str, null-termination flag \a term and format \a fmt. */
			template<accepted_char C = value_type, typename T = std::char_traits<C>>
			constexpr path_view(std::basic_string_view<C, T> str, bool term, format_type fmt = auto_format) noexcept : path_view_component(str, term, fmt) {}

		public:
			/** Returns iterator to the first sub-component of the path view. */
			[[nodiscard]] constexpr iterator begin() const noexcept
			{
				return path_view_component::visit([&]<typename V>(V base_view) noexcept
				{
					if constexpr (decays_to_same<V, std::span<const std::byte>>)
						return iterator(*this, this, static_cast<const std::byte *>(_data));
					else
					{
					    const auto comp_size = _path::iter_begin(base_view, format());
					    const auto term = is_null_terminated() && comp_size == base_view.size();
					    return iterator({base_view.data(), comp_size, term}, this, static_cast<const std::byte *>(_data));
					}
				});
			}
			/** @copydoc begin */
			[[nodiscard]] constexpr iterator cbegin() const noexcept { return begin(); }

			/** Returns iterator one past the last sub-component of the path view. */
			[[nodiscard]] constexpr iterator end() const noexcept
			{
				return iterator({}, this, static_cast<const std::byte *>(_data) + _size);
			}
			/** @copydoc end */
			[[nodiscard]] constexpr iterator cend() const noexcept { return end(); }

			/** Returns reverse iterator to the last sub-component of the path view. */
			[[nodiscard]] constexpr reverse_iterator rbegin() const { return reverse_iterator(end()); }
			/** @copydoc rbegin */
			[[nodiscard]] constexpr reverse_iterator crbegin() const { return reverse_iterator(cend()); }

			/** Returns reverse iterator one past the last sub-component of the path view. */
			[[nodiscard]] constexpr reverse_iterator rend() const { return reverse_iterator(begin()); }
			/** @copydoc rend */
			[[nodiscard]] constexpr reverse_iterator crend() const { return reverse_iterator(cbegin()); }

		private:
			template<typename C>
			[[nodiscard]] static constexpr bool ends_with(std::basic_string_view<C> parent, std::basic_string_view<C> substr) noexcept
			{
				const auto parent_end = parent.data() + parent.size();
				const auto substr_end = substr.data() + substr.size();
				return parent_end == substr_end;
			}

		public:
			/** Checks if the path view is absolute.
			 * @note Byte-backed path views are always considered to be absolute. */
			[[nodiscard]] constexpr bool is_absolute() const noexcept
			{
				return visit([&]<typename V>(V view)
				{
					if constexpr (!std::same_as<V, std::span<const std::byte>>)
						return _path::is_absolute(view, format());
					else
						return true;
				});
			}
			/** Checks if the path view is relative (not absolute).
			 * @note Byte-backed path views are always considered to be absolute. */
			[[nodiscard]] constexpr bool is_relative() const noexcept { return !is_absolute(); }

			/** Returns a path containing the root namespace component of this path. Namespaces are only supported on
			 * Windows and follow the Win32 path format specification with the addition of a special `\!!\` namespace
			 * used to identify native NT paths. */
			[[nodiscard]] constexpr path_view root_namespace() const noexcept
			{
				return visit([&]<typename V>(V view) -> path_view
                {
                    if constexpr (!std::same_as<V, std::span<const std::byte>>)
                    {
                        const auto comp = root_namespace_substr(view, format());
                        const auto term = is_null_terminated() && ends_with(view, comp);
                        return path_view(comp, term, format());
                    }
                    return {};
                });
			}
			/** Returns a view of the root directory component of this path view. */
			[[nodiscard]] constexpr path_view root_directory() const noexcept
			{
				return visit([&]<typename V>(V view) -> path_view
                {
                    if constexpr (!std::same_as<V, std::span<const std::byte>>)
                    {
                        const auto comp = root_directory_substr(view, format());
                        const auto term = is_null_terminated() && ends_with(view, comp);
                        return path_view(comp, term, format());
                    }
                    return {};
                });
			}
			/** Returns a view of the root path component of this path view. */
			[[nodiscard]] constexpr path_view root_path() const noexcept
			{
				return visit([&]<typename V>(V view) -> path_view
				{
					if constexpr (!std::same_as<V, std::span<const std::byte>>)
					{
						const auto comp = root_path_substr(view, format());
						const auto term = is_null_terminated() && ends_with(view, comp);
						return path_view(comp, term, format());
					}
					return {};
				});
			}
			/** Returns a view of the root name component of this path view. */
			[[nodiscard]] constexpr path_view root_name() const noexcept
			{
				return visit([&]<typename V>(V view) -> path_view
				{
					if constexpr (!std::same_as<V, std::span<const std::byte>>)
					{
						const auto comp = root_name_substr(view, format());
						const auto term = is_null_terminated() && ends_with(view, comp);
						return path_view(comp, term, format());
					}
					return {};
				});
			}

			/** Checks if the path has a non-empty root namespace. */
			[[nodiscard]] constexpr bool has_root_namespace() const noexcept { return !root_namespace().empty(); }
			/** Checks if the path view has a non-empty root directory. */
			[[nodiscard]] constexpr bool has_root_directory() const noexcept { return !root_directory().empty(); }
			/** Checks if the path view has a non-empty root path. */
			[[nodiscard]] constexpr bool has_root_path() const noexcept { return !root_path().empty(); }
			/** Checks if the path view has a non-empty root name. */
			[[nodiscard]] constexpr bool has_root_name() const noexcept { return !root_name().empty(); }

			/** Returns a view of the relative path component of this path view. */
			[[nodiscard]] constexpr path_view relative_path() const noexcept
			{
				return visit([&]<typename V>(V view) -> path_view
				{
					if constexpr (!std::same_as<V, std::span<const std::byte>>)
					{
						const auto comp = relative_path_substr(view, format());
						const auto term = is_null_terminated() && ends_with(view, comp);
						return path_view(comp, term, format());
					}
					return *this;
				});
			}
			/** Returns a view of the parent path component of this path view. */
			[[nodiscard]] constexpr path_view parent_path() const noexcept
			{
				return visit([&]<typename V>(V view) -> path_view
				{
					if constexpr (!std::same_as<V, std::span<const std::byte>>)
					{
						const auto comp = parent_path_substr(view, format());
						const auto term = is_null_terminated() && ends_with(view, comp);
						return path_view(comp, term, format());
					}
					return {};
				});
			}
			/** Returns a view of the filename component of this path view. */
			[[nodiscard]] constexpr path_view filename() const noexcept
			{
				return visit([&]<typename V>(V view) -> path_view
				{
					if constexpr (!std::same_as<V, std::span<const std::byte>>)
					{
						const auto comp = file_name_substr(view, format());
						const auto term = is_null_terminated() && ends_with(view, comp);
						return path_view(comp, term, format());
					}
					return *this;
				});
			}

			/** Checks if the path has a non-empty relative path component. */
			[[nodiscard]] constexpr bool has_relative_path() const noexcept { return !relative_path().empty(); }
			/** Checks if the path has a non-empty parent path component. */
			[[nodiscard]] constexpr bool has_parent_path() const noexcept { return !parent_path().empty(); }
			/** Checks if the path has a non-empty filename component. */
			[[nodiscard]] constexpr bool has_filename() const noexcept { return !filename().empty(); }

			/** Returns a view of the stem component of this path view. */
			[[nodiscard]] constexpr path_view stem() const noexcept
			{
				return visit([&]<typename V>(V view) -> path_view
				{
					if constexpr (!std::same_as<V, std::span<const std::byte>>)
					{
						const auto comp = file_stem_substr(file_name_substr(view, format()));
						const auto term = is_null_terminated() && ends_with(view, comp);
						return path_view(comp, term, format());
					}
					return {};
				});
			}
			/** Returns a view of the extension component of this path view. */
			[[nodiscard]] constexpr path_view extension() const noexcept
			{
				return visit([&]<typename V>(V view) -> path_view
				{
					if constexpr (!std::same_as<V, std::span<const std::byte>>)
					{
						const auto comp = file_ext_substr(file_name_substr(view, format()));
						const auto term = is_null_terminated() && ends_with(view, comp);
						return path_view(comp, term, format());
					}
					return {};
				});
			}

			/** Checks if the path has a non-empty stem component. */
			[[nodiscard]] constexpr bool has_stem() const noexcept { return !stem().empty(); }
			/** Checks if the path has a non-empty extension component. */
			[[nodiscard]] constexpr bool has_extension() const noexcept { return !extension().empty(); }

		public:
			/** Returns copy of the view with the filename component removed. */
			[[nodiscard]] constexpr path_view remove_filename() const noexcept
			{
				return visit([&]<typename V>(V view) -> path_view
				{
					if constexpr (!std::same_as<V, std::span<const std::byte>>)
					{
						const auto str = view.substr(0, find_file_name(view, format()));
						const auto term = is_null_terminated() && str.size() == view.size();
						return path_view(str, term, format());
					}
					return *this;
				});
			}
			/** Returns copy of the view with the extension component removed. */
			[[nodiscard]] constexpr path_view remove_extension() const noexcept
			{
				return visit([&]<typename V>(V view) -> path_view
				{
					if constexpr (!std::same_as<V, std::span<const std::byte>>)
					{
						const auto name = find_file_name(view, format());
						const auto stem = file_stem_size(view.substr(name));
						const auto term = is_null_terminated() && name + stem == view.size();
						return path_view(view.substr(0, name + stem), term, format());
					}
					return *this;
				});
			}
			/** Returns copy of the view with the trailing separators removed. */
			[[nodiscard]] constexpr path_view remove_separator() const noexcept
			{
				return visit([&]<typename V>(V view) -> path_view
				{
					if constexpr (!std::same_as<V, std::span<const std::byte>>)
					{
						const auto pos = rfind_separator_end(view, format());
						const auto term = is_null_terminated() && pos >= view.size();
						return path_view(view.substr(0, pos - 1), term, format());
					}
					return *this;
				});
			}

		public:
			/** Lexicographically compares `this` with \a other. */
			template<typename T = value_type, typename Alloc = default_rendered_path_allocator<T>, size_type BuffSize = default_buffer_size>
			[[nodiscard]] constexpr int compare(path_view other) const noexcept { return path_view_component::compare(other); }
		};

		struct path_view_like
		{
			constexpr path_view_like() noexcept = default;
			template<typename T> requires(std::constructible_from<path_view, T> && !std::is_convertible_v<T, path>)
			constexpr path_view_like(T &&value) noexcept(std::is_nothrow_constructible_v<path_view, T>) : view(std::forward<T>(value)) {}

			path_view view;
		};

		inline path from_view_like(path_view_like p, path::format_type fmt)
		{
			return visit([&]<typename V>(V view)
			{
				if constexpr (std::same_as<V, std::span<const std::byte>>)
					return from_binary(view);
				else
					return path(view, fmt);
			}, p.view);
		}
		inline path from_view_like(path_view_like p, const std::locale &loc, path::format_type fmt)
		{
			return visit([&]<typename V>(V view)
			{
				if constexpr (std::same_as<V, std::span<const std::byte>>)
					return from_binary(view);
				else
					return path(view, loc, fmt);
			}, p.view);
		}

		path::path(path_view_like p, path::format_type fmt) : path(from_view_like(p, fmt)) {}
		path::path(path_view_like p, const std::locale &loc, path::format_type fmt) : path(from_view_like(p, loc, fmt)) {}

		path &path::operator/=(path_view_like p) { return append(p); }
		path &path::append(path_view_like p) { return append(from_view_like(p, format())); }
		path &path::append(path_view_like p, const std::locale &loc) { return append(from_view_like(p, loc, format())); }

		inline path operator/(const path &a, path_view_like b) { return a / from_view_like(b, a.format()); }
		inline path operator/(path_view_like a, const path &b) { return from_view_like(a, b.format()) / b; }

		path &path::operator+=(path_view_like p) { return concat(p); }
		path &path::concat(path_view_like p) { return concat(from_view_like(p, format())); }
		path &path::concat(path_view_like p, const std::locale &loc) { return concat(from_view_like(p, loc, format())); }

		inline path operator+(const path &a, path_view_like b) { return a + from_view_like(b, a.format()); }
		inline path operator+(path_view_like a, const path &b) { return from_view_like(a, b.format()) + b; }

		/** Preforms a component-wise lexicographical equality comparison between \a a and \a b. Equivalent to `a.compare(path(b)) == 0`. */
		template<_path::accepted_source Src>
		[[nodiscard]] inline constexpr bool operator==(const path &a, const Src &b) noexcept { return path_view(a).compare(b) == 0; }
		/** Preforms a component-wise lexicographical equality comparison between \a a and \a b. Equivalent to `path(a).compare(b) == 0`. */
		template<_path::accepted_source Src>
		[[nodiscard]] inline constexpr bool operator==(const Src &a, const path &b) noexcept { return path_view(a).compare(b) == 0; }

		/** Preforms a component-wise lexicographical three-way comparison between \a a and \a b. Equivalent to `a.compare(path(b)) <=> 0`. */
		template<_path::accepted_source Src>
		[[nodiscard]] inline constexpr auto operator<=>(const path &a, const Src &b) noexcept { return path_view(a).compare(b) <=> 0; }
		/** Preforms a component-wise lexicographical three-way comparison between \a a and \a b. Equivalent to `path(a).compare(b) <=> 0`. */
		template<_path::accepted_source Src>
		[[nodiscard]] inline constexpr auto operator<=>(const Src &a, const path &b) noexcept { return path_view(a).compare(b) <=> 0; }

		template<typename C>
		bool operator==(path_view_component, const C *) = delete;
		template<typename C>
		bool operator==(const C *, path_view_component) = delete;
		template<typename C>
		bool operator==(path_view_component, std::basic_string_view<C>) = delete;
		template<typename C>
		bool operator==(std::basic_string_view<C>, path_view_component) = delete;

		bool operator==(path_view_component, const std::byte *) = delete;
		bool operator==(const std::byte *, path_view_component) = delete;
		bool operator==(path_view_component, std::span<const std::byte>) = delete;
		bool operator==(std::span<const std::byte>, path_view_component) = delete;

		template<typename C>
		bool operator<(path_view_component, const C *) = delete;
		template<typename C>
		bool operator<(const C *, path_view_component) = delete;
		template<typename C>
		bool operator<(path_view_component, std::basic_string_view<C>) = delete;
		template<typename C>
		bool operator<(std::basic_string_view<C>, path_view_component) = delete;

		bool operator<(path_view_component, const std::byte *) = delete;
		bool operator<(const std::byte *, path_view_component) = delete;
		bool operator<(path_view_component, std::span<const std::byte>) = delete;
		bool operator<(std::span<const std::byte>, path_view_component) = delete;

		template<typename C>
		auto operator<=>(path_view_component, const C *) = delete;
		template<typename C>
		auto operator<=>(const C *, path_view_component) = delete;
		template<typename C>
		auto operator<=>(path_view_component, std::basic_string_view<C>) = delete;
		template<typename C>
		auto operator<=>(std::basic_string_view<C>, path_view_component) = delete;

		auto operator<=>(path_view_component, const std::byte *) = delete;
		auto operator<=>(const std::byte *, path_view_component) = delete;
		auto operator<=>(path_view_component, std::span<const std::byte>) = delete;
		auto operator<=>(std::span<const std::byte>, path_view_component) = delete;
	}

	namespace fs
	{
		using _path::path_view_component;
		using _path::path_view;

		using _path::visit;
		using _path::operator<;
		using _path::operator==;
		using _path::operator<=>;

		/** Checks if type \a T is a path-like type, that is, `fs::path_view` can be constructed from it. */
		template<typename T>
		concept path_like = std::constructible_from<path_view, T>;
	}
}

template<>
inline constexpr bool std::ranges::enable_borrowed_range<rod::fs::path_view> = true;
template<>
inline constexpr bool std::ranges::enable_view<rod::fs::path_view> = true;

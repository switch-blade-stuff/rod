/*
 * Created by switch_blade on 2023-07-30.
 */

#pragma once

#include "path_base.hpp"

namespace rod::fs
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
	}

	template<_path::visitor F>
	inline constexpr _path::visitor_result<F> visit(F &&f, _path::path_view_component p) noexcept(_path::nothrow_visitor<F>::value);

	namespace _path
	{
		enum class encoding : std::uint8_t
		{
			byte = 0,
			cchar = 1,
			wchar = 2,
			char8 = 3,
			char16 = 4,
		};

		/** Non-owning view of a portion of a filesystem path. */
		class path_view_component : public constants
		{
		public:
			using size_type = typename _path::size_type;
			using format = typename constants::format;

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
			constexpr path_view_component(format fmt) noexcept : _data(nullptr), _size(0), _is_null_terminated(false), _formatting(fmt), _encoding(encoding::byte) {}

			/** Copy-initializes path component from \a other. */
			constexpr path_view_component(const path_view_component &other) noexcept = default;
			/** Copy-initializes path component from \a other with format \a fmt. */
			constexpr path_view_component(const path_view_component &other, format fmt) noexcept : path_view_component(other) { _formatting = fmt; }

			/** Move-initializes path component from \a other. */
			constexpr path_view_component(path_view_component &&other) noexcept = default;
			/** Move-initializes path component from \a other with format \a fmt. */
			constexpr path_view_component(path_view_component &&other, format fmt) noexcept : path_view_component(std::forward<path_view_component>(other)) { _formatting = fmt; }

			/** Initializes a path component from a null-terminated `char` string \a str of length \a len and format \a fmt. */
			constexpr path_view_component(const char *str, format fmt = binary_format) noexcept : path_view_component(str, rod::_detail::strlen(str), true, fmt) {}
			/** Initializes a path component from a `char` string \a str of length \a len, null-termination flag \a term and format \a fmt. */
			constexpr path_view_component(const char *str, size_type len, bool term, format fmt = binary_format) noexcept : _data(str), _size(len * sizeof(char)), _is_null_terminated(term), _formatting(fmt), _encoding(encoding::cchar) {}

			/** Initializes a path component from a `wchar_t` string \a str of length \a len, null-termination flag \a term and format \a fmt. */
			constexpr path_view_component(const wchar_t *str, format fmt = binary_format) noexcept : path_view_component(str, rod::_detail::strlen(str), true, fmt) {}
			/** Initializes a path component from a null-terminated `wchar_t` string \a str of length \a len and format \a fmt. */
			constexpr path_view_component(const wchar_t *str, size_type len, bool term, format fmt = binary_format) noexcept : _data(str), _size(len * sizeof(wchar_t)), _is_null_terminated(term), _formatting(fmt), _encoding(encoding::wchar) {}

			/** Initializes a path component from a `char8_t` string \a str of length \a len, null-termination flag \a term and format \a fmt. */
			constexpr path_view_component(const char8_t *str, format fmt = binary_format) noexcept : path_view_component(str, rod::_detail::strlen(str), true, fmt) {}
			/** Initializes a path component from a null-terminated `char8_t` string \a str of length \a len and format \a fmt. */
			constexpr path_view_component(const char8_t *str, size_type len, bool term, format fmt = binary_format) noexcept : _data(str), _size(len * sizeof(char8_t)), _is_null_terminated(term), _formatting(fmt), _encoding(encoding::char8) {}

			/** Initializes a path component from a `char16_t` string \a str of length \a len, null-termination flag \a term and format \a fmt. */
			constexpr path_view_component(const char16_t *str, format fmt = binary_format) noexcept : path_view_component(str, rod::_detail::strlen(str), true, fmt) {}
			/** Initializes a path component from a null-terminated `char16_t` string \a str of length \a len and format \a fmt. */
			constexpr path_view_component(const char16_t *str, size_type len, bool term, format fmt = binary_format) noexcept : _data(str), _size(len * sizeof(char16_t)), _is_null_terminated(term), _formatting(fmt), _encoding(encoding::char16) {}

			/** Initializes a path component from a null-terminated byte array \a data of length \a len and format \a fmt. */
			constexpr path_view_component(const std::byte *data, format fmt = binary_format) noexcept : path_view_component(data, rod::_detail::strlen(data), true, fmt) {}
			/** Initializes a path component from a byte array \a data of length \a len, null-termination flag \a term and format \a fmt. */
			constexpr path_view_component(const std::byte *data, size_type len, bool term, format fmt = binary_format) noexcept : _data(data), _size(len), _is_null_terminated(term), _formatting(fmt), _encoding(encoding::byte) {}

			/** Initializes path component from path \a p and format \a fmt. */
			path_view_component(const path &p) noexcept : path_view_component(p._value.data(), p._value.size(), true, p.formatting()) {}
			/** Initializes a path component from string \a str and format \a fmt. */
			template<accepted_char C = value_type, typename T = std::char_traits<C>, typename Alloc = std::allocator<C>>
			constexpr path_view_component(const std::basic_string<C, T, Alloc> &str, format fmt = binary_format) noexcept : path_view_component(str.data(), str.size(), true, fmt) {}
			/** Initializes a path component from string view \a str, null-termination flag \a term and format \a fmt. */
			template<accepted_char C = value_type, typename T = std::char_traits<C>>
			constexpr path_view_component(std::basic_string_view<C, T> str, bool term, format fmt = binary_format) noexcept : path_view_component(str.data(), str.size(), term, fmt) {}

			constexpr path_view_component &operator=(const path_view_component &) noexcept = default;
			constexpr path_view_component &operator=(path_view_component &&) noexcept = default;

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
				case encoding::cchar: return std::invoke(std::forward<F>(f), view<char>());
				case encoding::wchar: return std::invoke(std::forward<F>(f), view<wchar_t>());
				case encoding::char8: return std::invoke(std::forward<F>(f), view<char8_t>());
				case encoding::char16: return std::invoke(std::forward<F>(f), view<char16_t>());
				case encoding::byte: return std::invoke(std::forward<F>(f), view<std::byte>());
				default: unreachable();
				}
			}

			void iter_next(const path_view_component &base, const std::byte *&pos) noexcept
			{
				base.visit([&]<typename V>(V base_view) noexcept
				{
					using view_value = std::ranges::range_value_t<V>;
					if constexpr (decays_to<view_value, std::byte>)
						operator=(path_view_component(pos = base_view.data() + base_view.size(), 0, base.is_null_terminated()));
					else
					{
						const auto bytes = static_cast<const std::byte *>(base._data);
						const auto comp_data = static_cast<const view_value *>(_data);
						auto comp_view = std::basic_string_view(comp_data, native_size());
						auto new_pos = base_view.data() + (pos - bytes) / sizeof(value_type);

						comp_view = _path::iter_next<view_value>(comp_view, base_view, new_pos, base.formatting());
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
					if constexpr (decays_to<view_value, std::byte>)
						operator=(path_view_component(pos = base_view.data(), base_view.size(), base.is_null_terminated()));
					else
					{
						const auto bytes = static_cast<const std::byte *>(base._data);
						const auto comp_data = static_cast<const view_value *>(_data);
						auto comp_view = std::basic_string_view(comp_data, native_size());
						auto new_pos = base_view.data() + (pos - bytes) / sizeof(value_type);

						comp_view = _path::iter_prev<view_value>(comp_view, base_view, new_pos, base.formatting());
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
			[[nodiscard]] constexpr format formatting() const noexcept { return _formatting; }
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

		public:
			/** Renders a null-terminated path component encoded into character type \a T with allocator \a alloc and buffer size \a BuffSize. */
			template<typename T = value_type, typename Alloc = default_rendered_path_allocator<T>, size_type BuffSize = default_buffer_size>
			constexpr rendered_path<true, T, Alloc, BuffSize> render_null_terminated(const Alloc &alloc = Alloc());
			/** Renders a null-terminated path component encoded into character type \a T with allocator \a alloc and buffer size \a BuffSize, using locale \a loc for encoding conversion. */
			template<typename T = value_type, typename Alloc = default_rendered_path_allocator<T>, size_type BuffSize = default_buffer_size>
			constexpr rendered_path<true, T, Alloc, BuffSize> render_null_terminated(const std::locale &loc, const Alloc &alloc = Alloc());

			/** Renders an unterminated path component encoded into character type \a T with allocator \a alloc and buffer size \a BuffSize. */
			template<typename T = value_type, typename Alloc = default_rendered_path_allocator<T>, size_type BuffSize = default_buffer_size>
			constexpr rendered_path<false, T, Alloc, BuffSize> render_unterminated(const Alloc &alloc = Alloc());
			/** Renders an unterminated path component encoded into character type \a T with allocator \a alloc and buffer size \a BuffSize, using locale \a loc for encoding conversion. */
			template<typename T = value_type, typename Alloc = default_rendered_path_allocator<T>, size_type BuffSize = default_buffer_size>
			constexpr rendered_path<false, T, Alloc, BuffSize> render_unterminated(const std::locale &loc, const Alloc &alloc = Alloc());

		public:
			/** Swaps contents of `this` with \a other. */
			constexpr void swap(path_view_component &other) noexcept
			{
				std::swap(_data, other._data);
				std::swap(_size, other._size);
				std::swap(_encoding, other._encoding);
				std::swap(_formatting, other._formatting);
				std::swap(_is_null_terminated, other._is_null_terminated);
			}
			friend constexpr void swap(path_view_component &a, path_view_component &b) noexcept { a.swap(b); }

			/** Lexicographically compares `this` with \a other. */
			template<typename T = value_type, typename Alloc = default_rendered_path_allocator<T>, size_type BuffSize = default_buffer_size>
			[[nodiscard]] inline constexpr int compare(path_view_component other) const noexcept;

		public:
			template<visitor F>
			friend inline constexpr visitor_result<F> rod::fs::visit(F &&f, path_view_component p) noexcept(nothrow_visitor<F>::value);

		protected:
			const void *_data;
			std::size_t _size;
			encoding _encoding;
			format _formatting;
			bool _is_null_terminated;
		};

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
		class path_view_component::rendered_path : empty_base<Alloc>, rendered_path_data<T, BuffSize>, public alloc_alias<Alloc>
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
					data_base::data = other.data;
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
					if constexpr (decays_to<std::ranges::range_value_t<V>, T>)
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
					if constexpr (decays_to<std::ranges::range_value_t<V>, T>)
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
			[[nodiscard]] constexpr bool references_source() const noexcept { return data_base::cap && data() != buff_data(); }

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
				data_ptr[size()] = {};
			}
		};

		template<typename T, typename Alloc, size_type BuffSize>
		constexpr path_view_component::rendered_path<true, T, Alloc, BuffSize> path_view_component::render_null_terminated(const Alloc &alloc)
		{
			return rendered_path<true, T, Alloc, BuffSize>(*this, alloc);
		}
		template<typename T, typename Alloc, size_type BuffSize>
		constexpr path_view_component::rendered_path<true, T, Alloc, BuffSize> path_view_component::render_null_terminated(const std::locale &loc, const Alloc &alloc)
		{
			return rendered_path<true, T, Alloc, BuffSize>(*this, loc, alloc);
		}
		template<typename T, typename Alloc, size_type BuffSize>
		constexpr path_view_component::rendered_path<false, T, Alloc, BuffSize> path_view_component::render_unterminated(const Alloc &alloc)
		{
			return rendered_path<false, T, Alloc, BuffSize>(*this, alloc);
		}
		template<typename T, typename Alloc, size_type BuffSize>
		constexpr path_view_component::rendered_path<false, T, Alloc, BuffSize> path_view_component::render_unterminated(const std::locale &loc, const Alloc &alloc)
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
						return _path::compare(v, formatting(), other.view<std::decay_t<std::ranges::range_value_t<V>>>(), other.formatting());
					else
						return _path::compare_bytes(view<std::uint8_t>(), other.view<std::uint8_t>());
				});
			}
			else
			{
				/* Comparison via rendered_path. */
				const auto a_rendered = rendered_path<false, T, Alloc, BuffSize>(*this);
				const auto b_rendered = rendered_path<false, T, Alloc, BuffSize>(other);
				return _path::compare<T>({a_rendered.data(), a_rendered.size()}, formatting(), {b_rendered.data(), b_rendered.size()}, other.formatting());
			}
		}

		/** Non-owning view of a filesystem path. */
		class path_view : public path_view_component
		{
		public:
			using size_type = typename path_view_component::size_type;
			using format = typename path_view_component::format;

		public:
			using iterator = component_iterator<path_view_component, const std::byte *, iter_func < &path_view::iter_next>, iter_func<&path_view::iter_prev>>;
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
					if constexpr (decays_to<V, std::span<const std::byte>>)
						return iterator(*this, this, static_cast<const std::byte *>(_data));
					else
					{
					    const auto comp_size = _path::iter_begin(base_view, formatting());
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

		public:
			/** Swaps contents of `this` with \a other. */
			constexpr void swap(path_view &other) noexcept { path_view_component::swap(other); }
			friend constexpr void swap(path_view &a, path_view &b) noexcept { a.swap(b); }
		};

		struct path_view_like
		{
			constexpr path_view_like() noexcept = default;
			template<typename T> requires(std::is_convertible_v<T, path_view> && !std::is_convertible_v<T, path>)
			constexpr path_view_like(T &&value) noexcept(std::is_nothrow_constructible_v<path_view, T>) : view(std::forward<T>(value)) {}

			path_view view;
		};

		inline path from_view_like(path_view_like p, path::format fmt);
		inline path from_view_like(path_view_like p, const std::locale &loc, path::format fmt);

		path::path(path_view_like p, path::format fmt) : path(from_view_like(p, fmt)) {}
		path::path(path_view_like p, const std::locale &loc, path::format fmt) : path(from_view_like(p, loc, fmt)) {}
	}

	using _path::path_view;
	using _path::path_view_component;

	/** Invokes \a f with a string view or span of bytes depending on the internal encoding of path component \a p. */
	template<_path::visitor F>
	inline constexpr _path::visitor_result<F> visit(F &&f, path_view_component p) noexcept(_path::nothrow_visitor<F>::value) { return p.visit(std::forward<F>(f)); }

	path _path::from_view_like(path_view_like p, path::format fmt)
	{
		return visit([&]<typename V>(V view)
		{
			if constexpr (std::same_as<V, std::span<const std::byte>>)
				return from_binary(view);
			else
				return path(view, fmt);
		}, p.view);
	}
	path _path::from_view_like(path_view_like p, const std::locale &loc, path::format fmt)
	{
		return visit([&]<typename V>(V view)
		{
			if constexpr (std::same_as<V, std::span<const std::byte>>)
				return from_binary(view);
			else
				return path(view, loc, fmt);
		}, p.view);
	}
}

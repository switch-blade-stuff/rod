/*
 * Created by switchblade on 2023-10-26.
 */

#pragma once

#include "fs_handle_base.hpp"
#include "io_handle_base.hpp"
#include "path_handle.hpp"

#if !defined(ROD_HAS_SYMLINK_HANDLE) && (defined(ROD_WIN32) || defined(__linux__))
#define ROD_HAS_SYMLINK_HANDLE
#endif

namespace rod
{
	namespace _link
	{
		using namespace fs;

		/** Handle to a filesystem link. */
		class link_handle;

#ifdef ROD_HAS_SYMLINK_HANDLE
		using handle_base = basic_handle;
#else
		using handle_base = path_handle;
#endif

		/** Type of a filesystem link handle. */
		enum class link_type : std::uint8_t
		{
			unknown = 0,
			/** Standard symbolic link. */
			symbolic = 1,
			/** Windows mount junction. */
			junction = 2,
		};

		class link_handle : public io_handle_adaptor<link_handle, handle_base, fs_handle_adaptor>
		{
			using adaptor = io_handle_adaptor<link_handle, handle_base, fs_handle_adaptor>;

			friend io_handle_adaptor<link_handle, handle_base, fs_handle_adaptor>;
			friend fs_handle_adaptor<link_handle, handle_base>;
			friend handle_adaptor<link_handle, handle_base>;

		public:
			template<one_of<read_some_t, write_some_t> Op>
			class io_buffer
			{
				friend class link_handle;

			public:
				using value_type = std::conditional_t<std::same_as<Op, read_some_t>, typename path::value_type, std::add_const_t<typename path::value_type>>;
				using size_type = typename path::size_type;

			public:
				/** Initializes an empty link content buffer. */
				constexpr io_buffer() noexcept = default;
				/** Initializes the link content buffer from a pointer to a buffer of characters and a size. */
				constexpr io_buffer(value_type *buff, size_type size) noexcept : _buff(buff, size) {}
				/** Initializes the link content buffer from a range of characters defined by [\a begin, \a end). */
				template<std::contiguous_iterator I, std::sentinel_for<I> S> requires std::constructible_from<std::span<value_type>, I, S>
				constexpr io_buffer(I begin, S end) noexcept(std::is_nothrow_constructible_v<std::span<value_type>, I, S>) : _buff(begin, end) {}
				/** Initializes the link content buffer from a contiguous range of characters. */
				template<std::ranges::contiguous_range Buff> requires(!decays_to_same<Buff, io_buffer> && std::constructible_from<std::span<value_type>, Buff>)
				constexpr io_buffer(Buff &&buff) noexcept(std::is_nothrow_constructible_v<std::span<value_type>, Buff>) : _buff(std::forward<Buff>(buff)) {}

				/** Checks if the link content buffer is empty. */
				[[nodiscard]] constexpr bool empty() const noexcept { return _buff.empty(); }
				/** Returns size of the link content buffer. */
				[[nodiscard]] constexpr size_type size() const noexcept { return _buff.size(); }
				/** Returns pointer to the memory of the link content buffer. */
				[[nodiscard]] constexpr value_type *data() const noexcept { return _buff.data(); }

				/** Returns the data of the link content buffer as a `path_view`. */
				[[nodiscard]] constexpr path_view path() const noexcept { return static_cast<path_view>(*this); }
				/** Converts link content buffer to `path_view`. Equivalent to `path()`. */
				[[nodiscard]] constexpr explicit operator path_view() const noexcept { return {_buff.data(), _buff.size(), _is_terminated, path_view::native_format}; }

			private:
				std::span<value_type> _buff;
				bool _is_terminated = false;
			};
			template<one_of<read_some_t, write_some_t> Op>
			class io_buffer_sequence
			{
				friend class io_buffer_sequence;
				friend class link_handle;

				using buff_type = malloc_ptr<typename path::value_type[]>;
				using data_type = std::span<io_buffer<Op>>;

			public:
				using value_type = typename data_type::value_type;

				using pointer = typename data_type::pointer;
				using const_pointer = typename data_type::const_pointer;
				using reference = typename data_type::reference;
				using const_reference = typename data_type::const_reference;

				using iterator = typename data_type::iterator;
				using reverse_iterator = typename data_type::reverse_iterator;

				using difference_type = typename data_type::difference_type;
				using size_type = typename data_type::size_type;

			private:
				io_buffer_sequence(data_type &&buffs, buff_type &&chars, size_type chars_max) noexcept : _data(std::forward<data_type>(buffs)), _buff(std::forward<buff_type>(chars)), _buff_len(chars_max) {}

			public:
				io_buffer_sequence(const io_buffer_sequence &) = delete;
				io_buffer_sequence &operator=(const io_buffer_sequence &) = delete;

				/** Initializes an empty buffer sequence. */
				constexpr io_buffer_sequence() noexcept = default;

				/** Initializes the buffer sequence from a pointer to an entry buffer of characters, size, and an optional link type. */
				constexpr io_buffer_sequence(value_type *buff, size_type size, link_type type = link_type::symbolic) noexcept : _data(buff, size), _type(type) {}

				/** Initializes the buffer sequence from from a range of entry buffers defined by [\a begin, \a end) and an optional link type. */
				template<std::contiguous_iterator I, std::sentinel_for<I> S> requires std::constructible_from<data_type, I, S>
				constexpr io_buffer_sequence(I begin, S end, link_type type = link_type::symbolic) noexcept(std::is_nothrow_constructible_v<data_type, I, S>) : _data(begin, end), _type(type) {}
				/** Initializes the buffer sequence from a contiguous range of entry buffers and an optional link type. */
				template<std::ranges::contiguous_range Buff> requires(!decays_to_same<Buff, io_buffer_sequence> && std::constructible_from<data_type, Buff>)
				constexpr io_buffer_sequence(Buff &&buff, link_type type = link_type::symbolic) noexcept(std::is_nothrow_constructible_v<data_type, Buff>) : _data(std::forward<Buff>(buff)), _type(type) {}

				io_buffer_sequence(io_buffer_sequence &&other) noexcept : _data(std::exchange(other._data, {})), _buff(std::move(other._buff)), _buff_len(std::exchange(other._buff_len, {})) {}
				io_buffer_sequence &operator=(io_buffer_sequence &&other) noexcept { return (_data = std::exchange(other._data, {}), _buff = std::move(other._buff), _buff_len = std::exchange(other._buff_len, {}), *this); }

				/** Initializes the buffer sequence from a pointer to an entry buffer of characters, size, and an optional link type, using internal character buffer of \a other. */
				template<typename OtherOp = Op>
				io_buffer_sequence(io_buffer_sequence<OtherOp> &&other, value_type *buff, size_type size, link_type type = link_type::symbolic) noexcept : _data(buff, size), _buff(std::move(other._buff)), _buff_len(std::exchange(other._buff_len, {})), _type(type) {}
				/** Initializes the buffer sequence from from a range of entry buffers defined by [\a begin, \a end) and an optional link type, using internal character buffer of \a other. */
				template<typename OtherOp = Op, std::contiguous_iterator I, std::sentinel_for<I> S> requires std::constructible_from<data_type, I, S>
				io_buffer_sequence(io_buffer_sequence<OtherOp> &&other, I begin, S end, link_type type = link_type::symbolic) noexcept(std::is_nothrow_constructible_v<data_type, I, S>) : _data(begin, end), _buff(std::move(other._buff)), _buff_len(std::exchange(other._buff_len, {})), _type(type) {}
				/** Initializes the buffer sequence from a contiguous range of entry buffers and an optional link type, using internal character buffer of \a other. */
				template<typename OtherOp = Op, std::ranges::contiguous_range Buff> requires(!decays_to_same<Buff, io_buffer_sequence> && std::constructible_from<data_type, Buff>)
				io_buffer_sequence(io_buffer_sequence<OtherOp> &&other, Buff &&buff, link_type type = link_type::symbolic) noexcept(std::is_nothrow_constructible_v<data_type, Buff>) : _data(std::forward<Buff>(buff)), _buff(std::move(other._buff)), _buff_len(std::exchange(other._buff_len, {})), _type(type) {}

				/** Returns iterator to the first buffer of the buffer sequence. */
				[[nodiscard]] constexpr iterator begin() const noexcept { return _data.begin(); }
				/** Returns iterator one past the last buffer of the buffer sequence. */
				[[nodiscard]] constexpr iterator end() const noexcept { return _data.end(); }

				/** Returns reverse iterator one past the first buffer of the buffer sequence. */
				[[nodiscard]] constexpr reverse_iterator rbegin() const noexcept { return _data.rbegin(); }
				/** Returns reverse iterator to the last buffer of the buffer sequence. */
				[[nodiscard]] constexpr reverse_iterator rend() const noexcept { return _data.rend(); }

				/** Checks if the buffer sequence is empty. */
				[[nodiscard]] constexpr bool empty() const noexcept { return _data.empty(); }
				/** Returns size of the buffer sequence. */
				[[nodiscard]] constexpr size_type size() const noexcept { return _data.size(); }

				/** Returns pointer to the start of the buffer sequence. */
				[[nodiscard]] constexpr pointer data() const noexcept { return _data.data(); }
				/** Returns reference to the last buffer of the buffer sequence. */
				[[nodiscard]] constexpr reference back() const noexcept { return _data.back(); }
				/** Returns reference to the first buffer of the buffer sequence. */
				[[nodiscard]] constexpr reference front() const noexcept { return _data.front(); }
				/** Returns reference to the buffer located at offset \a i within the buffer sequence. */
				[[nodiscard]] constexpr reference operator[](size_type i) const { return _data[i]; }

				/** Converts buffer sequence to a span of link content buffers. */
				[[nodiscard]] constexpr auto as_span() const noexcept { return _data; }
				/** Returns the type of the link content. */
				[[nodiscard]] constexpr link_type type() const noexcept { return _type; }

				void swap(io_buffer_sequence &other) noexcept
				{
					std::swap(_data, other._data);
					std::swap(_buff, other._buff);
					std::swap(_buff_len, other._buff_len);
				}
				friend void swap(io_buffer_sequence &a, io_buffer_sequence &b) noexcept { a.swap(b); }

			private:
				data_type _data = {};
				buff_type _buff = {};
				size_type _buff_len = {};
				link_type _type = link_type::symbolic;
			};

			template<typename Op>
			using io_result = result<io_buffer_sequence<Op>>;

			template<one_of<read_some_t, write_some_t> Op>
			struct io_request
			{
				/** Sequence of link content buffers containing (or receiving) the link path and type, and an optional internal buffer used between calls to IO operations.
				 * @note `read_some` will return a truncated copy of the buffer sequence which may also contain an internally-allocated character buffer.
				 * This internal buffer can be re-used by passing the returned buffer sequence as \a buffs to the next call of `read_some`. */
				io_buffer_sequence<Op> buffs;
			};

		public:
			/** Re-opens the filesystem link referenced by \a other.
			 * @note The following values of \a flags are not supported:
			 * <ul>
			 * <li>`append`</li>
			 * <li>`no_sparse_files`</li>
			 * <li>`non_blocking`</li>
			 * <li>`case_sensitive`</li>
			 * </ul>
			 *
			 * @param other Handle to the filesystem link to be re-opened.
			 * @param flags Handle flags to re-open the handle with.
			 * @return Handle to the filesystem link or a status code on failure. */
			[[nodiscard]] static ROD_API_PUBLIC result<link_handle> reopen(const link_handle &other, file_flags flags = file_flags::read) noexcept;

			/** Opens or creates a filesystem link.
			 * @note The following values of \a flags are not supported:
			 * <ul>
			 * <li>`append`</li>
			 * <li>`no_sparse_files`</li>
			 * <li>`non_blocking`</li>
			 * <li>`case_sensitive`</li>
			 * </ul>
			 * @note The following values of \a mode are not supported:
			 * <ul>
			 * <li>`truncate`</li>
			 * <li>`supersede`</li>
			 * </ul>
			 *
			 * @param base Handle to the parent location. If set to an invalid handle, \a path must be a fully-qualified path.
			 * @param path Path to the link entry relative to \a base if it is a valid handle, otherwise a fully-qualified path.
			 * @param flags Handle flags to open the handle with. `file_flags::read` by default.
			 * @param mode Mode to use when opening or creating the handle. `open_mode::existing` by default.
			 * @return Handle to the filesystem link or a status code on failure. */
			[[nodiscard]] static ROD_API_PUBLIC result<link_handle> open(const path_handle &base, path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::existing) noexcept;

		public:
			link_handle(const link_handle &) = delete;
			link_handle &operator=(const link_handle &) = delete;

			/** Initializes a closed link handle. */
			link_handle() noexcept = default;
			link_handle(link_handle &&) noexcept = default;
			link_handle &operator=(link_handle &&) noexcept = default;

#ifdef ROD_HAS_SYMLINK_HANDLE
			/** Initializes link handle from a basic handle rvalue and file flags. */
			explicit link_handle(basic_handle &&hnd, file_flags flags) noexcept : link_handle(hnd.release(), flags) {}
			/** Initializes link handle from a native handle and file flags. */
			explicit link_handle(typename adaptor::native_handle_type hnd, file_flags flags) noexcept : adaptor(typename adaptor::native_handle_type(hnd, std::uint32_t(flags))) {}
#else
			/** Initializes link handle from a base path handle rvalue, leaf pathname and file flags. */
			explicit link_handle(path_handle &&base, path_view path, file_flags flags) noexcept : link_handle(base.release(), path, flags) {}
			/** Initializes link handle from a native base path handle, leaf pathname and file flags. */
			explicit link_handle(typename adaptor::native_handle_type base, path_view path, file_flags flags) noexcept : adaptor(typename adaptor::native_handle_type(base, std::uint32_t(flags))), _path(path) {}
#endif

			~link_handle() { if (is_open()) do_close(); }

			/** Returns the flags of the link handle. */
			[[nodiscard]] constexpr file_flags flags() const noexcept { return file_flags(native_handle().flags); }

#ifdef ROD_HAS_SYMLINK_HANDLE
			constexpr void swap(link_handle &other) noexcept { adaptor::swap(other); }
#else
			constexpr void swap(link_handle &other) noexcept
			{
				adaptor::swap(other);
				_path.swap(other._path);
			}
#endif

			friend constexpr void swap(link_handle &a, link_handle &b) noexcept { a.swap(b); }

		private:
			auto do_close() noexcept -> decltype(close(base()))
			{
				if (bool(flags() & file_flags::unlink_on_close))
				{
					const auto res = unlink(*this);
					if (res.has_error() && res.error() != std::make_error_condition(std::errc::no_such_file_or_directory))
						return res.error();
				}

#ifndef ROD_HAS_SYMLINK_HANDLE
				_path.clear();
#endif
				return close(base());
			}

#ifdef ROD_HAS_SYMLINK_HANDLE
			auto do_clone() const noexcept { return clone(base()).transform_value([&](handle_base &&hnd) { return link_handle(std::move(hnd), flags()); }); }
#else
			auto do_clone() const noexcept { return clone(base()).transform_value([&](handle_base &&hnd) { return link_handle(std::move(hnd), _path, flags()); }); }
#endif

#ifdef ROD_HAS_SYMLINK_HANDLE
			result<stat::query> do_get_stat(stat &st, stat::query q) const noexcept { return _handle::do_get_stat(st, native_handle(), q); }
			result<stat::query> do_set_stat(const stat &st, stat::query q) noexcept { return _handle::do_set_stat(st, native_handle(), q); }
			result<fs_stat::query> do_get_fs_stat(fs_stat &st, fs_stat::query q) const noexcept { return _handle::do_get_fs_stat(st, native_handle(), q); }
#else
			result<stat::query> do_get_stat(stat &st, stat::query q) const noexcept { return get_stat(st, base(), _path, q, true); }
			result<stat::query> do_set_stat(const stat &st, stat::query q) noexcept { return set_stat(st, base(), _path, q, true); }
			result<fs_stat::query> do_get_fs_stat(fs_stat &st, fs_stat::query q) const noexcept { return _handle::do_get_fs_stat(st, base(), _path, q, true); }
#endif

#ifdef ROD_HAS_SYMLINK_HANDLE
			result<path> do_to_object_path() const noexcept { return _path::do_to_object_path(native_handle()); }
#if defined(ROD_POSIX)
			result<path> do_to_native_path(native_path_format fmt) const noexcept { return _path::do_to_native_path(native_handle(), fmt, 0, 0); }
#endif
			result<path> do_to_native_path(native_path_format fmt, dev_t dev, ino_t ino) const noexcept { return _path::do_to_native_path(native_handle(), fmt, dev, ino); }
#else
			result<path> do_to_object_path() const noexcept { return to_object_path(base()).transform_value([&](auto &&base_path) { return base_path /= _path; }); }
			result<path> do_to_native_path(native_path_format fmt) const noexcept { return to_native_path(base(), fmt).transform_value([&](auto &&base_path) { return base_path /= _path; }); }
#endif

			ROD_API_PUBLIC result<> do_link(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept;
			ROD_API_PUBLIC result<> do_relink(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept;
			ROD_API_PUBLIC result<> do_unlink(const file_timeout &to) noexcept;

			ROD_API_PUBLIC io_result<read_some_t> do_read_some(io_request<read_some_t> &&req, const file_timeout &to) noexcept;
			ROD_API_PUBLIC io_result<write_some_t> do_write_some(io_request<write_some_t> &&req, const file_timeout &to) noexcept;

#ifndef ROD_HAS_SYMLINK_HANDLE
			path _path;
#endif
		};
	}

	namespace fs
	{
		using _link::link_handle;
		using _link::link_type;

		static_assert(stream_io_handle<link_handle>);
		static_assert(!sparse_io_handle<link_handle>);
	}
}

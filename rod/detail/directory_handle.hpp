/*
 * Created by switch_blade on 2023-09-01.
 */

#pragma once

#include "fs_handle_base.hpp"
#include "io_handle_base.hpp"
#include "path_handle.hpp"

namespace rod
{
	namespace _directory
	{
		template<typename>
		class io_buffer_sequence;
		template<typename>
		class io_buffer;

		template<typename>
		struct io_request;
		template<typename>
		class io_result;

		/** @brief Handle to a unique directory within the filesystem.
		 *
		 * `directory_handle` works as an extended version of `path_handle` (and is a subclass of it),
		 * providing additional support for directory creation, enumeration, and filesystem queries. */
		class directory_handle : public fs_handle_adaptor<directory_handle, path_handle>
		{
			friend fs_handle_adaptor<directory_handle, path_handle>;
			friend handle_adaptor<directory_handle, path_handle>;

			using adaptor_base = fs_handle_adaptor<directory_handle, path_handle>;

		public:
			using native_handle_type = typename adaptor_base::native_handle_type;

			/** Integer type used for handle offsets. */
			using extent_type = typename adaptor_base::extent_type;
			/** Integer type used for handle buffers. */
			using size_type = typename adaptor_base::size_type;

			template<typename Op>
			using io_buffer_sequence = _directory::io_buffer_sequence<Op>;
			template<typename Op>
			using io_buffer = _directory::io_buffer<Op>;

			template<typename Op>
			using io_result = result<io_buffer_sequence<Op>, io_status_code>;
			template<typename Op>
			using io_request = _directory::io_request<Op>;

		public:
			/** Opens or creates a directory specified by \a path.
			 *
			 * @note The following values of \a flags are not supported:
			 * <ul>
			 * <li>`unlink_on_close`</li>
			 * <li>`no_sparse_files`</li>
			 * </ul>
			 * @note The following values of \a mode are not supported:
			 * <ul>
			 * <li>`truncate`</li>
			 * <li>`supersede`</li>
			 * </ul>
			 *
			 * @param path Path to the new directory.
			 * @param flags Handle flags to open the handle with.
			 * @param mode Mode to use when opening or creating the handle.
			 * @return Handle to the directory or a status code on failure. */
			[[nodiscard]] static result<directory_handle> open(path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::existing) noexcept { return open({}, path, flags, mode); }
			/**  Opens or creates a directory specified by \a path relative to parent location \a base.
			 *
			 * @note The following values of \a flags are not supported:
			 * <ul>
			 * <li>`unlink_on_close`</li>
			 * <li>`no_sparse_files`</li>
			 * </ul>
			 * @note The following values of \a mode are not supported:
			 * <ul>
			 * <li>`truncate`</li>
			 * <li>`supersede`</li>
			 * </ul>
			 *
			 * @param base Handle to the parent location.
			 * @param path Path to the new directory relative to \a base.
			 * @param flags Handle flags to open the handle with.
			 * @param mode Mode to use when opening or creating the handle.
			 * @return Handle to the directory or a status code on failure. */
			[[nodiscard]] static ROD_API_PUBLIC result<directory_handle> open(const path_handle &base, path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::existing) noexcept;

			/** Creates a new uniquely-named directory relative to parent location \a base.
			 *
			 * @note The following values of \a flags are not supported:
			 * <ul>
			 * <li>`unlink_on_close`</li>
			 * <li>`no_sparse_files`</li>
			 * </ul>
			 *
			 * @param base Handle to the parent location.
			 * @param flags Handle flags to open the handle with.
			 * @return Handle to the directory or a status code on failure. */
			[[nodiscard]] static ROD_API_PUBLIC result<directory_handle> open_unique(const path_handle &base, file_flags flags = file_flags::read | file_flags::write) noexcept;
			/** Opens or creates a directory specified by \a path relative to the system's temporary file directory.
			 *
			 * @note If \a path is empty, generates an implementation-defined unique name.
			 * @note The following values of \a flags are not supported:
			 * <ul>
			 * <li>`unlink_on_close`</li>
			 * <li>`no_sparse_files`</li>
			 * </ul>
			 * @note The following values of \a mode are not supported:
			 * <ul>
			 * <li>`truncate`</li>
			 * <li>`supersede`</li>
			 * </ul>
			 *
			 * @param flags Handle flags to open the handle with.
			 * @param mode Mode to use when opening or creating the handle.
			 * @return Handle to the directory or a status code on failure. */
			[[nodiscard]] static ROD_API_PUBLIC result<directory_handle> open_temporary(path_view path = {}, file_flags flags = file_flags::read | file_flags::write, open_mode mode = open_mode::always) noexcept;

		private:
			directory_handle(path_handle &&other, file_flags flags) noexcept : adaptor_base(std::forward<path_handle>(other))
			{
				auto hnd = adaptor_base::release();
				hnd.flags = std::uint32_t(flags);
				adaptor_base::release(hnd);
			}

		public:
			directory_handle(const directory_handle &) = delete;
			directory_handle &operator=(const directory_handle &) = delete;

			/** Initializes a closed directory handle. */
			directory_handle() noexcept = default;
			directory_handle(directory_handle &&) noexcept = default;
			directory_handle &operator=(directory_handle &&) noexcept = default;

			/** Initializes directory handle from a native handle. */
			explicit directory_handle(native_handle_type hnd) noexcept : adaptor_base(hnd) {}
			/** Initializes directory handle from a native handle and explicit device & inode IDs. */
			explicit directory_handle(native_handle_type hnd, dev_t dev, ino_t ino) noexcept : adaptor_base(hnd, dev, ino) {}

			[[nodiscard]] constexpr operator const path_handle &() const & noexcept { return adaptor_base::base(); }
			[[nodiscard]] constexpr operator const path_handle &&() const && noexcept { return std::move(adaptor_base::base()); }

			/** Returns the flags of the directory handle. */
			[[nodiscard]] file_flags flags() const noexcept { return static_cast<file_flags>(adaptor_base::native_handle().flags); }

			constexpr void swap(directory_handle &other) noexcept { adaptor_base::swap(other); }
			friend constexpr void swap(directory_handle &a, directory_handle &b) noexcept { a.swap(b); }

		public:
			/* TODO: Implement read_some. */

		private:
			ROD_API_PUBLIC result<> do_link(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept;
			ROD_API_PUBLIC result<> do_relink(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept;
			ROD_API_PUBLIC result<> do_unlink(const file_timeout &to) noexcept;

			result<directory_handle> do_clone() const noexcept
			{
				if (auto res = clone(base()); res.has_value())
					return directory_handle(std::move(*res), flags());
				else
					return res.error();
			}

			/* TODO: Implement do_reopen */
		};

		template<>
		class io_buffer<read_some_t>
		{
			friend class directory_handle;

		public:
			using value_type = typename path::value_type;
			using size_type = typename path::size_type;

		public:
			/** Initializes an empty directory entry buffer. */
			constexpr io_buffer() noexcept = default;
			/** Initializes the directory entry buffer with an explicit `stat` query \a q. */
			constexpr io_buffer(stat::query q) noexcept : _mask(q) {}

			/** Initializes the directory entry buffer from a pointer to a buffer of characters and a size. */
			constexpr io_buffer(value_type *buff, size_type size) noexcept : _buff(buff, size) {}
			/** Initializes the directory entry buffer from a pointer to a buffer of characters and a size with an explicit `stat` query \a q. */
			constexpr io_buffer(value_type *buff, size_type size, stat::query q) noexcept : _buff(buff, size), _mask(q) {}

			/** Initializes the directory entry buffer from a range of characters defined by [\a first, \a last). */
			template<std::contiguous_iterator I, std::sentinel_for<I> S> requires std::constructible_from<std::span<value_type>, I, S>
			constexpr io_buffer(I first, S last) noexcept(std::is_nothrow_constructible_v<std::span<value_type>, I, S>) : _buff(first, last) {}
			/** Initializes the directory entry buffer from a range of characters defined by [\a first, \a last) with an explicit `stat` query \a q. */
			template<std::contiguous_iterator I, std::sentinel_for<I> S> requires std::constructible_from<std::span<value_type>, I, S>
			constexpr io_buffer(I first, S last, stat::query q) noexcept(std::is_nothrow_constructible_v<std::span<value_type>, I, S>) : _buff(first, last), _mask(q) {}

			/** Initializes the directory entry buffer from a contiguous range of characters. */
			template<std::ranges::contiguous_range Buff> requires(!decays_to_same<Buff, io_buffer> && std::constructible_from<std::span<value_type>, Buff>)
			constexpr io_buffer(Buff &&buff) noexcept(std::is_nothrow_constructible_v<std::span<value_type>, Buff>) : _buff(std::forward<Buff>(buff)) {}
			/** Initializes the directory entry buffer from a contiguous range of characters with an explicit `stat` query \a q. */
			template<std::ranges::contiguous_range Buff> requires(!decays_to_same<Buff, io_buffer> && std::constructible_from<std::span<value_type>, Buff>)
			constexpr io_buffer(Buff &&buff, stat::query q) noexcept(std::is_nothrow_constructible_v<std::span<value_type>, Buff>) : _buff(std::forward<Buff>(buff)), _mask(q) {}

			/** Returns the path of the directory entry. */
			[[nodiscard]] constexpr path_view path() const noexcept { return static_cast<path_view>(*this); }
			/** Converts directory entry buffer to a `path_view`. Equivalent to `path()`. */
			[[nodiscard]] constexpr explicit operator path_view() const noexcept { return {_buff.data(), _buff.size(), false, path_view::native_format}; }

			/** Returns the `stat` metadata of the directory entry. */
			[[nodiscard]] constexpr const stat &metadata() const noexcept { return _metadata; }
			/** Returns mask of the initialized fields of the `stat` structure returned by `metadata()`. */
			[[nodiscard]] constexpr stat::query metadata_mask() const noexcept { return _mask; }

			/** Checks if the directory entry buffer is empty. */
			[[nodiscard]] constexpr bool empty() const noexcept { return _buff.empty(); }
			/** Returns size of the directory entry buffer. */
			[[nodiscard]] constexpr size_type size() const noexcept { return _buff.size(); }
			/** Returns pointer to the memory of the directory entry buffer. */
			[[nodiscard]] constexpr value_type *data() const noexcept { return _buff.data(); }

			constexpr void swap(io_buffer &other) noexcept
			{
				std::swap(_buff, other._buff);
				std::swap(_metadata, other._metadata);
				std::swap(_mask, other._mask);
			}
			friend constexpr void swap(io_buffer &a, io_buffer &b) noexcept { a.swap(b); }

		private:
			std::span<value_type> _buff = {};
			stat _metadata = {};
			stat::query _mask = {};
		};
		template<>
		class io_buffer_sequence<read_some_t>
		{
			friend class directory_handle;

			using buff_span = std::span<io_buffer<read_some_t>>;
			using chars_ptr = std::unique_ptr<typename path::value_type[]>;

		public:
			using value_type = typename buff_span::value_type;

			using pointer = typename buff_span::pointer;
			using const_pointer = typename buff_span::const_pointer;
			using reference = typename buff_span::reference;
			using const_reference = typename buff_span::const_reference;

			using iterator = typename buff_span::iterator;
			using reverse_iterator = typename buff_span::reverse_iterator;

			using difference_type = typename buff_span::difference_type;
			using size_type = typename buff_span::size_type;

		private:
#if defined(__cpp_lib_constexpr_memory) && __cpp_lib_constexpr_memory >= 202202L
			constexpr io_buffer_sequence(buff_span &&buffs, chars_ptr &&chars) noexcept : _buffs(std::forward<buff_span>(buffs)), _chars(std::forward<chars_ptr>(chars)) {}
#else
			io_buffer_sequence(buff_span &&buffs, chars_ptr &&chars) noexcept : _buffs(std::forward<buff_span>(buffs)), _chars(std::forward<chars_ptr>(chars)) {}
#endif

		public:
			io_buffer_sequence(const io_buffer_sequence &) = delete;
			io_buffer_sequence &operator=(const io_buffer_sequence &) = delete;

			constexpr io_buffer_sequence() noexcept = default;
#if defined(__cpp_lib_constexpr_memory) && __cpp_lib_constexpr_memory >= 202202L
			constexpr io_buffer_sequence(io_buffer_sequence &&) noexcept = default;
			constexpr io_buffer_sequence &operator=(io_buffer_sequence &&) noexcept = default;
#else
			io_buffer_sequence(io_buffer_sequence &&) noexcept = default;
			io_buffer_sequence &operator=(io_buffer_sequence &&) noexcept = default;
#endif

		public:
			/** Returns iterator to the first buffer of the buffer sequence. */
			[[nodiscard]] constexpr iterator begin() const noexcept { return _buffs.begin(); }
			/** Returns iterator one past the last buffer of the buffer sequence. */
			[[nodiscard]] constexpr iterator end() const noexcept { return _buffs.end(); }

			/** Returns reverse iterator one past the first buffer of the buffer sequence. */
			[[nodiscard]] constexpr reverse_iterator rbegin() const noexcept { return _buffs.rbegin(); }
			/** Returns reverse iterator to the first buffer of the buffer sequence. */
			[[nodiscard]] constexpr reverse_iterator rend() const noexcept { return _buffs.rend(); }

			/** Checks if the buffer sequence is empty. */
			[[nodiscard]] constexpr bool empty() const noexcept { return _buffs.empty(); }
			/** Returns size of the buffer sequence. */
			[[nodiscard]] constexpr size_type size() const noexcept { return _buffs.size(); }

			/** Returns pointer to the start of the buffer sequence. */
			[[nodiscard]] constexpr pointer data() const noexcept { return _buffs.data(); }
			/** Returns reference to the last buffer of the buffer sequence. */
			[[nodiscard]] constexpr reference back() const noexcept { return _buffs.back(); }
			/** Returns reference to the first buffer of the buffer sequence. */
			[[nodiscard]] constexpr reference front() const noexcept { return _buffs.front(); }
			/** Returns reference to the buffer located at offset \a i within the buffer sequence. */
			[[nodiscard]] constexpr reference operator[](size_type i) const { return _buffs[i]; }

		public:
#if defined(__cpp_lib_constexpr_memory) && __cpp_lib_constexpr_memory >= 202202L
			constexpr void swap(io_buffer &other) noexcept
			{
				std::swap(_buffs, other._buffs);
				std::swap(_chars, other._chars);
			}
			friend constexpr void swap(io_buffer &a, io_buffer &b) noexcept { a.swap(b); }
#else
			void swap(io_buffer_sequence &other) noexcept
			{
				std::swap(_buffs, other._buffs);
				std::swap(_chars, other._chars);
			}
			friend void swap(io_buffer_sequence &a, io_buffer_sequence &b) noexcept { a.swap(b); }
#endif

		private:
			buff_span _buffs;
			chars_ptr _chars;
		};

		template<>
		struct io_request<read_some_t>
		{
			/** Span of directory entry buffers to be filled with directory entries. When buffers are not empty, directory
			 * enumeration will attempt to use the provided memory as long as there is enough space to store the path.
			 * @note Supplied buffer will potentially be modified with the size and/or pointer to the actual memory of the entry path. */
			std::span<io_buffer<read_some_t>> dst;
			/** Directory enumeration filter passed to the native platform API.
			 * @note If the platform does not provide directory filtering, it is preformed manually. */
			typename path::string_view_type filter;
		};

		static_assert(std::convertible_to<const directory_handle &, const path_handle &>);
		static_assert(fs_handle<directory_handle>);
	}

	using _directory::directory_handle;
}

template<typename Op>
inline constexpr bool std::ranges::enable_view<rod::directory_handle::io_buffer_sequence<Op>> = true;

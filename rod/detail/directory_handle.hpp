/*
 * Created by switch_blade on 2023-09-01.
 */

#pragma once

#include "fs_handle_base.hpp"
#include "io_handle_base.hpp"
#include "path_discovery.hpp"
#include "path_handle.hpp"

namespace rod
{
	namespace _directory
	{
		class directory_entry;
		class directory_iterator;

		template<typename>
		class io_buffer;
		template<typename>
		class io_buffer_sequence;

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
			constexpr io_buffer(stat::query q) noexcept : _query(q) {}

			/** Initializes the directory entry buffer from a pointer to a buffer of characters and a size. */
			constexpr io_buffer(value_type *buff, size_type size) noexcept : _buff(buff, size) {}
			/** Initializes the directory entry buffer from a pointer to a buffer of characters and a size with an explicit `stat` query \a q. */
			constexpr io_buffer(value_type *buff, size_type size, stat::query q) noexcept : _buff(buff, size), _query(q) {}

			/** Initializes the directory entry buffer from a range of characters defined by [\a first, \a last). */
			template<std::contiguous_iterator I, std::sentinel_for<I> S> requires std::constructible_from<std::span<value_type>, I, S>
			constexpr io_buffer(I first, S last) noexcept(std::is_nothrow_constructible_v<std::span<value_type>, I, S>) : _buff(first, last) {}
			/** Initializes the directory entry buffer from a range of characters defined by [\a first, \a last) with an explicit `stat` query \a q. */
			template<std::contiguous_iterator I, std::sentinel_for<I> S> requires std::constructible_from<std::span<value_type>, I, S>
			constexpr io_buffer(I first, S last, stat::query q) noexcept(std::is_nothrow_constructible_v<std::span<value_type>, I, S>) : _buff(first, last), _query(q) {}

			/** Initializes the directory entry buffer from a contiguous range of characters. */
			template<std::ranges::contiguous_range Buff> requires(!decays_to_same<Buff, io_buffer> && std::constructible_from<std::span<value_type>, Buff>)
			constexpr io_buffer(Buff &&buff) noexcept(std::is_nothrow_constructible_v<std::span<value_type>, Buff>) : _buff(std::forward<Buff>(buff)) {}
			/** Initializes the directory entry buffer from a contiguous range of characters with an explicit `stat` query \a q. */
			template<std::ranges::contiguous_range Buff> requires(!decays_to_same<Buff, io_buffer> && std::constructible_from<std::span<value_type>, Buff>)
			constexpr io_buffer(Buff &&buff, stat::query q) noexcept(std::is_nothrow_constructible_v<std::span<value_type>, Buff>) : _buff(std::forward<Buff>(buff)), _query(q) {}

			/** Checks if the directory entry buffer is empty. */
			[[nodiscard]] constexpr bool empty() const noexcept { return _buff.empty(); }
			/** Returns size of the directory entry buffer. */
			[[nodiscard]] constexpr size_type size() const noexcept { return _buff.size(); }
			/** Returns pointer to the memory of the directory entry buffer. */
			[[nodiscard]] constexpr value_type *data() const noexcept { return _buff.data(); }

			/** Returns the data of the directory entry buffer as a `path_view`. */
			[[nodiscard]] constexpr path_view path() const noexcept { return static_cast<path_view>(*this); }
			/** Converts directory entry buffer to a `path_view`. Equivalent to `path()`. */
			[[nodiscard]] constexpr explicit operator path_view() const noexcept { return {_buff.data(), _buff.size(), _is_terminated, path_view::native_format}; }

			/** Returns a pair of directory entry stats and a mask of initialized fields. */
			[[nodiscard]] constexpr std::pair<stat, stat::query> st() const noexcept { return {_st, _query}; }

		private:
			std::span<value_type> _buff;
			stat _st;

			stat::query _query = {};
			bool _is_terminated = {};
		};
		template<>
		class io_buffer_sequence<read_some_t>
		{
			friend class directory_handle;

			struct free_deleter { void operator()(auto *p) const noexcept { if (p) std::free(p); } };

			using buff_type = std::unique_ptr<typename path::value_type[], free_deleter>;
			using data_type = std::span<io_buffer<read_some_t>>;

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
			io_buffer_sequence(data_type &&buffs, buff_type &&chars, size_type chars_max) noexcept : _data(std::forward<data_type>(buffs)), _buff(std::forward<buff_type>(chars)), _buff_max(chars_max) {}

		public:
			io_buffer_sequence(const io_buffer_sequence &) = delete;
			io_buffer_sequence &operator=(const io_buffer_sequence &) = delete;

			constexpr io_buffer_sequence() noexcept = default;
			/** Initializes the buffer sequence from a pointer to an entry buffer of characters and a size. */
			constexpr io_buffer_sequence(value_type *buff, size_type size) noexcept : _data(buff, size) {}
			/** Initializes the buffer sequence from from a range of entry buffers defined by [\a first, \a last). */
			template<std::contiguous_iterator I, std::sentinel_for<I> S> requires std::constructible_from<data_type, I, S>
			constexpr io_buffer_sequence(I first, S last) noexcept(std::is_nothrow_constructible_v<data_type, I, S>) : _data(first, last) {}
			/** Initializes the buffer sequence from a contiguous range of entry buffers. */
			template<std::ranges::contiguous_range Buff> requires(!decays_to_same<Buff, io_buffer_sequence> && std::constructible_from<data_type, Buff>)
			constexpr io_buffer_sequence(Buff &&buff) noexcept(std::is_nothrow_constructible_v<data_type, Buff>) : _data(std::forward<Buff>(buff)) {}

			io_buffer_sequence(io_buffer_sequence &&other) noexcept : _data(std::exchange(other._data, {})), _buff(std::move(other._buff)), _buff_max(std::exchange(other._buff_max, {})) {}
			io_buffer_sequence &operator=(io_buffer_sequence &&other) noexcept { return (_data = std::exchange(other._data, {}), _buff = std::move(other._buff), _buff_max = std::exchange(other._buff_max, {}), *this); }

			/** Initializes the buffer sequence from a pointer to an entry buffer of characters and a size, using internal character buffer of \a other. */
			io_buffer_sequence(io_buffer_sequence &&other, value_type *buff, size_type size) noexcept : _data(buff, size), _buff(std::move(other._buff)), _buff_max(std::exchange(other._buff_max, {})) {}
			/** Initializes the buffer sequence from from a range of entry buffers defined by [\a first, \a last), using internal character buffer of \a other. */
			template<std::contiguous_iterator I, std::sentinel_for<I> S> requires std::constructible_from<data_type, I, S>
			io_buffer_sequence(io_buffer_sequence &&other, I first, S last) noexcept(std::is_nothrow_constructible_v<data_type, I, S>) : _data(first, last), _buff(std::move(other._buff)), _buff_max(std::exchange(other._buff_max, {})) {}
			/** Initializes the buffer sequence from a contiguous range of entry buffers, using internal character buffer of \a other. */
			template<std::ranges::contiguous_range Buff> requires(!decays_to_same<Buff, io_buffer_sequence> && std::constructible_from<data_type, Buff>)
			io_buffer_sequence(io_buffer_sequence &&other, Buff &&buff) noexcept(std::is_nothrow_constructible_v<data_type, Buff>) : _data(std::forward<Buff>(buff)), _buff(std::move(other._buff)), _buff_max(std::exchange(other._buff_max, {})) {}

			/** Returns iterator to the first buffer of the buffer sequence. */
			[[nodiscard]] constexpr iterator begin() const noexcept { return _data.begin(); }
			/** Returns iterator one past the last buffer of the buffer sequence. */
			[[nodiscard]] constexpr iterator end() const noexcept { return _data.end(); }

			/** Returns reverse iterator one past the first buffer of the buffer sequence. */
			[[nodiscard]] constexpr reverse_iterator rbegin() const noexcept { return _data.rbegin(); }
			/** Returns reverse iterator to the first buffer of the buffer sequence. */
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

			/** Converts buffer sequence to a span of directory entry buffers. */
			[[nodiscard]] constexpr auto as_span() const noexcept { return _data; }

			void swap(io_buffer_sequence &other) noexcept
			{
				std::swap(_data, other._data);
				std::swap(_buff, other._buff);
				std::swap(_buff_max, other._buff_max);
			}
			friend void swap(io_buffer_sequence &a, io_buffer_sequence &b) noexcept { a.swap(b); }

		private:
			data_type _data = {};
			buff_type _buff = {};
			size_type _buff_max = {};
		};

		template<typename>
		struct io_request;
		template<>
		struct io_request<read_some_t>
		{
			/** Sequence of directory entry buffers to be filled with directory entries. When buffers are not empty, directory enumeration will attempt to use
			 * the provided memory as long as there is enough space to store the path. Filled buffers will be returned in the buffer sequence of `io_result`.
			 * @note `read_some` will return a truncated copy of the buffer sequence which may also contain an internally-allocated character buffer.
			 * This internal buffer can be re-used by passing the returned buffer sequence as \a buffs to the next call of `read_some`.
			 * @note Supplied buffers will be modified with the size and/or pointers to the actual memory of the entry path. */
			io_buffer_sequence<read_some_t> buffs;
			/** Directory enumeration filter passed to the native platform API.
			 * @note If the platform does not provide directory filtering, it is preformed manually. */
			path_view filter;
			/** When set to `true`, directory enumeration will resume from the position of the last directory entry. */
			bool resume = {};
		};

		/** @brief Handle to a unique directory within the filesystem.
		 *
		 * `directory_handle` works as an extended version of `path_handle` (and is convertible to it),
		 * providing additional support for directory creation, enumeration, and filesystem operations. */
		class directory_handle : public fs_handle_adaptor<directory_handle, path_handle>
		{
			friend fs_handle_adaptor<directory_handle, path_handle>;
			friend handle_adaptor<directory_handle, path_handle>;
			friend std::lock_guard<directory_handle>;

			using adp_base = fs_handle_adaptor<directory_handle, path_handle>;

		public:
			using native_handle_type = typename adp_base::native_handle_type;
			/** Timeout type used for handle operations. */
			using timeout_type = typename adp_base::timeout_type;
			/** Integer type used for handle offsets. */
			using extent_type = typename adp_base::extent_type;
			/** Integer type used for handle buffers. */
			using size_type = typename adp_base::size_type;

			template<typename Op>
			using io_buffer_sequence = _directory::io_buffer_sequence<Op>;
			template<typename Op>
			using io_buffer = _directory::io_buffer<Op>;

			template<typename Op>
			using io_result = result<io_buffer_sequence<Op>>;
			template<typename Op>
			using io_request = _directory::io_request<Op>;

		public:
			/** Creates a uniquely-named directory relative to \a base.
			 * @note The following values of \a flags are not supported:
			 * <ul>
			 * <li>`unlink_on_close`</li>
			 * <li>`no_sparse_files`</li>
			 * </ul>
			 *
			 * @param base Handle to the parent location. If set to an invalid handle, generates a unique directory within the current working directory.
			 * @param flags Handle flags to open the handle with. `file_flags::read | file_flags::write` by default.
			 * @return Handle to the directory or a status code on failure. */
			[[nodiscard]] static result<directory_handle> open_unique(const path_handle &base, file_flags flags = file_flags::read | file_flags::write) noexcept
			{
				try { return open(base, _handle::generate_unique_name(), flags, open_mode::always); }
				catch (const std::bad_alloc &) { return std::make_error_code(std::errc::not_enough_memory); }
			}
			/** Opens or creates a directory inside the system's temporary files directory.
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
			 * @param path Path to the target directory, relative to the system's temporary file directory.
			 * @param flags Handle flags to open the handle with. `file_flags::read | file_flags::write` by default.
			 * @param mode Mode to use when opening or creating the handle. `open_mode::always` by default.
			 * @return Handle to the directory or a status code on failure. */
			[[nodiscard]] inline static result<directory_handle> open_temporary(path_view path, file_flags flags = file_flags::read | file_flags::write, open_mode mode = open_mode::always) noexcept;
			/** Opens or creates a directory.
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
			 * @param base Handle to the parent location. If set to an invalid handle, \a path must be a fully-qualified path.
			 * @param path Path to the target directory relative to \a base if it is a valid handle, otherwise a fully-qualified path.
			 * @param flags Handle flags to open the handle with. `file_flags::read` by default.
			 * @param mode Mode to use when opening or creating the handle. `open_mode::existing` by default.
			 * @return Handle to the directory or a status code on failure. */
			[[nodiscard]] static ROD_API_PUBLIC result<directory_handle> open(const path_handle &base, path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::existing) noexcept;

			/* TODO: Implement reopen */

		private:
			directory_handle(path_handle &&other, file_flags flags) noexcept : adp_base(std::forward<path_handle>(other))
			{
				auto hnd = adp_base::release();
				hnd.flags = std::uint32_t(flags);
				adp_base::release(hnd);
			}

		public:
			directory_handle(const directory_handle &) = delete;
			directory_handle &operator=(const directory_handle &) = delete;

			/** Initializes a closed directory handle. */
			directory_handle() noexcept = default;
			directory_handle(directory_handle &&other) noexcept : adp_base(std::forward<adp_base>(other)) {}
			directory_handle &operator=(directory_handle &&other) noexcept { return (adp_base::operator=(std::forward<adp_base>(other)), _read_guard = false, *this); }

			/** Initializes directory handle from a native handle. */
			explicit directory_handle(native_handle_type hnd) noexcept : adp_base(hnd) {}
			/** Initializes directory handle from a native handle and explicit device & inode IDs. */
			explicit directory_handle(native_handle_type hnd, dev_t dev, ino_t ino) noexcept : adp_base(hnd, dev, ino) {}

			[[nodiscard]] constexpr operator const path_handle &() const & noexcept { return static_cast<const path_handle &>(adp_base::base()); }
			[[nodiscard]] constexpr operator const path_handle &&() const && noexcept { return static_cast<const path_handle &&>(std::move(adp_base::base())); }

			/** Returns the flags of the directory handle. */
			[[nodiscard]] file_flags flags() const noexcept { return static_cast<file_flags>(adp_base::native_handle().flags); }

			constexpr void swap(directory_handle &other) noexcept { adp_base::swap(other); }
			friend constexpr void swap(directory_handle &a, directory_handle &b) noexcept { a.swap(b); }

		public:
			template<decays_to_same<read_some_t> Op, decays_to_same<directory_handle> Hnd>
			friend io_result<Op> tag_invoke(Op, Hnd &&hnd, io_request<Op> req, const file_timeout &to) noexcept { return hnd.do_read_some(std::move(req), to); }

		private:
			void unlock() noexcept
			{
				std::atomic_ref(_read_guard).store(false, std::memory_order_release);
				std::atomic_ref(_read_guard).notify_one();
			}
			void lock() noexcept
			{
				while (std::atomic_ref(_read_guard).exchange(true, std::memory_order_acq_rel))
					std::atomic_ref(_read_guard).wait(true);
			}

			result<directory_handle> do_clone() const noexcept
			{
				if (auto res = clone(base()); res.has_value())
					return directory_handle(std::move(*res), flags());
				else
					return res.error();
			}

			ROD_API_PUBLIC result<> do_link(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept;
			ROD_API_PUBLIC result<> do_relink(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept;
			ROD_API_PUBLIC result<> do_unlink(const file_timeout &to) noexcept;

			ROD_API_PUBLIC io_result<read_some_t> do_read_some(io_request<read_some_t> &&req, const timeout_type &to) noexcept;

			bool _read_guard = false;
		};

		static_assert(std::convertible_to<const directory_handle &, const path_handle &>);
		static_assert(io_handle<directory_handle, read_some_t>);
		static_assert(fs_handle<directory_handle>);

		/** Structure describing an element of a directory returned by directory iterators.
		 * @note `directory_entry` is a "soft" description of an entry and does not behave like a handle. */
		class directory_entry
		{
			friend class directory_iterator;

		public:
			/** Initializes an empty directory entry. */
			constexpr directory_entry() noexcept = default;

			/** Returns a const reference to path of the directory entry. */
			[[nodiscard]] constexpr const rod::path &path() const & noexcept { return _path; }
			/** Returns an rvalue reference to path of the directory entry. */
			[[nodiscard]] constexpr rod::path &&path() && noexcept { return std::move(_path); }

			/** Returns a pair of directory entry stats and a mask of initialized fields. */
			[[nodiscard]] constexpr std::pair<stat, stat::query> st() const noexcept { return {_st, _query}; }

			constexpr void swap(directory_entry &other) noexcept
			{
				_path.swap(other._path);
				std::swap(_st, other._st);
				std::swap(_query, other._query);
			}
			friend constexpr void swap(directory_entry &a, directory_entry &b) noexcept { a.swap(b); }

		private:
			/* Hack to get a mutable reference to path's string. */
			auto &to_path_string() noexcept { return (typename path::string_type &) std::move(_path).native(); }

			rod::path _path;
			stat _st = stat(nullptr);
			stat::query _query = {};
		};
		/** Iterator used to enumerate entries of a directory. */
		class directory_iterator
		{
		public:
			using value_type = directory_entry;
			using difference_type = std::ptrdiff_t;
			using pointer = const value_type *;
			using reference = const value_type &;
			using iterator_category = std::input_iterator_tag;

		public:
			/** Creates a directory iterator from a clone of handle \a hnd.
			 * @param hnd Handle to the target directory.
			 * @return Iterator to the first entry in the directory (may be the end iterator), or a status code on failure to create the iterator. */
			[[nodiscard]] static ROD_API_PUBLIC result<directory_iterator> from_handle(const path_handle &hnd) noexcept;
			/** Creates a directory iterator for directory specified by \a path relative to parent location \a base.
			 * @param base Handle to the parent location. If set to an invalid handle, \a path must be a fully-qualified path.
			 * @param path Path to the target directory relative to \a base if it is a valid handle, otherwise a fully-qualified path.
			 * @return Iterator to the first entry in the directory (may be the end iterator), or a status code on failure to create the iterator. */
			[[nodiscard]] static ROD_API_PUBLIC result<directory_iterator> from_path(const path_handle &base, path_view path) noexcept;

		private:
			explicit directory_iterator(typename basic_handle::native_handle_type &&hnd) noexcept : _dir_hnd(std::forward<decltype(hnd)>(hnd)) {}

		public:
			/** Initializes an empty (sentinel) iterator. */
			directory_iterator() noexcept = default;
			/** Move-constructs the iterator from \a other. */
			directory_iterator(directory_iterator &&) noexcept = default;
			/** Move-assigns the iterator from \a other. */
			directory_iterator &operator=(directory_iterator &&) noexcept = default;

			/** Copy-constructs the iterator from \a other.
			 * @note This will clone the internal handle and copy the path buffer.
			 * @throw std::system_error On failure to clone the internal handle.
			 * @throw std::bad_alloc On failure to copy the path buffer. */
			directory_iterator(const directory_iterator &other) : _dir_hnd(clone(other._dir_hnd).value()), _entry(other._entry) {}
			/** Copy-assigns the iterator from \a other.
			 * @note This will clone the internal handle and copy the path buffer.
			 * @throw std::system_error On failure to clone the internal handle.
			 * @throw std::bad_alloc On failure to copy the path buffer. */
			directory_iterator &operator=(const directory_iterator &other)
			{
				if (this != &other)
				{
					_dir_hnd = clone(other._dir_hnd).value();
					_entry = other._entry;
				}
				return *this;
			}

		public:
			/** Returns a copy of `this`.
			 * @throw std::system_error On failure to clone the internal handle.
			 * @throw std::bad_alloc On failure to copy the path buffer. */
			[[nodiscard]] directory_iterator begin() const { return *this; }
			/** Returns an empty sentinel iterator. */
			[[nodiscard]] directory_iterator end() const noexcept { return {}; }

			/** Advances the iterator with max timeout.
			 * @throw std::system_error On failure to clone or advance the internal handle.
			 * @throw std::bad_alloc On failure to copy the path buffer. */
			directory_iterator &operator++() { return (next().value(), *this); }
			/** Advances the iterator using an optional timeout and returns errors as `result`. */
			ROD_API_PUBLIC result<> next(const file_timeout & = file_timeout()) noexcept;

			/** Advances the iterator with max timeout and returns a copy of the previous position.
			 * @throw std::system_error On failure to clone or advance the internal handle.
			 * @throw std::bad_alloc On failure to copy the path buffer. */
			[[nodiscard]] directory_iterator operator++(int)
			{
				auto tmp = *this;
				next().value();
				return *this;
			}

			/** Returns reference to the internal directory entry. */
			constexpr const directory_entry &operator*() const noexcept { return _entry; }
			/** Returns pointer to the internal directory entry. */
			constexpr const directory_entry *operator->() const noexcept { return &_entry; }

			constexpr void swap(directory_iterator &other) noexcept
			{
				_dir_hnd.swap(other._dir_hnd);
				_entry.swap(other._entry);
			}
			friend constexpr void swap(directory_iterator &a, directory_iterator &b) noexcept { a.swap(b); }

			constexpr bool operator==(const directory_iterator &other) const noexcept { return _dir_hnd == other._dir_hnd; }
			constexpr bool operator!=(const directory_iterator &other) const noexcept { return _dir_hnd != other._dir_hnd; }

		private:
			/* Duplicate of the original handle with read access only. */
			basic_handle _dir_hnd;
			directory_entry _entry;
		};
	}

	using _directory::directory_handle;

	using _directory::directory_iterator;
	using _directory::directory_entry;
}

template<typename Op>
inline constexpr bool std::ranges::enable_view<rod::directory_handle::io_buffer_sequence<Op>> = true;

template<>
inline constexpr bool std::ranges::enable_borrowed_range<rod::directory_iterator> = true;
template<>
inline constexpr bool std::ranges::enable_view<rod::directory_iterator> = true;

static_assert(std::input_iterator<rod::directory_iterator>);
static_assert(std::ranges::view<rod::directory_iterator>);
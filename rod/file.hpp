/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#include "detail/adaptors/read_some.hpp"
#include "detail/adaptors/write_some.hpp"

#include <filesystem>
#include <cassert>
#include <cstdio>

/* Platform-specific implementations. */
#if defined(__unix__)
#include "unix/detail/file.hpp"
#elif defined(_WIN32)
#include "win32/detail/file.hpp"
#else
#error Unkonwn file platform
#endif

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	namespace _file
	{
		template<typename T>
		concept noexcept_sizeable_range = detail::nothrow_callable<decltype(std::ranges::begin), T> && detail::nothrow_callable<decltype(std::ranges::size), T>;

		template<typename T, typename Snd, typename Rcv, typename... Args>
		struct sync_operation { struct type; };
		template<typename T, typename Rcv, typename... Args>
		struct sync_receiver { struct type; };
		template<typename T, typename Snd, typename... Args>
		struct sync_sender { struct type; };

		template<typename T, typename Rcv, typename... Args>
		struct sync_operation_base
		{
			[[ROD_NO_UNIQUE_ADDRESS]] Rcv _rcv;
			std::tuple<Args...> _args;
		};

		template<typename T, typename Snd, typename Rcv, typename... Args>
		struct sync_operation<T, Snd, Rcv, Args...>::type : sync_operation_base<T, Rcv, Args...>
		{
			using _receiver_t = typename sync_receiver<T, Rcv, Args...>::type;
			using _operation_base_t = sync_operation_base<T, Rcv, Args...>;
			using _state_t = connect_result_t<Snd, _receiver_t>;

			type(type &&) = delete;
			type &operator=(type &&) = delete;

			constexpr type(Snd &&snd, Rcv rcv, std::tuple<Args...> args) : _operation_base_t{std::move(rcv), std::move(args)}, _state(connect(std::forward<Snd>(snd), _receiver_t{this})) {}

			friend constexpr void tag_invoke(start_t, type &op) noexcept { start(op._state); }

			_state_t _state;
		};
		template<typename T, typename Rcv, typename... Args>
		struct sync_receiver<T, Rcv, Args...>::type
		{
			using is_receiver = std::true_type;
			using _operation_base_t = sync_operation_base<T, Rcv, Args...>;

			friend constexpr env_of_t<Rcv> tag_invoke(get_env_t, const type &r) noexcept(nothrow_tag_invocable<get_env_t, const Rcv &>) { return get_env(r._op->_rcv); }

			friend constexpr void tag_invoke(set_value_t, type &&r) noexcept
			{
				if constexpr ((std::is_nothrow_move_constructible_v<Args> && ...))
					try { r._complete(); } catch (...) { set_error(std::move(r._op->_rcv), std::current_exception()); }
				else
					r._complete();
			}
			template<detail::completion_channel C, typename... Vs> requires(!std::same_as<C, set_value_t> && detail::callable<C, Rcv, Vs...>)
			friend constexpr void tag_invoke(C, type &&r, Vs &&...args) noexcept { C{}(std::move(r._op->_rcv), std::forward<Vs>(args)...); }

			constexpr void _complete() noexcept((std::is_nothrow_move_constructible_v<Args> && ...))
			{
				std::apply([&]<typename... Vs>(Vs &&...vs) noexcept((std::is_nothrow_move_constructible_v<Args> && ...))
				{
					std::error_code err;
					if (const auto res = T{}(std::forward<Vs>(vs)..., err); err)
						[[unlikely]] set_error(std::move(_op->_rcv), err);
					else
						set_value(std::move(_op->_rcv), res);
				}, std::move(_op->_args));
			}

			_operation_base_t *_op;
		};
		template<typename T, typename Snd, typename... Args>
		struct sync_sender<T, Snd, Args...>::type
		{
			using is_sender = std::true_type;

			template<typename U, typename Rcv>
			using _operation_t = typename sync_operation<T, copy_cvref_t<U, Snd>, Rcv, Args...>::type;
			template<typename Rcv>
			using _receiver_t = typename sync_receiver<T, Rcv, Args...>::type;

			template<typename... Ts>
			using _value_signs_t = completion_signatures<>;
			template<typename Err>
			using _error_signs_t = detail::concat_tuples_t<completion_signatures<detail::make_signature_t<set_error_t, Err>>,
			                                               std::conditional_t<std::conjunction_v<std::is_nothrow_move_constructible<Args>...>,
			                                                                  completion_signatures<set_error_t(std::exception_ptr)>,
			                                                                  completion_signatures<>>>;
			template<typename U, typename E>
			using _signs_t = make_completion_signatures<copy_cvref_t<U, Snd>, E, completion_signatures<set_value_t(std::size_t), set_error_t(std::error_code)>, _value_signs_t, _error_signs_t>;

			friend constexpr env_of_t<Snd> tag_invoke(get_env_t, const type &s) noexcept(nothrow_tag_invocable<get_env_t, const Snd &>) { return get_env(s._snd); }

			template<decays_to<type> U, typename E>
			friend constexpr _signs_t<U, E> tag_invoke(get_completion_signatures_t, U &&, E) noexcept { return {}; }
			template<decays_to<type> U, rod::receiver Rcv>
			friend constexpr _operation_t<U, Rcv> tag_invoke(connect_t, U &&s, Rcv r) noexcept(std::is_nothrow_constructible_v<_operation_t<U, Rcv>, copy_cvref_t<U, Snd>, Rcv, copy_cvref_t<U, std::tuple<Args...>>>)
			{
				return _operation_t<U, Rcv>(std::forward<U>(s)._snd, std::move(r), std::forward<U>(s)._args);
			}

			[[ROD_NO_UNIQUE_ADDRESS]] Snd _snd;
			std::tuple<Args...> _args;
		};

		/** Basic filesystem file handle (such as a POSIX file descriptor or Win32 HANDLE). */
		class basic_file
		{
			using native_t = detail::system_file;

		public:
			using native_handle_type = typename native_t::native_handle_type;

			using fileprot = int;

			/** File is created with execute permissions for the current user. */
			static constexpr fileprot user_exec = native_t::user_exec;
			/** File is created with read permissions for the current user. */
			static constexpr fileprot user_read = native_t::user_read;
			/** File is created with write permissions for the current user. */
			static constexpr fileprot user_write = native_t::user_write;
			/** File is created with execute permissions for the current group. */
			static constexpr fileprot group_exec = native_t::group_exec;
			/** File is created with read permissions for the current group. */
			static constexpr fileprot group_read = native_t::group_read;
			/** File is created with write permissions for the current group. */
			static constexpr fileprot group_write = native_t::group_write;
			/** File is created with execute permissions for other users. */
			static constexpr fileprot other_exec = native_t::other_exec;
			/** File is created with read permissions for the other users. */
			static constexpr fileprot other_read = native_t::other_read;
			/** File is created with write permissions for the other users. */
			static constexpr fileprot other_write = native_t::other_write;

			using openmode = int;

			/** Opens the file for input. */
			static constexpr openmode in = native_t::in;
			/** Opens the file for output. */
			static constexpr openmode out = native_t::out;

			/** Seeks the file to the end after it is opened. */
			static constexpr openmode ate = native_t::ate;
			/** Opens the file in append mode. Write operations will append to the end of the file. */
			static constexpr openmode app = native_t::app;
			/** Opens the file in truncate mode. Write operations will overwrite existing contents. */
			static constexpr openmode trunc = native_t::trunc;

			/** Opens the file in binary mode.  */
			static constexpr openmode binary = native_t::binary;
			/** Opens the file in direct mode. */
			static constexpr openmode direct = native_t::direct;
			/** Opens the file in open-only mode. Opening the file will fail if it does not exist. */
			static constexpr openmode nocreate = native_t::nocreate;
			/** Opens the file in create-only mode. Opening the file will fail if it already exists. */
			static constexpr openmode noreplace = native_t::noreplace;

			using seekdir = int;

			/** Seek from the start of the file. */
			static constexpr seekdir beg = native_t::beg;
			/** Seek from the current position within the file. */
			static constexpr seekdir cur = native_t::cur;
			/** Seek from the end of the file. */
			static constexpr seekdir end = native_t::end;

		public:
			/** @brief Opens the file specified by \a path using mode flags \a flags.
			 * @param[in] path Path to the file to be opened.
			 * @param[in] mode Mode flags to use for the opened file.
			 * @return `basic_file` handle to the opened file.
			 * @throw std::system_error On failure to open the file.  */
			[[nodiscard]] static basic_file open(const char *path, openmode mode)
			{
				std::error_code err;
				if (auto res = open(path, mode, err); err)
					throw std::system_error(err, "rod::basic_file::open");
				else
					return res;
			}
			/** @copydoc open */
			[[nodiscard]] static basic_file open(const wchar_t *path, openmode mode)
			{
				std::error_code err;
				if (auto res = open(path, mode, err); err)
					throw std::system_error(err, "rod::basic_file::open");
				else
					return res;
			}
			/** @copydoc open */
			[[nodiscard]] static basic_file open(const std::string &path, openmode mode) { return open(path.c_str(), mode); }
			/** @copydoc open */
			[[nodiscard]] static basic_file open(const std::wstring &path, openmode mode) { return open(path.c_str(), mode); }
			/** @copydoc open */
			[[nodiscard]] static basic_file open(const std::filesystem::path &path, openmode mode) { return open(path.c_str(), mode); }

			/** @copybrief open
			 * @param[in] path Path to the file to be opened.
			 * @param[in] mode Mode flags to open the file with.
			 * @param[out] err Reference to the error code set on failure to open the file.
			 * @return `basic_file` handle to the opened file, or a closed file handle if an error has occurred. */
			[[nodiscard]] static basic_file open(const char *path, openmode mode, std::error_code &err) noexcept { return {native_t::open(path, mode | binary, err)}; }
			/** @copydoc open */
			[[nodiscard]] static basic_file open(const wchar_t *path, openmode mode, std::error_code &err) noexcept { return {native_t::open(path, mode | binary, err)}; }
			/** @copydoc open */
			[[nodiscard]] static basic_file open(const std::string &path, openmode mode, std::error_code &err) noexcept { return open(path.c_str(), mode, err); }
			/** @copydoc open */
			[[nodiscard]] static basic_file open(const std::wstring &path, openmode mode, std::error_code &err) noexcept { return open(path.c_str(), mode, err); }
			/** @copydoc open */
			[[nodiscard]] static basic_file open(const std::filesystem::path &path, openmode mode, std::error_code &err) noexcept { return open(path.c_str(), mode, err); }

			/** @copybrief open
			 * @param[in] path Path to the file to be opened.
			 * @param[in] mode Mode flags to use for the opened file.
			 * @param[in] prot Protection flags to use for the opened file.
			 * @return `basic_file` handle to the opened file.
			 * @throw std::system_error On failure to open the file.
			 * @note Protection flags are currently ignored under Win32. */
			[[nodiscard]] static basic_file open(const char *path, openmode mode, fileprot prot)
			{
				std::error_code err;
				if (auto res = open(path, mode, prot, err); err)
					throw std::system_error(err, "rod::basic_file::open");
				else
					return res;
			}
			/** @copydoc open */
			[[nodiscard]] static basic_file open(const wchar_t *path, openmode mode, fileprot prot)
			{
				std::error_code err;
				if (auto res = open(path, mode, prot, err); err)
					throw std::system_error(err, "rod::basic_file::open");
				else
					return res;
			}
			/** @copydoc open */
			[[nodiscard]] static basic_file open(const std::string &path, openmode mode, fileprot prot) { return open(path.c_str(), mode, prot); }
			/** @copydoc open */
			[[nodiscard]] static basic_file open(const std::wstring &path, openmode mode, fileprot prot) { return open(path.c_str(), mode, prot); }
			/** @copydoc open */
			[[nodiscard]] static basic_file open(const std::filesystem::path &path, openmode mode, fileprot prot) { return open(path.c_str(), mode, prot); }

			/** @copybrief open
			 * @param[in] path Path to the file to be opened.
			 * @param[in] mode Mode flags to use for the opened file.
			 * @param[in] prot Protection flags to use for the opened file.
			 * @param[out] err Reference to the error code set on failure to open the file.
			 * @return `basic_file` handle to the opened file, or a closed file handle if an error has occurred.
			 * @note Protection flags are currently ignored under Win32. */
			[[nodiscard]] static basic_file open(const char *path, openmode mode, fileprot prot, std::error_code &err) noexcept { return {native_t::open(path, mode | binary, prot, err)}; }
			/** @copydoc open */
			[[nodiscard]] static basic_file open(const wchar_t *path, openmode mode, fileprot prot, std::error_code &err) noexcept { return {native_t::open(path, mode | binary, prot, err)}; }
			/** @copydoc open */
			[[nodiscard]] static basic_file open(const std::string &path, openmode mode, fileprot prot, std::error_code &err) noexcept { return open(path.c_str(), mode, prot, err); }
			/** @copydoc open */
			[[nodiscard]] static basic_file open(const std::wstring &path, openmode mode, fileprot prot, std::error_code &err) noexcept { return open(path.c_str(), mode, prot, err); }
			/** @copydoc open */
			[[nodiscard]] static basic_file open(const std::filesystem::path &path, openmode mode, fileprot prot, std::error_code &err) noexcept { return open(path.c_str(), mode, prot, err); }

			/** @brief Re-opens a file pointed to by a native file handle and mode flags.
			 * @param[in] file Native handle to an existing file to be re-opened.
			 * @param[in] mode Mode flags to use when re-opening the file.
			 * @return `basic_file` handle to the re-opened file.
			 * @throw std::system_error On failure to reopen the file. */
			[[nodiscard]] static basic_file reopen(native_handle_type file, openmode mode)
			{
				std::error_code err;
				if (auto res = reopen(file, mode, err); err)
					throw std::system_error(err, "rod::basic_file::reopen");
				else
					return res;
			}
			/** @copybrief reopen
			 * @param[in] file Native handle to an existing file to be re-opened.
			 * @param[in] mode Mode flags to use when re-opening the file.
			 * @param[out] err Reference to the error code set on failure to re-open the file.
			 * @return `basic_file` handle to the re-opened file. */
			[[nodiscard]] static basic_file reopen(native_handle_type file, openmode mode, std::error_code &err) noexcept { return {native_t::reopen(file, mode | binary, err)}; }

			/** @brief Re-opens a file from an existing file handle and mode flags.
			 * @param[in] file Native Handle to an existing file to be re-opened.
			 * @param[in] mode Mode flags to use when re-opening the file.
			 * @return `basic_file` handle to the re-opened file.
			 * @throw std::system_error On failure to reopen the file. */
			[[nodiscard]] static basic_file reopen(const basic_file &file, openmode mode) { return reopen(file.native_handle(), mode); }
			/** @copybrief reopen
			 * @param[in] file Native Handle to an existing file to be re-opened.
			 * @param[in] mode Mode flags to use when re-opening the file.
			 * @param[out] err Reference to the error code set on failure to re-open the file.
			 * @return `basic_file` handle to the re-opened file. */
			[[nodiscard]] static basic_file reopen(const basic_file &file, openmode mode, std::error_code &err) noexcept { return reopen(file.native_handle(), mode, err); }

		private:
			constexpr basic_file(native_t &&file) noexcept : _file(std::move(file)) {}

		public:
			constexpr basic_file() noexcept = default;
			constexpr basic_file(basic_file &&other) noexcept : _file(std::move(other._file)) {}
			constexpr basic_file &operator=(basic_file &&other) noexcept { return (_file = std::move(other._file), *this); }

			/** Initializes the file from a native file handle. */
			explicit basic_file(native_handle_type file) noexcept : _file(file) {}

			/** @brief Closes the file handle.
			 * @param[out] err Reference to the error code set on failure to close the file. */
			void close(std::error_code &err) noexcept { err = _file.close(); }
			/** @copybrief close
			 * @throw std::system_error On failure to close the file. */
			void close() { if (auto err = _file.close(); err) throw std::system_error(err, "rod::basic_file::close"); }

			/** Releases the underlying native file handle without closing. */
			native_handle_type release() noexcept { return _file.release(); }
			/** Releases the underlying native file handle without closing and replaces it with the specified handle. */
			native_handle_type release(native_handle_type new_file) noexcept { return _file.release(new_file); }

			/** @brief Synchronizes file with the underlying device.
			 * @param[out] err Reference to the error code set on failure to synchronize the file. */
			void sync(std::error_code &err) noexcept { err = _file.sync(); }
			/** @copybrief flush
			 * @throw std::system_error On failure to flush the file. */
			void sync() { if (const auto err = _file.sync(); err) throw std::system_error(err, "rod::basic_file::sync"); }

			/** @brief Flushes file buffers.
			 * @param[out] err Reference to the error code set on failure to flush the file. */
			void flush(std::error_code &err) noexcept { err = _file.flush(); }
			/** @copybrief flush
			 * @throw std::system_error On failure to flush the file. */
			void flush() { if (const auto err = _file.flush(); err) throw std::system_error(err, "rod::basic_file::flush"); }

			/** @brief Returns the current position within the file. Equivalent to `seek(0, cur)`.
			 * @return Current absolute position within the file.
			 * @throw std::system_error On failure to get position of the file. */
			[[nodiscard]] std::size_t tell() const
			{
				std::error_code err;
				if (auto res = tell(err); err)
					throw std::system_error(err, "rod::basic_file::tell");
				else
					return res;
			}
			/** @copybrief tell
			 * @param[out] err Reference to the error code set on failure to get position of the file.
			 * @return Current absolute position within the file. */
			[[nodiscard]] std::size_t tell(std::error_code &err) const noexcept { return _file.tell(err); }

			/** @brief Seeks to the specified offset within the file starting at the specified position.
			 * @param[in] off Offset into the file starting at \a dir.
			 * @param[in] dir Base direction to seek from.
			 * @return New absolute position within the file.
			 * @throw std::system_error On failure to seek the file. */
			std::size_t seek(std::ptrdiff_t off, seekdir dir)
			{
				std::error_code err;
				if (auto res = seek(off, dir, err); err)
					throw std::system_error(err, "rod::basic_file::seek");
				else
					return res;
			}
			/** @copybrief seek
			 * @param[in] off Offset into the file starting at \a dir.
			 * @param[in] dir Base direction to seek from.
			 * @param[out] err Reference to the error code set on failure to seek the file.
			 * @return New absolute position within the file. */
			std::size_t seek(std::ptrdiff_t off, seekdir dir, std::error_code &err) noexcept { return _file.seek(off, dir, err); }

			/** @brief Returns the size of the file.
			 * @return Current size of the file as reported by the filesystem.
			 * @throw std::system_error On failure to get size of the file. */
			[[nodiscard]] std::size_t size() const
			{
				std::error_code err;
				if (auto res = size(err); err)
					throw std::system_error(err, "rod::basic_file::size");
				else
					return res;
			}
			/** @copybrief tell
			 * @param[out] err Reference to the error code set on failure to get size of the file.
			 * @return Current size of the file as reported by the filesystem. */
			[[nodiscard]] std::size_t size(std::error_code &err) const noexcept { return _file.size(err); }

			/** @brief Re-sizes the file to the specified amount of bytes.
			 * @param[in] new_size New size of the file in bytes.
			 * @throw std::system_error On failure to seek the file. */
			void resize(std::size_t new_size) { if (const auto err = _file.resize(new_size); err) throw std::system_error(err, "rod::basic_file::resize"); }
			/** @copybrief seek
			 * @param[in] off Offset into the file starting at \a dir.
			 * @param[in] dir Base direction to seek from.
			 * @param[out] err Reference to the error code set on failure to seek the file.
			 * @return New absolute position within the file. */
			void resize(std::size_t new_size, std::error_code &err) noexcept { err = _file.resize(new_size); }

			/** Checks if the file is open. */
			[[nodiscard]] bool is_open() const noexcept { return _file.is_open(); }
			/** Returns the underlying native file handle. */
			[[nodiscard]] native_handle_type native_handle() const noexcept { return _file.native_handle(); }

			constexpr void swap(basic_file &other) noexcept { _file.swap(other._file); }
			friend constexpr void swap(basic_file &a, basic_file &b) noexcept { a.swap(b); }

		public:
			template<reference_to<basic_file> F, typename Buff>
			friend std::size_t tag_invoke(read_some_t, F &&f, Buff &&buff, std::error_code &err) noexcept(noexcept_sizeable_range<Buff>)
			{
				using value_t = std::ranges::range_value_t<Buff>;
				const auto dst_ptr = std::to_address(std::ranges::begin(buff));
				const auto dst_max = std::ranges::size(buff) * sizeof(value_t);
				return f._file.sync_read(static_cast<void *>(dst_ptr), dst_max, err);
			}
			template<reference_to<basic_file> F, typename Buff>
			friend std::size_t tag_invoke(write_some_t, F &&f, Buff &&buff, std::error_code &err) noexcept(noexcept_sizeable_range<Buff>)
			{
				using value_t = std::ranges::range_value_t<Buff>;
				const auto src_ptr = std::to_address(std::ranges::begin(buff));
				const auto src_max = std::ranges::size(buff) * sizeof(value_t);
				return f._file.sync_write(static_cast<const void *>(src_ptr), src_max, err);
			}
			template<reference_to<basic_file> F, std::convertible_to<std::size_t> Pos, typename Buff>
			friend std::size_t tag_invoke(read_some_at_t, F &&f, Pos pos, Buff &&buff, std::error_code &err) noexcept(noexcept_sizeable_range<Buff>)
			{
				using value_t = std::ranges::range_value_t<Buff>;
				const auto dst_ptr = std::to_address(std::ranges::begin(buff));
				const auto dst_max = std::ranges::size(buff) * sizeof(value_t);
				return f._file.sync_read_at(static_cast<void *>(dst_ptr), dst_max, static_cast<std::ptrdiff_t>(pos), err);
			}
			template<reference_to<basic_file> F, std::convertible_to<std::size_t> Pos, typename Buff>
			friend std::size_t tag_invoke(write_some_at_t, F &&f, Pos pos, Buff &&buff, std::error_code &err) noexcept(noexcept_sizeable_range<Buff>)
			{
				using value_t = std::ranges::range_value_t<Buff>;
				const auto src_ptr = std::to_address(std::ranges::begin(buff));
				const auto src_max = std::ranges::size(buff) * sizeof(value_t);
				return f._file.sync_write_at(static_cast<const void *>(src_ptr), src_max, static_cast<std::ptrdiff_t>(pos), err);
			}

			template<decays_to<async_read_some_t> T, reference_to<basic_file> F, typename Snd, typename Buff> requires detail::callable<T, Snd, native_handle_type, Buff>
			friend auto tag_invoke(T, Snd &&snd, F &&f, Buff &&buff) noexcept(detail::nothrow_callable<T, Snd, native_handle_type, Buff>) { return T{}(std::forward<Snd>(snd), f.native_handle(), std::forward<Buff>(buff)); }
			template<decays_to<async_write_some_t> T, reference_to<basic_file> F, typename Snd, typename Buff> requires detail::callable<T, Snd, native_handle_type, Buff>
			friend auto tag_invoke(T, Snd &&snd, F &&f, Buff &&buff) noexcept(detail::nothrow_callable<T, Snd, native_handle_type, Buff>) { return T{}(std::forward<Snd>(snd), f.native_handle(), std::forward<Buff>(buff)); }
			template<decays_to<async_read_some_at_t> T, reference_to<basic_file> F, typename Snd, std::convertible_to<std::size_t> Pos, typename Buff> requires detail::callable<T, Snd, native_handle_type, Pos, Buff>
			friend auto tag_invoke(T, Snd &&snd, F &&f, Pos pos, Buff &&buff) noexcept(detail::nothrow_callable<T, Snd, native_handle_type, Pos, Buff>) { return T{}(std::forward<Snd>(snd), f.native_handle(), pos, std::forward<Buff>(buff)); }
			template<decays_to<async_write_some_at_t> T, reference_to<basic_file> F, typename Snd, std::convertible_to<std::size_t> Pos, typename Buff> requires detail::callable<T, Snd, native_handle_type, Pos, Buff>
			friend auto tag_invoke(T, Snd &&snd, F &&f, Pos pos, Buff &&buff) noexcept(detail::nothrow_callable<T, Snd, native_handle_type, Pos, Buff>) { return T{}(std::forward<Snd>(snd), f.native_handle(), pos, std::forward<Buff>(buff)); }

			template<decays_to<schedule_read_some_t> T, reference_to<basic_file> F, typename Sch, typename Buff> requires detail::callable<T, Sch, native_handle_type, Buff>
			friend auto tag_invoke(T, Sch &&sch, F &&f, Buff &&buff) noexcept(detail::nothrow_callable<T, Sch, native_handle_type, Buff>) { return T{}(std::forward<Sch>(sch), f.native_handle(), std::forward<Buff>(buff)); }
			template<decays_to<schedule_write_some_t> T, reference_to<basic_file> F, typename Sch, typename Buff> requires detail::callable<T, Sch, native_handle_type, Buff>
			friend auto tag_invoke(T, Sch &&sch, F &&f, Buff &&buff) noexcept(detail::nothrow_callable<T, Sch, native_handle_type, Buff>) { return T{}(std::forward<Sch>(sch), f.native_handle(), std::forward<Buff>(buff)); }
			template<decays_to<schedule_read_some_at_t> T, reference_to<basic_file> F, typename Sch, std::convertible_to<std::size_t> Pos, typename Buff> requires detail::callable<T, Sch, native_handle_type, Pos, Buff>
			friend auto tag_invoke(T, Sch &&sch, F &&f, Pos pos, Buff &&buff) noexcept(detail::nothrow_callable<T, Sch, native_handle_type, Pos, Buff>) { return T{}(std::forward<Sch>(sch), f.native_handle(), pos, std::forward<Buff>(buff)); }
			template<decays_to<schedule_write_some_at_t> T, reference_to<basic_file> F, typename Sch, std::convertible_to<std::size_t> Pos, typename Buff> requires detail::callable<T, Sch, native_handle_type, Pos, Buff>
			friend auto tag_invoke(T, Sch &&sch, F &&f, Pos pos, Buff &&buff) noexcept(detail::nothrow_callable<T, Sch, native_handle_type, Pos, Buff>) { return T{}(std::forward<Sch>(sch), f.native_handle(), pos, std::forward<Buff>(buff)); }

			template<decays_to<schedule_read_some_t> T, reference_to<basic_file> F, typename Sch, typename Buff> requires(!detail::callable<T, Sch, native_handle_type, Buff> && detail::callable<async_read_some_t, schedule_result_t<Sch>, native_handle_type, Buff>)
			friend auto tag_invoke(T, Sch &&sch, F &&f, Buff &&buff) noexcept(detail::nothrow_callable<schedule_t, Sch> && detail::nothrow_callable<async_read_some_t, schedule_result_t<Sch>, native_handle_type, Buff>)
			{
				return async_read_some(schedule(sch), std::forward<Sch>(sch), f.native_handle(), std::forward<Buff>(buff));
			}
			template<decays_to<schedule_write_some_t> T, reference_to<basic_file> F, typename Sch, typename Buff> requires(!detail::callable<T, Sch, native_handle_type, Buff> && detail::callable<async_write_some_t, schedule_result_t<Sch>, native_handle_type, Buff>)
			friend auto tag_invoke(T, Sch &&sch, F &&f, Buff &&buff) noexcept(detail::nothrow_callable<schedule_t, Sch> && detail::nothrow_callable<async_write_some_t, schedule_result_t<Sch>, native_handle_type, Buff>)
			{
				return async_write_some(schedule(sch), std::forward<Sch>(sch), f.native_handle(), std::forward<Buff>(buff));
			}
			template<decays_to<schedule_read_some_at_t> T, reference_to<basic_file> F, typename Sch, typename Pos, typename Buff> requires(!detail::callable<T, Sch, native_handle_type, Pos, Buff> && detail::callable<async_read_some_at_t, schedule_result_t<Sch>, native_handle_type, Pos, Buff>)
			friend auto tag_invoke(T, Sch &&sch, F &&f, Pos pos, Buff &&buff) noexcept(detail::nothrow_callable<schedule_t, Sch> && detail::nothrow_callable<async_read_some_at_t, schedule_result_t<Sch>, native_handle_type, Pos, Buff>)
			{
				return async_read_some_at(schedule(sch), std::forward<Sch>(sch), f.native_handle(), pos, std::forward<Buff>(buff));
			}
			template<decays_to<schedule_write_some_at_t> T, reference_to<basic_file> F, typename Sch, typename Pos, typename Buff> requires(!detail::callable<T, Sch, native_handle_type, Pos, Buff> && detail::callable<async_write_some_at_t, schedule_result_t<Sch>, native_handle_type, Pos, Buff>)
			friend auto tag_invoke(T, Sch &&sch, F &&f, Pos pos, Buff &&buff) noexcept(detail::nothrow_callable<schedule_t, Sch> && detail::nothrow_callable<async_write_some_at_t, schedule_result_t<Sch>, native_handle_type, Pos, Buff>)
			{
				return async_write_some_at(schedule(sch), std::forward<Sch>(sch), f.native_handle(), pos, std::forward<Buff>(buff));
			}

		private:
			native_t _file = {};
		};

		template<decays_to<async_read_some_t> T, reference_to<basic_file> F, typename Snd, typename Buff> requires(!detail::callable<T, Snd, typename basic_file::native_handle_type, Buff>)
		inline auto tag_invoke(T, Snd &&snd, F &&f, Buff &&buff) noexcept(detail::nothrow_decay_copyable<Snd>::value && detail::nothrow_decay_copyable<Buff>::value)
		{
			using _sender_t = typename sync_sender<read_some_t, std::decay_t<Snd>, F, std::decay_t<Buff>>::type;
			return _sender_t{std::forward<Snd>(snd), std::forward_as_tuple(std::forward<F>(f), std::forward<Buff>(buff))};
		}
		template<decays_to<async_write_some_t> T, reference_to<basic_file> F, typename Snd, typename Buff> requires(!detail::callable<T, Snd, typename basic_file::native_handle_type, Buff>)
		inline auto tag_invoke(T, Snd &&snd, F &&f, Buff &&buff) noexcept(detail::nothrow_decay_copyable<Snd>::value && detail::nothrow_decay_copyable<Buff>::value)
		{
			using _sender_t = typename sync_sender<write_some_t, std::decay_t<Snd>, F, std::decay_t<Buff>>::type;
			return _sender_t{std::forward<Snd>(snd), std::forward_as_tuple(std::forward<F>(f), std::forward<Buff>(buff))};
		}
		template<decays_to<async_read_some_at_t> T, reference_to<basic_file> F, typename Snd, std::convertible_to<std::size_t> Pos, typename Buff> requires(!detail::callable<T, Snd, typename basic_file::native_handle_type, Pos, Buff>)
		inline auto tag_invoke(T, Snd &&snd, F &&f, Pos pos, Buff &&buff) noexcept(detail::nothrow_decay_copyable<Snd>::value && detail::nothrow_decay_copyable<Buff>::value)
		{
			using _sender_t = typename sync_sender<read_some_at_t, std::decay_t<Snd>, F, Pos, std::decay_t<Buff>>::type;
			return _sender_t{std::forward<Snd>(snd), std::forward_as_tuple(std::forward<F>(f), pos, std::forward<Buff>(buff))};
		}
		template<decays_to<async_write_some_at_t> T, reference_to<basic_file> F, typename Snd, std::convertible_to<std::size_t> Pos, typename Buff> requires(!detail::callable<T, Snd, typename basic_file::native_handle_type, Pos, Buff>)
		inline auto tag_invoke(T, Snd &&snd, F &&f, Pos pos, Buff &&buff) noexcept(detail::nothrow_decay_copyable<Snd>::value && detail::nothrow_decay_copyable<Buff>::value)
		{
			using _sender_t = typename sync_sender<write_some_at_t, std::decay_t<Snd>, F, Pos, std::decay_t<Buff>>::type;
			return _sender_t{std::forward<Snd>(snd), std::forward_as_tuple(std::forward<F>(f), pos, std::forward<Buff>(buff))};
		}
	}

	using _file::basic_file;
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

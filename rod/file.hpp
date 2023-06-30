/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#include "detail/adaptors/read_some.hpp"
#include "detail/adaptors/write_some.hpp"
#include "result.hpp"

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

namespace rod
{
	namespace _file
	{
		template<typename T>
		concept noexcept_sizeable_range = detail::nothrow_callable<decltype(std::ranges::begin), T> && detail::nothrow_callable<decltype(std::ranges::size), T>;

		/** Basic filesystem file handle (such as a POSIX file descriptor or Win32 HANDLE). */
		class basic_file
		{
			using native_t = detail::system_file;

		public:
			using native_handle_type = typename native_t::native_handle_type;

			using openprot = int;

			/** File is created with execute permissions for the current user. */
			static constexpr openprot user_exec = native_t::user_exec;
			/** File is created with read permissions for the current user. */
			static constexpr openprot user_read = native_t::user_read;
			/** File is created with write permissions for the current user. */
			static constexpr openprot user_write = native_t::user_write;
			/** File is created with execute permissions for the current group. */
			static constexpr openprot group_exec = native_t::group_exec;
			/** File is created with read permissions for the current group. */
			static constexpr openprot group_read = native_t::group_read;
			/** File is created with write permissions for the current group. */
			static constexpr openprot group_write = native_t::group_write;
			/** File is created with execute permissions for other users. */
			static constexpr openprot other_exec = native_t::other_exec;
			/** File is created with read permissions for the other users. */
			static constexpr openprot other_read = native_t::other_read;
			/** File is created with write permissions for the other users. */
			static constexpr openprot other_write = native_t::other_write;

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
			/** Opens the file specified by \a path using mode flags \a flags.
			 * @param[in] path Path to the file to be opened.
			 * @param[in] mode Mode flags to open the file with.
			 * @return `basic_file` handle to the opened file, or an error code on failure to open the file. */
			[[nodiscard]] static result<basic_file, std::error_code> open(const char *path, openmode mode) noexcept { return native_t::open(path, mode | binary); }
			/** @copydoc open */
			[[nodiscard]] static result<basic_file, std::error_code> open(const wchar_t *path, openmode mode) noexcept { return native_t::open(path, mode | binary); }
			/** @copydoc open */
			[[nodiscard]] static result<basic_file, std::error_code> open(const std::string &path, openmode mode) noexcept { return open(path.c_str(), mode); }
			/** @copydoc open */
			[[nodiscard]] static result<basic_file, std::error_code> open(const std::wstring &path, openmode mode) noexcept { return open(path.c_str(), mode); }
			/** @copydoc open */
			[[nodiscard]] static result<basic_file, std::error_code> open(const std::filesystem::path &path, openmode mode) noexcept { return open(path.c_str(), mode); }

			/** @copydoc open
			 * @param[in] prot Protection flags to use for the opened file.
			 * @note Protection flags are currently ignored under Win32. */
			[[nodiscard]] static result<basic_file, std::error_code> open(const char *path, openmode mode, openprot prot) noexcept { return native_t::open(path, mode | binary, prot); }
			/** @copydoc open */
			[[nodiscard]] static result<basic_file, std::error_code> open(const wchar_t *path, openmode mode, openprot prot) noexcept { return native_t::open(path, mode | binary, prot); }
			/** @copydoc open */
			[[nodiscard]] static result<basic_file, std::error_code> open(const std::string &path, openmode mode, openprot prot) noexcept { return open(path.c_str(), mode, prot); }
			/** @copydoc open */
			[[nodiscard]] static result<basic_file, std::error_code> open(const std::wstring &path, openmode mode, openprot prot) noexcept { return open(path.c_str(), mode, prot); }
			/** @copydoc open */
			[[nodiscard]] static result<basic_file, std::error_code> open(const std::filesystem::path &path, openmode mode, openprot prot) noexcept { return open(path.c_str(), mode, prot); }

			/** Re-opens a file from an existing file handle and mode flags.
			 * @param[in] file Native Handle to an existing file to be re-opened.
			 * @param[in] mode Mode flags to use when re-opening the file.
			 * @return `basic_file` handle to the opened file, or an error code on failure to reopen the file. */
			[[nodiscard]] static result<basic_file, std::error_code> reopen(const basic_file &file, openmode mode) noexcept { return reopen(file.native_handle(), mode); }
			/** Re-opens a file pointed to by a native file handle and mode flags.
			 * @param[in] file Native handle to an existing file to be re-opened.
			 * @param[in] mode Mode flags to use when re-opening the file.
			 * @return `basic_file` handle to the opened file, or an error code on failure to reopen the file. */
			[[nodiscard]] static result<basic_file, std::error_code> reopen(native_handle_type file, openmode mode) noexcept { return native_t::reopen(file, mode | binary); }

		public:
			constexpr basic_file() noexcept = default;
			constexpr basic_file(basic_file &&other) noexcept : _file(std::move(other._file)) {}
			constexpr basic_file &operator=(basic_file &&other) noexcept { return (_file = std::move(other._file), *this); }

			/** Initializes the file from a native file handle. */
			constexpr explicit basic_file(native_handle_type file) noexcept : _file(file) {}
			constexpr explicit basic_file(native_t &&file) noexcept : _file(std::move(file)) {}

			/** Releases the underlying native file handle without closing. */
			native_handle_type release() noexcept { return _file.release(); }
			/** Releases the underlying native file handle without closing and replaces it with the specified handle. */
			native_handle_type release(native_handle_type new_file) noexcept { return _file.release(new_file); }

			/** Synchronizes file with the underlying device.
			 * @return Error code on failure to synchronize the file. */
			std::error_code sync() noexcept { return _file.sync(); }
			/** Flushes file buffers.
			 * @return Error code on failure to flush the file. */
			std::error_code flush() noexcept { return _file.flush(); }
			/** Closes the file handle.
			 * @return Error code on failure to close the file. */
			std::error_code close() noexcept { return _file.close(); }

			/** Returns the size of the file.
			 * @return Current size of the file as reported by the filesystem or an error code on failure to get size of the file. */
			[[nodiscard]] result<std::size_t, std::error_code> size() const noexcept { return _file.size(); }
			/** Returns the current position within the file. Equivalent to `seek(0, cur)`.
			 * @return Current absolute position within the file or an error code on failure to get position of the file. */
			[[nodiscard]] result<std::size_t, std::error_code> tell() const noexcept { return _file.tell(); }
			/** Returns the path of the file (as if via POSIX `readlink`).
			 * @return Path to the file as reported by the filesystem or an error code on failure to get path of the file. */
			[[nodiscard]] result<std::filesystem::path, std::error_code> path() const { return _file.path(); }

			/** Resizes the file to the specified amount of bytes.
			 * @param[in] new_size New size of the file in bytes.
			 * @return New size of the file as reported by the filesystem or an error code on failure to resize the file. */
			result<std::size_t, std::error_code> resize(std::size_t new_size) noexcept { return _file.resize(new_size); }
			/** Seeks to the specified offset within the file starting at the specified position.
			 * @param[in] off Offset into the file starting at \a dir.
			 * @param[in] dir Base direction to seek from.
			 * @return New absolute position within the file or an error code on failure to seek the file. */
			result<std::size_t, std::error_code> seek(std::ptrdiff_t off, seekdir dir) noexcept { return _file.seek(off, dir); }

			/** Checks if the file is open. */
			[[nodiscard]] bool is_open() const noexcept { return _file.is_open(); }
			/** Returns the underlying native file handle. */
			[[nodiscard]] native_handle_type native_handle() const noexcept { return _file.native_handle(); }

			constexpr void swap(basic_file &other) noexcept { _file.swap(other._file); }
			friend constexpr void swap(basic_file &a, basic_file &b) noexcept { a.swap(b); }

		public:
			template<reference_to<basic_file> F, typename Buff>
			friend result<std::size_t, std::error_code> tag_invoke(read_some_t, F &&f, Buff &&buff) noexcept(noexcept_sizeable_range<Buff>)
			{
				using value_t = std::ranges::range_value_t<Buff>;
				const auto dst_ptr = std::to_address(std::ranges::begin(buff));
				const auto dst_max = std::ranges::size(buff) * sizeof(value_t);
				return f._file.sync_read(static_cast<void *>(dst_ptr), dst_max);
			}
			template<reference_to<basic_file> F, typename Buff>
			friend result<std::size_t, std::error_code> tag_invoke(write_some_t, F &&f, Buff &&buff) noexcept(noexcept_sizeable_range<Buff>)
			{
				using value_t = std::ranges::range_value_t<Buff>;
				const auto src_ptr = std::to_address(std::ranges::begin(buff));
				const auto src_max = std::ranges::size(buff) * sizeof(value_t);
				return f._file.sync_write(static_cast<const void *>(src_ptr), src_max);
			}
			template<reference_to<basic_file> F, std::convertible_to<std::size_t> Pos, typename Buff>
			friend result<std::size_t, std::error_code> tag_invoke(read_some_at_t, F &&f, Pos pos, Buff &&buff) noexcept(noexcept_sizeable_range<Buff>)
			{
				using value_t = std::ranges::range_value_t<Buff>;
				const auto dst_ptr = std::to_address(std::ranges::begin(buff));
				const auto dst_max = std::ranges::size(buff) * sizeof(value_t);
				return f._file.sync_read_at(static_cast<void *>(dst_ptr), dst_max, static_cast<std::ptrdiff_t>(pos));
			}
			template<reference_to<basic_file> F, std::convertible_to<std::size_t> Pos, typename Buff>
			friend result<std::size_t, std::error_code> tag_invoke(write_some_at_t, F &&f, Pos pos, Buff &&buff) noexcept(noexcept_sizeable_range<Buff>)
			{
				using value_t = std::ranges::range_value_t<Buff>;
				const auto src_ptr = std::to_address(std::ranges::begin(buff));
				const auto src_max = std::ranges::size(buff) * sizeof(value_t);
				return f._file.sync_write_at(static_cast<const void *>(src_ptr), src_max, static_cast<std::ptrdiff_t>(pos));
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
	}

	using _file::basic_file;
}

/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#include "detail/adaptors/read_some.hpp"
#include "detail/adaptors/write_some.hpp"

#include <filesystem>
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

		/** Unbuffered file handle (such as a regular posix file descriptor). */
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
			/** Opens the file in exclusive mode. Opening the file will fail if it already exists. */
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
					[[unlikely]] throw std::system_error(err, "rod::basic_file::open");
				else
					return res;
			}
			/** @copydoc open */
			[[nodiscard]] static basic_file open(const wchar_t *path, openmode mode)
			{
				std::error_code err;
				if (auto res = open(path, mode, err); err)
					[[unlikely]] throw std::system_error(err, "rod::basic_file::open");
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
			 * @throw std::system_error On failure to open the file.  */
			[[nodiscard]] static basic_file open(const char *path, openmode mode, fileprot prot)
			{
				std::error_code err;
				if (auto res = open(path, mode, prot, err); err)
					[[unlikely]] throw std::system_error(err, "rod::basic_file::open");
				else
					return res;
			}
			/** @copydoc open */
			[[nodiscard]] static basic_file open(const wchar_t *path, openmode mode, fileprot prot)
			{
				std::error_code err;
				if (auto res = open(path, mode, prot, err); err)
					[[unlikely]] throw std::system_error(err, "rod::basic_file::open");
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
			 * @return `basic_file` handle to the opened file, or a closed file handle if an error has occurred. */
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
					[[unlikely]] throw std::system_error(err, "rod::basic_file::reopen");
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
			constexpr basic_file(native_t &&file) noexcept : m_file(std::move(file)) {}

		public:
			constexpr basic_file() noexcept = default;
			constexpr basic_file(basic_file &&other) noexcept : m_file(std::move(other.m_file)) {}
			constexpr basic_file &operator=(basic_file &&other) noexcept { return (m_file = std::move(other.m_file), *this); }

			/** Initializes the file from a native file handle. */
			constexpr explicit basic_file(native_handle_type file) noexcept : m_file(file) {}

			/** @brief Closes the file handle.
			 * @param[out] err Reference to the error code set on failure to close the file. */
			void close(std::error_code &err) noexcept { err = m_file.close(); }
			/** @copybrief close
			 * @throw std::system_error On failure to close the file. */
			void close() { if (auto err = m_file.close(); err) [[unlikely]] throw std::system_error(err, "rod::basic_file::close"); }

			/** Releases the underlying native file handle without closing. */
			[[nodiscard]] constexpr native_handle_type release() noexcept { return m_file.release(); }

			/** @brief Flushes modified file data to the underlying device.
			 * @param[out] err Reference to the error code set on failure to flush the file. */
			void flush(std::error_code &err) noexcept { err = m_file.flush(); }
			/** @copybrief flush
			 * @throw std::system_error On failure to flush the file. */
			void flush() { if (auto err = m_file.flush(); err) [[unlikely]] throw std::system_error(err, "rod::basic_file::flush"); }

			/** @brief Seeks to the specified offset within the file starting at the specified position.
			 * @param[in] off Offset into the file starting at \a dir.
			 * @param[in] dir Base direction to seek from.
			 * @return New absolute position within the file.
			 * @throw std::system_error On failure to seek the file. */
			std::size_t seek(std::ptrdiff_t off, seekdir dir)
			{
				std::error_code err;
				if (auto res = seek(off, dir, err); err)
					[[unlikely]] throw std::system_error(err, "rod::basic_file::seek");
				else
					return res;
			}
			/** @copybrief seek
			 * @param[in] off Offset into the file starting at \a dir.
			 * @param[in] dir Base direction to seek from.
			 * @param[out] err Reference to the error code set on failure to seek the file.
			 * @return New absolute position within the file. */
			std::size_t seek(std::ptrdiff_t off, seekdir dir, std::error_code &err) noexcept { return m_file.seek(off, dir, err); }

			/** @brief Returns the current position within the file.
			 * @return Current absolute position within the file.
			 * @throw std::system_error On failure to get position of the file. */
			[[nodiscard]] std::size_t tell() const
			{
				std::error_code err;
				if (auto res = tell(err); err)
					[[unlikely]] throw std::system_error(err, "rod::basic_file::tell");
				else
					return res;
			}
			/** @copybrief tell
			 * @param[out] err Reference to the error code set on failure to get position of the file.
			 * @return Current absolute position within the file. */
			[[nodiscard]] std::size_t tell(std::error_code &err) const noexcept { return m_file.tell(err); }

			/** Checks if the file is open. */
			[[nodiscard]] constexpr bool is_open() const noexcept { return m_file.is_open(); }
			/** Returns the underlying native file handle. */
			[[nodiscard]] constexpr native_handle_type native_handle() const noexcept { return m_file.native_handle(); }

			constexpr void swap(basic_file &other) noexcept { m_file.swap(other.m_file); }
			friend constexpr void swap(basic_file &a, basic_file &b) noexcept { a.swap(b); }

		public:
			template<reference_to<basic_file> F, typename Buff>
			friend std::size_t tag_invoke(read_some_t, F &&f, Buff &&buff, std::error_code &err) noexcept(noexcept_sizeable_range<Buff>)
			{
				using value_t = std::ranges::range_value_t<Buff>;
				const auto dst_ptr = std::to_address(std::ranges::begin(buff));
				const auto dst_max = std::ranges::size(buff) * sizeof(value_t);
				return f.m_file.sync_read(static_cast<void *>(dst_ptr), dst_max, err);
			}
			template<reference_to<basic_file> F, typename Buff>
			friend std::size_t tag_invoke(write_some_t, F &&f, Buff &&buff, std::error_code &err) noexcept(noexcept_sizeable_range<Buff>)
			{
				using value_t = std::ranges::range_value_t<Buff>;
				const auto src_ptr = std::to_address(std::ranges::begin(buff));
				const auto src_max = std::ranges::size(buff) * sizeof(value_t);
				return f.m_file.sync_write(static_cast<const void *>(src_ptr), src_max, err);
			}
			template<reference_to<basic_file> F, std::convertible_to<std::ptrdiff_t> Pos, typename Buff>
			friend std::size_t tag_invoke(read_some_at_t, F &&f, Pos pos, Buff &&buff, std::error_code &err) noexcept(noexcept_sizeable_range<Buff>)
			{
				using value_t = std::ranges::range_value_t<Buff>;
				const auto dst_ptr = std::to_address(std::ranges::begin(buff));
				const auto dst_max = std::ranges::size(buff) * sizeof(value_t);
				return f.m_file.sync_read_at(static_cast<void *>(dst_ptr), dst_max, static_cast<std::ptrdiff_t>(pos), err);
			}
			template<reference_to<basic_file> F, std::convertible_to<std::ptrdiff_t> Pos, typename Buff>
			friend std::size_t tag_invoke(write_some_at_t, F &&f, Pos pos, Buff &&buff, std::error_code &err) noexcept(noexcept_sizeable_range<Buff>)
			{
				using value_t = std::ranges::range_value_t<Buff>;
				const auto src_ptr = std::to_address(std::ranges::begin(buff));
				const auto src_max = std::ranges::size(buff) * sizeof(value_t);
				return f.m_file.sync_write_at(static_cast<const void *>(src_ptr), src_max, static_cast<std::ptrdiff_t>(pos), err);
			}

			template<decays_to<async_read_some_t> T, reference_to<basic_file> F, typename Snd, typename Buff> requires detail::callable<T, Snd, native_handle_type, Buff>
			friend auto tag_invoke(T, Snd &&snd, F &&f, Buff &&buff) noexcept(detail::nothrow_callable<T, Snd, native_handle_type, Buff>) { return T{}(std::forward<Snd>(snd), f.native_handle(), std::forward<Buff>(buff)); }
			template<decays_to<async_write_some_t> T, reference_to<basic_file> F, typename Snd, typename Buff> requires detail::callable<T, Snd, native_handle_type, Buff>
			friend auto tag_invoke(T, Snd &&snd, F &&f, Buff &&buff) noexcept(detail::nothrow_callable<T, Snd, native_handle_type, Buff>) { return T{}(std::forward<Snd>(snd), f.native_handle(), std::forward<Buff>(buff)); }
			template<decays_to<async_read_some_at_t> T, reference_to<basic_file> F, typename Snd, std::convertible_to<std::ptrdiff_t> Pos, typename Buff> requires detail::callable<T, Snd, native_handle_type, Pos, Buff>
			friend auto tag_invoke(T, Snd &&snd, F &&f, Pos pos, Buff &&buff) noexcept(detail::nothrow_callable<T, Snd, native_handle_type, Pos, Buff>) { return T{}(std::forward<Snd>(snd), f.native_handle(), pos, std::forward<Buff>(buff)); }
			template<decays_to<async_write_some_at_t> T, reference_to<basic_file> F, typename Snd, std::convertible_to<std::ptrdiff_t> Pos, typename Buff> requires detail::callable<T, Snd, native_handle_type, Pos, Buff>
			friend auto tag_invoke(T, Snd &&snd, F &&f, Pos pos, Buff &&buff) noexcept(detail::nothrow_callable<T, Snd, native_handle_type, Pos, Buff>) { return T{}(std::forward<Snd>(snd), f.native_handle(), pos, std::forward<Buff>(buff)); }

			template<decays_to<schedule_read_some_t> T, reference_to<basic_file> F, typename Sch, typename Buff> requires detail::callable<schedule_read_some_t, Sch, native_handle_type, Buff>
			friend auto tag_invoke(T, Sch &&sch, F &&f, Buff &&buff) noexcept(detail::nothrow_callable<schedule_read_some_t, Sch, native_handle_type, Buff>) { return T{}(std::forward<Sch>(sch), f.native_handle(), std::forward<Buff>(buff)); }
			template<decays_to<schedule_write_some_t> T, reference_to<basic_file> F, typename Sch, typename Buff> requires detail::callable<schedule_write_some_t, Sch, native_handle_type, Buff>
			friend auto tag_invoke(T, Sch &&sch, F &&f, Buff &&buff) noexcept(detail::nothrow_callable<schedule_write_some_t, Sch, native_handle_type, Buff>) { return T{}(std::forward<Sch>(sch), f.native_handle(), std::forward<Buff>(buff)); }
			template<decays_to<schedule_read_some_at_t> T, reference_to<basic_file> F, typename Sch, std::convertible_to<std::ptrdiff_t> Pos, typename Buff> requires detail::callable<schedule_read_some_at_t, Sch, native_handle_type, Pos, Buff>
			friend auto tag_invoke(T, Sch &&sch, F &&f, Pos pos, Buff &&buff) noexcept(detail::nothrow_callable<schedule_read_some_at_t, Sch, native_handle_type, Pos, Buff>) { return T{}(std::forward<Sch>(sch), f.native_handle(), pos, std::forward<Buff>(buff)); }
			template<decays_to<schedule_write_some_at_t> T, reference_to<basic_file> F, typename Sch, std::convertible_to<std::ptrdiff_t> Pos, typename Buff> requires detail::callable<schedule_write_some_at_t, Sch, native_handle_type, Pos, Buff>
			friend auto tag_invoke(T, Sch &&sch, F &&f, Pos pos, Buff &&buff) noexcept(detail::nothrow_callable<schedule_write_some_at_t, Sch, native_handle_type, Pos, Buff>) { return T{}(std::forward<Sch>(sch), f.native_handle(), pos, std::forward<Buff>(buff)); }

		private:
			native_t m_file = {};
		};
		/** Buffered file handle. */
		class file;
	}

	using _file::basic_file;
//	using _file::file;
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

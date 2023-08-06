/*
 * Created by switch_blade on 2023-07-08.
 */

#pragma once

#include "file_base.hpp"

namespace rod::_file
{
	template<typename Hnd>
	class basic_file : public file_base, Hnd
	{
	public:
		using native_handle_type = typename Hnd::native_handle_type;
		using openmode = typename file_base::openmode;
		using openprot = typename file_base::openprot;
		using seekdir = typename file_base::seekdir;

	public:
		basic_file(const basic_file &) = delete;
		basic_file &operator=(const basic_file &) = delete;

		constexpr basic_file() noexcept(std::is_nothrow_default_constructible_v<Hnd>) = default;
		constexpr basic_file(basic_file &&) noexcept(std::is_nothrow_move_assignable_v<Hnd>) = default;
		constexpr basic_file &operator=(basic_file &&) noexcept(std::is_nothrow_move_assignable_v<Hnd>) = default;

		template<typename... Args> requires std::constructible_from<Hnd, Args...>
		constexpr explicit basic_file(Args &&...args) noexcept(std::is_nothrow_constructible_v<Hnd, Args...>) : Hnd(std::forward<Args>(args)...) {}

		/** Initializes the file handle for path \a path using specified mode & protection flags.
		 * @param[in] path Path to the file to be opened.
		 * @param[in] mode Mode flags to use when opening the file.
		 * @param[in] prot Optional protection flags to use for the opened file.
		 * @throw std::system_error On failure to open the file.
		 * @note Protection flags may be ignored by certain implementations and filesystems. */
		basic_file(const char *path, openmode mode, openprot prot = default_openprot) noexcept requires std::constructible_from<Hnd> { assert_error_code(Hnd::open(path, mode, prot), "basic_file::open"); }
		/** @copydoc basic_file */
		basic_file(const wchar_t *path, openmode mode, openprot prot = default_openprot) noexcept requires std::constructible_from<Hnd> { assert_error_code(Hnd::open(path, mode, prot), "basic_file::open"); }
		/** @copydoc basic_file */
		basic_file(const std::string &path, openmode mode, openprot prot = default_openprot) noexcept requires std::constructible_from<Hnd> : basic_file(path.c_str(), mode, prot) {}
		/** @copydoc basic_file */
		basic_file(const std::wstring &path, openmode mode, openprot prot = default_openprot) noexcept requires std::constructible_from<Hnd> : basic_file(path.c_str(), mode, prot) {}
		/** @copydoc basic_file */
		basic_file(const std::filesystem::path &path, openmode mode, openprot prot = default_openprot) noexcept requires std::constructible_from<Hnd> : basic_file(path.c_str(), mode, prot) {}

		/** Checks if the file handle is open. */
		[[nodiscard]] bool is_open() const noexcept { return Hnd::is_open(); }
		/** Checks if the file handle is open in blocking mode. */
		[[nodiscard]] bool is_blocking() const noexcept { return Hnd::is_blocking(); }

		/** Opens the file handle for path \a path using specified mode & protection flags.
		 * @param[in] path Path to the file to be opened.
		 * @param[in] mode Mode flags to use when opening the file.
		 * @param[in] prot Optional protection flags to use for the opened file.
		 * @return Error code on failure to open the file.
		 * @note Protection flags may be ignored by certain implementations and filesystems. */
		std::error_code open(const char *path, openmode mode, openprot prot = default_openprot) noexcept { return Hnd::open(path, mode, prot); }
		/** @copydoc open */
		std::error_code open(const wchar_t *path, openmode mode, openprot prot = default_openprot) noexcept { return Hnd::open(path, mode, prot); }
		/** @copydoc open */
		std::error_code open(const std::string &path, openmode mode, openprot prot = default_openprot) noexcept { return open(path.c_str(), mode, prot); }
		/** @copydoc open */
		std::error_code open(const std::wstring &path, openmode mode, openprot prot = default_openprot) noexcept { return open(path.c_str(), mode, prot); }
		/** @copydoc open */
		std::error_code open(const std::filesystem::path &path, openmode mode, openprot prot = default_openprot) noexcept { return open(path.c_str(), mode, prot); }

		/** Synchronizes file with the underlying device.
		 * @return Error code on failure to synchronize the file. */
		std::error_code sync() noexcept { return Hnd::sync(); }
		/** Flushes file buffers.
		 * @return Error code on failure to flush the file. */
		std::error_code flush() noexcept { return Hnd::flush(); }
		/** Closes the file handle.
		 * @return Error code on failure to close the file. */
		std::error_code close() noexcept { return Hnd::close(); }

		/** Returns the size of the file.
		 * @return Current size of the file as reported by the filesystem or an error code on failure to get size of the file. */
		[[nodiscard]] result<std::size_t, std::error_code> size() const noexcept { return Hnd::size(); }
		/** Returns the current position within the file. Equivalent to `seek(0, cur)`.
		 * @return Current absolute position within the file or an error code on failure to get position of the file. */
		[[nodiscard]] result<std::size_t, std::error_code> tell() const noexcept { return Hnd::tell(); }
		/** Returns the path of the file (as if via POSIX `readlink`).
		 * @return Path to the file as reported by the filesystem or an error code on failure to get path of the file. */
		[[nodiscard]] result<std::filesystem::path, std::error_code> path() const { return Hnd::path(); }

		/** Resizes the file to the specified amount of bytes.
		 * @param[in] new_size New size of the file in bytes.
		 * @return New size of the file as reported by the filesystem or an error code on failure to resize the file. */
		result<std::size_t, std::error_code> resize(std::size_t new_size) noexcept { return Hnd::resize(new_size); }
		/** Seeks to the specified offset within the file starting at the specified position.
		 * @param[in] off Offset into the file starting at \a dir.
		 * @param[in] dir Base direction to seek from.
		 * @return New absolute position within the file or an error code on failure to seek the file. */
		result<std::size_t, std::error_code> seek(std::ptrdiff_t off, seekdir dir) noexcept { return Hnd::seek(off, dir); }
		/** Sets absolute file offset. Equivalent to `seek(off, beg)`.
		 * @param[in] off New offset from the start of the file.
		 * @return New absolute position within the file or an error code on failure to seek the file. */
		result<std::size_t, std::error_code> setpos(std::size_t off) noexcept { return Hnd::setpos(off); }

		/** Releases the underlying native file handle without closing. */
		native_handle_type release() noexcept { return Hnd::release(); }
		/** Releases the underlying native file handle without closing and replaces it with the specified handle. */
		native_handle_type release(native_handle_type new_hnd) noexcept { return Hnd::release(new_hnd); }
		/** Returns the underlying native file handle. */
		[[nodiscard]] native_handle_type native_handle() const noexcept { return Hnd::native_handle(); }

		constexpr void swap(basic_file &other) noexcept(std::is_nothrow_swappable_v<Hnd>) { std::swap(static_cast<Hnd &>(*this), static_cast<Hnd &>(other)); }
		friend constexpr void swap(basic_file &a, basic_file &b) noexcept(std::is_nothrow_swappable_v<Hnd>) { a.swap(b); }

	public:
		template<derived_reference<basic_file> F, byte_buffer Buff> requires _detail::callable<write_some_t, copy_cvref_t<F, Hnd>, Buff>
		friend auto tag_invoke(read_some_t, F &&f, Buff &&buff) noexcept(_detail::nothrow_callable<read_some_t, copy_cvref_t<F, Hnd>, Buff>)
		{
			return read_some(std::forward<copy_cvref_t<F, Hnd>>(f), std::forward<Buff>(buff));
		}
		template<derived_reference<basic_file> F, byte_buffer Buff> requires _detail::callable<write_some_t, copy_cvref_t<F, Hnd>, Buff>
		friend auto tag_invoke(write_some_t, F &&f, Buff &&buff) noexcept(_detail::nothrow_callable<write_some_t, copy_cvref_t<F, Hnd>, Buff>)
		{
			return write_some(std::forward<copy_cvref_t<F, Hnd>>(f), std::forward<Buff>(buff));
		}
		template<derived_reference<basic_file> F, std::convertible_to<std::size_t> Pos, byte_buffer Buff> requires _detail::callable<read_some_at_t, copy_cvref_t<F, Hnd>, Pos, Buff>
		friend auto tag_invoke(read_some_at_t, F &&f, Pos pos, Buff &&buff) noexcept(_detail::nothrow_callable<read_some_at_t, copy_cvref_t<F, Hnd>, Pos, Buff>)
		{
			return read_some_at(std::forward<copy_cvref_t<F, Hnd>>(f), pos, std::forward<Buff>(buff));
		}
		template<derived_reference<basic_file> F, std::convertible_to<std::size_t> Pos, byte_buffer Buff> requires _detail::callable<write_some_at_t, copy_cvref_t<F, Hnd>, Pos, Buff>
		friend auto tag_invoke(write_some_at_t, F &&f, Pos pos, Buff &&buff) noexcept(_detail::nothrow_callable<write_some_at_t, copy_cvref_t<F, Hnd>, Pos, Buff>)
		{
			return write_some_at(std::forward<copy_cvref_t<F, Hnd>>(f), pos, std::forward<Buff>(buff));
		}

		template<derived_reference<basic_file> F, byte_buffer Buff> requires _detail::callable<async_read_some_t, copy_cvref_t<F, Hnd>, Buff>
		friend auto tag_invoke(async_read_some_t, F &&f, Buff &&buff) noexcept(_detail::nothrow_callable<async_read_some_t, copy_cvref_t<F, Hnd>, Buff>)
		{
			return async_read_some(static_cast<copy_cvref_t<F, Hnd>>(f), std::forward<Buff>(buff));
		}
		template<derived_reference<basic_file> F, byte_buffer Buff> requires _detail::callable<async_write_some_t, copy_cvref_t<F, Hnd>, Buff>
		friend auto tag_invoke(async_write_some_t, F &&f, Buff &&buff) noexcept(_detail::nothrow_callable<async_write_some_t, copy_cvref_t<F, Hnd>, Buff>)
		{
			return async_write_some(static_cast<copy_cvref_t<F, Hnd>>(f), std::forward<Buff>(buff));
		}
		template<derived_reference<basic_file> F, std::convertible_to<std::size_t> Pos, byte_buffer Buff> requires _detail::callable<async_read_some_at_t, copy_cvref_t<F, Hnd>, Pos, Buff>
		friend auto tag_invoke(async_read_some_at_t, F &&f, Pos pos, Buff &&buff) noexcept(_detail::nothrow_callable<async_read_some_at_t, copy_cvref_t<F, Hnd>, Pos, Buff>)
		{
			return async_read_some_at(static_cast<copy_cvref_t<F, Hnd>>(f), pos, std::forward<Buff>(buff));
		}
		template<derived_reference<basic_file> F, std::convertible_to<std::size_t> Pos, byte_buffer Buff> requires _detail::callable<async_write_some_at_t, copy_cvref_t<F, Hnd>, Pos, Buff>
		friend auto tag_invoke(async_write_some_at_t, F &&f, Pos pos, Buff &&buff) noexcept(_detail::nothrow_callable<async_write_some_at_t, copy_cvref_t<F, Hnd>, Pos, Buff>)
		{
			return async_write_some_at(static_cast<copy_cvref_t<F, Hnd>>(f), pos, std::forward<Buff>(buff));
		}
	};
}
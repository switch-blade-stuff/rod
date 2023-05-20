/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#include "detail/file_fwd.hpp"

/* Platform-specific implementations. */
#include "detail/unix/file.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::io
{
	static_assert(requires { typename detail::native_file; }, "Native file interface must be implemented for the current platform");

	class basic_file
	{
		using native_t = detail::native_file;

	public:
		using native_handle_type = typename native_t::native_handle_type;
		using openmode = int;
		using seekdir = int;

		/** Opens the file for input. */
		static constexpr openmode in = detail::openmode::in;
		/** Opens the file for output. */
		static constexpr openmode out = detail::openmode::out;

		/** Seeks the file to the end after it is opened. */
		static constexpr openmode ate = detail::openmode::ate;
		/** Opens the file in append mode. Write operations will append to the end of the file. */
		static constexpr openmode app = detail::openmode::app;
		/** Opens the file in truncate mode. Write operations will overwrite existing contents. */
		static constexpr openmode trunc = detail::openmode::trunc;

		/** Opens the file in binary mode.  */
		static constexpr openmode binary = detail::openmode::binary;
		/** Opens the file in direct mode. */
		static constexpr openmode direct = detail::openmode::direct;
		/** Opens the file in exclusive mode. Opening the file will fail if it already exists. */
		static constexpr openmode noreplace = detail::openmode::noreplace;

		/** Seek from the start of the file. */
		static constexpr seekdir beg = detail::seekdir::beg;
		/** Seek from the current position within the file. */
		static constexpr seekdir cur = detail::seekdir::cur;
		/** Seek from the end of the file. */
		static constexpr seekdir end = detail::seekdir::end;

		/** @brief Opens the file specified by \a path using mode flags \a flags.
		 * @note `binary` mode flag is implied.
		 * @param[in] path Path to the file to be opened.
		 * @param[in] mode Mode flags to open the file with.
		 * @return `basic_file` handle to the opened file.
	 	 * @throw std::system_error On failure to open the file.  */
		[[nodiscard]] static basic_file open(const char *path, openmode mode)
		{
			std::error_code err;
			if (auto res = open(path, mode, err); err)
				[[unlikely]] throw std::system_error(err);
			else
				return res;
		}
		/** @copydoc open */
		[[nodiscard]] static basic_file open(const std::string &path, openmode mode) { return open(path.c_str(), mode); }
		/** @copydoc open */
		[[nodiscard]] static basic_file open(const std::filesystem::path &path, openmode mode) { return open(path.c_str(), mode); }

		/** @copybrief open
		 * @note `binary` mode flag is implied.
		 * @param[in] path Path to the file to be opened.
		 * @param[in] mode Mode flags to open the file with.
		 * @param[out] err Reference to the error code set on failure to open the file.
		 * @return `basic_file` handle to the opened file, or a closed file handle if an error has occurred. */
		[[nodiscard]] static basic_file open(const char *path, openmode mode, std::error_code &err) noexcept
		{
			mode |= binary; /* binary mode is inferred. */
			return {native_t::open(path, mode, err), mode};
		}
		/** @copydoc open */
		[[nodiscard]] static basic_file open(const std::string &path, openmode mode, std::error_code &err) noexcept { return open(path.c_str(), mode, err); }
		/** @copydoc open */
		[[nodiscard]] static basic_file open(const std::filesystem::path &path, openmode mode, std::error_code &err) noexcept { return open(path.c_str(), mode, err); }

		/** @brief Re-opens a file pointed to by a native file handle and mode flags.
		 * @note `binary` mode flag is implied and `noreplace` flag is ignored.
		 * @param[in] file Native handle to an existing file to be re-opened.
		 * @param[in] mode Mode flags to use when re-opening the file.
		 * @return `basic_file` handle to the re-opened file.
	 	 * @throw std::system_error On failure to reopen the file. */
		[[nodiscard]] static basic_file reopen(native_handle_type file, openmode mode)
		{
			std::error_code err;
			if (auto res = reopen(file, mode, err); err)
				[[unlikely]] throw std::system_error(err);
			else
				return res;
		}
		/** @copybrief reopen
		 * @note `binary` mode flag is implied and `noreplace` flag is ignored.
		 * @param[in] file Native handle to an existing file to be re-opened.
		 * @param[in] mode Mode flags to use when re-opening the file.
		 * @param[out] err Reference to the error code set on failure to re-open the file.
		 * @return `basic_file` handle to the re-opened file. */
		[[nodiscard]] static basic_file reopen(native_handle_type file, openmode mode, std::error_code &err) noexcept
		{
			mode |= binary; /* binary mode is inferred. */
			mode ^= mode & noreplace;
			return {native_t::reopen(file, mode, err), mode};
		}

		/** @brief Re-opens a file from an existing file handle.
		 * @note `binary` mode flag is implied and `noreplace` flag is ignored.
		 * @param[in] file Native Handle to an existing file to be re-opened.
		 * @return `basic_file` handle to the re-opened file.
	 	 * @throw std::system_error On failure to reopen the file. */
		[[nodiscard]] static basic_file reopen(const basic_file &file) { return reopen(file.native_handle(), file.mode()); }
		/** @copybrief reopen
		 * @note `binary` mode is implied and`noreplace` is ignored.
		 * @param[in] file Native Handle to an existing file to be re-opened.
		 * @param[out] err Reference to the error code set on failure to re-open the file.
		 * @return `basic_file` handle to the re-opened file. */
		[[nodiscard]] static basic_file reopen(const basic_file &file, std::error_code &err) noexcept { return reopen(file.native_handle(), file.mode(), err); }

		/** @brief Re-opens a file from an existing file handle and mode flags.
		 * @note `binary` mode flag is implied and `noreplace` flag is ignored.
		 * @param[in] file Native Handle to an existing file to be re-opened.
		 * @param[in] mode Mode flags to use when re-opening the file.
		 * @return `basic_file` handle to the re-opened file.
	 	 * @throw std::system_error On failure to reopen the file. */
		[[nodiscard]] static basic_file reopen(const basic_file &file, openmode mode) { return reopen(file.native_handle(), mode); }
		/** @copybrief reopen
		 * @note `binary` mode flag is implied and `noreplace` flag is ignored.
		 * @param[in] file Native Handle to an existing file to be re-opened.
		 * @param[in] mode Mode flags to use when re-opening the file.
		 * @param[out] err Reference to the error code set on failure to re-open the file.
		 * @return `basic_file` handle to the re-opened file. */
		[[nodiscard]] static basic_file reopen(const basic_file &file, openmode mode, std::error_code &err) noexcept { return reopen(file.native_handle(), mode, err); }

	private:
		constexpr basic_file(native_t &&file, openmode mode) noexcept : m_file(std::move(file)), m_mode(mode) {}

		template<typename T>
		static constexpr bool nothrow_buffer = std::is_nothrow_invocable_v<decltype(std::ranges::begin), T> && std::is_nothrow_invocable_v<decltype(std::ranges::size), T>;

	public:
		constexpr basic_file() noexcept = default;
		constexpr basic_file(basic_file &&other) noexcept : m_file(std::move(other.m_file)) { std::swap(m_mode, other.m_mode); }
		constexpr basic_file &operator=(basic_file &&other) noexcept
		{
			m_file = std::move(other.m_file);
			std::swap(m_mode, other.m_mode);
			return *this;
		}

		/** Initializes the file handle from a native handle and mode flags.
		 * @note Validity of the mode flags is not checked. */
		constexpr basic_file(native_handle_type file, openmode mode) noexcept : m_file(file), m_mode(mode) {}

		/** @brief Closes the file handle.
		 * @param[out] err Reference to the error code set on failure to close the file. */
		void close(std::error_code &err) noexcept { err = m_file.close(); }
		/** @copybrief close
	 	 * @throw std::system_error On failure to close the file. */
		void close() { if (auto err = m_file.close(); err) [[unlikely]] throw std::system_error(err); }

		/** @brief Seeks to the specified offset within the file starting at the specified position.
		 * @param[in] off Offset into the file starting at \a dir.
		 * @param[in] dir Base direction to seek from.
		 * @return New absolute position within the file.
	 	 * @throw std::system_error On failure to seek the file. */
		std::size_t seek(std::ptrdiff_t off, seekdir dir)
		{
			std::error_code err;
			if (auto res = seek(off, dir, err); err)
				[[unlikely]] throw std::system_error(err);
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
				[[unlikely]] throw std::system_error(err);
			else
				return res;
		}
		/** @copybrief tell
		 * @param[out] err Reference to the error code set on failure to get position of the file.
		 * @return Current absolute position within the file. */
		[[nodiscard]] std::size_t tell(std::error_code &err) const noexcept { return m_file.tell(err); }

		/** Checks if the file is open. */
		[[nodiscard]] constexpr bool is_open() const noexcept { return m_file.is_open(); }
		/** @copydoc is_open */
		[[nodiscard]] constexpr operator bool() const noexcept { return m_file.is_open(); }

		/** Returns the mode flags used to open the file. */
		[[nodiscard]] constexpr openmode mode() const noexcept { return m_mode; }
		/** Returns the underlying native file handle. */
		[[nodiscard]] constexpr native_handle_type native_handle() const noexcept { return m_file.native_handle(); }

		template<rod::detail::decays_to<basic_file> T, typename Dst>
		friend std::size_t tag_invoke(read_t, T &&src, Dst &&dst, std::error_code &err) noexcept(nothrow_buffer<Dst>)
		{
			using value_t = std::ranges::range_value_t<Dst>;
			const auto dst_ptr = std::to_address(std::ranges::begin(dst));
			const auto dst_max = std::ranges::size(dst) * sizeof(value_t);
			return src.m_file.read(static_cast<void *>(dst_ptr), dst_max, err);
		}
		template<rod::detail::decays_to<basic_file> T, typename Src>
		friend std::size_t tag_invoke(write_t, T &&dst, Src &&src, std::error_code &err) noexcept(nothrow_buffer<Src>)
		{
			using value_t = std::ranges::range_value_t<Src>;
			const auto src_ptr = std::to_address(std::ranges::begin(src));
			const auto src_max = std::ranges::size(src) * sizeof(value_t);
			return dst.m_file.write(static_cast<const void *>(src_ptr), src_max, err);
		}

		template<rod::detail::decays_to<basic_file> T, std::convertible_to<std::ptrdiff_t> Pos, typename Dst>
		friend std::size_t tag_invoke(read_at_t, T &&src, Pos &&pos, Dst &&dst, std::error_code &err) noexcept(rod::detail::nothrow_callable<read_t, T, Dst, std::error_code &>)
		{
			/* Seek to the specified position within the file before reading. */
			src.seek(static_cast<std::ptrdiff_t>(pos), beg, err);
			if (err) [[unlikely]] return 0;

			return read(std::forward<T>(src), std::forward<Dst>(dst), err);
		}
		template<rod::detail::decays_to<basic_file> T, std::convertible_to<std::ptrdiff_t> Pos, typename Src>
		friend std::size_t tag_invoke(write_at_t, T &&dst, Pos &&pos, Src &&src, std::error_code &err) noexcept(rod::detail::nothrow_callable<write_t, T, Src, std::error_code &>)
		{
			/* Seek to the specified position within the file before writing. */
			dst.seek(static_cast<std::ptrdiff_t>(pos), beg, err);
			if (err) [[unlikely]] return 0;

			return write(std::forward<T>(dst), std::forward<Src>(src), err);
		}

		constexpr void swap(basic_file &other) noexcept
		{
			using std::swap;
			swap(m_file, other.m_file);
			swap(m_mode, other.m_mode);
		}
		friend constexpr void swap(basic_file &a, basic_file &b) noexcept { a.swap(b); }

	private:
		native_t m_file = {};
		openmode m_mode = {};
	};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

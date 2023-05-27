/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#include "detail/file_fwd.hpp"

/* Platform-specific implementations. */
#include "unix/detail/file.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::_file
{
	static_assert(requires { typename detail::native_file; }, "Native file interface must be implemented for the current platform");

	template<typename T>
	concept noexcept_sizeable_range = detail::nothrow_callable<decltype(std::ranges::begin), T> && detail::nothrow_callable<decltype(std::ranges::size), T>;
	
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

	private:
		[[nodiscard]] static constexpr openmode filter_mode(openmode mode) noexcept
		{
			return mode & ~(detail::ate | detail::trunc | detail::noreplace);
		}
		
	public:
		/** @brief Opens the file specified by \a path using mode flags \a flags.
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
		 * @param[in] path Path to the file to be opened.
		 * @param[in] mode Mode flags to open the file with.
		 * @param[out] err Reference to the error code set on failure to open the file.
		 * @return `basic_file` handle to the opened file, or a closed file handle if an error has occurred. */
		[[nodiscard]] static basic_file open(const char *path, openmode mode, std::error_code &err) noexcept { return {native_t::open(path, mode | binary, err)}; }
		/** @copydoc open */
		[[nodiscard]] static basic_file open(const std::string &path, openmode mode, std::error_code &err) noexcept { return open(path.c_str(), mode, err); }
		/** @copydoc open */
		[[nodiscard]] static basic_file open(const std::filesystem::path &path, openmode mode, std::error_code &err) noexcept { return open(path.c_str(), mode, err); }

		/** @brief Re-opens a file pointed to by a native file handle and mode flags.
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
		void close() { if (auto err = m_file.close(); err) [[unlikely]] throw std::system_error(err); }

		/** @brief Flushes modified file data to the underlying device.
		 * @param[out] err Reference to the error code set on failure to flush the file. */
		void flush(std::error_code &err) noexcept { err = m_file.flush(); }
		/** @copybrief flush
	 	 * @throw std::system_error On failure to flush the file. */
		void flush() { if (auto err = m_file.flush(); err) [[unlikely]] throw std::system_error(err); }

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
		/** Returns the underlying native file handle. */
		[[nodiscard]] constexpr native_handle_type native_handle() const noexcept { return m_file.native_handle(); }
		
		template<typename T, typename Dst> requires std::same_as<std::remove_reference_t<T>, basic_file>
		friend std::size_t tag_invoke(read_some_t, T &&src, Dst &&dst, std::error_code &err) noexcept(noexcept_sizeable_range<Dst>)
		{
			using value_t = std::ranges::range_value_t<Dst>;
			const auto dst_ptr = std::to_address(std::ranges::begin(dst));
			const auto dst_max = std::ranges::size(dst) * sizeof(value_t);
			return src.m_file.sync_read(static_cast<void *>(dst_ptr), dst_max, err);
		}
		template<typename T, typename Src> requires std::same_as<std::remove_reference_t<T>, basic_file>
		friend std::size_t tag_invoke(write_some_t, T &&dst, Src &&src, std::error_code &err) noexcept(noexcept_sizeable_range<Src>)
		{
			using value_t = std::ranges::range_value_t<Src>;
			const auto src_ptr = std::to_address(std::ranges::begin(src));
			const auto src_max = std::ranges::size(src) * sizeof(value_t);
			return dst.m_file.sync_write(static_cast<const void *>(src_ptr), src_max, err);
		}
		template<typename T, std::convertible_to<std::ptrdiff_t> Pos, typename Dst> requires std::same_as<std::remove_reference_t<T>, basic_file>
		friend std::size_t tag_invoke(read_some_at_t, T &&src, Pos pos, Dst &&dst, std::error_code &err) noexcept(noexcept_sizeable_range<Dst>)
		{
			using value_t = std::ranges::range_value_t<Dst>;
			const auto dst_ptr = std::to_address(std::ranges::begin(dst));
			const auto dst_max = std::ranges::size(dst) * sizeof(value_t);
			return src.m_file.sync_read_at(static_cast<void *>(dst_ptr), dst_max, static_cast<std::ptrdiff_t>(pos), err);
		}
		template<typename T, std::convertible_to<std::ptrdiff_t> Pos, typename Src> requires std::same_as<std::remove_reference_t<T>, basic_file>
		friend std::size_t tag_invoke(write_some_at_t, T &&dst, Pos pos, Src &&src, std::error_code &err) noexcept(noexcept_sizeable_range<Src>)
		{
			using value_t = std::ranges::range_value_t<Src>;
			const auto src_ptr = std::to_address(std::ranges::begin(src));
			const auto src_max = std::ranges::size(src) * sizeof(value_t);
			return dst.m_file.sync_write_at(static_cast<const void *>(src_ptr), src_max, static_cast<std::ptrdiff_t>(pos), err);
		}
		
		template<typename T, typename Snd, typename Dst> requires std::same_as<std::remove_reference_t<T>, basic_file>
		friend decltype(auto) tag_invoke(async_read_some_t, Snd &&snd, T &&src, Dst &&dst) noexcept(detail::nothrow_callable<async_read_some_t, Snd, native_handle_type, Dst>)
		{
			static_assert(detail::callable<async_read_some_t, Snd, native_handle_type, Dst>);
			return async_read_some(std::forward<Snd>(snd), src.native_handle(), std::forward<Dst>(dst));
		}
		template<typename T, typename Snd, typename Src> requires std::same_as<std::remove_reference_t<T>, basic_file>
		friend decltype(auto) tag_invoke(async_write_some_t, Snd &&snd, T &&dst, Src &&src) noexcept(detail::nothrow_callable<async_write_some_t, Snd, native_handle_type, Src>)
		{
			static_assert(detail::callable<async_write_some_t, Snd, native_handle_type, Src>);
			return async_write_some(std::forward<Snd>(snd), dst.native_handle(), std::forward<Src>(src));
		}
		template<typename T, typename Snd, std::convertible_to<std::ptrdiff_t> Pos, typename Dst> requires std::same_as<std::remove_reference_t<T>, basic_file>
		friend decltype(auto) tag_invoke(async_read_some_at_t, Snd &&snd, T &&src, Pos pos, Dst &&dst) noexcept(detail::nothrow_callable<async_read_some_at_t, Snd, native_handle_type, Pos, Dst>)
		{
			static_assert(detail::callable<async_read_some_at_t, Snd, native_handle_type, Pos, Dst>);
			return async_read_some_at(std::forward<Snd>(snd), src.native_handle(), pos, std::forward<Dst>(dst));
		}
		template<typename T, typename Snd, std::convertible_to<std::ptrdiff_t> Pos, typename Src> requires std::same_as<std::remove_reference_t<T>, basic_file>
		friend decltype(auto) tag_invoke(async_write_some_at_t, Snd &&snd, T &&dst, Pos pos, Src &&src) noexcept(detail::nothrow_callable<async_write_some_at_t, Snd, native_handle_type, Pos, Src>)
		{
			static_assert(detail::callable<async_write_some_at_t, Snd, native_handle_type, Pos, Src>);
			return async_write_some_at(std::forward<Snd>(snd), dst.native_handle(), pos, std::forward<Src>(src));
		}

		template<typename T, typename Sch, typename Dst> requires std::same_as<std::remove_reference_t<T>, basic_file>
		friend decltype(auto) tag_invoke(schedule_read_some_t, Sch &&sch, T &&src, Dst &&dst) noexcept(detail::nothrow_callable<schedule_read_some_t, Sch, native_handle_type, Dst>)
		{
			static_assert(detail::callable<schedule_read_some_t, Sch, native_handle_type, Dst>);
			return schedule_read_some(std::forward<Sch>(sch), src.native_handle(), std::forward<Dst>(dst));
		}
		template<typename T, typename Sch, typename Src> requires std::same_as<std::remove_reference_t<T>, basic_file>
		friend decltype(auto) tag_invoke(schedule_write_some_t, Sch &&sch, T &&dst, Src &&src) noexcept(detail::nothrow_callable<schedule_write_some_t, Sch, native_handle_type, Src>)
		{
			static_assert(detail::callable<schedule_write_some_t, Sch, native_handle_type, Src>);
			return schedule_write_some(std::forward<Sch>(sch), dst.native_handle(), std::forward<Src>(src));
		}
		template<typename T, typename Sch, std::convertible_to<std::ptrdiff_t> Pos, typename Dst> requires std::same_as<std::remove_reference_t<T>, basic_file>
		friend decltype(auto) tag_invoke(schedule_read_some_at_t, Sch &&sch, T &&src, Pos pos, Dst &&dst) noexcept(detail::nothrow_callable<schedule_read_some_at_t, Sch, native_handle_type, Pos, Dst>)
		{
			static_assert(detail::callable<schedule_read_some_at_t, Sch, native_handle_type, Pos, Dst>);
			return schedule_read_some_at(std::forward<Sch>(sch), src.native_handle(), pos, std::forward<Dst>(dst));
		}
		template<typename T, typename Sch, std::convertible_to<std::ptrdiff_t> Pos, typename Src> requires std::same_as<std::remove_reference_t<T>, basic_file>
		friend decltype(auto) tag_invoke(schedule_write_some_at_t, Sch &&sch, T &&dst, Pos pos, Src &&src) noexcept(detail::nothrow_callable<schedule_write_some_at_t, Sch, native_handle_type, Pos, Src>)
		{
			static_assert(detail::callable<schedule_write_some_at_t, Sch, native_handle_type, Pos, Src>);
			return schedule_write_some_at(std::forward<Sch>(sch), dst.native_handle(), pos, std::forward<Src>(src));
		}

		constexpr void swap(basic_file &other) noexcept { m_file.swap(other.m_file); }
		friend constexpr void swap(basic_file &a, basic_file &b) noexcept { a.swap(b); }
		
	private:
		native_t m_file = {};
	};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

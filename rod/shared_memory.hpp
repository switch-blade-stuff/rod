/*
 * Created by switch_blade on 2023-07-03.
 */

#pragma once

#include "detail/mmap.hpp"
#include "result.hpp"

/* Platform-specific implementations. */
#if defined(__unix__)
#include "unix/detail/shm.hpp"
#elif defined(_WIN32)
#include "win32/detail/shm.hpp"
#else
#error Unsupported platform
#endif

namespace rod
{
	/** Handle to a named shared memory object used primarily for inter-process communication. */
	class shared_memory
	{
		using native_t = detail::system_shm;

	public:
		using native_handle_type = typename native_t::native_handle_type;

		using openmode = int;

		/** Opens shared memory object in read-only mode. */
		static constexpr openmode readonly = native_t::readonly;
		/** Opens shared memory object in create-only mode. Opening the shared memory object will fail if it does not exist. */
		static constexpr openmode nocreate = native_t::nocreate;

	public:
		/** Opens the shared memory object identified by name \a name using mode flags \a flags.
		 * @param[in] name Name of the shared memory object to be opened.
		 * @param[in] size Size of the shared memory object. If set to `0`, uses an implementation-defined default size.
		 * @param[in] mode Mode flags to open the shared memory object with.
		 * @return Handle to the opened shared memory object, or an error code on failure to open the shared memory object.
		 * @note Shared memory might be allocated using a temporary filesystem file, and as such might persist beyond the lifetime of it's parent process. */
		[[nodiscard]] static result<shared_memory, std::error_code> open(const char *name, std::size_t size = 0, openmode mode = {}) noexcept { return native_t::open(name, size, mode); }
		/** @copydoc open */
		[[nodiscard]] static result<shared_memory, std::error_code> open(const std::string &name, std::size_t size = 0, openmode mode = {}) noexcept { return open(name.c_str(), size, mode); }
		/** @copydoc open */
		[[nodiscard]] static result<shared_memory, std::error_code> open(const wchar_t *path, std::size_t size = 0, openmode mode = {}) noexcept { return native_t::open(path, size, mode); }
		/** @copydoc open */
		[[nodiscard]] static result<shared_memory, std::error_code> open(const std::wstring &name, std::size_t size = 0, openmode mode = {}) noexcept { return open(name.c_str(), size, mode); }

	public:
		shared_memory() = delete;

		constexpr shared_memory(shared_memory &&other) noexcept : _shm(std::move(other._shm)) {}
		constexpr shared_memory &operator=(shared_memory &&other) noexcept { return (_shm = std::move(other._shm), *this); }

		constexpr explicit shared_memory(native_t &&shm) noexcept : _shm(std::forward<native_t>(shm)) {}

		/** Maps a portion of the shared memory object into memory of the current process.
		 * @param \a pos Starting position (in bytes) of the region from the beginning of the shared memory object.
		 * @param \a size Total size (in bytes) of the resulting memory-mapped region.
		 * @return View to the memory-mapped region or an error code on failure to map view of the shared memory object. */
		[[nodiscard]] result<mmap, std::error_code> map(std::size_t pos, std::size_t size) const noexcept { return _shm.map(pos, size); }
		/** @copydoc map
		 * @param[in] mode Mode flags to use when memory-mapping the shared memory object. */
		[[nodiscard]] result<mmap, std::error_code> map(std::size_t pos, std::size_t size, mmap::mapmode mode) noexcept { return _shm.map(pos, size, mode); }
		/** @copydoc map
		 * @note This overload will ignore the `mmap::expand` mode flag. */
		[[nodiscard]] result<mmap, std::error_code> map(std::size_t pos, std::size_t size, mmap::mapmode mode) const noexcept { return _shm.map(pos, size, mode & ~mmap::expand); }

		/** Attempts to lock the shared memory object.
		 * @return `true` if locked successfully, `false` if another process already has the lock. */
		bool try_lock() noexcept { return _shm.try_lock(); }
		/** Locks the shared memory object.
		 * @return Error code on failure to lock the shared memory object. */
		std::error_code lock() noexcept { return _shm.lock(); }
		/** Releases the shared memory object lock.
		 * @return Error code on failure to unlock the shared memory object. */
		std::error_code unlock() noexcept { return _shm.unlock(); }

		/** Returns the underlying native shared memory object handle. */
		[[nodiscard]] native_handle_type native_handle() const noexcept { return _shm.native_handle(); }

		constexpr void swap(shared_memory &other) noexcept { _shm.swap(other._shm); }
		friend constexpr void swap(shared_memory &a, shared_memory &b) noexcept { a.swap(b); }

	private:
		native_t _shm;
	};
}

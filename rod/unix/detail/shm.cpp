/*
 * Created by switch_blade on 2023-07-03.
 */

#ifdef __unix__

#include "shm.hpp"

#include <sys/mman.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace rod::detail
{
	result<system_shm, std::error_code> system_shm::open(const char *name, std::size_t size, int mode) noexcept
	{
		int flags = 0, prot = 0;
		if (!(mode & openmode::nocreate))
			flags = O_CREAT;
		if (mode & openmode::in)
		{
			prot |= S_IRUSR | S_IRGRP | S_IROTH;
			flags = O_RDONLY;
		}
		if (flsgs == O_CREAT || (mode & openmode::out))
		{
			prot |= S_IWUSR | S_IWGRP | S_IWOTH;
			flags = O_RDWR;
		}

		if (const auto fd = ::shm_open(name, flags, prot); fd >= 0) [[likely]]
		{
			if (std::error_code err; size != 0 && (err = basic_descriptor{fd}.reserve(size))) [[unlikely]]
			{
				::close(fd);
				return err;
			}
			return system_file{fd};
		}
		return std::error_code{errno, std::system_category()};
	}
	result<system_shm, std::error_code> system_shm::open(const wchar_t *name, std::size_t size, int mode) noexcept
	{
		try
		{
			auto state = std::mbstate_t{};
			auto name_size = std::wcsrtombs(nullptr, &path, 0, &state);
			if (name_size == static_cast<std::size_t>(-1)) [[unlikely]]
				return std::error_code{errno, std::system_category()};

			auto name_buff = std::string((name_size + 4) * 2 + 1, '\0');
			name_size = std::wcsrtombs(buff.data(), &path, name_buff.size(), &state);
			if (name_size == static_cast<std::size_t>(-1)) [[unlikely]]
				return std::error_code{errno, std::system_category()};

			return open(name_buff.c_str(), size, mode);
		}
		catch (const std::bad_alloc &)
		{
			return std::make_error_code(std::errc::not_enough_memory);
		}
	}

	result<system_mmap, std::error_code> system_shm::map(std::size_t off, std::size_t n, int mode) const noexcept
	{
		/* Align the offset to page size & resize if needed. */
		const auto base_off = native_mmap::pagesize_off(off);
		if (base_off.has_error())
			[[unlikely]] return base_off.error();
		else if (std::error_code err; (mode & system_mmap::mapmode::expand) && (err = reserve(n + off)))
			[[unlikely]] return err;

		int flags = 0;
		if (mode & system_mmap::mapmode::copy)
			flags = MAP_PRIVATE;

		int prot = 0;
		if (mode & system_mmap::mapmode::read)
			prot |= PROT_READ;
		if (mode & system_mmap::mapmode::write)
			prot |= PROT_WRITE;

#if PTRDIFF_MAX >= INT64_MAX
		const auto data = ::mmap64(nullptr, n + off - *base_off, prot, flags, native_handle(), static_cast<off64_t>(*base_off));
#else
		const auto data = ::mmap(nullptr, n + off - *base_off, prot, flags, native_handle(), static_cast<off64_t>(*base_off));
#endif
		if (data) [[likely]]
			return system_mmap{data, off - *base_off, n + off - *base_off};
		else
			return std::error_code{errno, std::system_category()};
	}

	bool system_shm::try_lock() noexcept { return !::flock(native_handle(), LOCK_EX | LOCK_NB); }
	std::error_code system_shm::lock() noexcept
	{
		if (::flock(native_handle(), LOCK_EX)) [[unlikely]]
			return std::error_code{errno, std::system_category()};
		else
			return {};
	}
	std::error_code system_shm::unlock() noexcept
	{
		if (::flock(native_handle(), LOCK_UN)) [[unlikely]]
			return std::error_code{errno, std::system_category()};
		else
			return {};
	}
}
#endif
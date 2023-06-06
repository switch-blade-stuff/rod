/*
 * Created by switchblade on 2023-05-30.
 */

#ifdef __unix__

#include <sys/mman.h>
#include <unistd.h>

#include "mmap.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	static std::size_t get_pagesize(std::error_code &err) noexcept
	{
		static const auto size = [&]() noexcept -> std::size_t
		{
#ifdef _SC_PAGE_SIZE
			const auto size = sysconf(_SC_PAGE_SIZE);
#else
			const auto size = sysconf(_SC_PAGESIZE);
#endif
			if (size < 0) [[unlikely]]
				return (err = {errno, std::system_category()}, 0);
			else
				return (err = {}, static_cast<std::size_t>(size));
		}();
		return size;
	}
	template<typename I>
	static I align_pagesize(bool up, I req, std::error_code &err) noexcept
	{
		if (const auto mult = get_pagesize(err); !mult) [[unlikely]]
			return mult;
		else if (const auto rem = static_cast<I>(req % mult); rem && up)
			return req + static_cast<I>(mult) - rem;
		else
			return req - rem;
	}

	inline std::error_code try_resize_descriptor(int fd, std::size_t new_size) noexcept
	{
		if (::stat stat = {}; ::fstat(native_handle(), &stat)) [[unlikely]]
			return {errno, std::system_category()};
		else if (static_cast<std::size_t>(stat.st_size) < size)
		{
#if SIZE_MAX >= UINT64_MAX
			if (::ftruncate64(fd, static_cast<off64_t>(size))) [[unlikely]]
				return {errno, std::system_category()};
#else
			if (::ftruncate(fd, static_cast<off_t>(size))) [[unlikely]]
				return {errno, std::system_category()};
#endif
		}
		return {};
	}

	mmap_handle mmap_handle::map(int fd, std::size_t off, std::size_t size, int mode, std::error_code &err) noexcept
	{
		/* Align the file offset to page size. */
		const auto base_off = off - align_pagesize(false, off, err);
		if (err)
			[[unlikely]] return mmap_handle{};
		else
			 size += base_off

		/* Expand the underlying file if needed. */
		if ((mode & mapmode::expand) && (err = try_resize_descriptor(fd, size + off)))
			 return mmap_handle{};

		int flags = 0;
		if (fd < 0) flags |= MAP_ANONYMOUS;
		if (mode & mapmode::copy) flags |= MAP_PRIVATE;

		int prot = 0;
		if (mode & mapmode::exec) prot |= PROT_EXEC;
		if (mode & mapmode::read) prot |= PROT_READ;
		if (mode & mapmode::write) prot |= PROT_WRITE;

#if PTRDIFF_MAX >= INT64_MAX
		const auto data = ::mmap64(size, prot, flags, fd, static_cast<off64_t>(off - base_off));
#else
		const auto data = ::mmap(size, prot, flags, fd, static_cast<off64_t>(off - base_off));
#endif
		if (data) [[likely]]
			return shared_mmap{static_cast<std::byte *>(data), base_off, size};
		else
			return (err = {errno, std::system_category()}, shared_mmap{});
	}

	mmap_handle::~mmap_handle()
	{
		const auto [data, size] = release();
		if (data) ::munmap(data, size);
	}
	std::error_code mmap_handle::unmap() noexcept
	{
		const auto [data, size] = release();
		if (data && ::munmap(data, size)) [[unlikely]]
			return {errno, std::system_category()};
		else
			return {};
	}

	static shared_mmap shared_mmap::open(const char *name, std::size_t size, int mode, int prot, std::error_code &err) noexcept
	{
		const int flags = O_CREAT | (mode & mapmode::write) ? O_RDWR : O_RDONLY;
		if (const auto fd = shm_open(name, flags, prot); fd < 0) [[unlikely]]
			return (err = {errno, std::system_category()}, shared_mmap{});
		else
		{
			auto result = map(fd, 0, size, mode | mapmode::expand, err);
			if (err) [[unlikely]] ::close(fd);
			return result;
		}
	}
	static shared_mmap shared_mmap::open(const wchar_t *name, std::size_t size, int mode, int prot, std::error_code &err) noexcept
	{
		auto state = std::mbstate_t{};
		auto res = std::wcsrtombs(nullptr, &path, 0, &state);
		if (res == static_cast<std::size_t>(-1)) [[unlikely]]
			return (err = {errno, std::system_category()}, system_file{});

		auto buff = std::string(res, '\0');
		res = std::wcsrtombs(buff.data(), &path, buff.size(), &state);
		if (res == static_cast<std::size_t>(-1)) [[unlikely]]
			return (err = {errno, std::system_category()}, system_file{});
		else
			return open(buff.c_str(), size, mode, prot, err);
	}

}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif
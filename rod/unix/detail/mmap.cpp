/*
 * Created by switchblade on 2023-05-30.
 */

#ifdef __unix__

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mmap.hpp"

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
	static std::size_t align_pagesize(std::size_t req, std::error_code &err) noexcept
	{
		if (const auto mult = get_pagesize(err); mult) [[likely]]
			return req - static_cast<std::size_t>(req % mult);
		else
			return mult;
	}
	static std::error_code try_resize_descriptor(int fd, std::size_t new_size) noexcept
	{
		if (struct stat stat = {}; ::fstat(fd, &stat)) [[unlikely]]
			return {errno, std::system_category()};
		else if (static_cast<std::size_t>(stat.st_size) < new_size)
		{
#if SIZE_MAX >= UINT64_MAX
			if (::ftruncate64(fd, static_cast<off64_t>(new_size))) [[unlikely]]
				return {errno, std::system_category()};
#else
			if (::ftruncate(fd, static_cast<off_t>(new_size))) [[unlikely]]
				return {errno, std::system_category()};
#endif
		}
		return {};
	}

	system_mapping system_mapping::map(int fd, std::size_t off, std::size_t size, int mode, std::error_code &err) noexcept
	{
		/* Align the file offset to page size & resize if needed. */
		const auto base_off = off - align_pagesize(off, err);
		if (err || ((mode & mapmode::expand) && (err = try_resize_descriptor(fd, size + off))))
			[[unlikely]] return system_mapping{};
		else
			 size += base_off;

		int flags = 0;
		if (fd < 0) flags |= MAP_ANONYMOUS;
		if (mode & mapmode::copy) flags |= MAP_PRIVATE;

		int prot = 0;
		if (mode & mapmode::exec) prot |= PROT_EXEC;
		if (mode & mapmode::read) prot |= PROT_READ;
		if (mode & mapmode::write) prot |= PROT_WRITE;

#if PTRDIFF_MAX >= INT64_MAX
		const auto data = ::mmap64(nullptr, size, prot, flags, fd, static_cast<off64_t>(off - base_off));
#else
		const auto data = ::mmap(nullptr, size, prot, flags, fd, static_cast<off64_t>(off - base_off));
#endif
		if (data) [[likely]]
			return system_mapping{static_cast<std::byte *>(data), base_off, size};
		else
			return (err = {errno, std::system_category()}, system_mapping{});
	}

	std::error_code system_mapping::unmap() noexcept
	{
		const auto [data, size] = release();
		if (data && ::munmap(data, size)) [[unlikely]]
			return {errno, std::system_category()};
		else
			return {};
	}
	system_mapping::~system_mapping()
	{
		const auto [data, size] = release();
		if (data) ::munmap(data, size);
	}
}
#endif
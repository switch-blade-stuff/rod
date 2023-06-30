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
	std::error_code mmap_handle::unmap() noexcept
	{
		const auto [data, size] = release();
		if (data && ::munmap(data, size)) [[unlikely]]
			return {errno, std::system_category()};
		else
			return {};
	}
	mmap_handle::~mmap_handle()
	{
		const auto [data, size] = release();
		if (data) ::munmap(data, size);
	}

	static result<std::size_t, std::error_code> get_pagesize() noexcept
	{
		result<std::size_t, std::error_code> result;
		static const auto size = [&]() noexcept -> std::size_t
		{
#ifdef _SC_PAGE_SIZE
			const auto size = sysconf(_SC_PAGE_SIZE);
#else
			const auto size = sysconf(_SC_PAGESIZE);
#endif
			if (size < 0) [[unlikely]]
				return (result = std::error_code{errno, std::system_category()}, 0);
			else
				return static_cast<std::size_t>(size);
		}();
		return result.empty() ? size : result;
	}
	static result<std::size_t, std::error_code> align_pagesize(std::size_t req) noexcept
	{
		if (const auto res = get_pagesize(); res.has_value()) [[likely]]
			return req - static_cast<std::size_t>(req % *res);
		else
			return res;
	}

	static std::error_code resize_fd(int fd, std::size_t new_size) noexcept
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

	result<system_mapping, std::error_code> system_mapping::map(int fd, std::size_t off, std::size_t size, int mode) noexcept
	{
		/* Align the file offset to page size & resize if needed. */
		const auto aligned_off = align_pagesize(off);
		if (aligned_off.has_error())
			[[unlikely]] return aligned_off.error();
		else if (std::error_code err; (mode & mapmode::expand) && (err = resize_fd(fd, size + off)))
			[[unlikely]] return err;

		int flags = 0;
		if (fd < 0) flags |= MAP_ANONYMOUS;
		if (mode & mapmode::copy) flags |= MAP_PRIVATE;

		int prot = 0;
		if (mode & mapmode::exec) prot |= PROT_EXEC;
		if (mode & mapmode::read) prot |= PROT_READ;
		if (mode & mapmode::write) prot |= PROT_WRITE;

#if PTRDIFF_MAX >= INT64_MAX
		const auto data = ::mmap64(nullptr, size + off - *aligned_off, prot, flags, fd, static_cast<off64_t>(*aligned_off));
#else
		const auto data = ::mmap(nullptr, size + off - *aligned_off, prot, flags, fd, static_cast<off64_t>(*aligned_off));
#endif
		if (data) [[likely]]
			return system_mapping{static_cast<std::byte *>(data), off - *aligned_off, size + off - *aligned_off};
		else
			return std::error_code{errno, std::system_category()};
	}
}
#endif
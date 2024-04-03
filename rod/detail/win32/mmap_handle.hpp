/*
 * Created by switchblade on 2023-11-23.
 */

#pragma once

#include "../mmap_handle.hpp"
#include "ntapi.hpp"

namespace rod::_win32
{
	inline constexpr std::pair<ULONG, ULONG> flags_to_prot_and_attr(mmap_flags flags) noexcept
	{
		ULONG prot = 0;
		if (bool(flags & mmap_flags::exec))
		{
			if (bool(flags & mmap_flags::write))
				prot = PAGE_EXECUTE_READWRITE;
			else if (bool(flags & mmap_flags::copy))
				prot = PAGE_EXECUTE_WRITECOPY;
			else if (bool(flags & mmap_flags::read))
				prot = PAGE_EXECUTE_READ;
			else
				prot = PAGE_EXECUTE;
		}
		else
		{
			if (bool(flags & (mmap_flags::write | mmap_flags::reserve | mmap_flags::system_image)))
				prot = PAGE_READWRITE;
			else if (bool(flags & mmap_flags::copy))
				prot = PAGE_WRITECOPY;
			else
				prot = PAGE_READONLY;
		}

		ULONG attr = SEC_COMMIT;
		if (bool(flags & mmap_flags::reserve))
			attr = SEC_RESERVE;
		if (bool(flags & mmap_flags::system_image))
			attr = SEC_IMAGE;
		if (bool(flags & mmap_flags::map_large_pages))
			attr |= SEC_LARGE_PAGES;

		return {prot, attr};
	}
	inline constexpr ULONG flags_to_prot(mmap_flags flags) noexcept
	{
		ULONG prot = 0;
		if (bool(flags & mmap_flags::exec))
		{
			if (bool(flags & mmap_flags::write))
				prot = PAGE_EXECUTE_READWRITE;
			else if (bool(flags & mmap_flags::copy))
				prot = PAGE_EXECUTE_WRITECOPY;
			else if (bool(flags & mmap_flags::read))
				prot = PAGE_EXECUTE_READ;
			else
				prot = PAGE_EXECUTE;
		}
		else
		{
			if (bool(flags & (mmap_flags::write)))
				prot = PAGE_READWRITE;
			else if (bool(flags & mmap_flags::copy))
				prot = PAGE_WRITECOPY;
			else if (bool(flags & mmap_flags::read))
				prot = PAGE_READONLY;
			else
				prot = PAGE_NOACCESS;
		}
		return prot;
	}

	inline constexpr extent_pair clamp_extent(extent_pair ext, _handle::extent_type max) noexcept
	{
		if (ext.second == 0)
			ext.second = max;
		if (ext.first >= max)
			ext.first = 0;
		return ext;
	}
	inline constexpr extent_pair over_align_extent(extent_pair ext, _handle::extent_type mul) noexcept
	{
		const auto mask = mul - 1;
		ext.second = (ext.second + mask) & ~mask;
		ext.first = ext.first & ~mask;
		return ext;
	}
	inline constexpr extent_pair under_align_extent(extent_pair ext, _handle::extent_type mul) noexcept
	{
		const auto mask = mul - 1;
		ext.first = (ext.first + mask) & ~mask;
		ext.second = ext.second & ~mask;
		return ext;
	}
}

/*
 * Created by switchblade on 2023-11-23.
 */

#include "mmap_handle.hpp"

namespace rod::_mmap
{
	using namespace _win32;

	constexpr auto prot_flags_mask = mmap_flags::read | mmap_flags::exec | mmap_flags::copy | mmap_flags::write;

	result<mmap_source> mmap_source::open(extent_type size, const fs::path_handle &base, mmap_flags flags) noexcept
	{
		if ((bool(flags & mmap_flags::copy) && bool(flags & mmap_flags::write)) || (bool(flags & (mmap_flags::commit | mmap_flags::prefault)) && bool(flags & mmap_flags::reserve))) [[unlikely]]
			return std::make_error_code(std::errc::invalid_argument);
		if (bool(flags & mmap_flags::reserve) && bool(flags & mmap_flags::map_large_pages)) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		if (size > extent_type(std::numeric_limits<LONGLONG>::max())) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);
		if ((flags & (mmap_flags::reserve | mmap_flags::commit)) == mmap_flags::none)
			flags |= mmap_flags::commit;

		auto tmp_file = fs::file_handle::open_anonymous(base);
		if (tmp_file.has_error()) [[unlikely]]
			return tmp_file.error();

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		const auto [prot, attr] = flags_to_prot_and_attr(flags);
		const auto max_size = LONGLONG(size);
		void *hnd = INVALID_HANDLE_VALUE;

		if (auto status = ntapi->NtCreateSection(&hnd, SECTION_ALL_ACCESS, nullptr, &max_size, prot, attr, tmp_file->native_handle()); is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return mmap_source(hnd, flags);
	}
	result<mmap_source> mmap_source::open(const fs::file_handle &file, extent_type size, mmap_flags flags) noexcept
	{
		if ((bool(flags & mmap_flags::copy) && bool(flags & mmap_flags::write)) || (bool(flags & (mmap_flags::commit | mmap_flags::prefault)) && bool(flags & mmap_flags::reserve))) [[unlikely]]
			return std::make_error_code(std::errc::invalid_argument);
		if (bool(flags & mmap_flags::reserve) && bool(flags & mmap_flags::map_large_pages)) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		if (size > extent_type(std::numeric_limits<LONGLONG>::max())) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);
		if ((flags & (mmap_flags::reserve | mmap_flags::commit)) == mmap_flags::none)
			flags |= mmap_flags::commit;

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto [prot, attr] = flags_to_prot_and_attr(flags);
		auto name_buff = std::array<wchar_t, 95>();
		auto obj_attr = object_attributes();
		auto max_size = LONGLONG(size);
		auto upath = unicode_string();

		if (bool(flags & mmap_flags::shared_memory))
		{
			static thread_local const auto path_format = []() noexcept
			{
				auto fmt = LR"(\Sessions\%u\BaseNamedObjects\w%%.16x%%.16x.mmap.rod)";
				auto res = std::array<wchar_t, 60>();

				if (DWORD session; ::ProcessIdToSessionId(::GetCurrentProcessId(), &session) != 0)
					swprintf_s(res.data(), res.size() - 1, fmt, session);
				else
					swprintf_s(res.data(), res.size() - 1, fmt, 0);

				return res;
			}();

			stat st;
			if (auto res = get_stat(st, file, stat::query::dev | stat::query::ino); res.has_error()) [[unlikely]]
				return res.error();
			else if (*res != (stat::query::dev | stat::query::ino)) [[unlikely]]
				return std::make_error_code(std::errc::not_supported);

			swprintf_s(name_buff.data(), name_buff.size(), path_format.data(), st.dev, st.ino);
			upath.size = USHORT(wcsnlen(name_buff.data(), name_buff.size()) * sizeof(wchar_t));
			upath.max = USHORT(name_buff.size() * sizeof(wchar_t));
			upath.buff = name_buff.data();

			obj_attr.length = sizeof(object_attributes);
			obj_attr.attr = 0x80; /*OBJ_OPENIF*/
			obj_attr.name = &upath;
		}

		auto attr_ptr = obj_attr.length ? &obj_attr : nullptr;
		auto size_ptr = max_size ? &max_size : nullptr;
		void *hnd = INVALID_HANDLE_VALUE;

		/* Always attempt to open writable shared sections first. */
		if (auto status = ntapi->NtOpenSection(&hnd, SECTION_ALL_ACCESS, attr_ptr); is_status_failure(status))
		{
			if (attr_ptr && !bool(flags & mmap_flags::write))
			{
				name_buff[std::wstring_view(name_buff.data(), name_buff.size()).rfind(L'\\') + 1] = L'r';
				status = ntapi->NtOpenSection(&hnd, SECTION_ALL_ACCESS, attr_ptr);
				if (!is_status_failure(status))
					goto open_success;
			}
			status = ntapi->NtCreateSection(&hnd, SECTION_ALL_ACCESS, attr_ptr, size_ptr, prot, attr, file.native_handle());
			if (is_status_failure(status)) [[unlikely]]
			{
				auto err = status_error_code(status);
				if (is_error_file_not_found(err))
					err = std::make_error_code(std::errc::no_such_file_or_directory);
				if (is_error_access_denied(err))
					err = std::make_error_code(std::errc::permission_denied);
				return err;
			}
		}
	open_success:
		return mmap_source(hnd, flags);
	}

	result<mmap_source::extent_type> mmap_source::do_endpos() const noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto info = section_basic_information();
		if (const auto status = ntapi->NtQuerySection(native_handle(), SectionBasicInformation, &info, sizeof(info), nullptr); !is_status_failure(status)) [[likely]]
			return extent_type(info.max_size);
		else
			return status_error_code(status);
	}
	result<mmap_source::extent_type> mmap_source::do_truncate(extent_type endp) noexcept
	{
		if (endp > extent_type(std::numeric_limits<LONGLONG>::max())) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto new_size = LONGLONG(endp);
		if (const auto status = ntapi->NtExtendSection(native_handle(), &new_size); is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return extent_type(new_size);
	}

	result<mmap_handle> mmap_handle::map(size_type size, mmap_flags flags) noexcept
	{
		if ((bool(flags & mmap_flags::copy) && bool(flags & mmap_flags::write)) || (bool(flags & (mmap_flags::commit | mmap_flags::prefault)) && bool(flags & mmap_flags::reserve))) [[unlikely]]
			return std::make_error_code(std::errc::invalid_argument);
		if (bool(flags & mmap_flags::reserve) && bool(flags & mmap_flags::map_large_pages)) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		if ((flags & (mmap_flags::reserve | mmap_flags::commit)) == mmap_flags::none)
			flags |= mmap_flags::commit;

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		ULONG prot = flags_to_prot(flags), type = SEC_RESERVE;
		if (bool(flags & mmap_flags::commit))
			type |= SEC_COMMIT;
		if (bool(flags & mmap_flags::map_large_pages))
			type |= SEC_LARGE_PAGES;

		const auto page_sizes = get_page_sizes();
		const auto page = (type & SEC_LARGE_PAGES) && page_sizes.size() > 1 ? page_sizes[1] : page_sizes[0];
		const auto total = (size + (page - 1)) & ~(page - 1);
		void *mem = nullptr;

		if ((mem = ::VirtualAlloc(mem, total, type | SEC_RESERVE | SEC_COMMIT, prot)) == nullptr) [[unlikely]]
			return dos_error_code(::GetLastError());

		if (bool(flags & mmap_flags::prefault)) /* Prefault manually if required. */
		{
			if (ntapi->PrefetchVirtualMemory)
			{
				auto entry = WIN32_MEMORY_RANGE_ENTRY{mem, total};
				ntapi->PrefetchVirtualMemory(::GetCurrentProcess(), 1, &entry, 0);
			}

			auto vmem = static_cast<volatile std::int8_t *>(mem);
			for (std::size_t i = 0; i < total; i += page)
				(void) vmem[i];
		}
		return mmap_handle(static_cast<std::byte *>(mem), 0, size, page, total, flags);
	}
	result<mmap_handle> mmap_handle::map(const mmap_source &src, extent_type offset, size_type size, mmap_flags flags) noexcept
	{
		if ((bool(flags & mmap_flags::copy) && bool(flags & mmap_flags::write)) || (bool(flags & (mmap_flags::commit | mmap_flags::prefault)) && bool(flags & mmap_flags::reserve)) || !src.is_open()) [[unlikely]]
			return std::make_error_code(std::errc::invalid_argument);
		if (bool(flags & mmap_flags::reserve) && bool(flags & mmap_flags::map_large_pages)) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		if (offset > extent_type(std::numeric_limits<LONGLONG>::max())) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);
		if (size > extent_type(std::numeric_limits<LONGLONG>::max())) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);
		if ((flags & (mmap_flags::reserve | mmap_flags::commit)) == mmap_flags::none)
			flags |= mmap_flags::commit;

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto map_size = endpos(src);
		if (map_size.has_error()) [[unlikely]]
			return map_size.error();

		if (*map_size > offset)
			*map_size -= offset;
		else
			*map_size = 0;
		if (size == 0) /* Make sure not to overflow when casting to size_type. */
			size = size_type(std::min(*map_size, extent_type(std::numeric_limits<size_type>::max())));

		ULONG prot = flags_to_prot(flags), type = 0;
		if (bool(flags & mmap_flags::reserve) || prot == PAGE_NOACCESS)
			type = SEC_RESERVE;
		if (bool(flags & mmap_flags::map_large_pages))
			type = SEC_RESERVE | SEC_LARGE_PAGES;

		const auto page_sizes = get_page_sizes();
		auto page = (type & SEC_LARGE_PAGES) && page_sizes.size() > 1 ? page_sizes[1] : page_sizes[0];
		auto total = size_type(offset & 65'535) + size;
		auto commit = (type & SEC_RESERVE) ? 0 : total;
		auto chunk = LONGLONG(offset & ~65'535);

		void *mem = nullptr;
		auto status = ntapi->NtMapViewOfSection(src.native_handle(), ::GetCurrentProcess(), &mem, 0, commit, &chunk, &total, ViewUnmap, type, prot);
		if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);

		if (bool(flags & mmap_flags::prefault)) /* Prefault manually if required. */
		{
			if (ntapi->PrefetchVirtualMemory)
			{
				auto entry = WIN32_MEMORY_RANGE_ENTRY{mem, total};
				ntapi->PrefetchVirtualMemory(::GetCurrentProcess(), 1, &entry, 0);
			}

			auto vmem = static_cast<volatile std::int8_t *>(mem);
			for (std::size_t i = 0; i < total; i += page)
				(void) vmem[i];
		}
		return mmap_handle(static_cast<std::byte *>(mem), offset & 65'535, size, page, total, flags, src);
	}

	result<void> mmap_handle::reset() noexcept
	{
		if (!empty()) [[likely]]
		{
			const auto &ntapi = ntapi::instance();
			if (ntapi.has_error()) [[unlikely]]
				return ntapi.error();

			if (has_source())
			{
				if (bool(flags() & (mmap_flags::write | mmap_flags::copy)) && bool(flags() & mmap_flags::sync_on_close))
				{
					const auto res = flush({0, 0});
					if (res.has_error()) [[unlikely]]
						return res.error();
				}

				auto status = ntapi->free_mapped_pages(base(), reserved());
				if (is_status_failure(status)) [[unlikely]]
					return status_error_code(status);
			}
			else
			{
				auto status = ntapi->free_virtual_pages(base(), reserved(), MEM_RELEASE);
				if (is_status_failure(status)) [[unlikely]]
					return status_error_code(status);
			}
		}

		operator=(mmap_handle());
		return {};
	}

	result<void> mmap_handle::flush(extent_pair ext) noexcept
	{
		if (!has_source()) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		ext = clamp_extent(ext, reserved());
		if (const auto endp = ext.first + ext.second; endp < ext.first || endp > reserved()) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		ext = over_align_extent(ext, page_size());
		return ntapi->apply_virtual_pages(base() + ext.first, ext.second, MEM_COMMIT, [](auto *addr, auto size) noexcept -> result<void>
		{
			if (!::FlushViewOfFile(addr, size)) [[unlikely]]
				return dos_error_code(::GetLastError());
			else
				return {};
		});
	}
	result<void> mmap_handle::discard(extent_pair ext) noexcept
	{
		if (!has_source()) [[unlikely]]
			return std::make_error_code(std::errc::not_supported);
		ext = clamp_extent(ext, reserved());
		if (const auto endp = ext.first + ext.second; endp < ext.first || endp > reserved()) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		if (ext = under_align_extent(ext, page_size()); ext.second > 0)
			return ntapi->apply_virtual_pages(base() + ext.first, ext.second, MEM_COMMIT, [&](auto *addr, auto size) noexcept -> result<void>
			{
				if (!has_source() && ntapi->DiscardVirtualMemory != nullptr)
				{
					if (!ntapi->DiscardVirtualMemory(addr, size))
					{
						const auto err = ::GetLastError();
						if (err != ERROR_NOT_LOCKED) [[unlikely]]
							return dos_error_code(err);
					}
					return {};
				}
				else if (!::VirtualUnlock(addr, size))
				{
					const auto err = ::GetLastError();
					if (err != ERROR_NOT_LOCKED) [[unlikely]]
						return dos_error_code(err);
				}
				return {};
			});
		return {};
	}

	result<void> mmap_handle::commit(extent_pair ext) noexcept
	{
		ext = clamp_extent(ext, reserved());
		if (const auto endp = ext.first + ext.second; endp < ext.first || endp > reserved()) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		if ((flags() & prot_flags_mask) == mmap_flags::none)
			return ntapi->apply_virtual_pages(base() + ext.first, ext.second, MEM_COMMIT, [](auto *addr, auto size) noexcept -> result<void>
			{
				if (DWORD old = 0; !::VirtualProtect(addr, size, PAGE_NOACCESS, &old)) [[unlikely]]
					return dos_error_code(::GetLastError());
				else
					return {};
			});

		const auto prot = flags_to_prot(flags());
		ext = over_align_extent(ext, page_size());
		if (::VirtualAlloc(base() + ext.first, ext.second, MEM_COMMIT, prot) == nullptr) [[unlikely]]
			return dos_error_code(::GetLastError());
		else
			return {};
	}
	result<void> mmap_handle::decommit(extent_pair ext) noexcept
	{
		ext = clamp_extent(ext, reserved());
		if (const auto endp = ext.first + ext.second; endp < ext.first || endp > reserved()) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		/* Memory is rounded down to avoid de-commiting partial pages. */
		ext = under_align_extent(ext, page_size());
		if (auto status = ntapi->free_virtual_pages(base() + ext.first, ext.second, MEM_DECOMMIT); is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		else
			return {};
	}

	result<void> mmap_handle::zero(extent_pair ext) noexcept
	{
		ext = clamp_extent(ext, reserved());
		if (const auto endp = ext.first + ext.second; endp < ext.first || endp > reserved()) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);

		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		/* Fill extent with zeros manually as windows doesnt have an efficient page-zero without discarding. */
		std::memset(base() + ext.first, 0, ext.second);
		/* Now do same as `discard` except dont fail fot un-backed views. */
		if (!has_source())
			return {};
		if (ext = under_align_extent(ext, page_size()); ext.second > 0)
			return ntapi->apply_virtual_pages(base() + ext.first, ext.second, MEM_COMMIT, [&](auto *addr, auto size) noexcept -> result<void>
			{
				if (ntapi->DiscardVirtualMemory != nullptr)
				{
					if (!ntapi->DiscardVirtualMemory(addr, size))
					{
						const auto err = ::GetLastError();
						if (err != ERROR_NOT_LOCKED) [[unlikely]]
							return dos_error_code(err);
					}
					return {};
				}
				else if (!::VirtualUnlock(addr, size))
				{
					const auto err = ::GetLastError();
					if (err != ERROR_NOT_LOCKED) [[unlikely]]
						return dos_error_code(err);
				}
				return {};
			});
		return {};
	}
	result<void> mmap_handle::prefault(std::span<const extent_pair> exts) noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		if (ntapi->PrefetchVirtualMemory)
		{
			auto buff = ROD_MAKE_BUFFER(WIN32_MEMORY_RANGE_ENTRY, exts.size() * sizeof(WIN32_MEMORY_RANGE_ENTRY));
			for (std::size_t i = 0; i < exts.size(); ++i)
				buff.get()[i] =
				{
					.VirtualAddress = exts[i].first + base(),
					.NumberOfBytes = exts[i].second
				};
			if (!ntapi->PrefetchVirtualMemory(::GetCurrentProcess(), exts.size(), buff.get(), 0)) [[unlikely]]
				return dos_error_code(::GetLastError());
		}
		return {};
	}
}

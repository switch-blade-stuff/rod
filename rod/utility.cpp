/*
 * Created by switchblade on 2023-08-19.
 */

#include "file.hpp"

#ifdef ROD_WIN32
#include "detail/win32/ntapi.hpp"
#endif

namespace rod
{
	std::size_t system_random(void *buff, std::size_t max) noexcept
	{
#if defined(ROD_WIN32)
		if (const auto &ntapi = _win32::ntapi::instance(); ntapi.has_value()) [[likely]]
		{
			const auto bcrypt_close = [&](void *p) { ntapi.value().BCryptCloseAlgorithmProvider(p, 0); };
			static auto algo = [&]() -> result<std::unique_ptr<void, decltype(bcrypt_close)>>
			{
				void *handle = nullptr;
#if 0 /* Don't use TMP for now as it has been shown to cause slowdown, and we only need *good enough* RNG. */
				const auto status = ntapi->BCryptOpenAlgorithmProvider(&handle, L"RNG", _win32::bcrypt_platform_crypto_provider, 0);
#else
				const auto status = ntapi->BCryptOpenAlgorithmProvider(&handle, L"RNG", _win32::bcrypt_primitive_provider, 0);
#endif
				if (status == 0 /* STATUS_SUCCESS */) [[likely]]
					return std::unique_ptr<void, decltype(bcrypt_close)>(handle, bcrypt_close);
				else
					return _win32::status_error_code(status);
			}();
			if (algo.has_value() && !ntapi->BCryptGenRandom(algo->get(), buff, DWORD(max), 0)) [[likely]]
				return DWORD(max);
		}
#elif defined(ROD_POSIX)
		static auto random = []() -> result<fs::file_handle>
		{
			/* Fall back to /dev/random if urandom is not available. */
			if (auto random = fs::file_handle::open({}, "/dev/urandom"); random.has_error()) [[unlikely]]
				return fs::file_handle::open({}, "/dev/random");
			else
				return random;
		}();
		if (random.has_value()) [[likely]]
		{
			auto bytes = byte_buffer(static_cast<std::byte *>(buff), max);
			auto result = read_some_at(*random, {.buffs = {&bytes, 1}, .off = 0});
			if (result.has_value()) [[likely]]
				return result->front().size();
		}
#endif
		return 0;
	}

	std::size_t get_page_size() noexcept
	{
#if defined(ROD_WIN32)
		static const auto value = []()
		{
			auto info = SYSTEM_INFO();
			::GetSystemInfo(&info);
			return info.dwPageSize;
		}();
		return value;
#elif defined(ROD_POSIX)
		static const auto value = getpagesize();
		return std::size_t(value);
#endif
	}
	std::span<const std::size_t> get_page_sizes(bool avail) noexcept
	{
		static const auto [all, user] = []() noexcept -> std::pair<std::vector<std::size_t>, std::vector<std::size_t>>
		{
#if defined(ROD_WIN32)
            try
			{
				std::pair<std::vector<std::size_t>, std::vector<std::size_t>> result = {{get_page_size()}, {get_page_size()}};
				if (const auto GetLargePageMinimum = reinterpret_cast<std::size_t (WINAPI *)()>(::GetProcAddress(::GetModuleHandleW(L"kernel32.dll"), "GetLargePageMinimum")); GetLargePageMinimum != nullptr)
				{
					result.first.push_back(GetLargePageMinimum());

					/* Attempt to enable SeLockMemoryPrivilege */
					if (void *token; ::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token) != 0)
					{
						auto guard = defer_invoke([token]() { ::CloseHandle(token); });
						auto privs = TOKEN_PRIVILEGES{.PrivilegeCount = 1};
						if (::LookupPrivilegeValueW(nullptr, L"SeLockMemoryPrivilege", &privs.Privileges[0].Luid))
						{
							privs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
							if (::AdjustTokenPrivileges(token, false, &privs, 0, nullptr, nullptr) != 0 && ::GetLastError() == ERROR_SUCCESS)
								result.second.push_back(GetLargePageMinimum());
						}
					}
				}
				return result;
			}
			catch (...) { return {}; }
#else
            return {};
#endif
		}();
		return avail ? user : all;
	}
}

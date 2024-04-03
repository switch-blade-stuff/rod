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
		static const auto value = ::getpagesize();
		return std::size_t(value);
#endif
	}
	std::span<const std::size_t> get_page_sizes(bool avail) noexcept
	{
		static const auto fallback = std::array<std::size_t, 1>{get_page_size()};
		static const auto [all, user] = []() noexcept -> std::pair<std::vector<std::size_t>, std::vector<std::size_t>>
		{
			try
			{
				std::pair<std::vector<std::size_t>, std::vector<std::size_t>> result;

#if defined(ROD_WIN32)
				result.second = result.first = {get_page_size()}
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
#elif defined(__FreeBSD__)
				result.first.resize(32);
				if (const auto out = ::getpagesizes(pagesizes.data(), 32); out < 0) [[unlikely]]
					result.first = {get_page_size()};
				else
					result.first.resize(out);
				result.second = result.first;
#elif defined(__APPLE__)
				result.second = result.first = {get_page_size(), std::size_t(2 * 1024 * 1024)};
#elif defined(__linux__)
				result.second = result.first = {get_page_size()};

				auto meminfo = fs::file_handle::open({}, "/proc/meminfo");
				if (meminfo.has_error()) [[unlikely]]
					return result;
				auto buff = std::array<char, 4096>();
				if (auto b = as_bytes(buff.begin(), buff.end()); read_some_at(*meminfo, {.buffs = {&b, 1}, .off = 0}).has_error()) [[unlikely]]
					return result;

				const auto meminfo_str = std::string_view(buff.begin(), buff.end());
				auto hp_num_pos = meminfo_str.find("HugePages_Total:");
				auto hp_len_pos = meminfo_str.find("Hugepagesize:");

				if (hp_num_pos != std::string_view::npos && hp_len_pos != std::string_view::npos)
				{
					constexpr auto whitespace = " \t\r\n";
					hp_num_pos = meminfo_str.find_first_not_of(whitespace, hp_num_pos);
					hp_len_pos = meminfo_str.find_first_not_of(whitespace, hp_len_pos);

					auto hp_num = std::size_t(), hp_len = std::size_t();
					if (hp_num_pos != std::string_view::npos) [[likely]]
						::sscanf(meminfo_str.data() + hp_num_pos, "%zu", &hp_num);
					if (hp_len_pos != std::string_view::npos) [[likely]]
						::sscanf(meminfo_str.data() + hp_len_pos, "%zu", &hp_len);

					if (hp_len != 0)
					{
						result.first.push_back(hp_len * 1024);
						if (hp_num != 0u)
							result.second.push_back(hp_len * 1024);
					}
				}
#else
				result.second = result.first = {get_page_size()}
#endif
				return result;
			}
			catch (...) { return {}; }
		}();

		auto result = std::span(avail ? user : all);
		if (result.empty()) [[unlikely]]
			result = fallback;

		return result;
	}

	std::size_t get_file_buff_size() noexcept
	{
		static const auto result = []() noexcept
		{
			auto buff_size = get_page_size();
			for (auto page_size : get_page_sizes(true))
				if (page_size >= std::size_t(1024 * 1024))
				{
					buff_size = page_size;
					break;
				}
			return buff_size;
		}();
		return result;
	}
}

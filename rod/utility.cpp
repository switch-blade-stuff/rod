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
			if (algo.has_value() && !ntapi->BCryptGenRandom(algo->get(), buff, DWORD(max), 0)) [[unlikely]]
				return DWORD(max);
		}
#elif defined(ROD_POSIX)
		static auto random = []() -> result<file_handle>
		{
			/* Fall back to /dev/random if urandom is not available. */
			if (auto random = open_file("/dev/urandom", open_mode::in); random.has_error()) [[unlikely]]
				return open_file("/dev/random", open_mode::in);
			else
				return random;
		};
		if (random.has_value()) [[likely]]
		{
			auto bytes = basic_file_handle::buffer_type(buff, max);
			auto result = read_some_at(random, bytes, 0);
			if (result.has_value()) [[likely]]
				return *result;
		}
#endif
		return 0;
	}
}

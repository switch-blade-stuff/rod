/*
 * Created by switchblade on 2023-11-03.
 */

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <system_error>
#include <string_view>
#include <memory>
#include <vector>
#include <chrono>

int main(int argc, char *argv[])
{
	if (argc != 3) [[unlikely]]
	{
		std::fprintf(stderr, "Invalid arguments. Expected file path and table name\n");
		return 1;
	}

	auto ntdll = ::LoadLibraryA("ntdll.dll");
	if (ntdll == nullptr) [[unlikely]]
	{
		std::fprintf(stderr, "Cannot load \"ntdll.dll\"\n");
		return 1;
	}

	auto RtlNtStatusToDosError = reinterpret_cast<ULONG (__stdcall *)(_In_ ULONG)>(::GetProcAddress(ntdll, "RtlNtStatusToDosError"));
	if (RtlNtStatusToDosError == nullptr) [[unlikely]]
	{
		std::fprintf(stderr, "Cannot load `RtlNtStatusToDosError` from \"ntdll.dll\"\n");
		return 1;
	}

	auto dst = std::fopen(argv[1], "w+");
	if (dst == nullptr) [[unlikely]]
	{
		std::fprintf(stderr, "Cannot open \"%s\"\n", argv[1]);
		return 1;
	}

	constexpr std::pair<ULONG, ULONG> err_ranges[] = {{0x0000'0000, 0x0000'ffff}, {0x4000'0000, 0x4000'ffff}, {0x8000'0001, 0x8000'ffff}, {0xc000'0001, 0xc000'ffff}};
	constexpr std::size_t buff_size = 32768;

	try
	{
		auto tmp_buffer = std::make_unique<wchar_t[]>(buff_size);
		auto msg_buffer = std::make_unique<char[]>(buff_size);

		std::fprintf(dst, "/*\n * This file contains an auto-generated table of NTSTATUS values mapped to DOS and POSIX error codes.\n */\n\n"
		                  "#define WIN32_LEAN_AND_MEAN\n#include <string_view>\n#include <windows.h>\n\n"
						  "namespace\n{\nstatic const struct{ ULONG status; ULONG dos_err; int posix_err; std::string_view msg; } %s[] = {\n", argv[2]);

		for (const auto &range : err_ranges)
			for (ULONG i = range.first; i < range.second; ++i)
			{
				/* Decode message string from NT status. Assume status code does not exist if FormatMessageW fails. */
				const auto flags = FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
				const auto tmp_length = ::FormatMessageW(flags, ntdll, i, 0, tmp_buffer.get(), buff_size, nullptr);
				if (tmp_length == 0) [[unlikely]]
					continue;
				const auto msg_length = ::WideCharToMultiByte(65001 /* UTF8 */, WC_ERR_INVALID_CHARS, tmp_buffer.get(), int(tmp_length), msg_buffer.get(), buff_size, nullptr, nullptr);
				if (msg_length < 0) [[unlikely]]
					continue;

				/* Decode Win32 & POSIX error codes from NT status. */
				int posix_err = 0;
				const auto win32_err = RtlNtStatusToDosError(i);
				const auto cnd = std::error_code(int(win32_err), std::system_category()).default_error_condition();
				if (cnd.category() == std::generic_category())
					posix_err = cnd.value();

				/* Stip trailing whitespace. */
				auto msg = std::string_view(msg_buffer.get(), msg_length);
				if (!msg.empty())
				{
					auto pos = msg.find_last_not_of("\r\n\t ");
					msg = msg.substr(0, pos + 1);
				}

				/* Write the resulting entry. */
				std::fprintf(dst, "\t{.status = %lu, .dos_err = %lu, .posix_err = %i, .msg = R\"(%.*s)\"},\n", i, win32_err, posix_err, int(msg.size()), msg.data());
			}

		std::fprintf(dst, "};\n}\n");
		std::fclose(dst);
	}
	catch (...)
	{
		std::fclose(dst);
		throw;
	}
}
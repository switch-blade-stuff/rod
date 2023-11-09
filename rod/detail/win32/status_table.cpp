/*
 * Created by switchblade on 2023-11-03.
 */

#include "status_table.hpp"

#if defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#include <string_view>
#include <iostream>
#include <fstream>
#include <memory>
#include <chrono>

constexpr std::pair<ULONG, ULONG> err_ranges[] = {{0x0000'0000, 0x0000'ffff}, {0x4000'0000, 0x4000'ffff}, {0x8000'0001, 0x8000'ffff}, {0xc000'0001, 0xc000'ffff}};
constexpr std::size_t buff_size = 32768;

int main(int argc, char *argv[])
{
	if (argc != 3) [[unlikely]]
	{
		std::cerr << "Invalid arguments. Expected file path and table name\n";
		return 1;
	}

	auto dst = std::fstream(argv[1], std::ios::out | std::ios::trunc);
	dst << "/*\n * This file contains an auto-generated table of NTSTATUS values mapped to DOS and POSIX error codes.\n */\n\n"
	    << "#include <string_view>\n\nnamespace\n{\nstatic const struct { unsigned long status; unsigned long dos_err; int posix_err; std::string_view msg; } "
	    << argv[2] << "[] = {\n";

#if defined(_MSC_VER)
	auto ntdll = ::LoadLibraryA("ntdll.dll");
	auto RtlNtStatusToDosError = reinterpret_cast<ULONG (__stdcall *)(_In_ ULONG)>(::GetProcAddress(ntdll, "RtlNtStatusToDosError"));
	if (ntdll && RtlNtStatusToDosError)
	{
		auto tmp_buffer = std::make_unique<wchar_t[]>(buff_size);
		auto msg_buffer = std::make_unique<char[]>(buff_size);

		for (const auto &range : err_ranges)
			for (ULONG status = range.first; status < range.second; ++status)
			{
				/* Decode message string from NT status. Assume status code does not exist if FormatMessageW fails. */
				const auto flags = FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
				const auto tmp_length = ::FormatMessageW(flags, ntdll, status, 0, tmp_buffer.get(), buff_size, nullptr);
				if (tmp_length == 0) [[unlikely]]
					continue;
				const auto msg_length = ::WideCharToMultiByte(65001 /* UTF8 */, WC_ERR_INVALID_CHARS, tmp_buffer.get(), int(tmp_length), msg_buffer.get(), buff_size, nullptr, nullptr);
				if (msg_length < 0) [[unlikely]]
					continue;

				/* Decode Win32 & POSIX error codes from NT status. */
				int posix_err = 0;
				const auto dos_err = RtlNtStatusToDosError(status);
				const auto cnd = std::error_code(int(dos_err), std::system_category()).default_error_condition();
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
				dst << "\t{.status = " << status
					<< ", .dos_err = " << dos_err
					<< ", .posix_err = " << posix_err
					<< ", .msg = R\"(" << msg
					<< ")\"},\n";
			}

		dst << "};\n}\n";
		return 0;
	}
	std::cerr << "Cannot load function `RtlNtStatusToDosError` from \"ntdll.dll\", falling back to a pre-generated table\n";
#endif

	/* If we are not on windows or NTDLL is not available, copy the pre-generated table. */
	for (const auto &ent : fallback_table)
	{
		dst << "\t{.status = " << ent.status
		    << ", .dos_err = " << ent.dos_err
		    << ", .posix_err = " << ent.posix_err
		    << ", .msg = R\"(" << ent.msg
		    << ")\"},\n";
	}
	dst << "};\n}\n";
}
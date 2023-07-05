/*
 * Created by switchblade on 2023-04-09.
 */

#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#include <windows.h>
#elif defined(__unix__)
#include <unistd.h>
#include <cerrno>
#endif

#include <rod/shared_memory.hpp>

#include <string_view>
#include <string>

#include "common.hpp"

#ifdef _WIN32
static void start_child(int, const char *argv[], std::string_view name) noexcept
{
	STARTUPINFO si = {.cb = sizeof(si)};
	PROCESS_INFORMATION pi = {};

	auto cmd = std::string{argv[0]} + " " + std::string{name};
	if (!CreateProcess(nullptr, cmd.data(), nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi))
	{
		fprintf(stderr, "CreateProcess failed (%lu).\n", GetLastError());
		std::exit(1);
	}
}
#else
static void start_child(int argc, const char *argv[], std::string_view name) noexcept
{
	::execl(argv[0], name.data(), nullptr);
}
#endif

enum status_t
{
	child_ready = 1,
	parent_done = 2,
};

int main(int argc, const char *argv[])
{
	const std::string_view name = argc > 1 ? argv[1] : "rod.tests.shared-memory";
	const auto str_data = std::string_view{"hello, world"};
	const auto size = str_data.size() + sizeof(status_t);

	if (argc > 1)
	{
		const auto shm = rod::shared_memory::open(name.data(), size, rod::shared_memory::nocreate);
		if (!shm.has_value())
		{
			fprintf(stderr, "rod::shared_memory::open failed (%i)", shm.error().value());
			std::exit(1);
		}

		const auto mmap = shm->map(0, size, rod::mmap::read | rod::mmap::write);
		if (!mmap.has_value())
		{
			fprintf(stderr, "rod::shared_memory::map failed (%i)", mmap.error().value());
			std::exit(1);
		}

		auto status = std::atomic_ref<status_t>{*static_cast<status_t *>(mmap->data())};
		auto str = static_cast<const char *>(mmap->data()) + sizeof(status_t);

		status = child_ready;

		while (status == status_t::child_ready);
		if (const auto cmp_res = ::strncmp(str, str_data.data(), str_data.size()); cmp_res != 0)
		{
			fprintf(stderr, "strncmp(%s, %s, %zu) = (%i)", str, str_data.data(), str_data.size(), cmp_res);
			std::exit(1);
		}
	}
	else
	{
		auto shm = rod::shared_memory::open(name.data(), size, rod::shared_memory::nocreate);
		TEST_ASSERT(!shm.has_value());
		shm = rod::shared_memory::open(name.data(), size);
		TEST_ASSERT(shm.has_value());

		auto mmap = shm->map(0, size, rod::mmap::read | rod::mmap::write);
		TEST_ASSERT(mmap.has_value());

		auto status = std::atomic_ref<status_t>{*static_cast<status_t *>(mmap->data())};
		auto str = static_cast<char *>(mmap->data()) + sizeof(status_t);

		status = status_t{};
		start_child(argc, argv, name);

		while (status == status_t{});
		::strncpy(str, str_data.data(), str_data.size());
		TEST_ASSERT(::strncmp(str, str_data.data(), str_data.size()) == 0);

		status = parent_done;
	}
}

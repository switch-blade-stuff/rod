/*
 * Created by switchblade on 2023-04-09.
 */

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#include <windows.h>
#else
#include <cerrno>
#endif

#include <rod/shared_memory.hpp>

#include "common.hpp"

int main()
{
	const auto name = std::string_view{"rod.tests.shared-memory"};
	const auto str0 = std::string_view{"mmap0->mmap1"};
	const auto str1 = std::string_view{"mmap1->mmap0"};

	auto shm0 = rod::shared_memory::open(name.data(), std::max(str0.size(), str1.size()), rod::shared_memory::noreplace);
	TEST_ASSERT(shm0.has_value());

	auto shm1 = rod::shared_memory::open(name.data(), std::max(str0.size(), str1.size()), rod::shared_memory::noreplace);
	TEST_ASSERT(shm1.has_error());
#ifdef _WIN32
	TEST_ASSERT(shm1.error().value() == ERROR_ALREADY_EXISTS);
#else
	TEST_ASSERT(shm1.error().value() == EEXIST);
#endif

	shm1 = rod::shared_memory::open(name.data(), std::max(str0.size(), str1.size()), rod::shared_memory::nocreate);
	TEST_ASSERT(shm1.has_value());

	auto mmap0 = shm0->map(0, 64, rod::mmap::read | rod::mmap::write);
	TEST_ASSERT(mmap0.has_value());
	auto mmap1 = shm1->map(0, 64, rod::mmap::read | rod::mmap::write);
	TEST_ASSERT(mmap0.has_value());

	::strncpy(static_cast<char *>(mmap0->data()), str0.data(), str0.size());
	TEST_ASSERT(::strncmp(static_cast<char *>(mmap1->data()), str0.data(), str0.size()) == 0);
	::strncpy(static_cast<char *>(mmap1->data()), str1.data(), str1.size());
	TEST_ASSERT(::strncmp(static_cast<char *>(mmap0->data()), str1.data(), str1.size()) == 0);
}

/*
 * Created by switchblade on 2023-05-20.
 */

#include <rod/io.hpp>

#include "common.hpp"

const auto path = std::filesystem::path{"test.txt"};
const auto data = std::string_view{"hello, world"};
auto buff = std::string(data.size() * 2, '\0');

void test_file(auto mode)
{
	mode |= rod::basic_file::in | rod::basic_file::out;
	if (mode & rod::basic_file::noreplace)
		std::filesystem::remove(path);

	auto file = rod::basic_file(path, mode);
	TEST_ASSERT(file.is_open());
	TEST_ASSERT(std::filesystem::equivalent(file.path().value(), path));

	{
		const auto write_n = rod::write_some(file, rod::as_byte_buffer(data));
		TEST_ASSERT(file.size().value() == data.size());
		TEST_ASSERT(write_n.value() == data.size());

		auto pos = file.tell().value();
		TEST_ASSERT(pos == data.size());
		pos = file.seek(0, rod::basic_file::beg).value();
		TEST_ASSERT(pos == 0);

		const auto read_n = rod::read_some(file, rod::as_byte_buffer(buff));
		TEST_ASSERT(read_n.value() == file.size().value());
		TEST_ASSERT(read_n.value() == data.size());
		TEST_ASSERT(buff.find(data) == 0);

		pos = file.tell().value();
		TEST_ASSERT(pos == data.size());
	}
	{
		const auto write_n = rod::write_some_at(file, data.size(), rod::as_byte_buffer(data));
		TEST_ASSERT(file.size().value() == data.size() * 2);
		TEST_ASSERT(write_n.value() == data.size());

		std::fill(buff.begin(), buff.end(), '\0');
		const auto read_n = rod::read_some_at(file, 0, rod::as_byte_buffer(buff));
		TEST_ASSERT(buff.find(data) != buff.rfind(data));
		TEST_ASSERT(read_n.value() == file.size().value());
		TEST_ASSERT(read_n.value() == data.size() * 2);
		TEST_ASSERT(buff.find(data) == 0);
	}

	std::filesystem::remove(path);
}
void test_async_file(auto mode)
{
	auto ctx = rod::system_context{};
	auto trd = std::jthread{[&]() { ctx.run(); }};

	mode |= rod::basic_file::in | rod::basic_file::out;
	if (mode & rod::basic_file::noreplace)
		std::filesystem::remove(path);

	auto file = rod::open_file(ctx.get_scheduler(), path, mode).value();
	TEST_ASSERT(file.is_open());
	TEST_ASSERT(std::filesystem::equivalent(file.path().value(), file.path().value()));

	{
		rod::sync_wait(rod::async_write_some(file, rod::as_byte_buffer(data))
		               | rod::then([&](auto n) { TEST_ASSERT(n == data.size()); })
		               | rod::then([&]() { TEST_ASSERT(file.setpos(0)); }));
		rod::sync_wait(rod::async_read_some(file, rod::as_byte_buffer(buff))
		               | rod::then([&](auto n) { TEST_ASSERT(n == file.size().value()); })
		               | rod::then([&]() { TEST_ASSERT(buff.find(data) == 0); }));
	}

	std::filesystem::remove(path);
	ctx.finish();
}

int main()
{
	test_file(rod::basic_file::trunc);
	test_file(rod::basic_file::noreplace);
	test_async_file(rod::basic_file::trunc);
	test_async_file(rod::basic_file::noreplace);
}

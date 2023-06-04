/*
 * Created by switchblade on 2023-05-20.
 */

#include <rod/io.hpp>

#include "common.hpp"

using basic_file_t = rod::basic_file;

const auto path = std::filesystem::path{"test.txt"};
const auto data = std::string_view{"hello, world"};

void test_basic_file(auto mode)
{
	auto buff = std::string(data.size() * 2, '\0');
	rod::system_context ctx = {};
	std::error_code err = {};

	if (mode & basic_file_t::noreplace)
		std::filesystem::remove(path);

	auto file = basic_file_t::open(path, basic_file_t::in | basic_file_t::out | mode, err);
	TEST_ASSERT(!err && file.is_open());

	auto trd = std::jthread{[&]() { ctx.run(); }};
	auto sch = ctx.get_scheduler();
	{
		const auto write_n = rod::write_some(file, rod::as_byte_buffer(data), err);
		TEST_ASSERT(!err && write_n == data.size());

		const auto pos = file.seek(0, basic_file_t::beg, err);
		TEST_ASSERT(!err && pos == 0);

		const auto read_n = rod::read_some(file, rod::as_byte_buffer(buff), err);
		TEST_ASSERT(!err && read_n == data.size());
		TEST_ASSERT(buff.find(data) == 0);
	}
	{
		const auto write_n = rod::write_some_at(file, data.size(), rod::as_byte_buffer(data), err);
		TEST_ASSERT(!err && write_n == data.size());

		std::fill(buff.begin(), buff.end(), '\0');
		const auto read_n = rod::read_some_at(file, 0, rod::as_byte_buffer(buff), err);
		TEST_ASSERT(!err && read_n == data.size() * 2);
		TEST_ASSERT(buff.find(data) != buff.rfind(data));
		TEST_ASSERT(buff.find(data) == 0);
	}
	{
		auto snd = rod::schedule(sch)
		           | rod::then([&]() { file.seek(0, basic_file_t::beg); })
		           | rod::async_write_some(file, rod::as_byte_buffer(data))
		           | rod::then([](auto n) { TEST_ASSERT(n == data.size()); })
		           | rod::then([&]() { file.seek(0, basic_file_t::beg); })
		           | rod::async_read_some(file, rod::as_byte_buffer(buff))
		           | rod::then([](auto n) { TEST_ASSERT(n == data.size() * 2); })
		           | rod::then([&]() { TEST_ASSERT(buff.find(data) == 0); });
		rod::sync_wait(snd);
	}
	{
		auto snd = rod::schedule_write_some_at(sch, file, data.size(), rod::as_byte_buffer(data))
		           | rod::then([](auto n) { TEST_ASSERT(n == data.size()); })
		           | rod::async_read_some_at(file, 0, rod::as_byte_buffer(buff))
		           | rod::then([](auto n) { TEST_ASSERT(n == data.size() * 2); })
		           | rod::then([&]() { TEST_ASSERT(buff.find(data) != buff.rfind(data)); })
		           | rod::then([&]() { TEST_ASSERT(buff.find(data) == 0); });
		rod::sync_wait(snd);
	}
	//std::filesystem::remove(path);
	ctx.finish();
}

int main()
{
	test_basic_file(basic_file_t::trunc);
	test_basic_file(basic_file_t::noreplace);
}

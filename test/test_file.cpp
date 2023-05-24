/*
 * Created by switchblade on 2023-05-20.
 */

#include <rod/file.hpp>

#include "common.hpp"

using basic_file_t = rod::basic_file;
using file_t = rod::file;

const auto path = std::filesystem::path{"test.txt"};
const auto data = std::string_view{"hello, world"};

void test_basic_file(auto mode)
{
	auto buff = std::string(data.size() * 2, '\0');
	std::error_code err;

	if (mode & basic_file_t::noreplace)
		std::filesystem::remove(path);

	auto file = basic_file_t::open(path, basic_file_t::in | basic_file_t::out | mode, err);
	TEST_ASSERT(!err && file);
	{
		const auto write_n = rod::write_some(file, data, err);
		TEST_ASSERT(!err && write_n == data.size());

		const auto pos = file.seek(0, basic_file_t::beg, err);
		TEST_ASSERT(!err && pos == 0);

		const auto read_n = rod::read_some(file, buff, err);
		TEST_ASSERT(!err && read_n == data.size());
		TEST_ASSERT(buff.find(data) == 0);
	}
	{
		const auto write_n = rod::write_some_at(file, data.size(), data, err);
		TEST_ASSERT(!err && write_n == data.size());

		const auto read_n = rod::read_some_at(file, 0, buff, err);
		TEST_ASSERT(!err && read_n == data.size() * 2);
		TEST_ASSERT(buff.find(data) != buff.rfind(data));
		TEST_ASSERT(buff.find(data) == 0);
	}
	std::filesystem::remove(path);
}

int main()
{
	test_basic_file(basic_file_t::trunc);
	test_basic_file(basic_file_t::noreplace);
}

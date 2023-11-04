/*
 * Created by switchblade on 2023-05-20.
 */

#include <rod/io.hpp>

#include "common.hpp"

int main()
{
	auto curr_path = rod::fs::current_path().value();
	auto curr_dir = rod::fs::path_handle::open({}, curr_path).value();
	TEST_ASSERT(curr_dir.is_open());

	auto g = rod::defer_invoke([&]()
	{
		auto removed = rod::fs::remove_all(curr_dir, "dir-a").value();
		TEST_ASSERT(removed == 4);
	});

	auto created = rod::fs::create_directories({}, curr_path / "dir-a/dir-b").value();
	TEST_ASSERT(created == 2);
	created = rod::fs::create_directories(curr_dir, "dir-a/dir-b").value();
	TEST_ASSERT(created == 0);

	auto file_src = rod::fs::file_handle::open(curr_dir, "dir-a/dir-b/test.txt", rod::fs::file_flags::read | rod::fs::file_flags::write /*| rod::fs::file_flags::unlink_on_close*/, rod::fs::open_mode::always).value();
	auto file_dst = rod::fs::file_handle::open(curr_dir, "dir-a/test.txt", rod::fs::file_flags::read | rod::fs::file_flags::write /*| rod::fs::file_flags::unlink_on_close*/, rod::fs::open_mode::always).value();

	auto iter = rod::fs::directory_iterator::from_path(curr_dir, "dir-a").value();
	TEST_ASSERT(std::distance(iter.begin(), iter.end()) == 2);
	iter = rod::fs::directory_iterator::from_path(curr_dir, "dir-a/dir-b").value();
	TEST_ASSERT(std::distance(iter.begin(), iter.end()) == 1);

	auto str_src = std::string_view("hello, world");
	auto str_dst = std::string(str_src.size(), 0);
	auto buff_src = rod::as_bytes(str_src);
	auto buff_dst = rod::as_bytes(str_dst);

	auto write_res = rod::write_some_at(file_src, {.buffs = {&buff_src, 1}, .off = 0}).value();
	TEST_ASSERT(write_res.front().size() == buff_src.size());
	auto clone_res = rod::clone_extents_to(file_src, {.extent = {0, -1}, .dst = file_dst, .off = 0, .emulate = true}).value();
	TEST_ASSERT(clone_res.second == buff_src.size());

	auto read_res = rod::read_some_at(file_dst, {.buffs = {&buff_dst, 1}, .off = 0}).value();
	TEST_ASSERT(read_res.front().size() == buff_dst.size() && str_dst == str_src);
}

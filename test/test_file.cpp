/*
 * Created by switchblade on 2023-05-20.
 */

#include <rod/io.hpp>

#include "common.hpp"

int main()
{
	auto &tmp_file_dir = rod::fs::temp_file_directory();
	TEST_ASSERT(tmp_file_dir.is_open());
	auto &tmp_pipe_dir = rod::fs::temp_pipe_directory();
	TEST_ASSERT(tmp_pipe_dir.is_open());

	auto curr_path = rod::fs::current_path().value();
	auto curr_dir = rod::fs::path_handle::open({}, curr_path).value();
	TEST_ASSERT(curr_dir.is_open());

	auto g = rod::defer_invoke([&]()
	{
		auto removed = rod::fs::remove_all(curr_dir, "dir-a").value();
		TEST_ASSERT(removed == 4);
		removed = rod::fs::remove_all(curr_dir, "dir-d").value();
		TEST_ASSERT(removed == 3);
		removed = rod::fs::remove_all(curr_dir, "dir-e").value();
		TEST_ASSERT(removed == 3);
#ifndef ROD_WIN32
		removed = rod::fs::remove_all(curr_dir, "dir-c").value();
		TEST_ASSERT(removed == 3);
#endif
	});

	auto created = rod::fs::create_directories({}, curr_path / "dir-a/dir-b").value();
	TEST_ASSERT(created == 2);
	created = rod::fs::create_directories(curr_dir, "dir-a/dir-b").value();
	TEST_ASSERT(created == 0);

	auto file_src = rod::fs::file_handle::open(curr_dir, "dir-a/dir-b/test.txt", rod::fs::file_flags::readwrite /*| rod::fs::file_flags::unlink_on_close*/, rod::fs::open_mode::always).value();
	auto file_dst = rod::fs::file_handle::open(curr_dir, "dir-a/test.txt", rod::fs::file_flags::readwrite /*| rod::fs::file_flags::unlink_on_close*/, rod::fs::open_mode::always).value();

	auto iter = rod::fs::directory_iterator::from_path(curr_dir, "dir-a").value();
	TEST_ASSERT(std::distance(iter.begin(), iter.end()) == 2);
	iter = rod::fs::directory_iterator::from_path(curr_dir, "dir-a/dir-b").value();
	TEST_ASSERT(std::distance(iter.begin(), iter.end()) == 1);

	auto str_src = std::string_view("hello, world.");
	auto str_dst = std::string(str_src.size(), 0);
	auto buff_src = rod::as_bytes(str_src);
	auto buff_dst = rod::as_bytes(str_dst);

	auto write_res = rod::write_some_at(file_src, {.buffs = {&buff_src, 1}, .off = 0}).value();
	TEST_ASSERT(write_res.front().size() == buff_src.size());
	auto clone_res = rod::clone_extents_to(file_src, {.extent = {-1, -1}, .dst = file_dst}).value();
	TEST_ASSERT(clone_res.second == buff_src.size());

	auto read_res = rod::read_some_at(file_dst, {.buffs = {&buff_dst, 1}, .off = 0}).value();
	TEST_ASSERT(read_res.front().size() == buff_dst.size() && str_dst == str_src);

	auto copied = rod::fs::copy_all(curr_dir, "dir-a", curr_dir, "dir-c", rod::fs::copy_mode::files | rod::fs::copy_mode::directories).value();
	TEST_ASSERT(copied == 3);

	auto file_copy = rod::fs::file_handle::open(curr_dir, "dir-c/test.txt", rod::fs::file_flags::read).value();
	TEST_ASSERT(file_copy.is_open());
	read_res = rod::read_some_at(file_copy, {.buffs = {&buff_dst, 1}, .off = 0}).value();
	TEST_ASSERT(read_res.front().size() == buff_dst.size() && str_dst == str_src);

	rod::fs::link(file_copy, curr_dir, "dir-c/test2.txt").value();
	{
		auto file_link = rod::fs::file_handle::open(curr_dir, "dir-c/test2.txt", rod::fs::file_flags::read | rod::fs::file_flags::unlink_on_close).value();
		TEST_ASSERT(file_link.is_open());
		read_res = rod::read_some_at(file_link, {.buffs = {&buff_dst, 1}, .off = 0}).value();
		TEST_ASSERT(read_res.front().size() == buff_dst.size() && str_dst == str_src);
	}
	rod::close(file_copy);

	auto dir_copy = rod::fs::directory_handle::open(curr_dir, "dir-c", rod::fs::file_flags::readwrite).value();
	rod::fs::relink(dir_copy, curr_dir, "dir-d").value();
	{
		auto dir_link = rod::fs::directory_handle::open(curr_dir, "dir-d", rod::fs::file_flags::read).value();
		auto file_link = rod::fs::file_handle::open(dir_link, "test.txt", rod::fs::file_flags::read).value();
		TEST_ASSERT(file_link.is_open());
		read_res = rod::read_some_at(file_link, {.buffs = {&buff_dst, 1}, .off = 0}).value();
		TEST_ASSERT(read_res.front().size() == buff_dst.size() && str_dst == str_src);
	}

	copied = rod::fs::copy_all(curr_dir, "dir-a", curr_dir, "dir-e", rod::fs::copy_mode::directories).value();
	TEST_ASSERT(copied == 2);

	copied = rod::fs::copy(curr_dir, "dir-a/test.txt", curr_dir, "dir-e/test.txt", rod::fs::copy_mode::files | rod::fs::copy_mode::create_hardlinks).value();
	TEST_ASSERT(copied == 1);

	file_copy = rod::fs::file_handle::open(curr_dir, "dir-e/test.txt", rod::fs::file_flags::read).value();
	TEST_ASSERT(file_copy.is_open());
	read_res = rod::read_some_at(file_copy, {.buffs = {&buff_dst, 1}, .off = 0}).value();
	TEST_ASSERT(read_res.front().size() == buff_dst.size() && str_dst == str_src);

	auto src = rod::mmap_source::open(file_src).value();
	auto map = rod::mmap_handle::map(src, 0, 0, rod::mmap_flags::readwrite | rod::mmap_flags::prefault).value();
	TEST_ASSERT(std::memcmp(map.data(), str_src.data(), str_src.size()) == 0);

	str_src = "hello, world!";
	std::memcpy(map.data(), str_src.data(), str_src.size());
	map.flush({}).value();
	TEST_ASSERT(std::memcmp(map.data(), str_src.data(), str_src.size()) == 0);

	read_res = rod::read_some_at(file_src, {.buffs = {&buff_dst, 1}, .off = 0}).value();
	TEST_ASSERT(read_res.front().size() == buff_dst.size() && str_dst == str_src);

#ifndef ROD_WIN32
	copied = rod::fs::copy_all(curr_dir, "dir-e", curr_dir, "dir-c", rod::fs::copy_mode::files | rod::fs::copy_mode::directories | rod::fs::copy_mode::create_symlinks).value();
	TEST_ASSERT(copied == 2);

	copied = rod::fs::copy(curr_dir, "dir-a/test.txt", curr_dir, "dir-e/test.txt", rod::fs::copy_mode::files | rod::fs::copy_mode::create_hardlinks).value();
	TEST_ASSERT(copied == 1);

	file_copy = rod::fs::file_handle::open(curr_dir, "dir-e/test.txt", rod::fs::file_flags::read).value();
	TEST_ASSERT(file_copy.is_open());
	read_res = rod::read_some_at(file_copy, {.buffs = {&buff_dst, 1}, .off = 0}).value();
	TEST_ASSERT(read_res.front().size() == buff_dst.size() && str_dst == str_src);
#endif
}

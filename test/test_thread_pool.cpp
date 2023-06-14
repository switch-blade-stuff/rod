/*
 * Created by switch_blade on 2023-06-14.
 */

#include <rod/scheduling.hpp>
#include <set>

#include "common.hpp"

int main()
{
	rod::thread_pool pool;
	auto sch = pool.get_scheduler();

	TEST_ASSERT(pool.size() != 0);

	std::set<std::thread::id> workers;
	const auto main_tid = std::this_thread::get_id();

	rod::sync_wait(rod::schedule(sch) | rod::bulk(pool.size(), [&](auto) { workers.emplace(std::this_thread::get_id()); }));
	rod::sync_wait(rod::schedule(sch) | rod::then([&]() { workers.emplace(std::this_thread::get_id()); }));

	TEST_ASSERT(!workers.contains(main_tid));
	TEST_ASSERT(!workers.empty());

	pool.finish();
}
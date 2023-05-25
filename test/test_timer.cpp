/*
 * Created by switchblade on 2023-05-20.
 */

#include <rod/io.hpp>

#include "common.hpp"

int main()
{
	rod::in_place_stop_source src;
	rod::epoll_context ctx;
	auto trd = std::jthread{[&]() { ctx.run(src.get_token()); }};
	auto sch = ctx.get_scheduler();

	auto start = sch.now();
	rod::sync_wait(rod::schedule_in(sch, std::chrono::milliseconds{50}));

	TEST_ASSERT((sch.now() - start) >= std::chrono::milliseconds{50});
	src.request_stop();
}

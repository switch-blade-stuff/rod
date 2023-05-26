/*
 * Created by switchblade on 2023-05-20.
 */

#include <string_view>
#include <rod/io.hpp>

#include "common.hpp"

using namespace std::chrono_literals;

int main()
{
	auto async_in = rod::async_file::open("/dev/stdin", rod::async_file::in | rod::async_file::out);

	rod::in_place_stop_source src;
	rod::epoll_context ctx;
	auto trd = std::jthread{[&]() { ctx.run(src.get_token()); }};
	auto sch = ctx.get_scheduler();

	{
		const auto start = sch.now();
		rod::sync_wait(rod::schedule_in(sch, 50ms) | rod::then([&]() { TEST_ASSERT((sch.now() - start) >= 50ms); }));
	}
	{
		std::array<char, 13> buff = {};
		//auto snd_read = rod::schedule(sch) | rod::async_read_some(async_in, std::span{buff});
		//auto snd_write = rod::schedule_in(sch, 10ms) | rod::async_write_some(async_in, std::string_view{"hello, world\n"});

		auto snd0 = rod::transfer(rod::just(), sch);
		rod::sync_wait(rod::async_write_some(snd0, async_in, std::span{buff}));
	}
	src.request_stop();
}

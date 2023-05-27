/*
 * Created by switchblade on 2023-05-20.
 */

#include <string_view>
#include <rod/io.hpp>
#include <fcntl.h>

#include "common.hpp"

using namespace std::chrono_literals;

int main()
{
	rod::in_place_stop_source src;
	rod::epoll_context ctx;
	auto trd = std::jthread{[&]() { ctx.run(src.get_token()); }};
	auto sch = ctx.get_scheduler();

	{
		const auto start = sch.now();
		rod::sync_wait(rod::schedule_in(sch, 50ms) | rod::then([&]() { TEST_ASSERT((sch.now() - start) >= 50ms); }));
	}
	{
		int pipe_fd[2] = {-1, -1};
		::pipe2(pipe_fd, O_NONBLOCK | O_CLOEXEC);

		std::array<char, 13> buff = {};
		const std::string_view data = "hello, world";
		auto snd_read = rod::schedule_read_some(sch, pipe_fd[0], rod::as_byte_buffer(buff)) | rod::then([&](auto n) { TEST_ASSERT((std::string_view{buff.data(), n} == data)); });
		auto snd_write = rod::schedule_write_some(sch, pipe_fd[1], rod::as_byte_buffer(data)) | rod::then([&](auto n) { TEST_ASSERT(n == data.size()); });
		rod::sync_wait(rod::when_all(snd_read, snd_write));

		close(pipe_fd[0]);
		close(pipe_fd[1]);
	}

	{
		auto snd0 = rod::schedule_in(sch, 50ms) | rod::then([&]() { src.request_stop(); });
		auto snd1 = rod::schedule_in(sch, 100ms) | rod::with_stop_token(src.get_token()) | rod::then([]() { std::terminate(); }) | rod::upon_stopped([]() {});
		auto snd2 = rod::schedule_in(sch, 100ms) | rod::with_stop_token(src.get_token()) | rod::then([]() { std::terminate(); }) | rod::upon_stopped([]() {});

		rod::sync_wait(rod::when_all(snd1, snd2, snd0));
	}
}

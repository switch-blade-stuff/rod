/*
 * Created by switchblade on 2023-06-03.
 */

#pragma once

#ifdef __unix__
#include <string_view>
#include <fcntl.h>
#endif

#include <rod/task.hpp>
#include <rod/io.hpp>

#include "common.hpp"

inline void test_io_context(auto &&ctx)
{
	using namespace std::chrono_literals;

	auto trd = std::jthread{[&]() { ctx.run(); }};
	auto sch = ctx.get_scheduler();
	{
		const auto start = sch.now();
		rod::sync_wait(rod::schedule_after(sch, 50ms) | rod::then([&]() { TEST_ASSERT((sch.now() - start) >= 50ms); }));
	}
#ifdef __unix__
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
#endif
	{
		rod::in_place_stop_source src;
		auto snd0 = rod::schedule_after(sch, 50ms) | rod::then([&]() { src.request_stop(); });
		auto snd1 = rod::schedule_after(sch, 100ms) | rod::with_stop_token(src.get_token()) | rod::then([]() { std::terminate(); });
		auto snd2 = rod::schedule_after(sch, 100ms) | rod::with_stop_token(src.get_token()) | rod::then([]() { std::terminate(); });

		rod::sync_wait(rod::when_all(snd1, snd2, snd0));
	}
#ifdef ROD_HAS_COROUTINES
	{
		rod::sync_wait([&]() -> rod::task<> { co_await rod::schedule_after(sch, 50ms); }());
	}
#endif
	ctx.finish();
}

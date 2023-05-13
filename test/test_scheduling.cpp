/*
 * Created by switchblade on 2023-04-09.
 */

#include <rod/scheduling.hpp>
#include <rod/task.hpp>

#include "common.hpp"

int main()
{
	rod::sync_wait([]() -> rod::task<>
	{
		struct test_error : std::exception {};
		auto snd = rod::just(1) | rod::then([](int i)
		{
			TEST_ASSERT(i == 1);
			throw test_error{};
		}) | rod::upon_error([&](const std::exception_ptr &e)
		{
			TEST_ASSERT(e);
			try { std::rethrow_exception(e); }
			catch (test_error &) {}
		});

		co_await snd;
	}());
}
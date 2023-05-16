/*
 * Created by switchblade on 2023-04-09.
 */

#include <rod/scheduling.hpp>
#include <rod/task.hpp>

#include "common.hpp"

int main()
{
	rod::sync_wait([]() noexcept -> rod::task<>
	{
		struct test_error : std::exception {};
		auto snd_base = rod::just(1) | rod::then([](int i)
		{
			TEST_ASSERT(i == 1);
			throw test_error{};
		}) | rod::upon_error([](const std::exception_ptr &e)
		{
			TEST_ASSERT(e);
			try { std::rethrow_exception(e); }
			catch (test_error &) {}
		}) | rod::let_value([](){ return rod::just(1); })
		   | rod::bulk(5, [](int i, int &j) { j += i; })
		   | rod::split();

		static_assert(std::same_as<decltype(rod::split(snd_base)), decltype(snd_base)>);

		auto snd0 = snd_base | rod::then([](int i) { TEST_ASSERT(i == 11); });
		auto snd1 = snd_base | rod::then([](int i) { return i + 1; });
		auto snd2 = snd_base | rod::then([](int i) { return -i; });

		co_await snd0;
		TEST_ASSERT(co_await snd1 == 12);
		TEST_ASSERT(co_await snd2 == -11);
	}());
}
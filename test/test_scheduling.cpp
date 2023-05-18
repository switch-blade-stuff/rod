/*
 * Created by switchblade on 2023-04-09.
 */

#include <rod/scheduling.hpp>
#include <rod/task.hpp>

#include "common.hpp"

int main()
{
	struct test_error : std::exception {};
	auto base_snd = rod::just_invoke([](int i)
	{
		TEST_ASSERT(i == 1);
		throw test_error{};
	}, 1) | rod::upon_error([](const std::exception_ptr &e)
	{
		TEST_ASSERT(e);
		try { std::rethrow_exception(e); }
		catch (test_error &) {}
	}) | rod::let_value([](){ return rod::just(1); })
	   | rod::bulk(5, [](int i, int &j) { j += i; })
	   | rod::split();

	static_assert(std::same_as<decltype(rod::split(base_snd)), decltype(base_snd)>);

	auto snd0 = base_snd | rod::then([](int i) { TEST_ASSERT(i == 11); });
	auto snd1 = base_snd | rod::then([](int i) { return i + 1; });
	auto snd2 = base_snd | rod::then([](int i) { return -i; });
	auto final_snd = rod::when_all(snd0, snd1, snd2) | rod::then([](int i, int j) { TEST_ASSERT(i == 12 && j == -11); });

#ifdef ROD_HAS_COROUTINES
	rod::sync_wait([&]() noexcept -> rod::task<> { co_await final_snd; }());
#else
	rod::sync_wait(final_snd);
#endif

	rod::start_detached(rod::ensure_started(rod::just(1)) | rod::then([](int i) { TEST_ASSERT(i == 1); }));
}
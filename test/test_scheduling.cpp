/*
 * Created by switchblade on 2023-04-09.
 */

#include <thread>

#include <rod/scheduling.hpp>
#include <rod/task.hpp>

#include "common.hpp"

int main()
{
	{
		struct test_error : std::exception {};
		auto base_snd = rod::just_invoke([](int i)
		{
			TEST_ASSERT(i == 1);
			ROD_THROW(test_error{});
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
		auto final_snd = rod::when_all(snd0, snd1, snd2) | rod::then([](int i, int j) { TEST_ASSERT(i == 12 && j == -11); })
		                 | rod::let_error([](const auto &e) { return rod::just_error(e); }); /* Copy by-reference errors. */

		static_assert(rod::sender_of<decltype(final_snd), rod::set_value_t()>);
		static_assert(rod::sender_of<decltype(final_snd), rod::set_error_t(std::exception_ptr)>);

#ifdef ROD_HAS_COROUTINES
		rod::sync_wait([&]() noexcept -> rod::task<> { co_await final_snd; }());
#else
		rod::sync_wait(final_snd);
#endif
	}
	{
		const auto snd = rod::just(std::make_shared<int>(1)) | rod::then([](const auto &p) { TEST_ASSERT(p && *p == 1); });
		rod::sync_wait(snd);
		rod::sync_wait(snd);
	}
	{
		using namespace std::chrono_literals;

		rod::run_loop loop;
		auto trd = std::jthread{[&]() { loop.run(); }};
		auto sch = loop.get_scheduler();

		const auto start = sch.now();
		const auto timeout = start + 50ms;
		rod::sync_wait(rod::transfer_when_all(sch, rod::schedule_at(sch, timeout), rod::on(sch, rod::just())) | rod::then([&]() { TEST_ASSERT(sch.now() >= timeout); }));
		loop.finish();
	}
	{
		std::size_t loops = 0;
		rod::sync_wait(rod::recurse(rod::just() | rod::then([&]() { return ++loops; }), [](auto i) { return i < 5; }));

		TEST_ASSERT(loops == 5);
	}
}

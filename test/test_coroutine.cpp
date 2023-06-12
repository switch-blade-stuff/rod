/*
 * Created by switchblade on 2023-04-09.
 */

#include <rod/generator.hpp>
#include <rod/task.hpp>

#include "common.hpp"

#ifdef ROD_HAS_COROUTINES

template<typename T>
inline rod::task<T> async_return(T value) { co_return value; };

struct test_coroutine
{
	struct promise_type
	{
		test_coroutine get_return_object() { return {std::coroutine_handle<promise_type>::from_promise(*this)}; }

		std::suspend_always initial_suspend() noexcept { return {}; }
		std::suspend_always final_suspend() noexcept { return {}; }

		void unhandled_exception() {}
		void return_void() {}
	};

	~test_coroutine() { (_handle.resume(), _handle.destroy()); }

	std::coroutine_handle<promise_type> _handle;
};

int main()
{
	[]() -> test_coroutine { TEST_ASSERT((co_await async_return(0)) == 0); }();

	{
		int i = 0, j = 5;
		auto g = []<typename T>(T i, T j) -> rod::generator<T>
		{
			for (; i < j; ++i) co_yield i;
		}(i, j);

		for (auto curr = g.begin(); curr != g.end(); ++curr, ++i)
			TEST_ASSERT(*curr == i);
	}

	[]() -> test_coroutine
	{
		int i = 0, j = 5;
		auto g = []<typename T>(T i, T j) -> rod::generator_task<T>
		{
			for (; i < j; ++i) co_yield co_await async_return(i);
		}(i, j);

		for (auto curr = co_await g.begin(); curr != g.end(); co_await ++curr, ++i)
			TEST_ASSERT(*curr == i);
	}();
}
#endif

/*
 * Created by switchblade on 2023-04-09.
 */

#include <memory_resource>
#include <array>

#include <rod/generator.hpp>
#include <rod/task.hpp>

#include "common.hpp"

#ifdef ROD_HAS_COROUTINES
template<typename T>
inline rod::task<T> async_return(T value) { co_return value; }
template<typename T, typename Alloc>
inline rod::task<T> async_return(std::allocator_arg_t, Alloc &&, T value) { co_return value; }

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
#endif

int main()
{
#ifdef ROD_HAS_COROUTINES
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

	std::array<std::byte, 4096> buff = {};
	{
		std::pmr::monotonic_buffer_resource res{buff.data(), buff.size()};
		[&]() -> test_coroutine { TEST_ASSERT((co_await async_return(std::allocator_arg, std::pmr::polymorphic_allocator(&res), 0)) == 0); }();
	}
	{
		std::pmr::monotonic_buffer_resource res{buff.data(), buff.size()};

		int i = 0, j = 5;
		auto g = []<typename T, typename Alloc>(std::allocator_arg_t, Alloc &&, T i, T j) -> rod::generator<T>
		{
			for (; i < j; ++i) co_yield i;
		}(std::allocator_arg, std::pmr::polymorphic_allocator(&res), i, j);

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

	auto test_snd = [](int v) -> rod::sender_of<rod::set_value_t(std::variant<int, std::exception_ptr>)> auto
	{
		return [](int v) -> rod::task<int> { if (v >= 0) co_return v; else ROD_THROW(-v); }(v)
				| rod::upon_value([](auto v){ return std::variant<int, std::exception_ptr>{v}; })
				| rod::upon_error([](auto v){ return std::variant<int, std::exception_ptr>{v}; });
	};

	rod::sync_wait(test_snd(-1) | rod::then([](auto &&v){ TEST_ASSERT(v.index() == 1); }));
	rod::sync_wait(test_snd(1) | rod::then([](auto &&v){ TEST_ASSERT(v.index() == 0); }));
#endif
}

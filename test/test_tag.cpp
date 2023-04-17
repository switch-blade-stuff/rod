/*
 * Created by switchblade on 2023-04-09.
 */

#include <rod/tag.hpp>

#include "common.hpp"

inline constexpr struct bar_t
{
	template<typename T>
	constexpr bool operator()(T &&obj) const noexcept;
} bar = {};

template<typename T>
constexpr bool bar_t::operator()(T &&obj) const noexcept { return rod::tag_invoke(bar, std::forward<T>(obj)); }

struct foo
{
	friend constexpr bool tag_invoke(rod::tag_t<bar>, const foo &f) noexcept { return f.value; }

	bool value;
};

int main()
{
	const auto foo0 = foo{true};
	const auto foo1 = foo{false};

	TEST_ASSERT(bar(foo0) == foo0.value);
	TEST_ASSERT(bar(foo1) == foo1.value);
}
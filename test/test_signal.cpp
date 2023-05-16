/*
 * Created by switchblade on 2023-05-15.
 */

#include <rod/signal.hpp>

#include "common.hpp"

int main()
{
	rod::delegate_signal<int()> signal;
	auto sink = rod::sink{signal};
	int invoked = 0;

	sink += [&]{ return ++invoked; };
	signal.emit();
	TEST_ASSERT(invoked == 1);

	invoked = 0;
	sink += [&] { return ++invoked; };
	for (auto i : signal.generate())
		TEST_ASSERT(i == invoked);
	TEST_ASSERT(invoked == 2);
}
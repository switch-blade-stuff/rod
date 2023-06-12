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

	auto l0 = sink += [&]{ return ++invoked; };
	signal.emit();
	TEST_ASSERT(invoked == 1);

	invoked = 0;
	auto l1 = sink += [&] { return ++invoked; };
#ifdef ROD_HAS_COROUTINES
	for (auto i : signal.generate())
		TEST_ASSERT(i == invoked);
#else
	signal.emit();
#endif
	TEST_ASSERT(invoked == 2);

	sink -= l1;
	signal.emit();
	TEST_ASSERT(invoked == 3);

	l1 = sink += [&] { return ++invoked; };
	signal.emit();
	TEST_ASSERT(invoked == 5);

	sink -= l0;
	signal.emit();
	TEST_ASSERT(invoked == 6);

	sink -= l1;
	signal.emit();
	TEST_ASSERT(invoked == 6);
}
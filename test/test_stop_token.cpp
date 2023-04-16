/*
 * Created by switchblade on 2023-04-09.
 */

#include <rod/scheduling.hpp>

#include "common.hpp"

int main()
{
	rod::in_place_stop_source src;

	TEST_ASSERT(src.stop_possible());
	TEST_ASSERT(!src.stop_requested());

	bool stopped = false;
	const auto tok = src.get_token();
	const auto cb = rod::in_place_stop_callback{tok, [&](){ stopped = true; }};

	TEST_ASSERT(!src.stop_requested());
	TEST_ASSERT(!tok.stop_requested());
	TEST_ASSERT(!stopped);

	TEST_ASSERT(src.request_stop());
	TEST_ASSERT(src.stop_requested());
	TEST_ASSERT(tok.stop_requested());
	TEST_ASSERT(stopped);
}
/*
 * Created by switchblade on 2023-05-21.
 */

#ifdef __unix__

#include "monotonic_clock.hpp"

rod::monotonic_clock::time_point rod::monotonic_clock::now() noexcept
{
	::timespec ts = {};
	::clock_gettime(CLOCK_MONOTONIC, &ts);
	return {ts.tv_sec, ts.tv_nsec};
}
#endif

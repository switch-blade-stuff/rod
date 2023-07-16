/*
 * Created by switchblade on 2023-05-21.
 */

#include "monotonic_clock.hpp"

#ifdef ROD_POSIX

rod::monotonic_clock::time_point rod::monotonic_clock::now() noexcept
{
	::timespec ts = {};
	::clock_gettime(CLOCK_MONOTONIC, &ts);
	return {ts.tv_sec, ts.tv_nsec};
}
#endif

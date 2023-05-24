/*
 * Created by switchblade on 2023-05-21.
 */

#ifdef __unix__

#include "monotonic_clock.hpp"

#include <ctime>

ROD_TOPLEVEL_NAMESPACE_OPEN
rod::monotonic_clock::time_point rod::monotonic_clock::now() noexcept
{
	::timespec ts = {};
	::clock_gettime(CLOCK_MONOTONIC, &ts);
	return {ts.tv_sec, ts.tv_nsec};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

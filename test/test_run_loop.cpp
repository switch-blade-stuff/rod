/*
 * Created by switchblade on 2023-04-09.
 */

#include <rod/run_loop.hpp>

#include "common.hpp"

int main()
{
	rod::run_loop loop;

	auto sch = loop.get_scheduler();
	auto snd = rod::transfer_just(sch, 1);

	loop.run();
	/*rod::sync_wait(snd);*/
}
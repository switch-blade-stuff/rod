/*
 * Created by switchblade on 2023-04-09.
 */

#include <rod/run_loop.hpp>
#include <rod/signal.hpp>

#include "common.hpp"

int main()
{
	rod::run_loop loop;

	rod::function_signal<void()> s;
	rod::sink{s} += []() {};
	s.emit();
}
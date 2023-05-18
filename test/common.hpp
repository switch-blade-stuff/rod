/*
 * Created by switchblade on 2023-03-30.
 */

#pragma once

#include <exception>
#include <utility>

#include <rod/utility.hpp>

#ifdef ROD_TOPLEVEL_NAMESPACE
using namespace ROD_TOPLEVEL_NAMESPACE;
#endif

#ifdef NDEBUG
#define TEST_ASSERT(cnd) do { if (!(cnd)) std::terminate(); } while (false)
#else
#include <cassert>
#define TEST_ASSERT(cnd) assert((cnd))
#endif
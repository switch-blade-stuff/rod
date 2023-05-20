/*
 * Created by switchblade on 2023-05-06.
 */

#pragma once

#include "api.hpp"

/* MSVC does not support standard no_unique_address */
#ifdef _MSC_VER
#define ROD_NO_UNIQUE_ADDRESS msvc::no_unique_address
#else
#define ROD_NO_UNIQUE_ADDRESS no_unique_address
#endif

#if !defined(ROD_NO_COROUTINES) && defined(__cpp_impl_coroutine) && __cpp_impl_coroutine >= 201902L

#ifndef ROD_HAS_COROUTINES
#define ROD_HAS_COROUTINES
#endif

/* MSVC does not support resuming coroutine handles returned from await_suspend. */
#if !(defined(_MSC_VER) && !defined(__clang__))
#define ROD_HAS_INLINE_RESUME
#endif

#elif defined(ROD_HAS_COROUTINES)
#undef ROD_HAS_COROUTINES
#endif



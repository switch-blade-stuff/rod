/*
 * Created by switchblade on 2023-05-06.
 */

#pragma once

#include "api.hpp"

/* MSVC does not support standard no_unique_address */
#if defined(_MSC_VER) && _MSC_VER >= 1929
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

#ifdef __linux__
#include <linux/version.h>

#if !defined(ROD_NO_EPOLL) && LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
#ifndef ROD_HAS_EPOLL
#define ROD_HAS_EPOLL
#endif
#elif defined(ROD_HAS_EPOLL)
#undef ROD_HAS_EPOLL
#endif

#if !defined(ROD_NO_LIBURING) && LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
#ifndef ROD_HAS_LIBURING
#define ROD_HAS_LIBURING
#endif
#elif defined(ROD_HAS_LIBURING)
#undef ROD_HAS_LIBURING
#endif

#endif

/*
 * Created by switchblade on 2023-05-06.
 */

#pragma once

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#define ROD_HIDDEN
#define ROD_VISIBLE
#if defined(_MSC_VER)
#define ROD_API_EXPORT __declspec(dllexport)
#define ROD_API_IMPORT __declspec(dllimport)
#elif defined(__clang__) || defined(__GNUC__)
#define ROD_API_EXPORT __attribute__((dllexport))
#define ROD_API_IMPORT __attribute__((dllimport))
#endif
#elif __GNUC__ >= 4
#define ROD_HIDDEN __attribute__((visibility("hidden")))
#define ROD_VISIBLE __attribute__((visibility("default")))
#define ROD_API_EXPORT ROD_VISIBLE
#define ROD_API_IMPORT ROD_VISIBLE
#else
#define ROD_HIDDEN
#define ROD_VISIBLE
#define ROD_API_EXPORT
#define ROD_API_IMPORT
#endif

#if defined(ROD_EXPORT)
#define ROD_API_PUBLIC ROD_API_EXPORT
#else
#define ROD_API_PUBLIC ROD_API_IMPORT
#endif

/* MSVC does not support standard no_unique_address */
#if defined(_MSC_VER) && _MSC_VER >= 1929
#define ROD_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#define ROD_NO_UNIQUE_ADDRESS [[no_unique_address]]
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

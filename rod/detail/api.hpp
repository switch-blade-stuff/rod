/*
 * Created by switchblade on 2023-05-19.
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

#if defined(ROD_EXPORT) || defined(ROD_LIB_STATIC)
#define ROD_PUBLIC ROD_API_EXPORT
#else
#define ROD_PUBLIC ROD_API_IMPORT
#endif

#ifdef ROD_TOPLEVEL_NAMESPACE
#define ROD_TOPLEVEL_NAMESPACE_OPEN namespace ROD_TOPLEVEL_NAMESPACE {
#define ROD_TOPLEVEL_NAMESPACE_CLOSE }
#else
#define ROD_TOPLEVEL_NAMESPACE_OPEN
#define ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

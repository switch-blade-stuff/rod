/*
 * Created by switchblade on 2023-09-15.
 */

#pragma once

#if __has_include("sys/uio.h")
#include <sys/uio.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#ifndef __APPLE__
#define ROD_HAS_PIOV
#endif

#include "../file_handle.hpp"
#include "handle_base.hpp"

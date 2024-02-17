/*
 * Created by switchblade on 2023-09-15.
 */

#pragma once

#include <numeric>

#include "../file_handle.hpp"
#include "ntapi.hpp"

#ifndef FSCTL_DUPLICATE_EXTENTS_TO_FILE
#define FSCTL_DUPLICATE_EXTENTS_TO_FILE CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 209, METHOD_BUFFERED, FILE_WRITE_DATA)
#endif

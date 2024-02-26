/*
 * Created by switchblade on 2023-10-30.
 */

#pragma once

#include <unistd.h>
#include <fcntl.h>

#if __has_include("wordexp.h")
#include <wordexp.h>
#define USE_WORDEXP
#endif

#include "../path_util.hpp"

namespace rod::_unix
{
	result<std::string> exec_cmd(std::string_view cmd) noexcept;
	result<std::string> expand_path(std::string_view str) noexcept;
}

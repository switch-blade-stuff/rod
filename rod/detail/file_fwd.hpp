/*
 * Created by switchblade on 2023-05-19.
 */

#pragma once

#include "read_write.hpp"

#include <filesystem>
#include <string>
#include <cstdio>

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::io
{
	namespace detail
	{
		enum openmode : int
		{
			in = 0b0001,
			out = 0b0010,
			binary = 0b0100,
			direct = 0b1000,
			ate = 0b0001'0000,
			app = 0b0010'0000,
			trunc = 0b0100'0000,
			noreplace = 0b1000'0000,
		};
		enum seekdir : int
		{
			beg = SEEK_SET,
			cur = SEEK_CUR,
			end = SEEK_END,
		};

		class native_file;
	}

	/** Structure representing an unbuffered file handle (such as a posix file descriptor or win32 HANDLE). */
	class basic_file;
	/** Structure representing a buffered file handle. */
	class file;
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

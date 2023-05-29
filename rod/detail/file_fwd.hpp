/*
 * Created by switchblade on 2023-05-19.
 */

#pragma once

#include "adaptors/read_some.hpp"
#include "adaptors/write_some.hpp"

#include <filesystem>
#include <string>
#include <cstdio>

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
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

		class system_file;
	}

	namespace _file
	{
		/** Unbuffered file handle (such as a regular posix file descriptor). */
		class basic_file;
		/** Buffered file handle. */
		class file;
	}

	using _file::basic_file;
	using _file::file;
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

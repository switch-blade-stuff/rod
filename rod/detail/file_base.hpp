/*
 * Created by switch_blade on 2023-07-10.
 */

#pragma once

#include <filesystem>
#include <cstdio>

#include "io_handle.hpp"

namespace rod
{
	namespace _file
	{
		/** Base type defining common members of file handles. */
		class file_base
		{
		public:
			using openprot = int;

			/** File is created with execute permissions for the current user. */
			static constexpr openprot user_exec = 0b000'000'001;
			/** File is created with execute permissions for the current group. */
			static constexpr openprot group_exec = 0b000'001'000;
			/** File is created with execute permissions for other users. */
			static constexpr openprot other_exec = 0b001'000'000;

			/** File is created with read permissions for the current user. */
			static constexpr openprot user_read = 0b000'000'010;
			/** File is created with read permissions for the current group. */
			static constexpr openprot group_read = 0b000'010'000;
			/** File is created with read permissions for the other users. */
			static constexpr openprot other_read = 0b010'000'000;

			/** File is created with write permissions for the current user. */
			static constexpr openprot user_write = 0b000'000'100;
			/** File is created with write permissions for the current group. */
			static constexpr openprot group_write = 0b000'100'000;
			/** File is created with write permissions for the other users. */
			static constexpr openprot other_write = 0b100'000'000;

			using openmode = int;

			/** Opens the file for input. */
			static constexpr openmode in = 0b0000'0001;
			/** Opens the file for output. */
			static constexpr openmode out = 0b0000'0010;

			/** Seeks the file to the end after it is opened. */
			static constexpr openmode ate = 0b0000'0100;
			/** Opens the file in append mode. Write operations will append to the end of the file. */
			static constexpr openmode app = 0b0000'1000;
			/** Opens the file in truncate mode. Write operations will overwrite existing contents. */
			static constexpr openmode trunc = 0b0001'0000;

			/** Opens the file in binary mode.
			 * @note This flag currently has no effect. All files are opened in binary mode. */
			static constexpr openmode binary = 0b00'0010'0000;
			/** Opens the file in direct mode. */
			static constexpr openmode direct = 0b00'0100'0000;
			/** Opens the file in non-blocking mode. File IO may not complete synchronously and has to be polled.
			 * @note Certain systems and  */
			static constexpr openmode nonblock = 0b00'1000'0000;
			/** Opens the file in open-only mode. Opening the file will fail if it does not exist. */
			static constexpr openmode nocreate = 0b01'0000'0000;
			/** Opens the file in create-only mode. Opening the file will fail if it already exists. */
			static constexpr openmode noreplace = 0b10'0000'0000;

			using seekdir = int;

			/** Seek from the start of the file. */
			static constexpr seekdir beg = SEEK_SET;
			/** Seek from the current position within the file. */
			static constexpr seekdir cur = SEEK_CUR;
			/** Seek from the end of the file. */
			static constexpr seekdir end = SEEK_END;

			/** Maximum value of `openprot` flags. */
			static constexpr openprot openprot_max = 0x1ff;
			/** Maximum value of `openmode` flags. */
			static constexpr openmode openmode_max = 0x3ff;

		public:
			template<std::derived_from<file_base> F>
			friend std::error_code tag_invoke(open_t, F &file, auto &&path, openmode mode) noexcept requires(requires { file.open(path, mode); }) { return file.open(path, mode); }
			template<std::derived_from<file_base> F>
			friend std::error_code tag_invoke(open_t, F &file, auto &&path, openmode mode, openprot prot) noexcept requires(requires { file.open(path, mode, prot); }) { return file.open(path, mode, prot); }

			template<std::derived_from<file_base> F>
			friend std::error_code tag_invoke(sync_t, F &file) noexcept requires(requires { file.sync(); }) { return file.sync(); }
			template<std::derived_from<file_base> F>
			friend std::error_code tag_invoke(flush_t, F &file) noexcept requires(requires { file.flush(); }) { return file.flush(); }
			template<std::derived_from<file_base> F>
			friend std::error_code tag_invoke(close_t, F &file) noexcept requires(requires { file.close(); }) { return file.close(); }
		};

		inline constexpr auto default_openprot = file_base::user_read | file_base::user_write | file_base::group_read | file_base::group_write | file_base::other_read | file_base::other_write;
	}

	using _file::file_base;
}
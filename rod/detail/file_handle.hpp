/*
 * Created by switch_blade on 2023-09-15.
 */

#pragma once

#include "directory_handle.hpp"
#include "path_discovery.hpp"
#include "byte_buffer.hpp"
#include "async_base.hpp"

namespace rod
{
	namespace _file
	{
		template<typename FileBase>
		struct streamable_file_adaptor { class type; };
		class file_handle;

		using extent_pair = std::pair<_handle::extent_type, _handle::extent_type>;

		template<typename Op>
		struct select_io_buffer;
		template<one_of<list_extents_t, zero_extents_t, clone_extents_to_t> Op>
		struct select_io_buffer<Op> { using type = extent_pair; };
		template<one_of<read_some_t, read_some_at_t> Op>
		struct select_io_buffer<Op> { using type = byte_buffer; };
		template<one_of<sync_at_t, write_some_t, write_some_at_t> Op>
		struct select_io_buffer<Op> { using type = const_byte_buffer; };

		template<typename Op>
		using io_buffer = typename select_io_buffer<Op>::type;
		template<typename Op>
		using io_buffer_sequence = std::span<io_buffer<Op>>;

		template<typename Op>
		struct select_io_result;
		template<one_of<zero_extents_t, clone_extents_to_t> Op>
		struct select_io_result<Op> { using type = result<io_buffer<Op>, io_status_code>; };
		template<std::same_as<list_extents_t> Op>
		struct select_io_result<Op> { using type = result<std::vector<io_buffer<Op>>, io_status_code>; };
		template<one_of<sync_at_t, read_some_t, read_some_at_t, write_some_t, write_some_at_t> Op>
		struct select_io_result<Op> { using type = result<io_buffer_sequence<Op>, io_status_code>; };

		template<typename Op>
		using io_result = typename select_io_result<Op>::type;

		template<typename Op>
		struct io_request;
		template<std::same_as<sync_at_t> Op>
		struct io_request<Op>
		{
			io_buffer_sequence<Op> buffs;
			_handle::extent_type off = 0;
			sync_mode mode = sync_mode::all;
		};
		template<one_of<read_some_t, write_some_t> Op>
		struct io_request<Op>
		{
			io_buffer_sequence<Op> buffs;
		};
		template<one_of<read_some_at_t, write_some_at_t> Op>
		struct io_request<Op>
		{
			io_buffer_sequence<Op> buffs;
			_handle::extent_type off = 0;
		};
		template<std::same_as<list_extents_t> Op>
		struct io_request<Op>
		{
			std::vector<io_buffer<Op>> buff = {};
		};
		template<std::same_as<zero_extents_t> Op>
		struct io_request<Op>
		{
			io_buffer<Op> extent = {};
			bool emulate = true;
		};
		template<std::same_as<clone_extents_to_t> Op>
		struct io_request<Op>
		{
			io_buffer<Op> extent = {};

			file_handle &dst;
			_handle::extent_type off = 0;

			bool emulate = true;
			bool force_copy = false;
		};

		template<typename Res>
		concept file_like_result = instance_of<Res, result> && sparse_io_handle<typename Res::value_type>;

		/** Handle to a sparse IO file. */
		class file_handle : public io_handle_adaptor<file_handle, basic_handle, fs::fs_handle_adaptor>
		{
			using adp_base = io_handle_adaptor<file_handle, basic_handle, fs::fs_handle_adaptor>;

			friend io_handle_adaptor<file_handle, basic_handle, fs::fs_handle_adaptor>;
			friend fs::fs_handle_adaptor<file_handle, basic_handle>;
			friend handle_adaptor<file_handle, basic_handle>;

		public:
			template<typename Op>
			using io_buffer = _file::io_buffer<Op>;
			template<typename Op>
			using io_buffer_sequence = _file::io_buffer_sequence<Op>;

			template<typename Op>
			using io_result = _file::io_result<Op>;
			template<typename Op>
			using io_request = _file::io_request<Op>;

		private:
			using path_view = fs::path_view;
			using path_handle = fs::path_handle;

			using file_perm = fs::file_perm;
			using file_flags = fs::file_flags;
			using file_caching = fs::file_caching;

			[[nodiscard]] static ROD_API_PUBLIC result<file_handle> do_reopen(const file_handle &other, file_flags flags, file_caching caching) noexcept;
			[[nodiscard]] static ROD_API_PUBLIC result<file_handle> do_open(const path_handle &base, path_view path, file_flags flags, open_mode mode, file_caching caching, file_perm perm) noexcept;

		public:
			struct open_t
			{
				template<scheduler Sch> requires tag_invocable<open_t, Sch, const path_handle &, path_view, file_flags, open_mode, file_caching, file_perm>
				file_like_result auto operator()(Sch &&sch, const path_handle &base, path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::existing, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
				{
					return tag_invoke(*this, std::forward<Sch>(sch), base, path, flags, mode, caching, perm);
				}

				result<file_handle> operator()(const path_handle &base, path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::existing, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
				{
					return do_open(base, path, flags, mode, caching, perm);
				}
			};

			/* TODO: Document usage */
			static constexpr auto open = open_t{};

			struct open_unique_t
			{
				template<scheduler Sch> requires(!tag_invocable<open_unique_t, Sch, const path_handle &, file_flags, file_caching, file_perm> && _detail::callable<open_t, Sch, const path_handle &, path_view>)
				file_like_result auto operator()(Sch &&sch, const path_handle &base, file_flags flags = file_flags::read, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
				{
					if (auto name = _handle::generate_unique_name(); name.has_value()) [[likely]]
						return open(std::forward<Sch>(sch), base, *name, flags, open_mode::always, caching, perm);
					else
						return name.error();
				}
				template<scheduler Sch> requires tag_invocable<open_unique_t, Sch, const path_handle &, file_flags, file_caching, file_perm>
				file_like_result auto operator()(Sch &&sch, const path_handle &base, file_flags flags = file_flags::read, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
				{
					return tag_invoke(*this, std::forward<Sch>(sch), base, flags, caching, perm);
				}

				result<file_handle> operator()(const path_handle &base, file_flags flags = file_flags::read, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
				{
					if (auto name = _handle::generate_unique_name(); name.has_value()) [[likely]]
						return open(base, *name, flags, open_mode::always, caching, perm);
					else
						return name.error();
				}
			};

			/* TODO: Document usage */
			static constexpr auto open_unique = open_unique_t{};

			struct open_temporary_t
			{
				template<scheduler Sch> requires(!tag_invocable<open_temporary_t, Sch, path_view, file_flags, open_mode, file_caching, file_perm> && _detail::callable<open_t, Sch, const path_handle &, path_view>)
				file_like_result auto operator()(Sch &&sch, path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::always, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
				{
					if (!path.empty())
						return open(std::forward<Sch>(sch), fs::temporary_file_directory(), path, flags, mode, caching, perm);
					else if (mode != open_mode::existing)
						return open_unique(std::forward<Sch>(sch), fs::temporary_file_directory(), flags, caching, perm);
					else
						return std::make_error_code(std::errc::invalid_argument);
				}
				template<scheduler Sch> requires tag_invocable<open_temporary_t, Sch, path_view, file_flags, open_mode, file_caching, file_perm>
				file_like_result auto operator()(Sch &&sch, path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::always, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
				{
					return tag_invoke(*this, std::forward<Sch>(sch), path, flags, mode, caching, perm);
				}

				result<file_handle> operator()(path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::always, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
				{
					if (!path.empty())
						return open(fs::temporary_file_directory(), path, flags, mode, caching, perm);
					else if (mode != open_mode::existing)
						return open_unique(fs::temporary_file_directory(), flags, caching, perm);
					else
						return std::make_error_code(std::errc::invalid_argument);
				}
			};

			/* TODO: Document usage */
			static constexpr auto open_temporary = open_temporary_t{};

			/* TODO: Implement reopen. */

		public:
			file_handle(const file_handle &) = delete;
			file_handle &operator=(const file_handle &) = delete;

			/** Initializes a closed file handle. */
			file_handle() noexcept = default;
			file_handle(file_handle &&other) noexcept : adp_base(std::forward<adp_base>(other)) {}
			file_handle &operator=(file_handle &&other) noexcept { return (adp_base::operator=(std::forward<adp_base>(other)), *this); }

			/** Initializes file handle from a native handle. */
			explicit file_handle(typename adp_base::native_handle_type hnd, file_flags flags, file_caching caching) noexcept : adp_base(typename adp_base::native_handle_type(hnd, std::uint32_t(flags) | (std::uint32_t(caching) << 16))) {}
			/** Initializes file handle from a native handle and explicit device & inode IDs. */
			explicit file_handle(typename adp_base::native_handle_type hnd, file_flags flags, file_caching caching, dev_t dev, ino_t ino) noexcept : adp_base(typename adp_base::native_handle_type(hnd, std::uint32_t(flags) | (std::uint32_t(caching) << 16)), dev, ino) {}

			/** Initializes file handle from a basic handle rvalue and file flags. */
			explicit file_handle(basic_handle &&hnd, file_flags flags, file_caching caching) noexcept : file_handle(hnd.release(), flags, caching) {}
			/** Initializes file handle from a basic handle rvalue, file flags and explicit device & inode IDs. */
			explicit file_handle(basic_handle &&hnd, file_flags flags, file_caching caching, dev_t dev, ino_t ino) noexcept : file_handle(hnd.release(), flags, caching, dev, ino) {}

			/** Returns the flags of the file handle. */
			[[nodiscard]] file_flags flags() const noexcept { return file_flags(native_handle().flags & 0xffff); }
			/** Returns the caching mode of the file handle. */
			[[nodiscard]] file_caching caching() const noexcept { return file_caching((native_handle().flags >> 16) & 0xff); }

			constexpr void swap(file_handle &other) noexcept { adp_base::swap(other); }
			friend constexpr void swap(file_handle &a, file_handle &b) noexcept { a.swap(b); }

		private:
			result<file_handle> do_clone() const noexcept { return clone(base()).transform_value([&](basic_handle &&hnd) { return file_handle(std::move(hnd), flags(), caching()); }); }

			ROD_API_PUBLIC result<> do_link(const path_handle &base, path_view path, bool replace, const fs::file_timeout &to) noexcept;
			ROD_API_PUBLIC result<> do_relink(const path_handle &base, path_view path, bool replace, const fs::file_timeout &to) noexcept;
			ROD_API_PUBLIC result<> do_unlink(const fs::file_timeout &to) noexcept;

			ROD_API_PUBLIC result<extent_type> do_truncate(extent_type endp) noexcept;
			ROD_API_PUBLIC result<void, io_status_code> do_sync(sync_mode mode) noexcept;
			ROD_API_PUBLIC io_result<sync_at_t> do_sync_at(io_request<sync_at_t> req) noexcept;

			template<auto IoFunc, typename Op>
			inline io_result<Op> invoke_io_func(io_request<Op> req, const fs::file_timeout &to) noexcept;

			ROD_API_PUBLIC io_result<read_some_at_t> do_read_some_at(io_request<read_some_at_t> req, const fs::file_timeout &to) noexcept;
			ROD_API_PUBLIC io_result<write_some_at_t> do_write_some_at(io_request<write_some_at_t> req, const fs::file_timeout &to) noexcept;

			ROD_API_PUBLIC io_result<zero_extents_t> do_zero_extents(io_request<zero_extents_t> req, const fs::file_timeout &to) noexcept;
			ROD_API_PUBLIC io_result<list_extents_t> do_list_extents(io_request<list_extents_t> req, const fs::file_timeout &to) const noexcept;
			ROD_API_PUBLIC io_result<clone_extents_to_t> do_clone_extents_to(io_request<clone_extents_to_t> req, const fs::file_timeout &to) noexcept;
		};

		template<typename FileBase>
		struct streamable_file_adaptor<FileBase>::type : public io_handle_adaptor<type, FileBase, fs::fs_handle_adaptor>
		{
			using adp_base = io_handle_adaptor<type, FileBase, fs::fs_handle_adaptor>;

			friend io_handle_adaptor<type, FileBase, fs::fs_handle_adaptor>;
			friend handle_adaptor<type, FileBase>;

		public:
			/* TODO: Implement open & reopen functions. */

		public:
			type(const type &) = delete;
			type &operator=(const type &) = delete;

			/** Initializes a closed file stream. */
			type() noexcept = default;
			type(type &&other) noexcept : adp_base(std::forward<adp_base>(other)), _off(std::exchange(other._off, 0)) {}
			type &operator=(type &&other) noexcept { return (adp_base::operator=(std::forward<adp_base>(other)), std::swap(_off, other._off), *this); }

			/** Initializes file stream from a base handle. */
			explicit type(FileBase &&hnd) noexcept(std::is_nothrow_constructible_v<adp_base, FileBase>) : adp_base(std::forward<FileBase>(hnd)) {}
			/** Initializes file stream from a base handle and initial offset. */
			explicit type(FileBase &&hnd, typename adp_base::extent_type off) noexcept(std::is_nothrow_constructible_v<adp_base, FileBase>) : adp_base(std::forward<FileBase>(hnd)), _off(off) {}

			/** Initializes file stream from a native handle and additional arguments \a args passed to the base handle constructor. */
			template<typename... Args>
			explicit type(typename adp_base::native_handle_type hnd, Args &&...args) noexcept(std::is_nothrow_constructible_v<adp_base, typename adp_base::native_handle_type, Args...>) : adp_base(hnd, std::forward<Args>(args)...) {}
			/** Initializes file stream from a native handle, initial offset, and additional arguments \a args passed to the base handle constructor. */
			template<typename... Args>
			explicit type(typename adp_base::native_handle_type hnd, typename adp_base::extent_type off, Args &&...args) noexcept(std::is_nothrow_constructible_v<adp_base, typename adp_base::native_handle_type, Args...>) : adp_base(hnd, std::forward<Args>(args)...), _off(off) {}

			constexpr void swap(type &other) noexcept
			{
				adp_base::swap(other);
				std::swap(_off, other._off);
			}
			friend constexpr void swap(type &a, type &b) noexcept { a.swap(b); }

		private:
			result<type> do_clone() const noexcept { return clone(adp_base::base()).transform_value([&](FileBase &&hnd) { return type(std::forward<FileBase>(hnd), _off); }); }

			result<typename adp_base::extent_type> do_getpos() const noexcept { return _off; }
			result<typename adp_base::extent_type> do_setpos(typename adp_base::extent_type newp) noexcept { return _off = newp; }

			template<typename Req, typename To>
			io_result<read_some_t> do_read_some(Req &&req, const To &to) noexcept
			{
				auto res = read_some_at(adp_base::base(), {.buffs = std::forward<Req>(req).buffs, .off = _off}, to);
				if (res.has_value()) [[likely]]
					std::ranges::for_each(*res, [&](auto &buff) noexcept { _off += buff.size(); });
				else
					_off += res.error().partial_bytes();
				return res;
			}
			template<typename Req, typename To>
			io_result<write_some_t> do_write_some(Req &&req, const To &to) noexcept
			{
				auto res = write_some_at(adp_base::base(), {.buffs = std::forward<Req>(req).buffs, .off = _off}, to);
				if (res.has_value()) [[likely]]
					std::ranges::for_each(*res, [&](auto &buff) noexcept { _off += buff.size(); });
				else
					_off += res.error().partial_bytes();
				return res;
			}

			/* TODO: Implement async overloads. */

			typename adp_base::extent_type _off = 0;
		};
	}

	namespace fs
	{
		using _file::file_handle;

		static_assert(!io_handle<file_handle, read_some_t> && !io_handle_with_timeout<file_handle, read_some_t, file_timeout>);
		static_assert(!io_handle<file_handle, write_some_t> && !io_handle_with_timeout<file_handle, write_some_t, file_timeout>);

		static_assert(io_handle<file_handle, read_some_at_t> && io_handle_with_timeout<file_handle, read_some_at_t, file_timeout>);
		static_assert(io_handle<file_handle, write_some_at_t> && io_handle_with_timeout<file_handle, write_some_at_t, file_timeout>);

		/** Streamable file handle adaptor which maintains an offset used for streaming IO. */
		template<typename FileBase>
		using streamable_file_adaptor = typename _file::streamable_file_adaptor<FileBase>::type;
		/** Alias for `streamable_file_adaptor&lt;file_handle&gt;`. */
		using streamable_file = streamable_file_adaptor<file_handle>;

		static_assert(io_handle<streamable_file, read_some_t> && io_handle_with_timeout<streamable_file, read_some_t, file_timeout>);
		static_assert(io_handle<streamable_file, write_some_t> && io_handle_with_timeout<streamable_file, write_some_t, file_timeout>);

		static_assert(io_handle<streamable_file, read_some_at_t> && io_handle_with_timeout<streamable_file, read_some_at_t, file_timeout>);
		static_assert(io_handle<streamable_file, write_some_at_t> && io_handle_with_timeout<streamable_file, write_some_at_t, file_timeout>);

//		/** `std::basic_streambuf` handle adaptor. */
//		template<typename Ch, typename Handle = file_handle>
//		using basic_streambuf = typename _file::basic_streambuf<Ch, Handle>::type;
//		/** `std::basic_ios` handle adaptor. */
//		template<typename Ch, typename Handle = file_handle>
//		using basic_stream = typename _file::basic_stream<Ch, Handle>::type;
//
//		/** Alias for `basic_stream&lt;std::byte, file_handle&gt;`. */
//		using byte_stream = basic_stream<std::byte, file_handle>;
//		/** Alias for `basic_stream&lt;char, file_handle&gt;`. */
//		using char_stream = basic_stream<char, file_handle>;
	}
}
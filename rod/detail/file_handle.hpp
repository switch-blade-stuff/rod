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
		class file_handle;

		template<typename Op, bool EnableStream, bool EnableSparse>
		struct select_io_buffer;
		template<bool EnableSparse>
		struct select_io_buffer<read_some_t, true, EnableSparse> { using type = byte_buffer; };
		template<bool EnableStream>
		struct select_io_buffer<read_some_at_t, EnableStream, true> { using type = byte_buffer; };
		template<bool EnableSparse>
		struct select_io_buffer<write_some_t, true, EnableSparse> { using type = const_byte_buffer; };
		template<bool EnableStream>
		struct select_io_buffer<write_some_at_t, EnableStream, true> { using type = const_byte_buffer; };

		template<typename Hnd>
		concept file_like = sparse_io_handle<Hnd> && std::convertible_to<const Hnd &, const file_handle &>;
		template<typename Res>
		concept file_like_result = instance_of<Res, result> && file_like<typename Res::value_type>;

		/** Handle to a sparse IO file. */
		class file_handle : public fs::fs_handle_adaptor<file_handle, basic_handle>
		{
			friend fs::fs_handle_adaptor<file_handle, basic_handle>;
			friend handle_adaptor<file_handle, basic_handle>;

			using adp_base = fs::fs_handle_adaptor<file_handle, basic_handle>;

		public:
			template<typename Op>
			using io_buffer = typename select_io_buffer<Op, false, true>::type;
			template<typename Op>
			using io_buffer_sequence = std::span<io_buffer<Op>>;

			template<typename Op>
			using io_result = result<io_buffer_sequence<Op>, io_status_code>;
			template<typename Op>
			struct io_request { io_buffer_sequence<Op> buffs; };

		private:
			using path_view = fs::path_view;
			using path_handle = fs::path_handle;

			using file_perm = fs::file_perm;
			using file_flags = fs::file_flags;
			using file_caching = fs::file_caching;

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

		public:
			template<decays_to_same<read_some_at_t> Op, decays_to_same<file_handle> Hnd, decays_to_same<io_request<Op>> Req, decays_to_same<std::uint64_t> Ext>
			friend io_result<Op> tag_invoke(Op, Hnd &&hnd, Req &&req, Ext pos, const fs::file_timeout &to) noexcept { return hnd.do_read_some_at(std::move(req), pos, to); }
			template<decays_to_same<write_some_at_t> Op, decays_to_same<file_handle> Hnd, decays_to_same<std::uint64_t> Ext, decays_to_same<io_request<Op>> Req>
			friend io_result<Op> tag_invoke(Op, Hnd &&hnd, Req &&req, Ext pos, const fs::file_timeout &to) noexcept { return hnd.do_write_some_at(std::move(req), pos, to); }

		private:
			result<file_handle> do_clone() const noexcept { return clone(base()).transform([&](basic_handle &&hnd) { return file_handle(std::move(hnd), flags(), caching()); }); }

			ROD_API_PUBLIC result<> do_link(const path_handle &base, path_view path, bool replace, const fs::file_timeout &to) noexcept;
			ROD_API_PUBLIC result<> do_relink(const path_handle &base, path_view path, bool replace, const fs::file_timeout &to) noexcept;
			ROD_API_PUBLIC result<> do_unlink(const fs::file_timeout &to) noexcept;

			template<auto IoFunc, typename Op>
			inline io_result<Op> invoke_io_func_at(io_request<Op> req, std::uint64_t pos, const fs::file_timeout &to) noexcept;

			ROD_API_PUBLIC io_result<read_some_at_t> do_read_some_at(io_request<read_some_at_t> req, std::uint64_t pos, const fs::file_timeout &to) noexcept;
			ROD_API_PUBLIC io_result<write_some_at_t> do_write_some_at(io_request<write_some_at_t> req, std::uint64_t pos, const fs::file_timeout &to) noexcept;
		};

		template<typename Base>
		class basic_file_stream;

		template<typename Child, typename Sch>
		struct async_file_adaptor { class type; };
	}

	namespace fs
	{
		using _file::file_handle;
		using _file::basic_file_stream;

		/** Alias for `basic_file_stream&lt;file_handle&gt;`. */
		using file_stream = basic_file_stream<file_handle>;
	}
}
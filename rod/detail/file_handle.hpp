/*
 * Created by switchblade on 2023-09-15.
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
		using namespace fs;

		class file_handle;
		template<typename FileBase>
		struct file_stream_adaptor { class type; };

		using extent_type = _handle::extent_type;
		using extent_pair = std::pair<extent_type, extent_type>;

		template<typename Op>
		struct select_io_buffer;
		template<one_of<read_some_t, read_some_at_t> Op>
		struct select_io_buffer<Op> { using type = byte_buffer; };
		template<one_of<write_some_t, write_some_at_t> Op>
		struct select_io_buffer<Op> { using type = const_byte_buffer; };

		template<typename Op>
		using select_io_buffer_t = typename select_io_buffer<Op>::type;

		template<typename Op>
		struct select_io_result;
		template<one_of<zero_extents_t, clone_extents_to_t> Op>
		struct select_io_result<Op> { using type = result<extent_pair, io_status_code>; };
		template<std::same_as<list_extents_t> Op>
		struct select_io_result<Op> { using type = result<std::vector<extent_pair>, io_status_code>; };
		template<one_of<read_some_t, read_some_at_t, write_some_t, write_some_at_t> Op>
		struct select_io_result<Op> { using type = result<std::span<select_io_buffer_t<Op>>, io_status_code>; };

		template<typename Op>
		using select_io_result_t = typename select_io_result<Op>::type;

		template<typename Hnd>
		concept file_handle_like = sized_handle<Hnd> && sparse_io_handle<Hnd>;
		template<typename Res>
		concept file_handle_like_result = instance_of<Res, result> && file_handle_like<typename Res::value_type>;

		/** Handle to a sparse IO file. */
		class file_handle : public io_handle_adaptor<file_handle, basic_handle, fs_handle_adaptor>
		{
			friend io_handle_adaptor<file_handle, basic_handle, fs_handle_adaptor>;
			friend fs_handle_adaptor<file_handle, basic_handle>;
			friend handle_adaptor<file_handle, basic_handle>;

			using adaptor = io_handle_adaptor<file_handle, basic_handle, fs_handle_adaptor>;

		public:
			template<typename Op>
			using io_buffer = _file::select_io_buffer_t<Op>;
			template<typename Op>
			using io_buffer_sequence = std::span<io_buffer<Op>>;

			template<typename Op>
			struct io_request;

			template<one_of<read_some_t, write_some_t> Op>
			struct io_request<Op> { std::span<io_buffer<Op>> buffs; };
			template<one_of<read_some_at_t, write_some_at_t> Op>
			struct io_request<Op> { std::span<io_buffer<Op>> buffs; extent_type off = 0; };

			template<std::same_as<list_extents_t> Op>
			struct io_request<Op> { std::vector<extent_pair> buff; };
			template<std::same_as<zero_extents_t> Op>
			struct io_request<Op>
			{
				extent_pair extent = {};
				bool emulate = true;
			};
			template<std::same_as<clone_extents_to_t> Op>
			struct io_request<Op>
			{
				extent_pair extent = {};

				file_handle &dst;
				extent_type off = 0;

				bool emulate = true;
				bool force_copy = false;
			};

			template<typename Op>
			using io_result = _file::select_io_result_t<Op>;

		private:
			[[nodiscard]] static ROD_API_PUBLIC result<file_handle> do_open(const path_handle &base, path_view path, file_flags flags, open_mode mode, file_caching caching, file_perm perm) noexcept;
			[[nodiscard]] static ROD_API_PUBLIC result<file_handle> do_open_anonymous(const path_handle &base, file_flags flags, file_caching caching, file_perm perm) noexcept;
			[[nodiscard]] static ROD_API_PUBLIC result<file_handle> do_reopen(const file_handle &other, file_flags flags, file_caching caching) noexcept;

		public:
			/* TODO: Implement reopen. */
			struct reopen_t
			{
				template<typename Hnd> requires tag_invocable<reopen_t, const Hnd &, file_flags, file_caching>
				file_handle_like_result auto operator()(const Hnd &other, file_flags flags = file_flags::read, file_caching caching = file_caching::all) const noexcept
				{
					return tag_invoke(*this, other, flags, caching);
				}
				result<file_handle> operator()(const file_handle &other, file_flags flags = file_flags::read, file_caching caching = file_caching::all) const noexcept
				{
					return do_reopen(other, flags, caching);
				}
			};

			/* TODO: Document usage */
			static constexpr auto reopen = reopen_t{};

			struct open_t
			{
				template<scheduler Sch> requires tag_invocable<open_t, Sch, const path_handle &, path_view, file_flags, open_mode, file_caching, file_perm>
				file_handle_like_result auto operator()(Sch &&sch, const path_handle &base, path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::existing, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
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
				file_handle_like_result auto operator()(Sch &&sch, const path_handle &base, file_flags flags = file_flags::read, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
				{
					if (auto name = _handle::generate_unique_name(); name.has_value()) [[likely]]
						return open(std::forward<Sch>(sch), base, *name, flags, open_mode::always, caching, perm);
					else
						return name.error();
				}
				template<scheduler Sch> requires tag_invocable<open_unique_t, Sch, const path_handle &, file_flags, file_caching, file_perm>
				file_handle_like_result auto operator()(Sch &&sch, const path_handle &base, file_flags flags = file_flags::read, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
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
				file_handle_like_result auto operator()(Sch &&sch, path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::always, file_caching caching = file_caching::all | file_caching::temporary, file_perm perm = file_perm::all) const noexcept
				{
					if (!path.empty())
						return open(std::forward<Sch>(sch), temp_file_directory(), path, flags, mode, caching | file_caching::temporary, perm);
					else if (mode != open_mode::existing)
						return open_unique(std::forward<Sch>(sch), temp_file_directory(), flags, caching | file_caching::temporary, perm);
					else
						return std::make_error_code(std::errc::invalid_argument);
				}
				template<scheduler Sch> requires tag_invocable<open_temporary_t, Sch, path_view, file_flags, open_mode, file_caching, file_perm>
				file_handle_like_result auto operator()(Sch &&sch, path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::always, file_caching caching = file_caching::all | file_caching::temporary, file_perm perm = file_perm::all) const noexcept
				{
					return tag_invoke(*this, std::forward<Sch>(sch), path, flags, mode, caching | file_caching::temporary, perm);
				}

				result<file_handle> operator()(path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::always, file_caching caching = file_caching::all | file_caching::temporary, file_perm perm = file_perm::all) const noexcept
				{
					if (!path.empty())
						return open(temp_file_directory(), path, flags, mode, caching | file_caching::temporary, perm);
					else if (mode != open_mode::existing)
						return open_unique(temp_file_directory(), flags, caching | file_caching::temporary, perm);
					else
						return std::make_error_code(std::errc::invalid_argument);
				}
			};

			/* TODO: Document usage */
			static constexpr auto open_temporary = open_temporary_t{};

			struct open_anonymous_t
			{
				template<scheduler Sch> requires tag_invocable<open_anonymous_t, Sch, const path_handle &, file_flags, file_caching, file_perm>
				file_handle_like_result auto operator()(Sch &&sch, const path_handle &base, file_flags flags = file_flags::readwrite, file_caching caching = file_caching::all | file_caching::temporary, file_perm perm = file_perm::all) const noexcept
				{
					return tag_invoke(*this, std::forward<Sch>(sch), base, flags, caching, perm);
				}
				result<file_handle> operator()(const path_handle &base, file_flags flags = file_flags::readwrite, file_caching caching = file_caching::all | file_caching::temporary, file_perm perm = file_perm::all) const noexcept
				{
					return do_open_anonymous(base, flags, caching, perm);
				}
			};

			/* TODO: Document usage */
			static constexpr auto open_anonymous = open_anonymous_t{};

		public:
			file_handle(const file_handle &) = delete;
			file_handle &operator=(const file_handle &) = delete;

			/** Initializes a closed file handle. */
			file_handle() noexcept = default;
			file_handle(file_handle &&other) noexcept : adaptor(std::forward<adaptor>(other)) {}
			file_handle &operator=(file_handle &&other) noexcept { return (adaptor::operator=(std::forward<adaptor>(other)), *this); }

			/** Initializes file handle from a basic handle rvalue, file flags, and caching mode. */
			explicit file_handle(basic_handle &&hnd, file_flags flags, file_caching caching) noexcept : file_handle(hnd.release(), flags, caching) {}
			/** Initializes file handle from a native handle, file flags, and caching mode. */
			explicit file_handle(typename adaptor::native_handle_type hnd, file_flags flags, file_caching caching) noexcept : adaptor(typename adaptor::native_handle_type(hnd, std::uint32_t(flags) | (std::uint32_t(caching) << 16))) {}

			~file_handle() { if (is_open()) do_close(); }

			/** Returns the flags of the file handle. */
			[[nodiscard]] constexpr file_flags flags() const noexcept { return file_flags(native_handle().flags & 0xffff); }
			/** Returns the caching mode of the file handle. */
			[[nodiscard]] constexpr file_caching caching() const noexcept { return file_caching((native_handle().flags >> 16) & 0xff); }

			constexpr void swap(file_handle &other) noexcept { adaptor::swap(other); }
			friend constexpr void swap(file_handle &a, file_handle &b) noexcept { a.swap(b); }

		private:
			auto do_close() noexcept -> decltype(close(base()))
			{
				if (bool(flags() & file_flags::unlink_on_close))
				{
					const auto res = unlink(*this);
					if (res.has_error() && res.error() != std::make_error_condition(std::errc::no_such_file_or_directory))
						return res.error();
				}
				return close(base());
			}
			auto do_clone() const noexcept { return clone(base()).transform_value([&](basic_handle &&hnd) { return file_handle(std::move(hnd), flags(), caching()); }); }

			result<stat::query> do_get_stat(stat &st, stat::query q) const noexcept { return _handle::do_get_stat(st, native_handle(), q); }
			result<stat::query> do_set_stat(const stat &st, stat::query q) noexcept { return _handle::do_set_stat(st, native_handle(), q); }
			result<fs_stat::query> do_get_fs_stat(fs_stat &st, fs_stat::query q) const noexcept { return _handle::do_get_fs_stat(st, native_handle(), q); }

			result<path> do_to_object_path() const noexcept { return _path::do_to_object_path(native_handle()); }
#if defined(ROD_POSIX)
			result<path> do_to_native_path(native_path_format fmt) const noexcept { return _path::do_to_native_path(native_handle(), fmt, 0, 0); }
#endif
			result<path> do_to_native_path(native_path_format fmt, dev_t dev, ino_t ino) const noexcept { return _path::do_to_native_path(native_handle(), fmt, dev, ino); }

			ROD_API_PUBLIC result<> do_link(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept;
			ROD_API_PUBLIC result<> do_relink(const path_handle &base, path_view path, bool replace, const file_timeout &to) noexcept;
			ROD_API_PUBLIC result<> do_unlink(const file_timeout &to) noexcept;

			ROD_API_PUBLIC result<extent_type> do_truncate(extent_type endp) noexcept;
			ROD_API_PUBLIC result<void, io_status_code> do_sync(sync_mode mode) noexcept;
			ROD_API_PUBLIC result<void, io_status_code> do_sync_at(extent_pair ext, sync_mode mode) noexcept;

			template<auto IoFunc, typename Op>
			inline io_result<Op> invoke_io_func(io_request<Op> req, const file_timeout &to) noexcept;

			ROD_API_PUBLIC io_result<read_some_at_t> do_read_some_at(io_request<read_some_at_t> req, const file_timeout &to) noexcept;
			ROD_API_PUBLIC io_result<write_some_at_t> do_write_some_at(io_request<write_some_at_t> req, const file_timeout &to) noexcept;

			ROD_API_PUBLIC io_result<zero_extents_t> do_zero_extents(io_request<zero_extents_t> req, const file_timeout &to) noexcept;
			ROD_API_PUBLIC io_result<list_extents_t> do_list_extents(io_request<list_extents_t> req, const file_timeout &to) const noexcept;
			ROD_API_PUBLIC io_result<clone_extents_to_t> do_clone_extents_to(io_request<clone_extents_to_t> req, const file_timeout &to) noexcept;
		};

		template<typename Hnd>
		concept file_stream_like = file_handle_like<Hnd> && seekable_handle<Hnd> && stream_io_handle<Hnd>;
		template<typename Res>
		concept file_stream_like_result = instance_of<Res, result> && file_stream_like<typename Res::value_type>;

		template<typename FileBase>
		class file_stream_adaptor<FileBase>::type : public io_handle_adaptor<type, FileBase, fs_handle_adaptor>
		{
			friend io_handle_adaptor<type, FileBase, fs_handle_adaptor>;
			friend handle_adaptor<type, FileBase>;

			using adaptor = io_handle_adaptor<type, FileBase, fs_handle_adaptor>;

		public:
			struct reopen_t
			{
				template<typename Base = FileBase> requires(!tag_invocable<reopen_t, const Base &, file_flags, file_caching> && tag_invocable<typename file_handle::reopen_t, const Base &, file_flags, file_caching>)
				file_stream_like_result auto operator()(const typename file_stream_adaptor<Base>::type &other, file_flags flags = file_flags::read, file_caching caching = file_caching::all) const noexcept
				{
					return file_handle::reopen(other.base(), flags, caching).transform_value([](Base &&base) { return typename file_stream_adaptor<Base>::type(std::forward<Base>(base)); });
				}
				template<typename Base = FileBase> requires tag_invocable<reopen_t, const Base &, file_flags, file_caching>
				file_stream_like_result auto operator()(const typename file_stream_adaptor<Base>::type &other, file_flags flags = file_flags::read, file_caching caching = file_caching::all) const noexcept
				{
					return tag_invoke(*this, other.base(), flags, caching);
				}
			};

			/* TODO: Document usage */
			static constexpr auto reopen = reopen_t{};

			struct open_t
			{
				template<scheduler Sch> requires(!tag_invocable<open_t, Sch, const path_handle &, path_view, file_flags, open_mode, file_caching, file_perm> && _detail::callable<typename FileBase::open_t, Sch, const path_handle &, path_view, file_flags, open_mode, file_caching, file_perm>)
				file_stream_like_result auto operator()(Sch &&sch, const path_handle &base, path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::existing, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
				{
					return typename FileBase::open_t{}(std::forward<Sch>(sch), base, path, flags, mode, caching, perm).transform_value([]<typename Hnd>(Hnd &&hnd) { return typename file_stream_adaptor<Hnd>::type(std::forward<Hnd>(hnd)); });
				}
				template<scheduler Sch> requires tag_invocable<open_t, Sch, const path_handle &, path_view, file_flags, open_mode, file_caching, file_perm>
				file_stream_like_result auto operator()(Sch &&sch, const path_handle &base, path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::existing, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
				{
					return tag_invoke(*this, std::forward<Sch>(sch), base, path, flags, mode, caching, perm);
				}

				result<type> operator()(const path_handle &base, path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::existing, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
				{
					return typename FileBase::open_t{}(base, path, flags, mode, caching, perm).transform_value([]<typename Base>(Base &&base) { return type(std::forward<Base>(base)); });
				}
			};

			/* TODO: Document usage */
			static constexpr auto open = open_t{};

			struct open_unique_t
			{
				template<scheduler Sch> requires(!tag_invocable<open_unique_t, Sch, const path_handle &, file_flags, file_caching, file_perm> && _detail::callable<typename FileBase::open_unique_t, Sch, const path_handle &, file_flags, file_caching, file_perm>)
				file_stream_like_result auto operator()(Sch &&sch, const path_handle &base, file_flags flags = file_flags::read, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
				{
					return typename FileBase::open_unique_t{}(std::forward<Sch>(sch), base, flags, caching, perm).transform_value([]<typename Hnd>(Hnd &&hnd) { return typename file_stream_adaptor<Hnd>::type(std::forward<Hnd>(hnd)); });
				}
				template<scheduler Sch> requires tag_invocable<open_unique_t, Sch, const path_handle &, file_flags, file_caching, file_perm>
				file_stream_like_result auto operator()(Sch &&sch, const path_handle &base, file_flags flags = file_flags::read, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
				{
					return tag_invoke(*this, std::forward<Sch>(sch), base, flags, caching, perm);
				}

				result<type> operator()(const path_handle &base, file_flags flags = file_flags::read, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
				{
					return typename FileBase::open_unique_t{}(base, flags, caching, perm).transform_value([]<typename Base>(Base &&base) { return type(std::forward<Base>(base)); });
				}
			};

			/* TODO: Document usage */
			static constexpr auto open_unique = open_unique_t{};

			struct open_temporary_t
			{
				template<scheduler Sch> requires(!tag_invocable<open_temporary_t, Sch, path_view, file_flags, open_mode, file_caching, file_perm> && _detail::callable<typename FileBase::open_temporary_t, Sch, path_view, file_flags, open_mode, file_caching, file_perm>)
				file_stream_like_result auto operator()(Sch &&sch, path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::always, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
				{
					return typename FileBase::open_temporary_t{}(std::forward<Sch>(sch), path, flags, mode, caching, perm).transform_value([]<typename Hnd>(Hnd &&hnd) { return typename file_stream_adaptor<Hnd>::type(std::forward<Hnd>(hnd)); });
				}
				template<scheduler Sch> requires tag_invocable<open_temporary_t, Sch, path_view, file_flags, open_mode, file_caching, file_perm>
				file_stream_like_result auto operator()(Sch &&sch, path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::always, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
				{
					return tag_invoke(*this, std::forward<Sch>(sch), path, flags, mode, caching, perm);
				}

				result<type> operator()(path_view path, file_flags flags = file_flags::read, open_mode mode = open_mode::always, file_caching caching = file_caching::all, file_perm perm = file_perm::all) const noexcept
				{
					return typename FileBase::open_temporary_t{}(path, flags, mode, caching, perm).transform_value([]<typename Base>(Base &&base) { return type(std::forward<Base>(base)); });
				}
			};

			/* TODO: Document usage */
			static constexpr auto open_temporary = open_temporary_t{};

			struct open_anonymous_t
			{
				template<scheduler Sch> requires(!tag_invocable<open_temporary_t, Sch, const path_handle &, file_flags, file_caching, file_perm> && _detail::callable<typename FileBase::open_anonymous_t, Sch, const path_handle &, file_flags, file_caching, file_perm>)
				file_stream_like_result auto operator()(Sch &&sch, const path_handle &base, file_flags flags = file_flags::readwrite, file_caching caching = file_caching::all | file_caching::temporary, file_perm perm = file_perm::all) const noexcept
				{
					return typename FileBase::open_anonymous_t{}(std::forward<Sch>(sch), base, flags, caching, perm).transform_value([]<typename Hnd>(Hnd &&hnd) { return typename file_stream_adaptor<Hnd>::type(std::forward<Hnd>(hnd)); });
				}
				template<scheduler Sch> requires tag_invocable<open_temporary_t, Sch, const path_handle &, file_flags, file_caching, file_perm>
				file_stream_like_result auto operator()(Sch &&sch, const path_handle &base, file_flags flags = file_flags::readwrite, file_caching caching = file_caching::all | file_caching::temporary, file_perm perm = file_perm::all) const noexcept
				{
					return tag_invoke(*this, std::forward<Sch>(sch), base, flags, caching, perm);
				}

				result<type> operator()(const path_handle &base, file_flags flags = file_flags::readwrite, file_caching caching = file_caching::all | file_caching::temporary, file_perm perm = file_perm::all) const noexcept
				{
					return typename FileBase::open_anonymous_t{}(base, flags, caching, perm).transform_value([]<typename Base>(Base &&base) { return type(std::forward<Base>(base)); });
				}
			};

			/* TODO: Document usage */
			static constexpr auto open_anonymous = open_anonymous_t{};

		private:
			friend struct reopen_t;

		public:
			type(const type &) = delete;
			type &operator=(const type &) = delete;

			/** Initializes a closed file stream. */
			type() noexcept = default;
			type(type &&other) noexcept : adaptor(std::forward<adaptor>(other)), _pos(std::exchange(other._pos, 0)) {}
			type &operator=(type &&other) noexcept { return (adaptor::operator=(std::forward<adaptor>(other)), std::swap(_pos, other._pos), *this); }

			/** Initializes file stream from a base handle. */
			explicit type(FileBase &&hnd) noexcept(std::is_nothrow_constructible_v<adaptor, FileBase>) : adaptor(std::forward<FileBase>(hnd)) {}
			/** Initializes file stream from a base handle and initial offset. */
			explicit type(FileBase &&hnd, typename adaptor::extent_type off) noexcept(std::is_nothrow_constructible_v<adaptor, FileBase>) : adaptor(std::forward<FileBase>(hnd)), _pos(off) {}

			/** Initializes file stream from a native handle and additional arguments \a args passed to the base handle constructor. */
			template<typename... Args> requires std::constructible_from<adaptor, typename adaptor::native_handle_type, Args...>
			explicit type(typename adaptor::native_handle_type hnd, Args &&...args) noexcept(std::is_nothrow_constructible_v<adaptor, typename adaptor::native_handle_type, Args...>) : adaptor(hnd, std::forward<Args>(args)...) {}
			/** Initializes file stream from a native handle, initial offset, and additional arguments \a args passed to the base handle constructor. */
			template<typename... Args> requires std::constructible_from<adaptor, typename adaptor::native_handle_type, Args...>
			explicit type(typename adaptor::native_handle_type hnd, typename adaptor::extent_type off, Args &&...args) noexcept(std::is_nothrow_constructible_v<adaptor, typename adaptor::native_handle_type, Args...>) : adaptor(hnd, std::forward<Args>(args)...), _pos(off) {}

			constexpr void swap(type &other) noexcept
			{
				adaptor::swap(other);
				std::swap(_pos, other._pos);
			}
			friend constexpr void swap(type &a, type &b) noexcept { a.swap(b); }

		private:
			result<type> do_clone() const noexcept { return clone(adaptor::base()).transform_value([&](FileBase &&hnd) { return type(std::forward<FileBase>(hnd), _pos); }); }

			result<typename adaptor::extent_type> do_getpos() const noexcept { return _pos; }
			result<typename adaptor::extent_type> do_setpos(auto pos) noexcept { return (_pos = pos); }
			result<typename adaptor::extent_type> do_seekpos(auto off, seek_dir dir) noexcept
			{
				switch (dir)
				{
				case seek_dir::end: return endpos(*this).transform_value([&]<typename Ext>(Ext pos) { return (_pos = Ext(pos - off)); });
				case seek_dir::cur: return (_pos = typename adaptor::extent_type(_pos + off));
				case seek_dir::beg: return (_pos = typename adaptor::extent_type(off));
				}
				return std::make_error_code(std::errc::invalid_argument);
			}

			template<typename Req, typename To>
			io_result_t<FileBase, read_some_t> do_read_some(Req &&req, const To &to) noexcept
			{
				auto res = read_some_at(adaptor::base(), {.buffs = std::forward<Req>(req).buffs, .off = _pos}, to);
				if (res.has_value()) [[likely]]
					std::ranges::for_each(*res, [&](auto &buff) noexcept { _pos += buff.size(); });
				else
					_pos += res.error().partial_bytes();
				return res;
			}
			template<typename Req, typename To>
			io_result_t<FileBase, write_some_t> do_write_some(Req &&req, const To &to) noexcept
			{
				auto res = write_some_at(adaptor::base(), {.buffs = std::forward<Req>(req).buffs, .off = _pos}, to);
				if (res.has_value()) [[likely]]
					std::ranges::for_each(*res, [&](auto &buff) noexcept { _pos += buff.size(); });
				else
					_pos += res.error().partial_bytes();
				return res;
			}

			/* TODO: Implement async overloads. */

			typename adaptor::extent_type _pos = 0;
		};
	}

	namespace fs
	{
		using _file::file_handle;

		static_assert(sized_handle<file_handle> && sparse_io_handle<file_handle>);
		static_assert(!seekable_handle<file_handle> && !stream_io_handle<file_handle>);

		static_assert(_file::file_handle_like<file_handle> && _file::file_handle_like_result<decltype(file_handle::open({}, {}))>);
		static_assert(!_file::file_stream_like<file_handle> && !_file::file_stream_like_result<decltype(file_handle::open({}, {}))>);

		/** File handle with an internal offset used for streaming IO. */
		using file_stream = typename _file::file_stream_adaptor<file_handle>::type;

		static_assert(sized_handle<file_stream> && sparse_io_handle<file_stream>);
		static_assert(seekable_handle<file_stream> && stream_io_handle<file_stream>);

		static_assert(_file::file_handle_like<file_stream> && _file::file_handle_like_result<decltype(file_stream::open({}, {}))>);
		static_assert(_file::file_stream_like<file_stream> && _file::file_stream_like_result<decltype(file_stream::open({}, {}))>);
	}
}
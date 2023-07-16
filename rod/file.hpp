/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#include "detail/adaptors/sync_io.hpp"
#include "detail/basic_file.hpp"

/* Platform-specific implementations. */
#if defined(ROD_WIN32)
#include "win32/detail/file.hpp"
#elif defined(ROD_POSIX)
#include "posix/detail/file.hpp"
#else
#error Unsupported platform
#endif

namespace rod
{
	namespace _file
	{
		template<typename Hnd, typename Sch>
		class scheduler_handle : public Hnd
		{
		public:
			scheduler_handle() = delete;
			scheduler_handle(const scheduler_handle &) = delete;
			scheduler_handle &operator=(const scheduler_handle &) = delete;

			constexpr scheduler_handle(scheduler_handle &&) noexcept(std::is_nothrow_move_constructible_v<Hnd> && std::is_nothrow_move_constructible_v<Sch>) = default;
			constexpr scheduler_handle &operator=(scheduler_handle &&) noexcept(std::is_nothrow_move_assignable_v<Hnd> && std::is_nothrow_move_assignable_v<Sch>) = default;

			template<decays_to<Sch> Sch2, typename... Args> requires std::constructible_from<Hnd, Args...>
			constexpr explicit scheduler_handle(Sch2 &&sch, Args &&...args) noexcept(std::is_nothrow_constructible_v<Sch, Sch2> && std::is_nothrow_constructible_v<Hnd, Args...>) : Hnd(std::forward<Args>(args)...), _sch(std::forward<Sch2>(sch)) {}

			constexpr void swap(scheduler_handle &other) noexcept(std::is_nothrow_swappable_v<Hnd> && std::is_nothrow_swappable_v<Sch>) { swap(*this, other); }
			friend constexpr void swap(scheduler_handle &a, scheduler_handle &b) noexcept(std::is_nothrow_swappable_v<Hnd> && std::is_nothrow_swappable_v<Sch>)
			{
				using std::swap;
				swap(static_cast<Hnd &>(a), static_cast<Hnd &>(b));
				swap(a._sch, b._sch);
			}

		public:
			template<reference_to<scheduler_handle> F, byte_buffer Buff>
			friend sender_of<set_value_t(std::size_t)> auto tag_invoke(async_read_some_t, F &&f, Buff &&buff) noexcept(std::is_nothrow_move_constructible_v<Buff>)
			{
				using sender_t = typename _sync_io::sender<read_some_t, schedule_result_t<Sch>, F, std::decay_t<Buff>>::type;
				return sender_t{schedule(f._sch), std::forward<F>(f), std::forward<Buff>(buff)};
			}
			template<reference_to<scheduler_handle> F, byte_buffer Buff>
			friend sender_of<set_value_t(std::size_t)> auto tag_invoke(async_write_some_t, F &&f, Buff &&buff) noexcept(std::is_nothrow_move_constructible_v<Buff>)
			{
				using sender_t = typename _sync_io::sender<write_some_t, schedule_result_t<Sch>, F, std::decay_t<Buff>>::type;
				return sender_t{schedule(f._sch), std::forward<F>(f), std::forward<Buff>(buff)};
			}
			template<reference_to<scheduler_handle> F, std::convertible_to<std::size_t> Pos, byte_buffer Buff>
			friend sender_of<set_value_t(std::size_t)> auto tag_invoke(async_read_some_at_t, F &&f, Pos pos, Buff &&buff) noexcept(std::is_nothrow_move_constructible_v<Buff>)
			{
				using sender_t = typename _sync_io::sender<read_some_at_t, schedule_result_t<Sch>, F, Pos, std::decay_t<Buff>>::type;
				return sender_t{schedule(f._sch), std::forward<F>(f), pos, std::forward<Buff>(buff)};
			}
			template<reference_to<scheduler_handle> F, std::convertible_to<std::size_t> Pos, byte_buffer Buff>
			friend sender_of<set_value_t(std::size_t)> auto tag_invoke(async_write_some_at_t, F &&f, Pos pos, Buff &&buff) noexcept(std::is_nothrow_move_constructible_v<Buff>)
			{
				using sender_t = typename _sync_io::sender<write_some_at_t, schedule_result_t<Sch>, F, Pos, std::decay_t<Buff>>::type;
				return sender_t{schedule(f._sch), std::forward<F>(f), pos, std::forward<Buff>(buff)};
			}

		private:
			ROD_NO_UNIQUE_ADDRESS Sch _sch;
		};
		template<typename Hnd, typename Sch>
		using scheduler_file = basic_file<scheduler_handle<Hnd, std::decay_t<Sch>>>;

		class open_file_t
		{
			using openmode = typename file_base::openmode;
			using openprot = typename file_base::openprot;

		public:
			template<scheduler Sch, movable_value Path> requires tag_invocable<open_file_t, Sch, Path, openmode, openprot>
			[[nodiscard]] instance_of<result> auto operator()(Sch &&sch, Path &&path, openmode mode, openprot prot = default_openprot) const noexcept(nothrow_tag_invocable<open_file_t, Sch, Path, openmode, openprot>)
			{
				return tag_invoke(*this, std::forward<Sch>(sch), std::forward<Path>(path), mode, prot);
			}
			template<scheduler Sch, movable_value Path> requires(!tag_invocable<open_file_t, Sch, Path, openmode, openprot>)
			[[nodiscard]] result<scheduler_file<system_handle, Sch>, std::error_code> operator()(Sch &&sch, Path &&path, openmode mode, openprot prot = default_openprot) const noexcept
			{
				auto file = scheduler_file<system_handle, Sch>{sch};
				if (auto err = open(file, std::forward<Path>(path), mode, prot); !err) [[likely]]
					return file;
				else
					return err;
			}
		};
	}

	using _file::open_file_t;

	/** Customization point object used to open an asynchronous file handle that can be used to schedule IO operations.
	 * @param[in] sch Scheduler who's execution context to use for the opened file.
	 * @param[in] path Path of the file to be opened.
	 * @param[in] mode Mode flags to use when opening the file.
	 * @param[in] prot Optional protection flags to use for the opened file.
	 * @return `rod::result` containing either the opened file handle or an error code on failure to open the file. */
	inline constexpr auto open_file = open_file_t{};

	/** Handle to a native file used for synchronous IO operations. */
	using basic_file = _file::basic_file<_file::system_handle>;
}

/*
 * Created by switchblade on 2023-05-22.
 */

#pragma once

#include "../../result.hpp"
#include "../io_buffer.hpp"

namespace rod
{
	namespace _read_some
	{
		struct read_some_t
		{
			template<typename Hnd, io_buffer Dst> requires tag_invocable<read_some_t, Hnd, Dst>
			[[nodiscard]] result<std::size_t, std::error_code> operator()(Hnd &&hnd, Dst &&dst) const noexcept(nothrow_tag_invocable<read_some_t, Hnd, Dst>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd), std::forward<Dst>(dst));
			}
		};
		struct read_some_at_t
		{
			template<typename Hnd, std::integral Pos, io_buffer Dst> requires tag_invocable<read_some_at_t, Hnd, Pos, Dst>
			[[nodiscard]] result<std::size_t, std::error_code> operator()(Hnd &&hnd, Pos pos, Dst &&dst) const noexcept(nothrow_tag_invocable<read_some_at_t, Hnd, Pos, Dst>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd), pos, std::forward<Dst>(dst));
			}
		};

		template<typename T, typename Snd, typename... Args>
		concept value_overload = detail::tag_invocable_with_completion_scheduler<T, set_value_t, Snd, Snd, Args...>;
		template<typename Snd>
		concept read_some_sender = sender_of<Snd, set_value_t(std::size_t)> || sender_of<Snd, set_value_t(std::size_t, std::error_code)>;

		template<typename Snd>
		using value_scheduler = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));

		class async_read_some_t
		{
			template<typename Hnd, typename Dst>
			using back_adaptor = detail::back_adaptor<async_read_some_t, Hnd, std::decay_t<Dst>>;

		public:
			template<sender Snd, typename Hnd, io_buffer Dst> requires value_overload<async_read_some_t, Snd, Hnd, Dst>
			[[nodiscard]] read_some_sender auto operator()(Snd &&snd, Hnd &&hnd, Dst &&dst) const noexcept(nothrow_tag_invocable<async_read_some_t, value_scheduler<Snd>, Snd, Hnd, Dst>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::forward<Hnd>(hnd), std::forward<Dst>(dst));
			}
			template<sender Snd, typename Hnd, io_buffer Dst> requires(!value_overload<async_read_some_t, Snd, Hnd, Dst> && tag_invocable<async_read_some_t, Snd, Hnd, Dst>)
			[[nodiscard]] read_some_sender auto operator()(Snd &&snd, Hnd &&hnd, Dst &&dst) const noexcept(nothrow_tag_invocable<async_read_some_t, Snd, Hnd, Dst>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<Hnd>(hnd), std::forward<Dst>(dst));
			}

			template<typename Hnd, io_buffer Dst>
			[[nodiscard]] back_adaptor<Hnd, Dst> operator()(Hnd &&hnd, Dst &&dst) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Hnd, Dst>, async_read_some_t, Hnd, Dst>)
			{
				return back_adaptor<Hnd, Dst>{*this, std::forward_as_tuple(std::forward<Hnd>(hnd), std::forward<Dst>(dst))};
			}
		};
		class async_read_some_at_t
		{
			template<typename Hnd, typename Pos, typename Dst>
			using back_adaptor = detail::back_adaptor<async_read_some_at_t, Hnd, std::decay_t<Pos>, std::decay_t<Dst>>;

		public:
			template<sender Snd, typename Hnd, std::integral Pos, io_buffer Dst> requires value_overload<async_read_some_at_t, Snd, Hnd, Pos, Dst>
			[[nodiscard]] read_some_sender auto operator()(Snd &&snd, Hnd &&hnd, Pos pos, Dst &&dst) const noexcept(nothrow_tag_invocable<async_read_some_at_t, value_scheduler<Snd>, Snd, Hnd, Pos, Dst>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::forward<Hnd>(hnd), pos, std::forward<Dst>(dst));
			}
			template<sender Snd, typename Hnd, std::integral Pos, io_buffer Dst> requires(!value_overload<async_read_some_at_t, Snd, Hnd, Pos, Dst> && tag_invocable<async_read_some_at_t, Snd, Hnd, Pos, Dst>)
			[[nodiscard]] read_some_sender auto operator()(Snd &&snd, Hnd &&hnd, Pos pos, Dst &&dst) const noexcept(nothrow_tag_invocable<async_read_some_at_t, Snd, Hnd, Pos, Dst>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<Hnd>(hnd), pos, std::forward<Dst>(dst));
			}

			template<typename Hnd, std::integral Pos, io_buffer Dst>
			[[nodiscard]] back_adaptor<Hnd, Pos, Dst> operator()(Hnd &&hnd, Pos pos, Dst &&dst) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Hnd, Pos, Dst>, async_read_some_at_t, Hnd, Pos, Dst>)
			{
				return back_adaptor<Hnd, Pos, Dst>{*this, std::forward_as_tuple(std::forward<Hnd>(hnd), pos, std::forward<Dst>(dst))};
			}
		};

		struct schedule_read_some_t
		{
			template<scheduler Sch, typename Hnd, io_buffer Dst> requires tag_invocable<schedule_read_some_t, Sch, Hnd, Dst>
			[[nodiscard]] read_some_sender auto operator()(Sch &&sch, Hnd &&hnd, Dst &&dst) const noexcept(nothrow_tag_invocable<schedule_read_some_t, Sch, Hnd, Dst>)
			{
				return tag_invoke(*this, std::forward<Sch>(sch), std::forward<Hnd>(hnd), std::forward<Dst>(dst));
			}
			template<scheduler Sch, typename Hnd, io_buffer Dst> requires(!tag_invocable<schedule_read_some_t, Sch, Hnd, Dst> && detail::callable<async_read_some_t, schedule_result_t<Sch>, Hnd, Dst>)
			[[nodiscard]] read_some_sender auto operator()(Sch &&sch, Hnd &&hnd, Dst &&dst) const noexcept(detail::nothrow_callable<schedule_t, Sch> && detail::nothrow_callable<async_read_some_t, schedule_result_t<Sch>, Hnd, Dst>)
			{
				return async_read_some_t{}(schedule(std::forward<Sch>(sch)), std::forward<Hnd>(hnd), std::forward<Dst>(dst));
			}
		};
		struct schedule_read_some_at_t
		{
			template<scheduler Sch, typename Hnd, std::integral Pos, io_buffer Dst> requires tag_invocable<schedule_read_some_at_t, Sch, Hnd, Pos, Dst>
			[[nodiscard]] read_some_sender auto operator()(Sch &&sch, Hnd &&hnd, Pos pos, Dst &&dst) const noexcept(nothrow_tag_invocable<schedule_read_some_at_t, Sch, Hnd, Pos, Dst>)
			{
				return tag_invoke(*this, std::forward<Sch>(sch), std::forward<Hnd>(hnd), pos, std::forward<Dst>(dst));
			}
			template<scheduler Sch, typename Hnd, std::integral Pos, io_buffer Dst> requires(!tag_invocable<schedule_read_some_at_t, Sch, Hnd, Pos, Dst> && detail::callable<async_read_some_at_t, schedule_result_t<Sch>, Hnd, Pos, Dst>)
			[[nodiscard]] read_some_sender auto operator()(Sch &&sch, Hnd &&hnd, Pos pos, Dst &&dst) const noexcept(detail::nothrow_callable<schedule_t, Sch> && detail::nothrow_callable<async_read_some_at_t, schedule_result_t<Sch>, Hnd, Pos, Dst>)
			{
				return async_read_some_at_t{}(schedule(std::forward<Sch>(sch)), std::forward<Hnd>(hnd), pos, std::forward<Dst>(dst));
			}
		};
	}

	using _read_some::read_some_t;

	/** Customization point object used to read a contiguous buffer of integral values from a readable source handle.
	 * @param[in] hnd Handle to read the data from.
	 * @param[out] dst Contiguous output range of integral values.
	 * @return `rod::result<std::size_t, std::error_code>` indicating the amount of bytes read or an error code on read failure. */
	inline constexpr auto read_some = read_some_t{};

	using _read_some::read_some_at_t;

	/** Customization point object used to read a contiguous buffer of integral values from a readable source handle at the specified offset.
	 * @param[in] hnd Handle to read the data from.
	 * @param[in] pos Offset into the source handle at which to read the data.
	 * @param[out] dst Contiguous output range of integral values.
	 * @return `rod::result<std::size_t, std::error_code>` indicating the amount of bytes read or an error code on read failure. */
	inline constexpr auto read_some_at = read_some_at_t{};

	using _read_some::async_read_some_t;

	/** Customization point object returning a sender used to read a contiguous buffer of integral values from an async-readable source handle.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] snd Input sender who's value completion channel will be used for the async read operation. If omitted, creates a pipe-able sender adaptor.
	 * @param[in] hnd Handle to read the data from.
	 * @param[out] dst Contiguous output range of integral values.
	 * @return Sender completing with the amount of bytes read and/or an error code on read failure. */
	inline constexpr auto async_read_some = async_read_some_t{};

	using _read_some::async_read_some_at_t;

	/** Customization point object returning a sender used to read a contiguous buffer of integral values from an async-readable source handle.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] snd Input sender who's value completion channel will be used for the async read operation. If omitted, creates a pipe-able sender adaptor.
	 * @param[in] hnd Handle to read the data from.
	 * @param[in] pos Offset into the source handle at which to read the data.
	 * @param[out] dst Contiguous output range of integral values.
	 * @return Sender completing with the amount of bytes read and/or an error code on read failure. */
	inline constexpr auto async_read_some_at = async_read_some_at_t{};

	using _read_some::schedule_read_some_t;

	/** Customization point object used to schedule an asynchronous read operation using the specified scheduler.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] sch Scheduler used to schedule the read operation.
	 * @param[in] hnd Handle to read the data from.
	 * @param[out] dst Contiguous output range of integral values.
	 * @return Sender completing on \a sch with the amount of bytes read and/or an error code on read failure. */
	inline constexpr auto schedule_read_some = schedule_read_some_t{};

	using _read_some::schedule_read_some_at_t;

	/** Customization point object used to schedule an asynchronous read operation at the specified position using the specified scheduler.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] sch Scheduler used to schedule the read operation.
	 * @param[in] hnd Handle to read the data from.
	 * @param[in] pos Offset into the source handle at which to read the data.
	 * @param[out] dst Contiguous output range of integral values.
	 * @return Sender completing on \a sch with the amount of bytes read and/or an error code on read failure. */
	inline constexpr auto schedule_read_some_at = schedule_read_some_at_t{};
}
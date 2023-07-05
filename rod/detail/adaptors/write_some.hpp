/*
 * Created by switchblade on 2023-05-22.
 */

#pragma once

#include "sync_io.hpp"

namespace rod
{
	namespace _write_some
	{
		struct write_some_t
		{
			template<typename Hnd, byte_buffer Src> requires tag_invocable<write_some_t, Hnd, Src>
			[[nodiscard]] result<std::size_t, std::error_code> operator()(Hnd &&hnd, Src &&src) const noexcept(nothrow_tag_invocable<write_some_t, Hnd, Src>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd), std::forward<Src>(src));
			}
		};
		struct write_some_at_t
		{
			template<typename Hnd, std::integral Pos, byte_buffer Src> requires tag_invocable<write_some_at_t, Hnd, Pos, Src>
			[[nodiscard]] result<std::size_t, std::error_code> operator()(Hnd &&hnd, Pos pos, Src &&src) const noexcept(nothrow_tag_invocable<write_some_at_t, Hnd, Pos, Src>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd), pos, std::forward<Src>(src));
			}
		};

		template<typename T, typename Snd, typename... Args>
		concept value_overload = detail::tag_invocable_with_completion_scheduler<T, set_value_t, Snd, Snd, Args...>;
		template<typename Snd>
		concept write_some_sender = sender_of<Snd, set_value_t(std::size_t)>;

		template<typename Snd>
		using value_scheduler = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));

		class async_write_some_t
		{
			template<typename Snd, typename Hnd, typename Dst>
			using sync_sender = typename _sync_io::sender<write_some_t, std::decay_t<Snd>, Hnd, std::decay_t<Dst>>::type;
			template<typename Hnd, typename Src>
			using back_adaptor = detail::back_adaptor<async_write_some_t, Hnd, std::decay_t<Src>>;

		public:
			template<sender Snd, typename Hnd, byte_buffer Src> requires value_overload<async_write_some_t, Snd, Hnd, Src>
			[[nodiscard]] write_some_sender auto operator()(Snd &&snd, Hnd &&hnd, Src &&src) const noexcept(nothrow_tag_invocable<async_write_some_t, value_scheduler<Snd>, Snd, Hnd, Src>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::forward<Hnd>(hnd), std::forward<Src>(src));
			}
			template<sender Snd, typename Hnd, byte_buffer Src> requires(!value_overload<async_write_some_t, Snd, Hnd, Src> && tag_invocable<async_write_some_t, Snd, Hnd, Src>)
			[[nodiscard]] write_some_sender auto operator()(Snd &&snd, Hnd &&hnd, Src &&src) const noexcept(nothrow_tag_invocable<async_write_some_t, Snd, Hnd, Src>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<Hnd>(hnd), std::forward<Src>(src));
			}
			template<sender Snd, typename Hnd, byte_buffer Src> requires(!value_overload<async_write_some_t, Snd, Hnd, Src> && !tag_invocable<async_write_some_t, Snd, Hnd, Src> && tag_invocable<write_some_t, Hnd, Src>)
			[[nodiscard]] sync_sender<Snd, Hnd, Src> operator()(Snd &&snd, Hnd &&hnd, Src &&src) const noexcept(std::is_nothrow_constructible_v<sync_sender<Snd, Hnd, Src>, Snd, Hnd, Src>)
			{
				return sync_sender<Snd, Hnd, Src>{std::forward<Snd>(snd), std::forward<Hnd>(hnd), std::forward<Src>(src)};
			}

			template<typename Hnd, byte_buffer Src>
			[[nodiscard]] back_adaptor<Hnd, Src> operator()(Hnd &&hnd, Src &&src) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Hnd, Src>, async_write_some_t, Hnd, Src>)
			{
				return back_adaptor<Hnd, Src>{*this, std::forward_as_tuple(std::forward<Hnd>(hnd), std::forward<Src>(src))};
			}
		};
		class async_write_some_at_t
		{
			template<typename Snd, typename Hnd, typename Pos, typename Dst>
			using sync_sender = typename _sync_io::sender<write_some_at_t, std::decay_t<Snd>, Hnd, Pos, std::decay_t<Dst>>::type;
			template<typename Hnd, typename Pos, typename Src>
			using back_adaptor = detail::back_adaptor<async_write_some_at_t, Hnd, std::decay_t<Pos>, std::decay_t<Src>>;

		public:
			template<sender Snd, typename Hnd, std::integral Pos, byte_buffer Src> requires value_overload<async_write_some_at_t, Snd, Hnd, Pos, Src>
			[[nodiscard]] write_some_sender auto operator()(Snd &&snd, Hnd &&hnd, Pos pos, Src &&src) const noexcept(nothrow_tag_invocable<async_write_some_at_t, value_scheduler<Snd>, Snd, Hnd, Pos, Src>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::forward<Hnd>(hnd), pos, std::forward<Src>(src));
			}
			template<sender Snd, typename Hnd, std::integral Pos, byte_buffer Src> requires(!value_overload<async_write_some_at_t, Snd, Hnd, Pos, Src> && tag_invocable<async_write_some_at_t, Snd, Hnd, Pos, Src>)
			[[nodiscard]] write_some_sender auto operator()(Snd &&snd, Hnd &&hnd, Pos pos, Src &&src) const noexcept(nothrow_tag_invocable<async_write_some_at_t, Snd, Hnd, Pos, Src>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<Hnd>(hnd), pos, std::forward<Src>(src));
			}
			template<sender Snd, typename Hnd, std::integral Pos, byte_buffer Src> requires(!value_overload<async_write_some_at_t, Snd, Hnd, Pos, Src> && !tag_invocable<async_write_some_at_t, Snd, Hnd, Pos, Src> && tag_invocable<write_some_at_t, Hnd, Pos, Src>)
			[[nodiscard]] sync_sender<Snd, Hnd, Pos, Src> operator()(Snd &&snd, Hnd &&hnd, Pos pos, Src &&src) const noexcept(std::is_nothrow_constructible_v<sync_sender<Snd, Hnd, Pos, Src>, Snd, Hnd, Pos, Src>)
			{
				return sync_sender<Snd, Hnd, Pos, Src>{std::forward<Snd>(snd), std::forward<Hnd>(hnd), pos, std::forward<Src>(src)};
			}

			template<typename Hnd, std::integral Pos, byte_buffer Src>
			[[nodiscard]] back_adaptor<Hnd, Pos, Src> operator()(Hnd &&hnd, Pos pos, Src &&src) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Hnd, Pos, Src>, async_write_some_at_t, Hnd, Pos, Src>)
			{
				return back_adaptor<Hnd, Pos, Src>{*this, std::forward_as_tuple(std::forward<Hnd>(hnd), pos, std::forward<Src>(src))};
			}
		};

		struct schedule_write_some_t
		{
			template<scheduler Sch, typename Hnd, byte_buffer Src> requires tag_invocable<schedule_write_some_t, Sch, Hnd, Src>
			[[nodiscard]] write_some_sender auto operator()(Sch &&sch, Hnd &&hnd, Src &&src) const noexcept(nothrow_tag_invocable<schedule_write_some_t, Sch, Hnd, Src>)
			{
				return tag_invoke(*this, std::forward<Sch>(sch), std::forward<Hnd>(hnd), std::forward<Src>(src));
			}
			template<scheduler Sch, typename Hnd, byte_buffer Src> requires(!tag_invocable<schedule_write_some_t, Sch, Hnd, Src> && detail::callable<async_write_some_t, schedule_result_t<Sch>, Hnd, Src>)
			[[nodiscard]] write_some_sender auto operator()(Sch &&sch, Hnd &&hnd, Src &&src) const noexcept(detail::nothrow_callable<schedule_t, Sch> && detail::nothrow_callable<async_write_some_t, schedule_result_t<Sch>, Hnd, Src>)
			{
				return async_write_some_t{}(schedule(std::forward<Sch>(sch)), std::forward<Hnd>(hnd), std::forward<Src>(src));
			}
		};
		struct schedule_write_some_at_t
		{
			template<scheduler Sch, typename Hnd, std::integral Pos, byte_buffer Src> requires tag_invocable<schedule_write_some_at_t, Sch, Hnd, Pos, Src>
			[[nodiscard]] write_some_sender auto operator()(Sch &&sch, Hnd &&hnd, Pos pos, Src &&src) const noexcept(nothrow_tag_invocable<schedule_write_some_at_t, Sch, Hnd, Pos, Src>)
			{
				return tag_invoke(*this, std::forward<Sch>(sch), std::forward<Hnd>(hnd), pos, std::forward<Src>(src));
			}
			template<scheduler Sch, typename Hnd, std::integral Pos, byte_buffer Src> requires(!tag_invocable<schedule_write_some_at_t, Sch, Hnd, Pos, Src> && detail::callable<async_write_some_at_t, schedule_result_t<Sch>, Hnd, Pos, Src>)
			[[nodiscard]] write_some_sender auto operator()(Sch &&sch, Hnd &&hnd, Pos pos, Src &&src) const noexcept(detail::nothrow_callable<schedule_t, Sch> && detail::nothrow_callable<async_write_some_at_t, schedule_result_t<Sch>, Hnd, Pos, Src>)
			{
				return async_write_some_at_t{}(schedule(std::forward<Sch>(sch)), std::forward<Hnd>(hnd), pos, std::forward<Src>(src));
			}
		};
	}

	using _write_some::write_some_t;

	/** Customization point object used to write a contiguous buffer of integral values to a writeable destination object.
	 * @param[in] hnd Handle to write the data into.
	 * @param[in] src Input buffer or bytes.
	 * @return `rod::result&lt;std::size_t, std::error_code&gt;` indicating the amount of bytes written or an error code on write failure. */
	inline constexpr auto write_some = write_some_t{};

	using _write_some::write_some_at_t;

	/** Customization point object used to write a contiguous buffer of integral values to a writeable destination object at the specified offset.
	 * @param[in] hnd Handle to write the data into.
	 * @param[in] pos Offset into the destination object at which to write the data.
	 * @param[in] src Input buffer or bytes.
	 * @return `rod::result&lt;std::size_t, std::error_code&gt;` indicating the amount of bytes written or an error code on write failure. */
	inline constexpr auto write_some_at = write_some_at_t{};

	using _write_some::async_write_some_t;

	/** Customization point object returning a sender used to write a contiguous buffer of integral values to an async-writeable destination object.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] snd Input sender who's value completion channel will be used for the async write operation. If omitted, creates a pipe-able sender adaptor.
	 * @param[in] hnd Handle to write the data into.
	 * @param[in] src Input buffer or bytes.
	 * @return Sender completing with the amount of bytes written or an error code on write failure. */
	inline constexpr auto async_write_some = async_write_some_t{};

	using _write_some::async_write_some_at_t;

	/** Customization point object returning a sender used to write a contiguous buffer of integral values to an async-writeable destination object.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] snd Input sender who's value completion channel will be used for the async write operation. If omitted, creates a pipe-able sender adaptor.
	 * @param[in] hnd Handle to write the data into.
	 * @param[in] pos Offset into the destination object at which to write the data.
	 * @param[in] src Input buffer or bytes.
	 * @return Sender completing with the amount of bytes written or an error code on write failure. */
	inline constexpr auto async_write_some_at = async_write_some_at_t{};

	using _write_some::schedule_write_some_t;

	/** Customization point object used to schedule an asynchronous write operation using the specified scheduler.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] sch Scheduler used to schedule the write operation.
	 * @param[in] hnd Handle to write the data into.
	 * @param[in] src Input buffer or bytes.
	 * @return Sender completing on \a sch with the amount of bytes written or an error code on write failure. */
	inline constexpr auto schedule_write_some = schedule_write_some_t{};

	using _write_some::schedule_write_some_at_t;

	/** Customization point object used to schedule an asynchronous write operation at the specified position using the specified scheduler.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] sch Scheduler used to schedule the write operation.
	 * @param[in] hnd Handle to write the data into.
	 * @param[in] pos Offset into the destination object at which to write the data.
	 * @param[in] src Input buffer or bytes.
	 * @return Sender completing on \a sch with the amount of bytes written or an error code on write failure. */
	inline constexpr auto schedule_write_some_at = schedule_write_some_at_t{};
}
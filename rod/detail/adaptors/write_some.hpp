/*
 * Created by switchblade on 2023-05-22.
 */

#pragma once

#include "../io_buffer.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	namespace _write_some
	{
		struct write_some_t
		{
			template<typename Hnd, io_buffer Src> requires tag_invocable<write_some_t, Hnd, Src, std::error_code &>
			[[nodiscard]] std::size_t operator()(Hnd &&hnd, Src &&src, std::error_code &err) const noexcept(nothrow_tag_invocable<write_some_t, Hnd, Src, std::error_code &>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd), std::forward<Src>(src), err);
			}
			template<typename Hnd, io_buffer Src> requires tag_invocable<write_some_t, Hnd, Src>
			[[nodiscard]] std::size_t operator()(Hnd &&hnd, Src &&src) const noexcept(nothrow_tag_invocable<write_some_t, Hnd, Src>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd), std::forward<Src>(src));
			}
			template<typename Hnd, io_buffer Src> requires(!tag_invocable<write_some_t, Hnd, Src>)
			[[nodiscard]] std::size_t operator()(Hnd &&hnd, Src &&src) const
			{
				static_assert(tag_invocable<write_some_t, Hnd, Src, std::error_code &>);

				std::error_code err = {};
				if (const auto res = operator()(std::forward<Hnd>(hnd), std::forward<Src>(src), err); err)
					throw std::system_error(err);
				else
					return res;
			}
		};

		struct write_some_at_t
		{
			template<typename Hnd, std::integral Pos, io_buffer Src> requires tag_invocable<write_some_at_t, Hnd, Pos, Src, std::error_code &>
			[[nodiscard]] std::size_t operator()(Hnd &&hnd, Pos pos, Src &&src, std::error_code &err) const noexcept(nothrow_tag_invocable<write_some_at_t, Hnd, Pos, Src, std::error_code &>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd), pos, std::forward<Src>(src), err);
			}
			template<typename Hnd, std::integral Pos, io_buffer Src> requires tag_invocable<write_some_at_t, Hnd, Pos, Src>
			[[nodiscard]] std::size_t operator()(Hnd &&hnd, Pos pos, Src &&src) const noexcept(nothrow_tag_invocable<write_some_at_t, Hnd, Pos, Src>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd), pos, std::forward<Src>(src));
			}
			template<typename Hnd, std::integral Pos, io_buffer Src> requires(!tag_invocable<write_some_at_t, Hnd, Pos, Src>)
			[[nodiscard]] std::size_t operator()(Hnd &&hnd, Pos pos, Src &&src) const
			{
				static_assert(tag_invocable<write_some_at_t, Hnd, Pos, Src, std::error_code &>);

				std::error_code err = {};
				if (const auto res = operator()(std::forward<Hnd>(hnd), pos, std::forward<Src>(src), err); err)
					throw std::system_error(err);
				else
					return res;
			}
		};

		template<typename Snd>
		concept write_some_sender = sender_of<Snd, set_value_t(std::size_t)> || sender_of<Snd, set_value_t(std::size_t, std::error_code)>;
		template<typename T, typename Snd, typename... Args>
		concept value_overload = detail::tag_invocable_with_completion_scheduler<T, set_value_t, Snd, Snd, Args...>;

		template<typename Snd>
		using value_scheduler = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));

		class async_write_some_t
		{
			template<typename Hnd, typename Src>
			using back_adaptor = detail::back_adaptor<async_write_some_t, Hnd, std::decay_t<Src>>;

		public:
			template<sender Snd, typename Hnd, io_buffer Src> requires (value_overload<async_write_some_t, Snd, Hnd, Src>)
			[[nodiscard]] write_some_sender decltype(auto) operator()(Snd &&snd, Hnd &&hnd, Src &&src) const noexcept(nothrow_tag_invocable<async_write_some_t, value_scheduler<Snd>, Snd, Hnd, Src>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::forward<Hnd>(hnd), std::forward<Src>(src));
			}
			template<sender Snd, typename Hnd, io_buffer Src> requires(!value_overload<async_write_some_t, Snd, Hnd, Src> && tag_invocable<async_write_some_t, Snd, Hnd, Src>)
			[[nodiscard]] write_some_sender decltype(auto) operator()(Snd &&snd, Hnd &&hnd, Src &&src) const noexcept(nothrow_tag_invocable<async_write_some_t, Snd, Hnd, Src>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<Hnd>(hnd), std::forward<Src>(src));
			}

			template<typename Hnd, io_buffer Src>
			[[nodiscard]] back_adaptor<Hnd, Src> operator()(Hnd &&hnd, Src &&src) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Hnd, Src>, async_write_some_t, Hnd, Src>)
			{
				return back_adaptor<Hnd, Src>{*this, std::forward_as_tuple(std::forward<Hnd>(hnd), std::forward<Src>(src))};
			}
		};

		class async_write_some_at_t
		{
			template<typename Hnd, typename Pos, typename Src>
			using back_adaptor = detail::back_adaptor<async_write_some_at_t, Hnd, std::decay_t<Pos>, std::decay_t<Src>>;

		public:
			template<sender Snd, typename Hnd, std::integral Pos, io_buffer Src> requires (value_overload<async_write_some_at_t, Snd, Hnd, Pos, Src>)
			[[nodiscard]] write_some_sender decltype(auto) operator()(Snd &&snd, Hnd &&hnd, Pos pos, Src &&src) const noexcept(nothrow_tag_invocable<async_write_some_at_t, value_scheduler<Snd>, Snd, Hnd, Pos, Src>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::forward<Hnd>(hnd), pos, std::forward<Src>(src));
			}
			template<sender Snd, typename Hnd, std::integral Pos, io_buffer Src> requires(!value_overload<async_write_some_at_t, Snd, Hnd, Pos, Src> && tag_invocable<async_write_some_at_t, Snd, Hnd, Pos, Src>)
			[[nodiscard]] write_some_sender decltype(auto) operator()(Snd &&snd, Hnd &&hnd, Pos pos, Src &&src) const noexcept(nothrow_tag_invocable<async_write_some_at_t, Snd, Hnd, Pos, Src>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<Hnd>(hnd), pos, std::forward<Src>(src));
			}

			template<typename Hnd, std::integral Pos, io_buffer Src>
			[[nodiscard]] back_adaptor<Hnd, Pos, Src> operator()(Hnd &&hnd, Pos pos, Src &&src) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Hnd, Pos, Src>, async_write_some_at_t, Hnd, Pos, Src>)
			{
				return back_adaptor<Hnd, Pos, Src>{*this, std::forward_as_tuple(std::forward<Hnd>(hnd), pos, std::forward<Src>(src))};
			}
		};
	}

	using _write_some::write_some_t;

	/** Customization point object used to write a contiguous srcer of integral values to a writeable destination object.
	 * @param[in] hnd Handle to write the data into.
	 * @param[in] src Contiguous input range of integral values.
	 * @param[out] err Reference to the error code set on failure to complete the operation. If omitted, an exception is thrown instead.
	 * @throw std::system_error If an error has occurred and no output error code has been specified.
	 * @return Amount of elements written. */
	inline constexpr auto write_some = write_some_t{};

	using _write_some::write_some_at_t;

	/** Customization point object used to write a contiguous srcer of integral values to a writeable destination object at the specified offset.
	 * @param[in] hnd Handle to write the data into.
	 * @param[in] pos Offset into the destination object at which to write the values.
	 * @param[in] src Contiguous input range of integral values.
	 * @param[out] err Reference to the error code set on failure to complete the operation. If omitted, an exception is thrown instead.
	 * @throw std::system_error If an error has occurred and no output error code has been specified.
	 * @return Amount of elements written. */
	inline constexpr auto write_some_at = write_some_at_t{};

	using _write_some::async_write_some_t;

	/** Customization point object returning a sender used to write a contiguous srcer of integral values to an async-writeable destination object.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] snd Input sender who's value completion channel will be used for the async write operation. If omitted, creates a pipe-able sender adaptor.
	 * @param[in] hnd Handle to write the data into.
	 * @param[in] src Contiguous input range of integral values.
	 * @return Sender completing either with the amount of elements read and an optional error code, or an error on read failure. */
	inline constexpr auto async_write_some = async_write_some_t{};

	using _write_some::async_write_some_at_t;

	/** Customization point object returning a sender used to write a contiguous srcer of integral values to an async-writeable destination object.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] snd Input sender who's value completion channel will be used for the async write operation. If omitted, creates a pipe-able sender adaptor.
	 * @param[in] hnd Destination object used to write the data into.
	 * @param[in] pos Offset into the destination object at which to write the values.
	 * @param[in] src Contiguous input range of integral values.
	 * @return Sender completing either with the amount of elements read and an optional error code, or an error on read failure. */
	inline constexpr auto async_write_some_at = async_write_some_at_t{};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
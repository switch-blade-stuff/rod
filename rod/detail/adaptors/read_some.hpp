/*
 * Created by switchblade on 2023-05-22.
 */

#pragma once

#include "../io_buffer.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	namespace _read_some
	{
		struct read_some_t
		{
			template<typename Hnd, io_buffer Dst> requires tag_invocable<read_some_t, Hnd, Dst, std::error_code &>
			[[nodiscard]] std::size_t operator()(Hnd &&hnd, Dst &&dst, std::error_code &err) const noexcept(nothrow_tag_invocable<read_some_t, Hnd, Dst, std::error_code &>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd), std::forward<Dst>(dst), err);
			}
			template<typename Hnd, io_buffer Dst> requires tag_invocable<read_some_t, Hnd, Dst>
			[[nodiscard]] std::size_t operator()(Hnd &&hnd, Dst &&dst) const noexcept(nothrow_tag_invocable<read_some_t, Hnd, Dst>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd), std::forward<Dst>(dst));
			}
			template<typename Hnd, io_buffer Dst> requires(!tag_invocable<read_some_t, Hnd, Dst>)
			[[nodiscard]] std::size_t operator()(Hnd &&hnd, Dst &&dst) const
			{
				static_assert(tag_invocable<read_some_t, Hnd, Dst, std::error_code &>);

				std::error_code err = {};
				if (const auto res = operator()(std::forward<Hnd>(hnd), std::forward<Dst>(dst), err); err)
					throw std::system_error(err);
				else
					return res;
			}
		};

		struct read_some_at_t
		{
			template<typename Hnd, std::integral Pos, io_buffer Dst> requires tag_invocable<read_some_at_t, Hnd, Pos, Dst, std::error_code &>
			[[nodiscard]] std::size_t operator()(Hnd &&hnd, Pos pos, Dst &&dst, std::error_code &err) const noexcept(nothrow_tag_invocable<read_some_at_t, Hnd, Pos, Dst, std::error_code &>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd), pos, std::forward<Dst>(dst), err);
			}
			template<typename Hnd, std::integral Pos, io_buffer Dst> requires tag_invocable<read_some_at_t, Hnd, Pos, Dst>
			[[nodiscard]] std::size_t operator()(Hnd &&hnd, Pos pos, Dst &&dst) const noexcept(nothrow_tag_invocable<read_some_at_t, Hnd, Pos, Dst>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd), pos, std::forward<Dst>(dst));
			}
			template<typename Hnd, std::integral Pos, io_buffer Dst> requires(!tag_invocable<read_some_at_t, Hnd, Pos, Dst>)
			[[nodiscard]] std::size_t operator()(Hnd &&hnd, Pos pos, Dst &&dst) const
			{
				static_assert(tag_invocable<read_some_at_t, Hnd, Pos, Dst, std::error_code &>);

				std::error_code err = {};
				if (const auto res = operator()(std::forward<Hnd>(hnd), pos, std::forward<Dst>(dst), err); err)
					throw std::system_error(err);
				else
					return res;
			}
		};

		template<typename Snd>
		concept read_some_sender = sender_of<Snd, set_value_t(std::size_t)> || sender_of<Snd, set_value_t(std::size_t, std::error_code)>;
		template<typename T, typename Snd, typename... Args>
		concept value_overload = detail::tag_invocable_with_completion_scheduler<T, set_value_t, Snd, Snd, Args...>;

		template<typename Snd>
		using value_scheduler = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));

		class async_read_some_t
		{
			template<typename Hnd, typename Dst>
			using back_adaptor = detail::back_adaptor<async_read_some_t, Hnd, std::decay_t<Dst>>;

		public:
			template<sender Snd, typename Hnd, io_buffer Dst> requires value_overload<async_read_some_t, Snd, Hnd, Dst>
			[[nodiscard]] read_some_sender decltype(auto) operator()(Snd &&snd, Hnd &&hnd, Dst &&dst) const noexcept(nothrow_tag_invocable<async_read_some_t, value_scheduler<Snd>, Snd, Hnd, Dst>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::forward<Hnd>(hnd), std::forward<Dst>(dst));
			}
			template<sender Snd, typename Hnd, io_buffer Dst> requires(!value_overload<async_read_some_t, Snd, Hnd, Dst> && tag_invocable<async_read_some_t, Snd, Hnd, Dst>)
			[[nodiscard]] read_some_sender decltype(auto) operator()(Snd &&snd, Hnd &&hnd, Dst &&dst) const noexcept(nothrow_tag_invocable<async_read_some_t, Snd, Hnd, Dst>)
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
			[[nodiscard]] read_some_sender decltype(auto) operator()(Snd &&snd, Hnd &&hnd, Pos pos, Dst &&dst) const noexcept(nothrow_tag_invocable<async_read_some_at_t, value_scheduler<Snd>, Snd, Hnd, Pos, Dst>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::forward<Hnd>(hnd), pos, std::forward<Dst>(dst));
			}
			template<sender Snd, typename Hnd, std::integral Pos, io_buffer Dst> requires(!value_overload<async_read_some_at_t, Snd, Hnd, Pos, Dst> && tag_invocable<async_read_some_at_t, Snd, Hnd, Pos, Dst>)
			[[nodiscard]] read_some_sender decltype(auto) operator()(Snd &&snd, Hnd &&hnd, Pos pos, Dst &&dst) const noexcept(nothrow_tag_invocable<async_read_some_at_t, Snd, Hnd, Pos, Dst>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<Hnd>(hnd), pos, std::forward<Dst>(dst));
			}

			template<typename Hnd, std::integral Pos, io_buffer Dst>
			[[nodiscard]] back_adaptor<Hnd, Pos, Dst> operator()(Hnd &&hnd, Pos pos, Dst &&dst) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Hnd, Pos, Dst>, async_read_some_at_t, Hnd, Pos, Dst>)
			{
				return back_adaptor<Hnd, Pos, Dst>{*this, std::forward_as_tuple(std::forward<Hnd>(hnd), pos, std::forward<Dst>(dst))};
			}
		};
	}

	using _read_some::read_some_t;

	/** Customization point object used to read a contiguous dster of integral values from a readable source object.
	 * @param[in] hnd Handle to read the data from.
	 * @param[out] dst Contiguous output range of integral values.
	 * @param[out] err Reference to the error code set on failure to complete the operation. If omitted, an exception is thrown instead.
	 * @throw std::system_error If an error has occurred and no output error code has been specified.
	 * @return Amount of elements read. */
	inline constexpr auto read_some = read_some_t{};

	using _read_some::read_some_at_t;

	/** Customization point object used to read a contiguous dster of integral values from a readable source object at the specified offset.
	 * @param[in] hnd Handle to read the data from.
	 * @param[in] pos Offset into the source object at which to read the values.
	 * @param[out] dst Contiguous output range of integral values.
	 * @param[out] err Reference to the error code set on failure to complete the operation. If omitted, an exception is thrown instead.
	 * @throw std::system_error If an error has occurred and no output error code has been specified.
	 * @return Amount of elements read. */
	inline constexpr auto read_some_at = read_some_at_t{};

	using _read_some::async_read_some_t;

	/** Customization point object returning a sender used to read a contiguous dster of integral values from an async-readable source object.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] snd Input sender who's value completion channel will be used for the async read operation. If omitted, creates a pipe-able sender adaptor.
	 * @param[in] hnd Handle to read the data from.
	 * @param[out] dst Contiguous output range of integral values.
	 * @return Sender completing either with the amount of elements read and an optional error code, or an error on read failure. */
	inline constexpr auto async_read_some = async_read_some_t{};

	using _read_some::async_read_some_at_t;

	/** Customization point object returning a sender used to read a contiguous dster of integral values from an async-readable source object.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] snd Input sender who's value completion channel will be used for the async read operation. If omitted, creates a pipe-able sender adaptor.
	 * @param[in] hnd Handle to read the data from.
	 * @param[out] dst Contiguous output range of integral values.
	 * @return Sender completing either with the amount of elements read and an optional error code, or an error on read failure. */
	inline constexpr auto async_read_some_at = async_read_some_at_t{};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
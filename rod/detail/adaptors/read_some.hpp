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
			template<typename Src, io_buffer Buff> requires tag_invocable<read_some_t, Src, Buff, std::error_code &>
			[[nodiscard]] std::size_t operator()(Src &&src, Buff &&buff, std::error_code &err) const noexcept(nothrow_tag_invocable<read_some_t, Src, Buff, std::error_code &>)
			{
				return tag_invoke(*this, std::forward<Src>(src), std::forward<Buff>(buff), err);
			}
			template<typename Src, io_buffer Buff> requires tag_invocable<read_some_t, Src, Buff>
			[[nodiscard]] std::size_t operator()(Src &&src, Buff &&buff) const noexcept(nothrow_tag_invocable<read_some_t, Src, Buff>)
			{
				return tag_invoke(*this, std::forward<Src>(src), std::forward<Buff>(buff));
			}
			template<typename Src, io_buffer Buff> requires(!tag_invocable<read_some_t, Src, Buff>)
			[[nodiscard]] std::size_t operator()(Src &&src, Buff &&buff) const
			{
				static_assert(tag_invocable<read_some_t, Src, Buff, std::error_code &>);

				std::error_code err = {};
				if (const auto res = operator()(std::forward<Src>(src), std::forward<Buff>(buff), err); err)
					throw std::system_error(err);
				else
					return res;
			}
		};

		struct read_some_at_t
		{
			template<typename Src, std::integral Pos, io_buffer Buff> requires tag_invocable<read_some_at_t, Src, Pos, Buff, std::error_code &>
			[[nodiscard]] std::size_t operator()(Src &&src, Pos &&pos, Buff &&buff, std::error_code &err) const noexcept(nothrow_tag_invocable<read_some_at_t, Src, Pos, Buff, std::error_code &>)
			{
				return tag_invoke(*this, std::forward<Src>(src), std::forward<Pos>(pos), std::forward<Buff>(buff), err);
			}
			template<typename Src, std::integral Pos, io_buffer Buff> requires tag_invocable<read_some_at_t, Src, Pos, Buff>
			[[nodiscard]] std::size_t operator()(Src &&src, Pos &&pos, Buff &&buff) const noexcept(nothrow_tag_invocable<read_some_at_t, Src, Pos, Buff>)
			{
				return tag_invoke(*this, std::forward<Src>(src), std::forward<Pos>(pos), std::forward<Buff>(buff));
			}
			template<typename Src, std::integral Pos, io_buffer Buff> requires(!tag_invocable<read_some_at_t, Src, Pos, Buff>)
			[[nodiscard]] std::size_t operator()(Src &&src, Pos &&pos, Buff &&buff) const
			{
				static_assert(tag_invocable<read_some_at_t, Src, Pos, Buff, std::error_code &>);

				std::error_code err = {};
				if (const auto res = operator()(std::forward<Src>(src), std::forward<Pos>(pos), std::forward<Buff>(buff), err); err)
					throw std::system_error(err);
				else
					return res;
			}
		};

		template<typename Snd>
		concept read_some_sender = sender_of<Snd, set_value_t(std::size_t)> || sender_of<Snd, set_value_t(std::size_t, std::error_code)>;

		class async_read_some_t
		{
			template<typename Snd>
			using value_completion = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Src, typename Buff>
			using back_adaptor = detail::back_adaptor<async_read_some_t, Src, std::decay_t<Buff>>;

		public:
			template<sender Snd, typename Src, io_buffer Buff> requires detail::tag_invocable_with_completion_scheduler<async_read_some_t, set_value_t, Snd, Snd, Src, Buff>
			[[nodiscard]] read_some_sender decltype(auto) operator()(Snd &&snd, Src &&src, Buff &&buff) const noexcept(nothrow_tag_invocable<async_read_some_t, value_completion<Snd>, Snd, Src, Buff>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::forward<Src>(src), std::forward<Buff>(buff));
			}
			template<sender Snd, typename Src, io_buffer Buff> requires(!detail::tag_invocable_with_completion_scheduler<async_read_some_t, set_value_t, Snd, Snd, Src, Buff> && tag_invocable<async_read_some_t, Snd, Src, Buff>)
			[[nodiscard]] read_some_sender decltype(auto) operator()(Snd &&snd, Src &&src, Buff &&buff) const noexcept(nothrow_tag_invocable<async_read_some_t, Snd, Src, Buff>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<Src>(src), std::forward<Buff>(buff));
			}

			template<typename Src, io_buffer Buff>
			[[nodiscard]] back_adaptor<Src, Buff> operator()(Src &&src, Buff &&buff) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Src, Buff>, async_read_some_t, Src, Buff>)
			{
				return back_adaptor<Src, Buff>{*this, std::forward_as_tuple(std::forward<Src>(src), std::forward<Buff>(buff))};
			}
		};

		class async_read_some_at_t
		{
			template<typename Snd>
			using value_completion = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Src, typename Pos, typename Buff>
			using back_adaptor = detail::back_adaptor<async_read_some_t, Src, std::decay_t<Pos>, std::decay_t<Buff>>;

		public:
			template<sender Snd, typename Src, std::integral Pos, io_buffer Buff> requires detail::tag_invocable_with_completion_scheduler<async_read_some_at_t, set_value_t, Snd, Snd, Src, Pos, Buff>
			[[nodiscard]] read_some_sender decltype(auto) operator()(Snd &&snd, Src &&src, Pos &&pos, Buff &&buff) const noexcept(nothrow_tag_invocable<async_read_some_at_t, value_completion<Snd>, Snd, Src, Pos, Buff>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::forward<Src>(src), std::forward<Pos>(pos), std::forward<Buff>(buff));
			}
			template<sender Snd, typename Src, std::integral Pos, io_buffer Buff> requires(!detail::tag_invocable_with_completion_scheduler<async_read_some_at_t, set_value_t, Snd, Snd, Src, Pos, Buff> && tag_invocable<async_read_some_at_t, Snd, Src, Pos, Buff>)
			[[nodiscard]] read_some_sender decltype(auto) operator()(Snd &&snd, Src &&src, Pos &&pos, Buff &&buff) const noexcept(nothrow_tag_invocable<async_read_some_at_t, Snd, Src, Pos, Buff>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<Src>(src), std::forward<Pos>(pos), std::forward<Buff>(buff));
			}

			template<typename Src, std::integral Pos, io_buffer Buff>
			[[nodiscard]] back_adaptor<Src, Pos, Buff> operator()(Src &&src, Pos &&pos, Buff &&buff) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Src, Pos, Buff>, async_read_some_t, Src, Pos, Buff>)
			{
				return back_adaptor<Src, Pos, Buff>{*this, std::forward_as_tuple(std::forward<Src>(src), std::forward<Pos>(pos), std::forward<Buff>(buff))};
			}
		};
	}

	using _read_some::read_some_t;

	/** Customization point object used to read a contiguous buffer of integral values from a readable source object.
	 * @param[in] src Source object to read the data from.
	 * @param[out] buff Contiguous output range of integral values.
	 * @param[out] err Reference to the error code set on failure to complete the operation. If omitted, an exception is thrown instead.
	 * @throw std::system_error If an error has occurred and no output error code has been specified.
	 * @return Amount of elements read. */
	inline constexpr auto read_some = read_some_t{};

	using _read_some::read_some_at_t;

	/** Customization point object used to read a contiguous buffer of integral values from a readable source object at the specified offset.
	 * @param[in] src Source object to read the data from.
	 * @param[in] pos Offset into the source object at which to read the values.
	 * @param[out] buff Contiguous output range of integral values.
	 * @param[out] err Reference to the error code set on failure to complete the operation. If omitted, an exception is thrown instead.
	 * @throw std::system_error If an error has occurred and no output error code has been specified.
	 * @return Amount of elements read. */
	inline constexpr auto read_some_at = read_some_at_t{};

	using _read_some::async_read_some_t;

	/** Customization point object returning a sender used to read a contiguous buffer of integral values from an async-readable source object.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] snd Input sender who's value completion channel will be used for the async read operation. If omitted, creates a pipe-able sender adaptor.
	 * @param[in] src Source object used to read the data from.
	 * @param[out] buff Contiguous output range of integral values.
	 * @return Sender completing either with the amount of elements read and an optional error code, or an error on read failure. */
	inline constexpr auto async_read_some = async_read_some_t{};

	using _read_some::async_read_some_at_t;

	/** Customization point object returning a sender used to read a contiguous buffer of integral values from an async-readable source object.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] snd Input sender who's value completion channel will be used for the async read operation. If omitted, creates a pipe-able sender adaptor.
	 * @param[in] src Source object used to read the data from.
	 * @param[out] buff Contiguous output range of integral values.
	 * @return Sender completing either with the amount of elements read and an optional error code, or an error on read failure. */
	inline constexpr auto async_read_some_at = async_read_some_at_t{};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
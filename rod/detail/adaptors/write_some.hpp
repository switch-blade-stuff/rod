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
			template<typename Dst, typename Buff> requires tag_invocable<write_some_t, Dst, Buff, std::error_code &>
			[[nodiscard]] std::size_t operator()(Dst &&dst, Buff &&buff, std::error_code &err) const noexcept(nothrow_tag_invocable<write_some_t, Dst, Buff, std::error_code &>)
			{
				return tag_invoke(*this, std::forward<Dst>(dst), std::forward<Buff>(buff), err);
			}
			template<typename Dst, typename Buff> requires tag_invocable<write_some_t, Dst, Buff>
			[[nodiscard]] std::size_t operator()(Dst &&dst, Buff &&buff) const noexcept(nothrow_tag_invocable<write_some_t, Dst, Buff>)
			{
				return tag_invoke(*this, std::forward<Dst>(dst), std::forward<Buff>(buff));
			}
			template<typename Dst, typename Buff> requires(!tag_invocable<write_some_t, Dst, Buff>)
			[[nodiscard]] std::size_t operator()(Dst &&dst, Buff &&buff) const
			{
				static_assert(tag_invocable<write_some_t, Dst, Buff, std::error_code &>);

				std::error_code err = {};
				if (const auto res = operator()(std::forward<Dst>(dst), std::forward<Buff>(buff), err); err)
					throw std::system_error(err);
				else
					return res;
			}
		};

		struct write_some_at_t
		{
			template<typename Dst, std::integral Pos, io_buffer Buff> requires tag_invocable<write_some_at_t, Dst, Pos, Buff, std::error_code &>
			[[nodiscard]] std::size_t operator()(Dst &&dst, Pos &&pos, Buff &&buff, std::error_code &err) const noexcept(nothrow_tag_invocable<write_some_at_t, Dst, Pos, Buff, std::error_code &>)
			{
				return tag_invoke(*this, std::forward<Dst>(dst), std::forward<Pos>(pos), std::forward<Buff>(buff), err);
			}
			template<typename Dst, std::integral Pos, io_buffer Buff> requires tag_invocable<write_some_at_t, Dst, Pos, Buff>
			[[nodiscard]] std::size_t operator()(Dst &&dst, Pos &&pos, Buff &&buff) const noexcept(nothrow_tag_invocable<write_some_at_t, Dst, Pos, Buff>)
			{
				return tag_invoke(*this, std::forward<Dst>(dst), std::forward<Pos>(pos), std::forward<Buff>(buff));
			}
			template<typename Dst, std::integral Pos, io_buffer Buff> requires(!tag_invocable<write_some_at_t, Dst, Pos, Buff>)
			[[nodiscard]] std::size_t operator()(Dst &&dst, Pos &&pos, Buff &&buff) const
			{
				static_assert(tag_invocable<write_some_at_t, Dst, Pos, Buff, std::error_code &>);

				std::error_code err = {};
				if (const auto res = operator()(std::forward<Dst>(dst), std::forward<Pos>(pos), std::forward<Buff>(buff), err); err)
					throw std::system_error(err);
				else
					return res;
			}
		};

		template<typename Snd>
		concept write_somesender = sender_of<Snd, set_value_t(std::size_t)> || sender_of<Snd, set_value_t(std::size_t, std::error_code)>;

		class async_write_some_t
		{
			template<typename Snd>
			using value_completion = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Dst, typename Buff>
			using back_adaptor = detail::back_adaptor<async_write_some_t, Dst, std::decay_t<Buff>>;

		public:
			template<sender Snd, typename Dst, io_buffer Buff> requires detail::tag_invocable_with_completion_scheduler<async_write_some_t, set_value_t, Snd, Snd, Dst, Buff>
			[[nodiscard]] write_somesender decltype(auto) operator()(Snd &&snd, Dst &&dst, Buff &&buff) const noexcept(nothrow_tag_invocable<async_write_some_t, value_completion<Snd>, Snd, Dst, Buff>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::forward<Dst>(dst), std::forward<Buff>(buff));
			}
			template<sender Snd, typename Dst, io_buffer Buff> requires(!detail::tag_invocable_with_completion_scheduler<async_write_some_t, set_value_t, Snd, Snd, Dst, Buff> && tag_invocable<async_write_some_t, Snd, Dst, Buff>)
			[[nodiscard]] write_somesender decltype(auto) operator()(Snd &&snd, Dst &&dst, Buff &&buff) const noexcept(nothrow_tag_invocable<async_write_some_t, Snd, Dst, Buff>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<Dst>(dst), std::forward<Buff>(buff));
			}

			template<typename Dst, io_buffer Buff>
			[[nodiscard]] back_adaptor<Dst, Buff> operator()(Dst &&dst, Buff &&buff) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Dst, Buff>, async_write_some_t, Dst, Buff>)
			{
				return back_adaptor<Dst, Buff>{*this, std::forward_as_tuple(std::forward<Dst>(dst), std::forward<Buff>(buff))};
			}
		};

		class async_write_some_at_t
		{
			template<typename Snd>
			using value_completion = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Dst, typename Pos, typename Buff>
			using back_adaptor = detail::back_adaptor<async_write_some_at_t, Dst, std::decay_t<Pos>, std::decay_t<Buff>>;

		public:
			template<sender Snd, typename Dst, std::integral Pos, io_buffer Buff> requires detail::tag_invocable_with_completion_scheduler<async_write_some_at_t, set_value_t, Snd, Snd, Dst, Pos, Buff>
			[[nodiscard]] write_somesender decltype(auto) operator()(Snd &&snd, Dst &&dst, Pos &&pos, Buff &&buff) const noexcept(nothrow_tag_invocable<async_write_some_at_t, value_completion<Snd>, Snd, Dst, Pos, Buff>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::forward<Dst>(dst), std::forward<Pos>(pos), std::forward<Buff>(buff));
			}
			template<sender Snd, typename Dst, std::integral Pos, io_buffer Buff> requires(!detail::tag_invocable_with_completion_scheduler<async_write_some_at_t, set_value_t, Snd, Snd, Dst, Pos, Buff> && tag_invocable<async_write_some_at_t, Snd, Dst, Pos, Buff>)
			[[nodiscard]] write_somesender decltype(auto) operator()(Snd &&snd, Dst &&dst, Pos &&pos, Buff &&buff) const noexcept(nothrow_tag_invocable<async_write_some_at_t, Snd, Dst, Pos, Buff>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<Dst>(dst), std::forward<Pos>(pos), std::forward<Buff>(buff));
			}

			template<typename Dst, std::integral Pos, io_buffer Buff>
			[[nodiscard]] back_adaptor<Dst, Pos, Buff> operator()(Dst &&dst, Pos &&pos, Buff &&buff) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Dst, Pos, Buff>, async_write_some_at_t, Dst, Pos, Buff>)
			{
				return back_adaptor<Dst, Pos, Buff>{*this, std::forward_as_tuple(std::forward<Dst>(dst), std::forward<Pos>(pos), std::forward<Buff>(buff))};
			}
		};
	}

	using _write_some::write_some_t;

	/** Customization point object used to write a contiguous buffer of integral values to a writeable destination object.
	 * @param[in] dst Destination object to write the data into.
	 * @param[in] buff Contiguous input range of integral values.
	 * @param[out] err Reference to the error code set on failure to complete the operation. If omitted, an exception is thrown instead.
	 * @throw std::system_error If an error has occurred and no output error code has been specified.
	 * @return Amount of elements written. */
	inline constexpr auto write_some = write_some_t{};

	using _write_some::write_some_at_t;

	/** Customization point object used to write a contiguous buffer of integral values to a writeable destination object at the specified offset.
	 * @param[in] dst Destination object to write the data into.
	 * @param[in] pos Offset into the destination object at which to write the values.
	 * @param[in] buff Contiguous input range of integral values.
	 * @param[out] err Reference to the error code set on failure to complete the operation. If omitted, an exception is thrown instead.
	 * @throw std::system_error If an error has occurred and no output error code has been specified.
	 * @return Amount of elements written. */
	inline constexpr auto write_some_at = write_some_at_t{};

	using _write_some::async_write_some_t;

	/** Customization point object returning a sender used to write a contiguous buffer of integral values to an async-writeable destination object.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] dst Destination object used to write the data into.
	 * @param[in] buff Contiguous input range of integral values.
	 * @return Sender completing either with the amount of elements read and an optional error code, or an error on read failure. */
	inline constexpr auto async_write_some = async_write_some_t{};

	using _write_some::async_write_some_at_t;

	/** Customization point object returning a sender used to write a contiguous buffer of integral values to an async-writeable destination object.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] dst Destination object used to write the data into.
	 * @param[in] pos Offset into the destination object at which to write the values.
	 * @param[in] buff Contiguous input range of integral values.
	 * @return Sender completing either with the amount of elements read and an optional error code, or an error on read failure. */
	inline constexpr auto async_write_some_at = async_write_some_at_t{};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
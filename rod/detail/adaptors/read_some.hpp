/*
 * Created by switchblade on 2023-05-22.
 */

#pragma once

#include "../../result.hpp"
#include "../byte_buffer.hpp"

namespace rod
{
	namespace _read_some
	{
		struct read_some_t
		{
			template<typename Hnd, byte_buffer Dst> requires tag_invocable<read_some_t, Hnd, Dst>
			[[nodiscard]] result<std::size_t, std::error_code> operator()(Hnd &&hnd, Dst &&dst) const noexcept(nothrow_tag_invocable<read_some_t, Hnd, Dst>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd), std::forward<Dst>(dst));
			}
		};
		struct read_some_at_t
		{
			template<typename Hnd, std::integral Pos, byte_buffer Dst> requires tag_invocable<read_some_at_t, Hnd, Pos, Dst>
			[[nodiscard]] result<std::size_t, std::error_code> operator()(Hnd &&hnd, Pos pos, Dst &&dst) const noexcept(nothrow_tag_invocable<read_some_at_t, Hnd, Pos, Dst>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd), pos, std::forward<Dst>(dst));
			}
		};

		struct async_read_some_t
		{
			template<typename Hnd, byte_buffer Dst> requires tag_invocable<async_read_some_t, Hnd, Dst>
			[[nodiscard]] sender_of<set_value_t(std::size_t)> auto operator()(Hnd &&hnd, Dst &&dst) const noexcept(nothrow_tag_invocable<async_read_some_t, Hnd, Dst>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd), std::forward<Dst>(dst));
			}
		};
		struct async_read_some_at_t
		{
			template<typename Hnd, std::integral Pos, byte_buffer Dst> requires tag_invocable<async_read_some_at_t, Hnd, Pos, Dst>
			[[nodiscard]] sender_of<set_value_t(std::size_t)> auto operator()(Hnd &&hnd, Pos pos, Dst &&dst) const noexcept(nothrow_tag_invocable<async_read_some_at_t, Hnd, Pos, Dst>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd), pos, std::forward<Dst>(dst));
			}
		};
	}

	using _read_some::read_some_t;

	/** Customization point object used to read a contiguous buffer of bytes from a readable source IO handle.
	 * @param[in] hnd IO handle to read the data from.
	 * @param[out] dst Output buffer of bytes.
	 * @return `rod::result&lt;std::size_t, std::error_code&gt;` indicating the amount of bytes read or an error code on read failure. */
	inline constexpr auto read_some = read_some_t{};

	using _read_some::read_some_at_t;

	/** Customization point object used to read a contiguous buffer of bytes from a readable source IO handle at the specified offset.
	 * @param[in] hnd IO handle to read the data from.
	 * @param[in] pos Offset into the IO handle at which to read the data.
	 * @param[out] dst Output buffer of bytes.
	 * @return `rod::result&lt;std::size_t, std::error_code&gt;` indicating the amount of bytes read or an error code on read failure. */
	inline constexpr auto read_some_at = read_some_at_t{};

	using _read_some::async_read_some_t;

	/** Customization point object returning a sender used to read a contiguous buffer of bytes from a source IO handle.
	 * @param[in] hnd Asynchronous IO handle used to schedule the read operation.
	 * @param[out] src Output buffer of bytes.
	 * @return Sender completing with the amount of bytes written or an error code on read failure. */
	inline constexpr auto async_read_some = async_read_some_t{};

	using _read_some::async_read_some_at_t;

	/** Customization point object returning a sender used to read a contiguous buffer of bytes from a source IO handle.
	 * @param[in] hnd Asynchronous IO handle used to schedule the read operation.
	 * @param[in] pos Offset into the IO handle at which to read the data.
	 * @param[out] src Output buffer of bytes.
	 * @return Sender completing with the amount of bytes written or an error code on read failure. */
	inline constexpr auto async_read_some_at = async_read_some_at_t{};
}
/*
 * Created by switchblade on 2023-05-22.
 */

#pragma once

#include "../../result.hpp"
#include "../byte_buffer.hpp"

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

		struct async_write_some_t
		{
			template<typename Hnd, byte_buffer Src> requires tag_invocable<async_write_some_t, Hnd, Src>
			[[nodiscard]] sender_of<set_value_t(std::size_t)> auto operator()(Hnd &&hnd, Src &&src) const noexcept(nothrow_tag_invocable<async_write_some_t, Hnd, Src>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd), std::forward<Src>(src));
			}
		};
		struct async_write_some_at_t
		{
			template<typename Hnd, std::integral Pos, byte_buffer Src> requires tag_invocable<async_write_some_at_t, Hnd, Pos, Src>
			[[nodiscard]] sender_of<set_value_t(std::size_t)> auto operator()(Hnd &&hnd, Pos pos, Src &&src) const noexcept(nothrow_tag_invocable<async_write_some_at_t, Hnd, Pos, Src>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd), pos, std::forward<Src>(src));
			}
		};
	}

	using _write_some::write_some_t;

	/** Customization point object used to write a contiguous buffer of bytes to a destination IO handle.
	 * @param[in] hnd IO handle to write the data into.
	 * @param[in] src Input buffer or bytes.
	 * @return `rod::result&lt;std::size_t, std::error_code&gt;` indicating the amount of bytes written or an error code on write failure. */
	inline constexpr auto write_some = write_some_t{};

	using _write_some::write_some_at_t;

	/** Customization point object used to write a contiguous buffer of bytes to a destination IO handle at the specified offset.
	 * @param[in] hnd IO handle to write the data into.
	 * @param[in] pos Offset into the IO handle at which to write the data.
	 * @param[in] src Input buffer or bytes.
	 * @return `rod::result&lt;std::size_t, std::error_code&gt;` indicating the amount of bytes written or an error code on write failure. */
	inline constexpr auto write_some_at = write_some_at_t{};

	using _write_some::async_write_some_t;

	/** Customization point object returning a sender used to write a contiguous buffer of bytes to a destination IO handle.
	 * @param[in] hnd Asynchronous IO handle used to schedule the write operation.
	 * @param[in] src Input buffer or bytes.
	 * @return Sender completing with the amount of bytes written or an error code on write failure. */
	inline constexpr auto async_write_some = async_write_some_t{};

	using _write_some::async_write_some_at_t;

	/** Customization point object returning a sender used to write a contiguous buffer of bytes to a destination IO handle.
	 * @param[in] hnd Asynchronous IO handle used to schedule the write operation.
	 * @param[in] pos Offset into the IO handle at which to write the data.
	 * @param[in] src Input buffer or bytes.
	 * @return Random-writeable sender completing with the amount of bytes written or an error code on write failure. */
	inline constexpr auto async_write_some_at = async_write_some_at_t{};
}
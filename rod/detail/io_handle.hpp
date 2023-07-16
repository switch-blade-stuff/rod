/*
 * Created by switch_blade on 2023-07-10.
 */

#pragma once

#include "byte_buffer.hpp"
#include "adaptors/read_some.hpp"
#include "adaptors/write_some.hpp"

namespace rod
{
	namespace _handle
	{
		struct open_t
		{
			template<typename Hnd, typename... Args> requires tag_invocable<open_t, Hnd, Args...>
			[[nodiscard]] constexpr tag_invoke_result_t<open_t, Hnd, Args...> operator()(Hnd &&hnd, Args &&...args) const noexcept(nothrow_tag_invocable<open_t, Hnd, Args...>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd), std::forward<Args>(args)...);
			}
		};
		struct sync_t
		{
			template<typename Hnd> requires tag_invocable<sync_t, Hnd>
			[[nodiscard]] constexpr tag_invoke_result_t<sync_t, Hnd> operator()(Hnd &&hnd) const noexcept(nothrow_tag_invocable<sync_t, Hnd>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd));
			}
		};
		struct flush_t
		{
			template<typename Hnd> requires tag_invocable<flush_t, Hnd>
			[[nodiscard]] constexpr tag_invoke_result_t<flush_t, Hnd> operator()(Hnd &&hnd) const noexcept(nothrow_tag_invocable<flush_t, Hnd>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd));
			}
		};
		struct close_t
		{
			template<typename Hnd> requires tag_invocable<close_t, Hnd>
			[[nodiscard]] constexpr tag_invoke_result_t<close_t, Hnd> operator()(Hnd &&hnd) const noexcept(nothrow_tag_invocable<close_t, Hnd>)
			{
				return tag_invoke(*this, std::forward<Hnd>(hnd));
			}
		};
	}

	using _handle::open_t;
	using _handle::sync_t;
	using _handle::flush_t;
	using _handle::close_t;

	/** Customization point object used to open an IO handle with provided arguments.
	 * @param[in] hnd IO handle to open.
	 * @param[in] args Arguments used to open the IO handle.
	 * @return Result of the open operation, such as `std::error_code`. */
	inline constexpr auto open = open_t{};
	/** Customization point object used to synchronize an IO handle with it's underlying resource.
	 * @note May be equivalent to `rod::flush`.
	 * @param[in] hnd IO handle to synchronize.
	 * @return Result of the synchronization operation, such as `std::error_code`. */
	inline constexpr auto sync = sync_t{};
	/** Customization point object used to flush internal buffers of an IO handle.
	 * @note May be equivalent to `rod::sync`.
	 * @param[in] hnd IO handle who's buffers to flush.
	 * @return Result of the flush operation, such as `std::error_code`. */
	inline constexpr auto flush = flush_t{};
	/** Customization point object used to close an IO handle.
	 * @param[in] hnd IO handle to close.
	 * @return Result of the close operation, such as `std::error_code`. */
	inline constexpr auto close = close_t{};
}
/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include "../factories/read.hpp"

namespace rod
{
	inline namespace _get_allocator
	{
		struct get_allocator_t
		{
			[[nodiscard]] constexpr friend bool tag_invoke(forwarding_query_t, get_allocator_t) noexcept { return true; }

			template<typename R> requires tag_invocable<get_allocator_t, const std::remove_cvref_t<R> &>
			[[nodiscard]] constexpr decltype(auto) operator()(R &&r) const noexcept { return tag_invoke(*this, std::as_const(r)); }
			[[nodiscard]] constexpr rod::sender auto operator()() const noexcept { return read(*this); }
		};
	}

	/** Customization point object used to obtain allocator associated with the passed object. */
	inline constexpr auto get_allocator = get_allocator_t{};
	/** Alias for `decltype(get_allocator(std::declval<T>()))` */
	template<typename T>
	using allocator_of_t = std::remove_cvref_t<decltype(get_allocator(std::declval<T>()))>;
}

/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include "../algorithms/read.hpp"

namespace rod
{
	inline namespace _get_allocator
	{
		class get_allocator_t
		{
			template<typename T, typename U = decltype(std::as_const(std::declval<T>()))>
			static constexpr bool is_invocable = tag_invocable<get_allocator_t, U>;

		public:
			[[nodiscard]] constexpr friend bool tag_invoke(forwarding_query_t, get_allocator_t) noexcept { return true; }

			[[nodiscard]] constexpr auto operator()() const noexcept { return read(*this); }
			template<typename R>
			[[nodiscard]] constexpr decltype(auto) operator()(R &&r) const noexcept requires is_invocable<R>
			{
				return tag_invoke(*this, std::as_const(r));
			}
		};
	}

	/** Customization point object used to obtain allocator associated with the passed object. */
	inline constexpr auto get_allocator = get_allocator_t{};
	/** Alias for `decltype(get_allocator(std::declval<T>()))` */
	template<typename T>
	using allocator_of_t = std::remove_cvref_t<decltype(get_allocator(std::declval<T>()))>;
}

/*
 * Created by switchblade on 2023-04-09.
 */

#pragma once

#include <concepts>
#include <utility>

namespace rod
{
	template<typename T>
	concept queryable = std::destructible<T>;

	namespace detail
	{
		/* Exposition-only. */
		inline void tag_invoke();

		struct impl_tag_invoke
		{
			template<typename Tag, typename... Args>
			constexpr decltype(auto) operator()(Tag &&tag, Args &&...args) const
			{
				return tag_invoke(tag, std::forward<Args>(args)...);
			}
		};
	}

	inline namespace _tag_invoke
	{
		/** Utility function used to invoke an implementation of `tag_invoke` with tag \a tag and arguments \a args selected via ADL. */
		inline constexpr auto tag_invoke = detail::impl_tag_invoke{};
	}

	/** Concept used to check if a call to `tag_invocable` is well-formed for tag type \a Tag and arguments \a Args. */
	template<typename Tag, typename... Args>
	concept tag_invocable = std::invocable<decltype(tag_invoke), Tag, Args...>;
	/** Concept used to check if a call to `tag_invocable` is well-formed for tag type \a Tag and arguments \a Args and does not throw exceptions. */
	template<typename Tag, typename... Args>
	concept nothrow_tag_invocable = tag_invocable<Tag, Args...> && std::is_nothrow_invocable_v<decltype(tag_invoke), Tag, Args...>;

	/** Metaprogramming utility used to obtain the result of a call to `tag_invoke` for tag type \a Tag and arguments \a Args. */
	template<typename Tag, typename... Args>
	using tag_invoke_result = std::invoke_result<decltype(tag_invoke), Tag, Args...>;
	/** Alias for `typename tag_invoke_result<Tag, Args...>::type`. */
	template<typename Tag, typename... Args>
	using tag_invoke_result_t = typename tag_invoke_result<Tag, Args...>::type;

	/** @brief Metaprogramming utility used define a tag type for object \a V.
	 *
	 * `tag_t` is primarily used to define tag types for invocable query objects
	 * in order to enable CPO via `tag_invoke`. */
	template<auto &Tag>
	using tag_t = std::remove_cvref_t<decltype(Tag)>;

	inline namespace _forwarding_query
	{
		struct forwarding_query_t
		{
			template<typename Q>
			[[nodiscard]] constexpr bool operator()(Q &&q) const noexcept(nothrow_tag_invocable<forwarding_query_t, Q>) requires tag_invocable<forwarding_query_t, Q>
			{
				return tag_invoke(*this, std::forward<Q>(q));
			}
			template<typename Q>
			[[nodiscard]] constexpr bool operator()(Q &&) const noexcept { return std::derived_from<Q, forwarding_query_t>; }
		};
	}

	/**  Customization point object used to check if a query object should be forwarded through queryable adaptors. */
	inline constexpr auto forwarding_query = forwarding_query_t{};

}

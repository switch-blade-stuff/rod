/*
 * Created by switchblade on 2023-04-09.
 */

#pragma once

#include <type_traits>
#include <concepts>
#include <utility>

#include "detail/config.hpp"

namespace rod
{
	template<typename T>
	concept queryable = std::destructible<T>;

#ifndef ROD_TAG_INVOKE_NAMESPACE
	namespace _tag_invoke
	{
		void tag_invoke();

		/** Concept used to check if a call to `tag_invocable` is well-formed for tag type \a Tag and arguments \a Args. */
		template<typename Tag, typename... Args>
		concept tag_invocable = requires(Tag tag, Args &&...args) { tag_invoke(tag, std::forward<Args>(args)...); };
		/** Concept used to check if a call to `tag_invocable` is well-formed for tag type \a Tag and arguments \a Args and does not throw exceptions. */
		template<typename Tag, typename... Args>
		concept nothrow_tag_invocable = tag_invocable<Tag, Args...> && requires(Tag tag, Args &&...args) { { tag_invoke(tag, std::forward<Args>(args)...) } noexcept; };
		/** Alias for `decltype(tag_invoke(std::declval&lt;Tag&gt;(), std::declval&lt;Args&gt;()...))`. */
		template<typename Tag, typename... Args>
		using tag_invoke_result_t = decltype(tag_invoke(std::declval<Tag>(), std::declval<Args>()...));

		/** Metaprogramming utility used to obtain the result of a call to `tag_invoke` for tag type \a Tag and arguments \a Args. */
		template<typename Tag, typename... Args>
		struct tag_invoke_result;
		template<typename Tag, typename... Args> requires tag_invocable<Tag, Args...>
		struct tag_invoke_result<Tag, Args...> { using type = tag_invoke_result_t<Tag, Args...>; };

		struct tag_invoke_t
		{
			template<typename Tag, typename... Args> requires tag_invocable<Tag, Args...>
			constexpr tag_invoke_result_t<Tag, Args...> operator()(Tag tag, Args &&...args) const noexcept(nothrow_tag_invocable<Tag, Args...>)
			{
				return tag_invoke(tag, std::forward<Args>(args)...);
			}
		};
	}

	using _tag_invoke::nothrow_tag_invocable;
	using _tag_invoke::tag_invoke_result_t;
	using _tag_invoke::tag_invoke_result;
	using _tag_invoke::tag_invocable;

	using _tag_invoke::tag_invoke_t;

	/** Utility function used to invoke an implementation of `tag_invoke` with tag \a tag and arguments \a args selected via ADL. */
	inline constexpr auto tag_invoke = tag_invoke_t{};
#else
	using ROD_TAG_INVOKE_NAMESPACE::nothrow_tag_invocable;
	using ROD_TAG_INVOKE_NAMESPACE::tag_invoke_result_t;
	using ROD_TAG_INVOKE_NAMESPACE::tag_invoke_result;
	using ROD_TAG_INVOKE_NAMESPACE::tag_invocable;

	using ROD_TAG_INVOKE_NAMESPACE::tag_invoke;
	using tag_invoke_t = decltype(tag_invoke);
#endif

	/** @brief Metaprogramming utility used define a tag type for object \a V.
	 *
	 * `tag_t` is primarily used to define tag types for invocable query objects
	 * in order to enable CPO via `tag_invoke`. */
	template<auto &Tag>
	using tag_t = std::decay_t<decltype(Tag)>;

	inline namespace _forwarding_query
	{
		struct forwarding_query_t
		{
			template<typename Q> requires tag_invocable<forwarding_query_t, Q>
			[[nodiscard]] constexpr bool operator()(Q &&q) const noexcept { return tag_invoke(*this, std::forward<Q>(q)); }
			template<typename Q>
			[[nodiscard]] constexpr bool operator()(Q &&) const noexcept { return std::derived_from<Q, forwarding_query_t>; }
		};
	}

	/** Customization point object used to check if a query object should be forwarded through queryable adaptors. */
	inline constexpr auto forwarding_query = forwarding_query_t{};

	/** Concept used to check if type \a T is a forwarding query. */
	template<typename T>
	concept is_forwarding_query = forwarding_query(T{});
}

/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include "../queries/signatures.hpp"
#include "opstate.hpp"
#include "connect.hpp"

namespace rod
{
	namespace _read
	{
		template<typename, typename>
		struct operation { struct type; };
		template<typename>
		struct sender { struct type; };

		template<typename T, typename R>
		struct operation<T, R>::type : detail::ebo_helper<R>
		{
			friend class sender<T>::type;

			using detail::ebo_helper<R>::ebo_helper;

			friend constexpr void tag_invoke(start_t, type &op) noexcept
			{
				auto &rcv = op.detail::template ebo_helper<T>::value();
				detail::rcv_try_invoke(std::move(rcv), set_value, std::move(rcv), T{}(get_env(rcv)));
			}
		};
		template<typename T>
		struct sender<T>::type
		{
			template<typename R>
			[[nodiscard]] constexpr friend operation<T, std::decay_t<R>> tag_invoke(connect_t, type, R &&r) { return {std::forward<R>(r)}; }

			template<typename Env> requires tag_invocable<T, Env>
			friend constexpr auto tag_invoke(get_completion_signatures_t, type, Env) -> completion_signatures<set_value_t(tag_invoke_result_t<T, Env>), set_error_t(std::exception_ptr)> { return {}; }
			template<typename Env> requires nothrow_tag_invocable<T, Env>
			friend constexpr auto tag_invoke(get_completion_signatures_t, type, Env) -> completion_signatures<set_value_t(tag_invoke_result_t<T, Env>)> { return {}; }

			friend constexpr detail::empty_env_t tag_invoke(get_env_t, const type &) noexcept { return {}; }
		};

		struct read_t
		{
			template<typename T>
			[[nodiscard]] constexpr typename sender<T>::type operator()(T) const noexcept { return {}; }
		};
	}

	using _read::read_t;

	/** Customization point object used to create a sender that returns a value from the associated environment through the value channel.
	 * @param query Query tag invoked on the associated execution environment via `tag_invoke(query, env)`.
	 * @return Sender that completes via `set_value(tag_invoke(query, env))` or `set_error(std::current_exception())` if the former throws an exception. */
	inline constexpr auto read = read_t{};

}
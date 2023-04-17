/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include "../queries/signatures.hpp"
#include "opstate.hpp"
#include "connect.hpp"

namespace rod
{
	inline namespace _read
	{
		class read_t
		{
			template<typename>
			class sender;

			template<typename T, typename R>
			class operation : detail::ebo_helper<R>
			{
				friend class sender<T>;

			public:
				using detail::ebo_helper<R>::ebo_helper;

				friend constexpr void tag_invoke(start_t, operation &op) noexcept
				{
					auto &rcv = op.receiver();
					try { set_value(std::move(rcv), T{}(get_env(rcv))); }
					catch(...) { set_error(std::move(rcv), std::current_exception()); }
				}

			private:
				[[nodiscard]] constexpr auto &receiver() noexcept { detail::ebo_helper<T>::value(); }
			};

			template<typename T>
			class sender
			{
			public:
				template<typename R>
				[[nodiscard]] constexpr friend operation<T, std::decay_t<R>> tag_invoke(connect_t, sender, R &&r) { return {std::forward<R>(r)}; }

				template<typename Env> requires tag_invocable<T, Env>
				friend auto tag_invoke(get_completion_signatures_t, sender, Env) -> completion_signatures<set_value_t(tag_invoke_result_t<T, Env>), set_error_t(std::exception_ptr)>;
				template<typename Env> requires nothrow_tag_invocable<T, Env>
				friend auto tag_invoke(get_completion_signatures_t, sender, Env) -> completion_signatures<set_value_t(tag_invoke_result_t<T, Env>)>;

				friend constexpr detail::empty_env_t tag_invoke(get_env_t, const sender &) noexcept { return {}; }
			};

		public:
			template<typename T>
			[[nodiscard]] constexpr sender<T> operator()(T) const noexcept { return {}; }
		};
	}

	/** Customization point object used to create a sender that returns a value from the associated environment through the value channel. */
	inline constexpr auto read = read_t{};

}
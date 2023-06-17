/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include "../queries/completion.hpp"
#include "../concepts.hpp"

namespace rod
{
	namespace _read
	{
		template<typename, typename>
		struct operation { class type; };
		template<typename>
		struct sender { struct type; };

		template<typename T, typename Rcv>
		class operation<T, Rcv>::type
		{
		public:
			constexpr explicit type(Rcv &&rcv) noexcept(std::is_nothrow_move_constructible_v<Rcv>) : _rcv(std::forward<Rcv>(rcv)) {}

			friend constexpr void tag_invoke(start_t, type &op) noexcept { detail::rcv_try_invoke(std::move(op._rcv), set_value, std::move(op._rcv), T{}(get_env(op._rcv))); }

		private:
			ROD_NO_UNIQUE_ADDRESS Rcv _rcv;
		};
		template<typename T>
		struct sender<T>::type
		{
			using is_sender = std::true_type;

			friend constexpr empty_env tag_invoke(get_env_t, const type &) noexcept { return {}; }

			template<typename Env> requires tag_invocable<T, Env>
			friend constexpr auto tag_invoke(get_completion_signatures_t, type, Env) -> completion_signatures<set_value_t(std::invoke_result_t<T, Env>), set_error_t(std::exception_ptr)> { return {}; }
			template<typename Env> requires nothrow_tag_invocable<T, Env>
			friend constexpr auto tag_invoke(get_completion_signatures_t, type, Env) -> completion_signatures<set_value_t(std::invoke_result_t<T, Env>)> { return {}; }

			template<rod::receiver R>
			friend constexpr auto tag_invoke(connect_t, type, R &&r) { return typename operation<T, std::decay_t<R>>::type{std::forward<R>(r)}; }
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
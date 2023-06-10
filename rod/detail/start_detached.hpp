/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#include <optional>

#include "../stop_token.hpp"
#include "shared_ref.hpp"
#include "concepts.hpp"

namespace rod
{
	namespace _start_detached
	{
		template<typename Snd>
		struct operation { struct type; };
		template<typename Snd>
		struct receiver { struct type; };

		template<typename Snd>
		struct receiver<Snd>::type
		{
			using is_receiver = std::true_type;
			using _operation_t = typename operation<Snd>::type;

			friend constexpr empty_env tag_invoke(get_env_t, const type &) noexcept { return {}; }

			[[noreturn]] friend void tag_invoke(set_error_t, type &&, auto...) noexcept { std::terminate(); }
			[[noreturn]] friend void tag_invoke(set_error_t, type &&, std::error_code e) noexcept { throw std::system_error(e); }
			[[noreturn]] friend void tag_invoke(set_error_t, type &&, const std::exception_ptr &e) noexcept { rethrow_exception(e); }
			template<detail::completion_channel C, typename... Args>
			friend constexpr void tag_invoke(C, type &&r, Args &&...) noexcept { delete r._op; }

			_operation_t *_op = {};
		};

		template<typename Snd>
		struct operation<Snd>::type
		{
			using _receiver_t = typename receiver<Snd>::type;
			using _state_t = connect_result_t<Snd, _receiver_t>;

			type(type &&) = delete;
			type &operator=(type &&) = delete;

			type(Snd &&snd) : _state(connect(std::forward<Snd>(snd), _receiver_t{this})) {}

			friend constexpr void tag_invoke(start_t, type &op) noexcept { start(op._state); }

			_state_t _state;
		};

		class start_detached_t
		{
			template<typename Snd>
			using value_scheduler = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Snd>
			using operation_t = typename operation<std::decay_t<Snd>>::type;

		public:
			template<rod::sender Snd> requires detail::tag_invocable_with_completion_scheduler<start_detached_t, set_value_t, Snd, Snd>
			constexpr void operator()(Snd &&snd) const noexcept(nothrow_tag_invocable<start_detached_t, value_scheduler<Snd>, Snd>)
			{
				tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd));
			}
			template<rod::sender Snd> requires(!detail::tag_invocable_with_completion_scheduler<start_detached_t, set_value_t, Snd, Snd> && tag_invocable<start_detached_t, Snd>)
			constexpr void operator()(Snd &&snd) const noexcept(nothrow_tag_invocable<start_detached_t, Snd>)
			{
				tag_invoke(*this, std::forward<Snd>(snd));
			}
			template<rod::sender Snd> requires(!detail::tag_invocable_with_completion_scheduler<start_detached_t, set_value_t, Snd> && !tag_invocable<start_detached_t, Snd>)
			constexpr void operator()(Snd &&snd) const
			{
				auto *op = new operation_t<Snd>{std::forward<Snd>(snd)};
				try { start(*op); } catch (...) { delete op; throw; }
			}
		};
	}

	using _start_detached::start_detached_t;

	/** Synchronously starts the passed sender and detaches it's state. Detached state is destroyed upon completion.
	 * @param snd Sender to start & detach.
	 * @note If the sender completes via the error channel, `std::terminate` is called. */
	inline constexpr auto start_detached = start_detached_t{};
}

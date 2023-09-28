/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#include <cassert>

#include "../../stop_token.hpp"
#include "../shared_ref.hpp"
#include "../async_base.hpp"

namespace rod
{
	namespace _start_detached
	{
		template<typename Del, typename Snd>
		struct receiver { class type; };
		template<typename Del, typename Snd>
		class detached_state;

		template<typename Del, typename Snd>
		class receiver<Del, Snd>::type
		{
			using state_t = detached_state<Del, Snd>;

		public:
			using is_receiver = std::true_type;

		public:
			friend constexpr empty_env tag_invoke(get_env_t, const type &) noexcept { return {}; }

			[[noreturn]] friend void tag_invoke(set_error_t, type &&, auto...) noexcept { std::terminate(); }
			template<typename Err> requires requires (const Err &err) { throw_exception(err); }
			[[noreturn]] friend void tag_invoke(set_error_t, type &&, const Err &err) noexcept { throw_exception(err); }

			template<_detail::completion_channel C, typename... Args>
			friend void tag_invoke(C, type &&r, Args &&...) noexcept { Del{}(r._state); }

		private:
			state_t *_state = {};
		};

		struct alloc_deleter;
		struct local_deleter;

		enum flags_t : char
		{
			started = 1,
			waiting = 2,
			complete = 4,
		};

		template<typename Del, typename Snd>
		class detached_state : connect_result_t<Snd, typename receiver<Del, Snd>::type>
		{
			friend struct alloc_deleter;
			friend struct local_deleter;

			using base_t = connect_result_t<Snd, typename receiver<Del, Snd>::type>;
			using receiver_t = typename receiver<Del, Snd>::type;

		public:
			explicit detached_state(Snd &&snd) noexcept(_detail::nothrow_callable<connect_t, Snd, receiver_t>) : base_t(connect(std::forward<Snd>(snd), receiver_t(this))) { start(static_cast<base_t &>(*this)); }

			~detached_state()
			{
				const auto old_flags = _flags.fetch_or(waiting, std::memory_order_acq_rel);
				assert(old_flags & started);

				if (!(old_flags & complete)) [[unlikely]]
					_flags.wait(old_flags | waiting);
			}

		private:
			std::atomic<char> _flags = started;
		};

		struct local_deleter
		{
			template<typename Snd>
			bool operator()(detached_state<alloc_deleter, Snd> *ptr) const noexcept
			{
				if (auto flags = started; !ptr->_flags.compare_exchange_strong(flags, complete))
				{
					assert(flags == (started | waiting));
					ptr->_flags.notify_one();
					return false;
				}
				return true;
			}
		};
		struct alloc_deleter : local_deleter
		{
			template<typename Snd>
			void operator()(detached_state<alloc_deleter, Snd> *ptr) const
			{
				if (local_deleter::operator()(ptr))
					delete ptr;
			}
		};

		class start_inline_t
		{
			template<typename Snd>
			using value_scheduler = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Snd>
			using state_t = typename detached_state<local_deleter, std::decay_t<Snd>>::type;

		public:
			template<rod::sender Snd> requires _detail::tag_invocable_with_completion_scheduler<start_inline_t, set_value_t, Snd, Snd>
			constexpr std::destructible auto operator()(Snd &&snd) const noexcept(nothrow_tag_invocable<start_inline_t, value_scheduler<Snd>, Snd>) { return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd)); }
			template<rod::sender Snd> requires(!_detail::tag_invocable_with_completion_scheduler<start_inline_t, set_value_t, Snd, Snd> && tag_invocable<start_inline_t, Snd>)
			constexpr std::destructible auto operator()(Snd &&snd) const noexcept(nothrow_tag_invocable<start_inline_t, Snd>) { return tag_invoke(*this, std::forward<Snd>(snd)); }

			template<rod::sender Snd> requires(!_detail::tag_invocable_with_completion_scheduler<start_inline_t, set_value_t, Snd> && !tag_invocable<start_inline_t, Snd>)
			constexpr auto operator()(Snd &&snd) const noexcept(std::is_nothrow_constructible_v<state_t<Snd>, Snd>) { return state_t<Snd>{std::forward<Snd>(snd)}; }
		};
		class start_detached_t
		{
			template<typename Snd>
			using value_scheduler = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Snd>
			using state_t = typename detached_state<alloc_deleter, std::decay_t<Snd>>::type;

		public:
			template<rod::sender Snd> requires _detail::tag_invocable_with_completion_scheduler<start_detached_t, set_value_t, Snd, Snd>
			constexpr void operator()(Snd &&snd) const noexcept(nothrow_tag_invocable<start_detached_t, value_scheduler<Snd>, Snd>) { tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd)); }
			template<rod::sender Snd> requires(!_detail::tag_invocable_with_completion_scheduler<start_detached_t, set_value_t, Snd, Snd> && tag_invocable<start_detached_t, Snd>)
			constexpr void operator()(Snd &&snd) const noexcept(nothrow_tag_invocable<start_detached_t, Snd>) { tag_invoke(*this, std::forward<Snd>(snd)); }

			template<rod::sender Snd> requires(!_detail::tag_invocable_with_completion_scheduler<start_detached_t, set_value_t, Snd> && !tag_invocable<start_detached_t, Snd>)
			constexpr void operator()(Snd &&snd) const { start(new state_t<Snd>{std::forward<Snd>(snd)}); }
		};
	}

	using _start_detached::start_inline_t;

	/** Synchronously starts the passed sender and returns it's state.
	 * @param snd Sender to start & detach.
	 * @return State of the started operation.
	 * @note Destructor of the returned state blocks until the started operation completes.
	 * @note If the operation completes via the error channel, `std::terminate` is called. */
	inline constexpr auto start_inline = start_inline_t{};

	/** Type alias for a state object obtained from a call to `rod::start_inline(snd)`. */
	template<sender Snd>
	using detached_state_t = tag_invoke_result_t<start_inline_t, Snd>;

	using _start_detached::start_detached_t;

	/** Synchronously starts the passed sender and detaches it's state. Detached state is destroyed upon completion.
	 * @param snd Sender to start & detach.
	 * @note If the operation completes via the error channel, `std::terminate` is called. */
	inline constexpr auto start_detached = start_detached_t{};
}

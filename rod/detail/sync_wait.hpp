/*
 * Created by switchblade on 2023-04-18.
 */

#pragma once

#include <optional>

#include "adaptors/into_variant.hpp"
#include "run_loop.hpp"

namespace rod
{
	namespace _sync_wait
	{
		template<typename...>
		struct receiver { struct type; };

		using scheduler_t = decltype(std::declval<run_loop>().get_scheduler());

		struct env
		{
			friend constexpr auto tag_invoke(get_delegatee_scheduler_t, const env &e) noexcept { return e.sch; }
			friend constexpr auto tag_invoke(get_scheduler_t, const env &e) noexcept { return e.sch; }

			scheduler_t sch;
		};

		template<typename S, template<typename...> typename T = type_list_t>
		using sync_wait_types = value_types_of_t<S, env, std::type_identity_t, T>;

		template<typename... Ts>
		using state_t = std::variant<std::monostate, std::tuple<Ts...>, std::exception_ptr, set_stopped_t>;

		template<typename... Ts>
		struct receiver<Ts...>::type
		{
			using is_receiver = std::true_type;
			using _state_t = state_t<Ts...>;

			friend constexpr env tag_invoke(get_env_t, const type &r) noexcept { return {r.loop->get_scheduler()}; }

			template<typename... Args> requires std::constructible_from<std::tuple<Ts...>, Args...>
			friend constexpr void tag_invoke(set_value_t, type &&r, Args &&...args) noexcept
			{
				try { r.state->template emplace<1>(std::forward<Args>(args)...); }
				catch (...) { r.set_error(std::current_exception()); }
				r.loop->finish();
			}
			template<typename Err>
			friend constexpr void tag_invoke(set_error_t, type &&r, Err &&err) noexcept
			{
				r.set_error(std::forward<Err>(err));
				r.loop->finish();
			}
			friend constexpr void tag_invoke(set_stopped_t, type &&r) noexcept
			{
				r.state->template emplace<3>();
				r.loop->finish();
			}

			_state_t *state;
			run_loop *loop;

		private:
			template<typename Err>
			constexpr void set_error(Err &&err) noexcept
			{
				if constexpr (decays_to<Err, std::exception_ptr>)
					state->template emplace<2>(std::forward<Err>(err));
				if constexpr (decays_to<Err, std::error_code>)
					state->template emplace<2>(std::make_exception_ptr(std::system_error(std::forward<Err>(err))));
				else
					state->template emplace<2>(std::make_exception_ptr(std::forward<Err>(err)));
			}
		};

		class sync_wait_t
		{
			template<typename S>
			using value_scheduler = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<S>())));
			template<typename S>
			using receiver_t = typename sync_wait_types<std::decay_t<S>, receiver>::type;
			template<typename S>
			using result_t = sync_wait_types<std::decay_t<S>, std::tuple>;
			template<typename S>
			using state_t = typename receiver_t<S>::_state_t;

		public:
			template<_detail::single_sender<env> S> requires _detail::tag_invocable_with_completion_scheduler<sync_wait_t, set_value_t, S, S>
			constexpr std::optional<result_t<S>> operator()(S &&snd) const noexcept(nothrow_tag_invocable<sync_wait_t, value_scheduler<S>, S>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_attrs(std::forward<S>(snd))), std::forward<S>(snd));
			}
			template<_detail::single_sender<env> S> requires(!_detail::tag_invocable_with_completion_scheduler<sync_wait_t, set_value_t, S, S> && tag_invocable<sync_wait_t, S>)
			constexpr std::optional<result_t<S>> operator()(S &&snd) const noexcept(nothrow_tag_invocable<sync_wait_t, S>)
			{
				return tag_invoke(*this, std::forward<S>(snd));
			}

			template<_detail::single_sender<env> S> requires(!_detail::tag_invocable_with_completion_scheduler<sync_wait_t, set_value_t, S, S> && !tag_invocable<sync_wait_t, S>)
			std::optional<result_t<S>> operator()(S &&snd) const
			{
				static_assert(sender_to<S, receiver_t<S>>);

				state_t<S> state = {};
				run_loop loop = {};

				/* Start the sender chain & wait for it to finish executing. */
				{
					auto op = connect(std::forward<S>(snd), receiver_t<S>{&state, &loop});
					start(op);
					loop.run();
				}

				/* Rethrow exceptions & return value results. */
				switch (state.index())
				{
				default: [[unlikely]] std::terminate();
				case 1: [[likely]] return std::move(std::get<1>(state));
				case 2: std::rethrow_exception(std::move(std::get<2>(state)));
				case 3: return std::nullopt;
				}
			}
		};
		class sync_wait_with_variant_t
		{
			template<typename S>
			using result_t = value_types_of_t<S, env, std::tuple, std::variant>;

		public:
			template<sender_in<env> S> requires _detail::tag_invocable_with_completion_scheduler<sync_wait_with_variant_t, set_value_t, S, S>
			constexpr std::optional<result_t<S>> operator()(S &&snd) const { return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_attrs(std::forward<S>(snd))), std::forward<S>(snd)); }
			template<sender_in<env> S> requires(!_detail::tag_invocable_with_completion_scheduler<sync_wait_with_variant_t, set_value_t, S, S> && tag_invocable<sync_wait_with_variant_t, S>)
			constexpr std::optional<result_t<S>> operator()(S &&snd) const { return tag_invoke(*this, std::forward<S>(snd)); }
			template<sender_in<env> S> requires(!_detail::tag_invocable_with_completion_scheduler<sync_wait_with_variant_t, set_value_t, S, S> && !tag_invocable<sync_wait_with_variant_t, S> && _detail::callable<into_variant_t, S>)
			std::optional<result_t<S>> operator()(S &&snd) const { return sync_wait_t{}(into_variant(std::forward<S>(snd))); }
		};
	}

	using _sync_wait::sync_wait_t;
	using _sync_wait::sync_wait_with_variant_t;

	/** Synchronously starts the passed sender and blocks until completion. Rethrows any errors.
	 * @param snd Sender to execute synchronously.
	 * @return Optional set to the value completion result, or `std::nullopt` on stop completion. */
	inline constexpr auto sync_wait = sync_wait_t{};
	/** Synchronously starts the passed sender and blocks until completion. Rethrows any errors.
	 * @param snd Sender to execute synchronously.
	 * @return Optional of a variant set to the value completion result, or `std::nullopt` on stop completion. */
	inline constexpr auto sync_wait_with_variant = sync_wait_with_variant_t{};
}

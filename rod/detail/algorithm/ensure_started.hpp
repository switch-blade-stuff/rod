/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#include <optional>

#include "../adaptors/closure.hpp"
#include "../../stop_token.hpp"
#include "../shared_ref.hpp"
#include "../async_base.hpp"

namespace rod
{
	namespace _ensure_started
	{
		struct stop_trigger
		{
			void operator()() const noexcept { _src.request_stop(); }
			in_place_stop_source &_src;
		};

		template<typename Snd, typename Env, typename Rcv>
		struct operation { class type; };
		struct operation_base;

		template<typename Snd, typename Env>
		struct receiver { class type; };
		template<typename Snd, typename Env>
		struct sender { class type; };
		template<typename Env>
		struct env { class type; };

		struct operation_base { void (*_notify)(operation_base *) noexcept = {}; };

		template<typename Env>
		class env<Env>::type
		{
		public:
			constexpr explicit type(in_place_stop_token tok, Env *env) noexcept : _tok(tok), _env(env) {}

			template<decays_to_same<type> E>
			friend constexpr auto tag_invoke(get_stop_token_t, E &&e) noexcept { return e._tok; }
			template<is_forwarding_query Q, decays_to_same<type> E, typename... Args> requires _detail::callable<Q, Env, Args...>
			friend constexpr decltype(auto) tag_invoke(Q, E &&e, Args &&...args) noexcept(_detail::nothrow_callable<Q, Env, Args...>)
			{
				return Q{}(*std::forward<E>(e)._env, std::forward<Args>(args)...);
			}

		private:
			in_place_stop_token _tok;
			Env *_env = {};
		};

		template<typename Snd, typename Env>
		struct shared_state : public _detail::shared_base, empty_base<Env>
		{
			using receiver_t = typename receiver<Snd, Env>::type;
			using state_t = connect_result_t<Snd, receiver_t>;
			using env_t = typename env<Env>::type;

			using value_data_t = _detail::gather_signatures_t<set_value_t, Snd, env_t, _detail::bind_front<_detail::decayed_tuple, set_value_t>::template type, std::variant>;
			using error_data_t = _detail::gather_signatures_t<set_error_t, Snd, env_t, _detail::bind_front<_detail::decayed_tuple, set_value_t>::template type, std::variant>;
			using data_t = unique_tuple_t<_detail::concat_tuples_t<std::variant<std::tuple<set_stopped_t>, std::tuple<set_error_t, std::exception_ptr>>, value_data_t, error_data_t>>;

			constexpr shared_state(Snd &&snd) : empty_base<Env>(get_env(snd)), state2(connect(std::forward<Snd>(snd))) { start(state2); }

			[[nodiscard]] constexpr decltype(auto) env() noexcept { return empty_base<Env>::value(); }
			[[nodiscard]] constexpr decltype(auto) env() const noexcept { return empty_base<Env>::value(); }

			inline auto connect(Snd &&) noexcept(_detail::nothrow_callable<connect_t, Snd, receiver_t>);
			void detach() noexcept { stop_src.request_stop(); }
			void notify() noexcept
			{
				auto *ptr = state1.exchange(this, std::memory_order_acq_rel);
				if (auto *op = static_cast<operation_base *>(ptr); op)
					op->_notify(op);
			}

			data_t data = std::tuple<set_stopped_t>{};
			in_place_stop_source stop_src = {};
			std::atomic<void *> state1 = {};
			state_t state2;
		};

		template<typename Snd, typename Env>
		class receiver<Snd, Env>::type
		{
			using env_t = typename env<Env>::type;

		public:
			using is_receiver = std::true_type;

		public:
			constexpr explicit type(_detail::shared_handle<shared_state<Snd, Env>> hnd) noexcept : _state(std::move(hnd)) {}

			friend env_t tag_invoke(get_env_t, const type &r) noexcept { return env_t{r._state->stop_src.get_token(), &r._state->env()}; }
			template<_detail::completion_channel C, typename... Args>
			friend void tag_invoke(C, type &&r, Args &&...args) noexcept
			{
				auto &state = *r._state;

				using value_t = _detail::decayed_tuple<C, Args...>;
				static_assert(std::constructible_from<value_t, C, Args...>);

				/* Bail if stop has been requested as per spec. */
				if (state.stop_src.stop_requested()) return;

				const auto do_emplace = [&]() { state.data.template emplace<value_t>(C{}, std::forward<Args>(args)...); };
				if constexpr (!std::is_nothrow_constructible_v<value_t, C, Args...>)
					try { do_emplace(); } catch (...) { state.data.template emplace<_detail::decayed_tuple<set_error_t, std::exception_ptr>>(set_error, std::current_exception()); }
				else
					do_emplace();

				state.notify();
				r._state.reset();
			}

		private:
			_detail::shared_handle<shared_state<Snd, Env>> _state;
		};

		template<typename Snd, typename Env>
		auto shared_state<Snd, Env>::connect(Snd &&snd) noexcept(_detail::nothrow_callable<connect_t, Snd, receiver_t>)
		{
			return connect(std::forward<Snd>(snd), receiver_t{static_cast<shared_state *>(this->acquire())});
		}

		template<typename Snd, typename Rcv, typename Env>
		class operation<Snd, Rcv, Env>::type : operation_base, empty_base<Rcv>
		{
			using stop_cb_t = std::optional<stop_callback_for_t<stop_token_of_t<env_of_t<Rcv> &>, stop_trigger>>;

			static void notify_complete(operation_base *ptr) noexcept
			{
				auto *op = static_cast<type *>(ptr);
				op->_on_stop.reset();

				std::visit([&](auto &data) noexcept
				{
					std::apply([&](auto c, auto &...args) noexcept { c(std::move(op->empty_base<Rcv>::value()), std::move(args)...); }, data);
				}, op->_state->data);
			}

		public:
			type(type &&) = delete;
			type &operator=(type &&) = delete;

			type(Rcv &&rcv, _detail::shared_handle<shared_state<Snd, Env>> handle) noexcept(std::is_nothrow_move_constructible_v<Rcv>)
					: operation_base{notify_complete}, _state(std::move(handle)), empty_base<Rcv>(std::forward<Rcv>(rcv)) {}
			~type() { if (!_state->state1.load(std::memory_order_acquire)) _state->detach(); }

			friend void tag_invoke(start_t, type &op) noexcept
			{
				auto &state = *op._state;
				if (auto ptr = state.state1.load(std::memory_order_acquire); ptr == &state)
					op._notify(&op);
				else
				{
					op._on_stop.emplace(get_stop_token(get_env(op._rcv)), stop_trigger{state.stop_src});
					if (state.stop_src.stop_requested())
						set_stopped(std::move(op._rcv));
					else if (ptr = nullptr; !state.state1.compare_exchange_weak(ptr, &op, std::memory_order_release, std::memory_order_acquire))
						op._notify(&op);
				}
			}

		private:
			_detail::shared_handle<shared_state<Snd, Env>> _state;
			stop_cb_t _on_stop = {};
		};

		struct ensure_started_tag {};
		template<typename Snd, typename Env>
		class sender<Snd, Env>::type : public ensure_started_tag
		{
		public:
			using is_sender = std::true_type;

		private:
			template<typename Rcv>
			using operation_t = typename operation<Snd, Rcv, Env>::type;
			using env_t = typename env<Env>::type;

			template<typename... Ts>
			using value_signs_t = completion_signatures<set_value_t(std::decay_t<Ts> &&...)>;
			template<typename T>
			using error_signs_t = completion_signatures<set_error_t(std::decay_t<T> &&)>;
			using signs_t = make_completion_signatures<Snd, env_t, completion_signatures<set_error_t(std::exception_ptr &&)>, value_signs_t, error_signs_t>;

		public:
			constexpr explicit type(Snd snd) : _state(new shared_state<Snd, Env>{std::move(snd)}) {}
			constexpr explicit type(_detail::shared_handle<shared_state<Snd, Env>> state) noexcept : _state(std::move(state)) {}

			~type() { if (_state) _state->detach(); }

			template<decays_to_same<type> T>
			friend constexpr signs_t tag_invoke(get_completion_signatures_t, T &&, auto) noexcept { return {}; }
			template<decays_to_same<type> T, receiver_of<signs_t> Rcv>
			friend operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(std::is_nothrow_move_constructible_v<Rcv>) { return operation_t<Rcv>{std::move(rcv), s._state}; }

			/* Since we already have a reference counted state, there is no need to split it further. */
			template<decays_to_same<type> T>
			friend constexpr type tag_invoke(split_t, T &&s) noexcept { return type{std::forward(s)._state}; }

		private:
			_detail::shared_handle<shared_state<Snd, Env>> _state;
		};

		class ensure_started_t
		{
			template<typename Snd>
			using value_scheduler = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Snd>
			using sender_t = typename sender<std::decay_t<Snd>, env_of_t<std::decay_t<Snd>>>::type;
			using back_adaptor = _detail::back_adaptor<ensure_started_t>;

		public:
			template<rod::sender Snd> requires _detail::tag_invocable_with_completion_scheduler<ensure_started_t, set_value_t, Snd, Snd>
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd) const noexcept(nothrow_tag_invocable<ensure_started_t, value_scheduler<Snd>, Snd>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd));
			}
			template<rod::sender Snd> requires(!_detail::tag_invocable_with_completion_scheduler<ensure_started_t, set_value_t, Snd, Snd> && tag_invocable<ensure_started_t, Snd>)
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd) const noexcept(nothrow_tag_invocable<ensure_started_t, Snd>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd));
			}

			/* ensure_started(_ensure_started::sender::type) should forward the argument. */
			template<rod::sender Snd> requires(!_detail::tag_invocable_with_completion_scheduler<ensure_started_t, set_value_t, Snd> && !tag_invocable<ensure_started_t, Snd> && !std::derived_from<std::decay_t<Snd>, ensure_started_tag>)
			[[nodiscard]] sender_t<Snd> operator()(Snd &&snd) const { return sender_t<Snd>{std::forward<Snd>(snd)}; }
			template<typename Snd> requires(std::derived_from<std::decay_t<Snd>, ensure_started_tag>)
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd) const noexcept { return std::forward<Snd>(snd); }

			[[nodiscard]] constexpr back_adaptor operator()() const noexcept { return {}; }
		};
	}

	using _ensure_started::ensure_started_t;

	/** Eagerly starts execution of the passed sender and returns a sender for it's completion.
	 * @param snd Sender to eagerly execute.
	 * @return Sender that completes with the completion results of the eagerly-started operation. */
	inline constexpr auto ensure_started = ensure_started_t{};
}

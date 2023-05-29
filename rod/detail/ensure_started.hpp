/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#include <optional>

#include "adaptors/closure.hpp"
#include "../stop_token.hpp"
#include "shared_ref.hpp"
#include "concepts.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
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
		struct operation { struct type; };
		struct operation_base;

		template<typename Snd, typename Env>
		struct shared_state { struct type; };
		template<typename Snd, typename Env>
		struct receiver { struct type; };
		template<typename Snd, typename Env>
		struct sender { struct type; };
		template<typename Env>
		struct env { struct type; };

		struct operation_base { void (*_notify)(operation_base *) noexcept = {}; };

		template<typename Env>
		struct env<Env>::type
		{
			template<decays_to<type> E>
			friend constexpr auto tag_invoke(get_stop_token_t, E &&e) noexcept { return e._token; }

			template<is_forwarding_query Q, decays_to<type> E, typename... Args> requires detail::callable<Q, Env, Args...>
			friend constexpr decltype(auto) tag_invoke(Q, E &&e, Args &&...args) noexcept(detail::nothrow_callable<Q, Env, Args...>)
			{
				return Q{}(*std::forward<E>(e)._env, std::forward<Args>(args)...);
			}

			in_place_stop_token _token;
			Env *_env = {};
		};

		template<typename Snd, typename Env>
		struct shared_state<Snd, Env>::type : public detail::shared_base
		{
			using receiver_t = typename receiver<Snd, Env>::type;
			using state_t = connect_result_t<Snd, receiver_t>;
			using env_t = typename env<Env>::type;

			using value_data_t = detail::gather_signatures_t<set_value_t, Snd, env_t, detail::bind_front<detail::decayed_tuple, set_value_t>::template type, std::variant>;
			using error_data_t = detail::gather_signatures_t<set_error_t, Snd, env_t, detail::bind_front<detail::decayed_tuple, set_value_t>::template type, std::variant>;
			using data_t = unique_tuple_t<detail::concat_tuples_t<std::variant<std::tuple<set_stopped_t>, std::tuple<set_error_t, std::exception_ptr>>, value_data_t, error_data_t>>;

			constexpr type(Snd &&snd) : env(get_env(snd)), state2(do_connect(std::forward<Snd>(snd))) { start(state2); }

			inline auto do_connect(Snd &&) noexcept(detail::nothrow_callable<connect_t, Snd, receiver_t>);
			void detach() noexcept { stop_src.request_stop(); }
			void notify() noexcept
			{
				auto *ptr = state1.exchange(this, std::memory_order_acq_rel);
				if (auto *op = static_cast<operation_base *>(ptr); op)
					op->_notify(op);
			}

			[[ROD_NO_UNIQUE_ADDRESS]] Env env;
			data_t data = std::tuple<set_stopped_t>{};

			in_place_stop_source stop_src = {};
			std::atomic<void *> state1 = {};
			state_t state2;
		};

		template<typename Snd, typename Env>
		struct receiver<Snd, Env>::type
		{
			using is_receiver = std::true_type;

			using _shared_state_t = typename shared_state<Snd, Env>::type;
			using _env_t = typename env<Env>::type;

			friend _env_t tag_invoke(get_env_t, const type &r) noexcept { return {r._state->stop_src.get_token(), &r._state->env}; }
			template<detail::completion_channel C, typename... Args>
			friend void tag_invoke(C, type &&r, Args &&...args) noexcept
			{
				auto &state = *r._state;

				using value_t = detail::decayed_tuple<C, Args...>;
				static_assert(std::constructible_from<value_t, C, Args...>);

				/* Bail if stop has been requested as per spec. */
				if (state.stop_src.stop_requested()) return;

				const auto do_emplace = [&]() { state.data.template emplace<value_t>(C{}, std::forward<Args>(args)...); };
				if constexpr (!std::is_nothrow_constructible_v<value_t, C, Args...>)
					try { do_emplace(); } catch (...) { state.data.template emplace<detail::decayed_tuple<set_error_t, std::exception_ptr>>(set_error, std::current_exception()); }
				else
					do_emplace();

				state.notify();
				r._state.reset();
			}

			detail::shared_handle<_shared_state_t> _state;
		};

		template<typename Snd, typename Env>
		auto shared_state<Snd, Env>::type::do_connect(Snd &&snd) noexcept(detail::nothrow_callable<connect_t, Snd, receiver_t>)
		{
			return connect(std::forward<Snd>(snd), receiver_t{static_cast<type *>(this->acquire())});
		}

		template<typename Snd, typename Rcv, typename Env>
		struct operation<Snd, Rcv, Env>::type : operation_base
		{
			using _stop_cb_t = std::optional<stop_callback_for_t<stop_token_of_t<env_of_t<Rcv> &>, stop_trigger>>;
			using _shared_state_t = typename shared_state<Snd, Env>::type;

			static void _bind_notify(operation_base *ptr) noexcept
			{
				auto *op = static_cast<type *>(ptr);
				op->_on_stop.reset();

				std::visit([&](auto &data) noexcept
				{
					std::apply([&](auto c, auto &...args) noexcept { c(std::move(op->_rcv), std::move(args)...); }, data);
				}, op->_state->data);
			}

			type(type &&) = delete;
			type &operator=(type &&) = delete;

			type(Rcv &&rcv, detail::shared_handle<_shared_state_t> handle) noexcept(std::is_nothrow_move_constructible_v<Rcv>)
					: operation_base{_bind_notify}, _state(std::move(handle)), _rcv(std::forward<Rcv>(rcv)) {}
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

			detail::shared_handle<_shared_state_t> _state;
			[[ROD_NO_UNIQUE_ADDRESS]] Rcv _rcv;
			_stop_cb_t _on_stop = {};
		};

		struct ensure_started_tag {};
		template<typename Snd, typename Env>
		struct sender<Snd, Env>::type : ensure_started_tag
		{
			using is_sender = std::true_type;

			using _shared_state_t = typename shared_state<Snd, Env>::type;
			template<typename Rcv>
			using _operation_t = typename operation<Snd, Rcv, Env>::type;
			using _env_t = typename env<Env>::type;

			template<typename... Ts>
			using _value_signs_t = completion_signatures<set_value_t(std::decay_t<Ts> &&...)>;
			template<typename T>
			using _error_signs_t = completion_signatures<set_error_t(std::decay_t<T> &&)>;
			using _signs_t = make_completion_signatures<Snd, _env_t, completion_signatures<set_error_t(std::exception_ptr &&)>, _value_signs_t, _error_signs_t>;

			type(Snd snd) : _state(new _shared_state_t{std::move(snd)}) {}
			~type() { if (_state) _state->detach(); }

			template<decays_to<type> T>
			friend constexpr _signs_t tag_invoke(get_completion_signatures_t, T &&, auto) noexcept { return {}; }
			template<decays_to<type> T, receiver_of<_signs_t> Rcv>
			friend _operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(std::is_nothrow_move_constructible_v<Rcv>)
			{
				return _operation_t<Rcv>{std::move(rcv), std::forward<T>(s)._state};
			}

			detail::shared_handle<_shared_state_t> _state;
		};

		class ensure_started_t
		{
			template<typename Snd>
			using value_scheduler = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Snd>
			using sender_t = typename sender<std::decay_t<Snd>, env_of_t<std::decay_t<Snd>>>::type;
			using back_adaptor = detail::back_adaptor<ensure_started_t>;

		public:
			template<rod::sender Snd> requires detail::tag_invocable_with_completion_scheduler<ensure_started_t, set_value_t, Snd, Snd>
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd) const noexcept(nothrow_tag_invocable<ensure_started_t, value_scheduler<Snd>, Snd>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd));
			}
			template<rod::sender Snd> requires(!detail::tag_invocable_with_completion_scheduler<ensure_started_t, set_value_t, Snd, Snd> && tag_invocable<ensure_started_t, Snd>)
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd) const noexcept(nothrow_tag_invocable<ensure_started_t, Snd>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd));
			}

			/* ensure_started(_ensure_started::sender::type) should forward the argument. */
			template<rod::sender Snd> requires(!detail::tag_invocable_with_completion_scheduler<ensure_started_t, set_value_t, Snd> && !tag_invocable<ensure_started_t, Snd> && !std::derived_from<std::decay_t<Snd>, ensure_started_tag>)
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
ROD_TOPLEVEL_NAMESPACE_CLOSE

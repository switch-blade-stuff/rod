/*
 * Created by switchblade on 2023-05-26.
 */

#pragma once

#include <optional>

#include "../../stop_token.hpp"
#include "../concepts.hpp"
#include "closure.hpp"

namespace rod
{
	namespace _with_stop_token
	{
		template<typename, typename>
		struct operation_base { struct type; };
		template<typename, typename, typename>
		struct operation { struct type; };
		template<typename, typename>
		struct receiver { struct type; };
		template<typename, typename>
		struct sender { struct type; };
		template<typename, typename>
		struct env { struct type; };
		template<typename, typename>
		struct stop_trigger;

		template<typename Env, typename Tok>
		struct env<Env, Tok>::type
		{
			template<decays_to<type> E>
			friend constexpr Tok tag_invoke(get_stop_token_t, E &&e) noexcept { return e._tok; }
			template<is_forwarding_query Q, decays_to<type> E, typename... Args> requires detail::callable<Q, Env, Args...>
			friend constexpr decltype(auto) tag_invoke(Q, E &&e, Args &&...args) noexcept(detail::nothrow_callable<Q, Env, Args...>)
			{
				return Q{}(std::forward<E>(e)._env, std::forward<Args>(args)...);
			}

			[[ROD_NO_UNIQUE_ADDRESS]] Env _env;
			[[ROD_NO_UNIQUE_ADDRESS]] Tok _tok;
		};

		template<typename Rcv, typename Tok>
		struct stop_trigger
		{
			inline void operator()() const noexcept;
			typename operation_base<Rcv, Tok>::type *op;
		};

		template<typename Rcv, typename Tok>
		struct operation_base<Rcv, Tok>::type
		{
			using _stop_cb_t = std::optional<stop_callback_for_t<Tok, stop_trigger<Rcv, Tok>>>;

			template<typename C, typename... Args>
			constexpr void _complete(Args &&...args) noexcept
			{
				if (!_is_done.test_and_set())
				{
					_stop_cb.reset();
					C{}(std::move(_rcv), std::forward<Args>(args)...);
				}
			}

			[[ROD_NO_UNIQUE_ADDRESS]] Rcv _rcv;
			[[ROD_NO_UNIQUE_ADDRESS]] Tok _tok;
			std::atomic_flag _is_done = {};
			_stop_cb_t _stop_cb = {};
		};

		template<typename Rcv, typename Tok>
		void stop_trigger<Rcv, Tok>::operator()() const noexcept { op->template _complete<set_stopped_t>(); }

		template<typename Rcv, typename Tok>
		struct receiver<Rcv, Tok>::type
		{
			using is_receiver = std::true_type;
			using _operation_base_t = typename operation_base<Rcv, Tok>::type;
			using _env_t = typename env<env_of_t<Rcv>, Tok>::type;

			friend constexpr _env_t tag_invoke(get_env_t, const type &r) noexcept(detail::nothrow_callable<get_env_t, const Rcv &>) { return {get_env(r._op->_rcv), r._op->_tok}; }
			template<detail::completion_channel C, typename... Args>
			friend constexpr void tag_invoke(C, type &&r, Args &&...args) noexcept { r._op->template _complete<C>(std::forward<Args>(args)...); }

			_operation_base_t *_op = {};
		};

		template<typename Snd, typename Rcv, typename Tok>
		struct operation<Snd, Rcv, Tok>::type : operation_base<Rcv, Tok>::type
		{
			using _operation_base_t = typename operation_base<Rcv, Tok>::type;
			using _receiver_t = typename receiver<Rcv, Tok>::type;
			using _state_t = connect_result_t<Snd, _receiver_t>;

			type(type &&) = delete;
			type &operator=(type &&) = delete;

			constexpr type(Snd &&snd, Tok tok, Rcv rcv) : _operation_base_t{std::move(rcv), tok}, _state(connect(std::forward<Snd>(snd), _receiver_t{this})) {}

			friend constexpr void tag_invoke(start_t, type &op) noexcept
			{
				op._stop_cb.emplace(op._tok, stop_trigger<Rcv, Tok>{&op});
				start(op._state);
			}

			_state_t _state;
		};

		template<typename Snd, typename Tok>
		struct sender<Snd, Tok>::type
		{
			using is_sender = std::true_type;

			template<typename T, typename Rcv>
			using _operation_t = typename operation<copy_cvref_t<T, Snd>, Rcv, Tok>::type;
			template<typename Rcv>
			using _receiver_t = typename receiver<Rcv, Tok>::type;
			using _env_t = typename env<env_of_t<Snd>, Tok>::type;

			using _signs_t = make_completion_signatures<Snd, env_of_t<Snd>, completion_signatures<set_stopped_t()>>;

			friend constexpr _env_t tag_invoke(get_env_t, const type &s) noexcept { return _env_t{get_env(s._snd), s._tok}; }
			template<decays_to<type> T, typename E>
			friend constexpr _signs_t tag_invoke(get_completion_signatures_t, T &&, E) { return {}; }

			template<decays_to<type> T, typename Rcv>
			friend constexpr _operation_t<T, Rcv> tag_invoke(connect_t, T &&s, Rcv rcv)
			{
				static_assert(sender_to<copy_cvref_t<T, Snd>, _receiver_t<Rcv>>);
				return _operation_t<T, Rcv>{std::move(s._snd), std::move(s._tok), std::move(rcv)};
			}

			[[ROD_NO_UNIQUE_ADDRESS]] Snd _snd;
			[[ROD_NO_UNIQUE_ADDRESS]] Tok _tok;
		};

		struct with_stop_token_t
		{
			template<typename Snd>
			using value_scheduler = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Tok>
			using back_adaptor = detail::back_adaptor<with_stop_token_t, Tok>;
			template<typename Snd, typename Tok>
			using sender_t = typename sender<std::decay_t<Snd>, Tok>::type;

		public:
			template<rod::sender Snd, stoppable_token Tok> requires detail::tag_invocable_with_completion_scheduler<with_stop_token_t, set_value_t, Snd, Snd, Tok>
			[[nodiscard]] constexpr rod::sender decltype(auto) operator()(Snd &&snd, Tok tok) const noexcept(nothrow_tag_invocable<with_stop_token_t, value_scheduler<Snd>, Snd, Tok>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), tok);
			}
			template<rod::sender Snd, stoppable_token Tok> requires(!detail::tag_invocable_with_completion_scheduler<with_stop_token_t, set_value_t, Snd, Snd, Tok> && tag_invocable<with_stop_token_t, Snd, Tok>)
			[[nodiscard]] constexpr rod::sender decltype(auto) operator()(Snd &&snd, Tok tok) const noexcept(nothrow_tag_invocable<with_stop_token_t, Snd, Tok>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), tok);
			}
			template<rod::sender Snd, stoppable_token Tok> requires(!detail::tag_invocable_with_completion_scheduler<with_stop_token_t, set_value_t, Snd, Snd, Tok> && !tag_invocable<with_stop_token_t, Snd, Tok>)
			[[nodiscard]] constexpr sender_t<Snd, Tok> operator()(Snd &&snd, Tok tok) const noexcept(std::is_nothrow_constructible_v<sender_t<Snd, Tok>, Snd, Tok>)
			{
				return sender_t<Snd, Tok>{std::forward<Snd>(snd), tok};
			}

			template<stoppable_token Tok>
			[[nodiscard]] constexpr back_adaptor<Tok> operator()(Tok tok) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Tok>, with_stop_token_t, Tok>)
			{
				return {*this, tok};
			}
		};
	}

	using _with_stop_token::with_stop_token_t;

	/** Customization point object used to adapt a sender to be stoppable via a stop token.
	 * @param snd Sender who's completion channels to adapt to be stoppable with a stop token. If omitted, creates a pipe-able sender adaptor.
	 * @param tok Stop token to use for stopping the sender.
	 * @return Sender stoppable via \a tok. */
	inline constexpr auto with_stop_token = with_stop_token_t{};
}

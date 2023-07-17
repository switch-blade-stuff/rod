/*
 * Created by switchblade on 2023-05-26.
 */

#pragma once

#include <optional>

#include "../receiver_adaptor.hpp"
#include "../../stop_token.hpp"
#include "../concepts.hpp"
#include "closure.hpp"

namespace rod
{
	namespace _with_stop_token
	{
		struct with_stop_token_t;

		template<typename, typename>
		struct receiver { class type; };
		template<typename, typename>
		struct sender { class type; };
		template<typename, typename>
		struct env { class type; };

		template<typename Env, typename Tok>
		class env<Env, Tok>::type
		{
		public:
			template<typename Env2>
			constexpr explicit type(Env2 &&env, Tok tok) noexcept(std::is_nothrow_constructible_v<Env, Env2> && std::is_nothrow_move_constructible_v<Tok>) : _env(std::forward<Env2>(env)), _tok(std::move(tok)) {}

			template<decays_to<type> E>
			friend constexpr Tok tag_invoke(get_stop_token_t, E &&e) noexcept { return e._tok; }

			template<is_forwarding_query Q, decays_to<type> E, typename... Args> requires detail::callable<Q, Env, Args...>
			friend constexpr decltype(auto) tag_invoke(Q, E &&e, Args &&...args) noexcept(detail::nothrow_callable<Q, Env, Args...>)
			{
				return Q{}(std::forward<E>(e)._env, std::forward<Args>(args)...);
			}

		private:
			ROD_NO_UNIQUE_ADDRESS Env _env;
			ROD_NO_UNIQUE_ADDRESS Tok _tok;
		};

		template<typename Rcv, typename Tok>
		class receiver<Rcv, Tok>::type : public receiver_adaptor<type, Rcv>
		{
			friend receiver_adaptor<type, Rcv>;

			using env_t = typename env<env_of_t<Rcv>, Tok>::type;

		public:
			constexpr explicit type(Rcv &&rcv, Tok tok) noexcept(std::is_nothrow_move_constructible_v<Rcv> && std::is_nothrow_move_constructible_v<Tok>) : receiver_adaptor<type, Rcv>(std::forward<Rcv>(rcv)), _tok(std::move(tok)) {}

		private:
			constexpr env_t get_env() const noexcept(detail::nothrow_callable<get_env_t, const Rcv &> && std::is_nothrow_copy_constructible_v<Tok>)
			{
				return env_t{rod::get_env(receiver_adaptor<type, Rcv>::base()), _tok};
			}
			template<typename... Args>
			constexpr void set_value(Args &&...args) noexcept
			{
				if (!_tok.stop_requested())
					rod::set_value(std::move(receiver_adaptor<type, Rcv>::base()), std::forward<Args>(args)...);
				else
					rod::set_stopped(std::move(receiver_adaptor<type, Rcv>::base()));
			}

			ROD_NO_UNIQUE_ADDRESS Tok _tok;
		};

		template<typename Snd, typename Tok>
		class sender<Snd, Tok>::type
		{
		public:
			using is_sender = std::true_type;

		private:
			template<typename Rcv>
			using receiver_t = typename receiver<Rcv, Tok>::type;
			using env_t = typename env<env_of_t<Snd>, Tok>::type;

			using signs_t = make_completion_signatures<Snd, env_t, completion_signatures<set_stopped_t()>>;

		public:
			template<typename Snd2>
			constexpr explicit type(Snd2 &&snd, Tok tok) noexcept(std::is_nothrow_constructible_v<Snd, Snd2> && std::is_nothrow_move_constructible_v<Tok>) : _snd(std::forward<Snd2>(snd)), _tok(std::move(tok)) {}

			friend constexpr env_t tag_invoke(get_env_t, const type &s) noexcept(detail::nothrow_callable<get_env_t, const Snd &> && std::is_nothrow_constructible_v<env_t, env_of_t<Snd>, const Tok &>) { return env_t{get_env(s._snd), s._tok}; }
			template<decays_to<type> T, typename E>
			friend constexpr signs_t tag_invoke(get_completion_signatures_t, T &&, E) noexcept { return {}; }

			template<decays_to<type> T, typename Rcv> requires sender_to<copy_cvref_t<T, Snd>, receiver_t<Rcv>>
			friend constexpr auto tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(std::is_nothrow_constructible_v<receiver_t<Rcv>, copy_cvref_t<T, Snd>, Rcv, copy_cvref_t<T, Snd>>)
			{
				return connect(std::forward<T>(s)._snd, receiver_t<Rcv>{std::move(rcv), std::forward<T>(s)._tok});
			}

		private:
			ROD_NO_UNIQUE_ADDRESS Snd _snd;
			ROD_NO_UNIQUE_ADDRESS Tok _tok;
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
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd, Tok tok) const noexcept(nothrow_tag_invocable<with_stop_token_t, value_scheduler<Snd>, Snd, Tok>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), tok);
			}
			template<rod::sender Snd, stoppable_token Tok> requires(!detail::tag_invocable_with_completion_scheduler<with_stop_token_t, set_value_t, Snd, Snd, Tok> && tag_invocable<with_stop_token_t, Snd, Tok>)
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd, Tok tok) const noexcept(nothrow_tag_invocable<with_stop_token_t, Snd, Tok>)
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

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
		class env<Env, Tok>::type : empty_base<Env>, empty_base<Tok>
		{
			using env_base = empty_base<Env>;
			using tok_base = empty_base<Tok>;

		public:
			template<typename Env2>
			constexpr explicit type(Env2 &&env, Tok tok) noexcept(std::is_nothrow_constructible_v<Env, Env2> && std::is_nothrow_move_constructible_v<Tok>) : env_base(std::forward<Env2>(env)), tok_base(std::move(tok)) {}

			template<decays_to<type> E>
			friend constexpr Tok tag_invoke(get_stop_token_t, E &&e) noexcept { return e.tok_base::value(); }

			template<is_forwarding_query Q, decays_to<type> E, typename... Args> requires _detail::callable<Q, Env, Args...>
			friend constexpr decltype(auto) tag_invoke(Q, E &&e, Args &&...args) noexcept(_detail::nothrow_callable<Q, Env, Args...>)
			{
				return Q{}(std::forward<E>(e).env_base::value(), std::forward<Args>(args)...);
			}
		};

		template<typename Rcv, typename Tok>
		class receiver<Rcv, Tok>::type : public receiver_adaptor<type, Rcv>, empty_base<Tok>
		{
			friend receiver_adaptor<type, Rcv>;

			using env_t = typename env<env_of_t<Rcv>, Tok>::type;
			using tok_base = empty_base<Tok>;

		public:
			constexpr explicit type(Rcv &&rcv, Tok tok) noexcept(std::is_nothrow_move_constructible_v<Rcv> && std::is_nothrow_move_constructible_v<Tok>) : receiver_adaptor<type, Rcv>(std::forward<Rcv>(rcv)), tok_base(std::move(tok)) {}

		private:
			constexpr env_t get_env() const noexcept(_detail::nothrow_callable<get_env_t, const Rcv &> && std::is_nothrow_copy_constructible_v<Tok>)
			{
				return env_t{rod::get_env(receiver_adaptor<type, Rcv>::base()), tok_base::value()};
			}
			template<typename... Args>
			constexpr void set_value(Args &&...args) noexcept
			{
				if (!tok_base::value().stop_requested())
					rod::set_value(std::move(receiver_adaptor<type, Rcv>::base()), std::forward<Args>(args)...);
				else
					rod::set_stopped(std::move(receiver_adaptor<type, Rcv>::base()));
			}
		};

		template<typename Snd, typename Tok>
		class sender<Snd, Tok>::type : empty_base<Snd>, empty_base<Tok>
		{
			using snd_base = empty_base<Snd>;
			using tok_base = empty_base<Tok>;

			template<typename Rcv>
			using receiver_t = typename receiver<Rcv, Tok>::type;
			using env_t = typename env<env_of_t<Snd>, Tok>::type;

			using signs_t = make_completion_signatures<Snd, env_t, completion_signatures<set_stopped_t()>>;

		public:
			using is_sender = std::true_type;

		public:
			template<typename Snd2>
			constexpr explicit type(Snd2 &&snd, Tok tok) noexcept(std::is_nothrow_constructible_v<Snd, Snd2> && std::is_nothrow_move_constructible_v<Tok>) : snd_base(std::forward<Snd2>(snd)), tok_base(std::move(tok)) {}

			friend constexpr env_t tag_invoke(get_env_t, const type &s) noexcept(_detail::nothrow_callable<get_env_t, const Snd &> && std::is_nothrow_constructible_v<env_t, env_of_t<Snd>, const Tok &>)
			{
				return env_t(get_env(s.snd_base::value()), s.tok_base::value());
			}
			template<decays_to<type> T, typename E>
			friend constexpr signs_t tag_invoke(get_completion_signatures_t, T &&, E) noexcept { return {}; }

			template<decays_to<type> T, typename Rcv> requires sender_to<copy_cvref_t<T, Snd>, receiver_t<Rcv>>
			friend constexpr auto tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(std::is_nothrow_constructible_v<receiver_t<Rcv>, copy_cvref_t<T, Snd>, Rcv, copy_cvref_t<T, Snd>>)
			{
				return connect(std::forward<T>(s).snd_base::value(), receiver_t<Rcv>{std::move(rcv), std::forward<T>(s).tok_base::value()});
			}
		};

		struct with_stop_token_t
		{
			template<typename Snd>
			using value_scheduler = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Tok>
			using back_adaptor = _detail::back_adaptor<with_stop_token_t, Tok>;
			template<typename Snd, typename Tok>
			using sender_t = typename sender<std::decay_t<Snd>, Tok>::type;

		public:
			template<rod::sender Snd, stoppable_token Tok> requires _detail::tag_invocable_with_completion_scheduler<with_stop_token_t, set_value_t, Snd, Snd, Tok>
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd, Tok tok) const noexcept(nothrow_tag_invocable<with_stop_token_t, value_scheduler<Snd>, Snd, Tok>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), tok);
			}
			template<rod::sender Snd, stoppable_token Tok> requires(!_detail::tag_invocable_with_completion_scheduler<with_stop_token_t, set_value_t, Snd, Snd, Tok> && tag_invocable<with_stop_token_t, Snd, Tok>)
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd, Tok tok) const noexcept(nothrow_tag_invocable<with_stop_token_t, Snd, Tok>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), tok);
			}
			template<rod::sender Snd, stoppable_token Tok> requires(!_detail::tag_invocable_with_completion_scheduler<with_stop_token_t, set_value_t, Snd, Snd, Tok> && !tag_invocable<with_stop_token_t, Snd, Tok>)
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

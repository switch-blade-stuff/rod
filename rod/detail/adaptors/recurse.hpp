/*
 * Created by switch_blade on 2023-06-12.
 */

#pragma once

#include "../concepts.hpp"

namespace rod
{
	namespace _recurse
	{
		class recurse_until_t;
		class recurse_t;

		template<typename, typename, typename>
		struct operation { class type; };
		template<typename, typename, typename>
		struct receiver { class type; };
		template<typename, typename>
		struct sender { class type; };

		template<typename Snd, typename Rcv, typename Pred>
		class receiver<Snd, Rcv, Pred>::type
		{
			using operation_t = typename operation<Snd, Rcv, Pred>::type;

		public:
			using is_receiver = std::true_type;

		public:
			constexpr explicit type(operation_t *op) noexcept : _op(op) {}

			friend env_of_t<Rcv> tag_invoke(get_env_t, const type &r) noexcept(_detail::nothrow_callable<get_env_t, const Rcv &>) { return r.get_env(); }

			template<_detail::completion_channel C, typename... Args> requires decays_to<C, set_value_t> && _detail::callable<Pred, Args...>
			friend void tag_invoke(C, type &&r, Args &&...args) noexcept { r.complete_value(std::forward<Args>(args)...); }
			template<_detail::completion_channel C, typename... Args> requires(!decays_to<C, set_value_t> && _detail::callable<C, Rcv, Args...>)
			friend void tag_invoke(C, type &&r, Args &&...args) noexcept { r.complete_forward(C{}, std::forward<Args>(args)...); }

		private:
			inline env_of_t<Rcv> get_env() const;
			template<typename... Args>
			inline void complete_value(Args &&...) noexcept;
			template<typename C, typename... Args>
			inline void complete_forward(C, Args &&...) noexcept;

			operation_t *_op = {};
		};

		template<typename Snd, typename Rcv, typename Pred>
		class operation<Snd, Rcv, Pred>::type
		{
			friend class receiver<Snd, Rcv, Pred>::type;

			using receiver_t = typename receiver<Snd, Rcv, Pred>::type;
			using state_t = connect_result_t<Snd &, receiver_t>;

		public:
			type(type &&) = delete;
			type &operator=(type &&) = delete;

			template<typename Snd2, typename Pred2>
			constexpr explicit type(Snd2 &&snd, Rcv &&rcv, Pred2 &&pred) noexcept(std::is_nothrow_constructible_v<Snd, Snd2> && std::is_nothrow_constructible_v<Pred, Pred2> && noexcept(_detail::nothrow_callable<connect_t, Snd &, receiver_t>))
					: _pred(std::forward<Pred2>(pred)), _snd(std::forward<Snd2>(snd)), _rcv(std::forward<Rcv>(rcv)), _state(_detail::eval_t{[&]() { return connect(_snd, receiver_t{this}); }}) {}

			friend constexpr void tag_invoke(start_t, type &op) noexcept { start(op._state); }

		private:
			void restart() noexcept(_detail::nothrow_callable<connect_t, Snd &, receiver_t>)
			{
				if constexpr (std::is_move_assignable_v<state_t>)
					_state = connect(_snd, receiver_t{this});
				else
				{
					std::destroy_at(&_state);
					std::construct_at(&_state, _detail::eval_t{[&]() { return connect(_snd, receiver_t{this}); }});
				}
				start(_state);
			}

			ROD_NO_UNIQUE_ADDRESS Pred _pred;
			ROD_NO_UNIQUE_ADDRESS Snd _snd;
			ROD_NO_UNIQUE_ADDRESS Rcv _rcv;
			state_t _state;
		};

		template<typename Snd, typename Rcv, typename Pred>
		env_of_t<Rcv> receiver<Snd, Rcv, Pred>::type::get_env() const { return rod::get_env(_op->_rcv); }

		template<typename Snd, typename Rcv, typename Pred>
		template<typename... Args>
		void receiver<Snd, Rcv, Pred>::type::complete_value(Args &&...args) noexcept
		{
			const auto restart_or_complete = [&]()
			{
				if (get_stop_token(get_env()).stop_requested())
					set_stopped(std::move(_op->_rcv));
				else if (!std::invoke(_op->_pred, args...))
					set_value(std::move(_op->_rcv));
				else
					_op->restart();
			};

			if constexpr (!std::is_nothrow_invocable_v<Pred, Args...> && noexcept(_op->restart()))
				try { restart_or_complete(); } catch(...) { set_error(std::move(_op->_rcv), std::current_exception()); }
			else
				restart_or_complete();
		}
		template<typename Snd, typename Rcv, typename Pred>
		template<typename C, typename... Args>
		void receiver<Snd, Rcv, Pred>::type::complete_forward(C, Args &&...args) noexcept { C{}(std::move(_op->_rcv), std::forward<Args>(args)...); }

		template<typename Snd, typename Pred>
		class sender<Snd, Pred>::type
		{
		public:
			using is_sender = std::true_type;

		private:
			template<typename Rcv>
			using operation_t = typename operation<Snd, Rcv, Pred>::type;
			template<typename Rcv>
			using receiver_t = typename receiver<Snd, Rcv, Pred>::type;

			template<typename...>
			using empty_value_t = completion_signatures<>;
			using signs_t = make_completion_signatures<Snd, env_of_t<Snd>, completion_signatures<set_error_t(std::exception_ptr), set_value_t()>, empty_value_t>;

		public:
			template<typename Snd2, typename Pred2>
			constexpr explicit type(Snd2 &&snd, Pred2 &&pred) noexcept(std::is_nothrow_constructible_v<Snd, Snd2> && std::is_nothrow_constructible_v<Pred, Pred2>) : _pred(std::forward<Pred2>(pred)), _snd(std::forward<Snd2>(snd)) {}

			friend constexpr env_of_t<Snd> tag_invoke(get_env_t, const type &s) noexcept(_detail::nothrow_callable<get_env_t, const Snd &>) { return get_env(s._snd); }
			template<decays_to<type> T, typename Env>
			friend constexpr signs_t tag_invoke(get_completion_signatures_t, T &&, Env) { return {}; }

			template<decays_to<type> T, rod::receiver Rcv> requires sender_to<Snd, receiver_t<Rcv>> && receiver_of<Rcv, signs_t>
			friend constexpr operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(std::is_nothrow_constructible_v<operation_t<Rcv>, copy_cvref_t<T, Snd>, Rcv, copy_cvref_t<T, Pred>>)
			{
				return operation_t<Rcv>{std::forward<T>(s)._snd, std::move(rcv), std::forward<T>(s)._pred};
			}

		private:
			ROD_NO_UNIQUE_ADDRESS Pred _pred;
			ROD_NO_UNIQUE_ADDRESS Snd _snd;
		};

		class recurse_t
		{
			struct always_true { constexpr bool operator()(auto &&...) const noexcept { return true; } };

			template<typename Snd, typename Pred>
			using sender_t = typename sender<std::decay_t<Snd>, std::decay_t<Pred>>::type;

		public:
			template<rod::sender Snd, movable_value Pred> requires tag_invocable<recurse_t, Snd, Pred>
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd, Pred &&pred) const noexcept(nothrow_tag_invocable<recurse_t, Snd, Pred>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<Pred>(pred));
			}
			template<rod::sender Snd> requires tag_invocable<recurse_t, Snd>
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd) const noexcept(nothrow_tag_invocable<recurse_t, Snd>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd));
			}

			template<rod::sender Snd, movable_value Pred> requires(!tag_invocable<recurse_t, Snd, Pred> && std::copy_constructible<std::decay_t<Snd>>)
			[[nodiscard]] constexpr sender_t<Snd, Pred> operator()(Snd &&snd, Pred &&pred) const noexcept(std::is_nothrow_constructible_v<sender_t<Snd, Pred>, Pred, Snd>)
			{
				return sender_t<Snd, Pred>{std::forward<Snd>(snd), std::forward<Pred>(pred)};
			}
			template<rod::sender Snd> requires(!tag_invocable<recurse_t, Snd> && std::copy_constructible<std::decay_t<Snd>>)
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd) const noexcept(_detail::nothrow_callable<recurse_t, Snd, always_true>)
			{
				return operator()(std::forward<Snd>(snd), always_true{});
			}
		};
	}

	using _recurse::recurse_t;

	/** Customization point object used to create an operation that will recursively re-start itself until stopped.
	 * @param snd Input sender to use for starting the recursive operation.
	 * @return Sender completing via the error and stopped channels if \a snd completes with error or a stop request. If omitted, creates an infinitely-recursing sender.
	 * @warning Using `recurse` with a sender that completes immediately upon the call to `start` (such as `rod::just`) can result in stack overflow. */
	inline constexpr auto recurse = recurse_t{};
}

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
			friend operation<Snd, Rcv, Pred>::type;

		public:
			using is_receiver = std::true_type;

		private:
			using operation_t = typename operation<Snd, Rcv, Pred>::type;

			constexpr type(operation_t *op) noexcept : _op(op) {}

		public:
			friend constexpr env_of_t<Rcv> tag_invoke(get_env_t, const type &r) noexcept(detail::nothrow_callable<get_env_t, const Rcv &>) { return r.get_env(); }

			template<typename C, typename... Args> requires(requires(type &&r, Args &&...args) { r.complete(C{}, std::forward<Args>(args)...); })
			friend constexpr void tag_invoke(C, type &&r, Args &&...args) noexcept { r.complete(C{}, std::forward<Args>(args)...); }

		private:
			constexpr auto get_env() const;
			template<typename C, typename... Args> requires(!decays_to<C, set_value_t> && detail::callable<C, Rcv, Args...>)
			constexpr void complete(C, Args &&...args) noexcept;
			template<typename C, typename... Args> requires decays_to<C, set_value_t> && detail::callable<Pred, Args...>
			constexpr void complete(C, Args &&...args) noexcept;

			operation_t *_op = {};
		};

		template<typename Snd, typename Rcv, typename Pred>
		class operation<Snd, Rcv, Pred>::type
		{
			friend receiver<Snd, Rcv, Pred>::type;
			friend sender<Snd, Pred>::type;

			using receiver_t = typename receiver<Snd, Rcv, Pred>::type;
			using state_t = connect_result_t<Snd &, receiver_t>;

			template<typename Snd2, typename Pred2>
			constexpr type(Snd2 &&snd, Rcv &&rcv, Pred2 &&pred) noexcept(std::is_nothrow_constructible_v<Snd, Snd2> && std::is_nothrow_constructible_v<Pred, Pred2> && noexcept(detail::nothrow_callable<connect_t, Snd &, receiver_t>))
					: _pred(std::forward<Pred2>(pred)), _snd(std::forward<Snd2>(snd)), _rcv(std::forward<Rcv>(rcv)), _state(connect()) {}

		public:
			type(type &&) = delete;
			type &operator=(type &&) = delete;

			friend constexpr void tag_invoke(start_t, type &op) noexcept { start(op._state); }

		private:
			constexpr auto connect() {return detail::implicit_eval{[&]() { return rod::connect(_snd, receiver_t{this}); }}; }
			constexpr void reconnect() noexcept(detail::nothrow_callable<connect_t, Snd &, receiver_t>)
			{
				std::destroy_at(&_state);
				std::construct_at(&_state, connect());
			}
			constexpr void restart() noexcept
			{
				if constexpr (!noexcept(this->reconnect()))
					try { reconnect(); } catch (...) { complete(set_error, std::current_exception()); }
				else
					reconnect();
				start(_state);
			}

			template<typename C, typename... Args> requires(!decays_to<C, set_value_t> && detail::callable<C, Rcv, Args...>)
			constexpr void complete(C, Args &&...args) noexcept { C{}(std::move(_rcv), std::forward<Args>(args)...); }
			template<typename C, typename... Args> requires decays_to<C, set_value_t> && detail::callable<Pred, Args...>
			constexpr void complete(C, Args &&...args) noexcept
			{
				if (get_stop_token(get_env(_rcv)).stop_requested())
					set_stopped(std::move(_rcv));
				else if (!std::invoke(_pred, args...))
					C{}(std::move(_rcv));
				else
					restart();
			}

			ROD_NO_UNIQUE_ADDRESS Pred _pred;
			ROD_NO_UNIQUE_ADDRESS Snd _snd;
			ROD_NO_UNIQUE_ADDRESS Rcv _rcv;
			state_t _state;
		};

		template<typename Snd, typename Rcv, typename Pred>
		constexpr auto receiver<Snd, Rcv, Pred>::type::get_env() const { return rod::get_env(_op->_rcv); }
		template<typename Snd, typename Rcv, typename Pred>
		template<typename C, typename... Args> requires(!decays_to<C, set_value_t> && detail::callable<C, Rcv, Args...>)
		constexpr void receiver<Snd, Rcv, Pred>::type::complete(C, Args &&...args) noexcept { _op->complete(C{}, std::forward<Args>(args)...); }
		template<typename Snd, typename Rcv, typename Pred>
		template<typename C, typename... Args> requires decays_to<C, set_value_t> && detail::callable<Pred, Args...>
		constexpr void receiver<Snd, Rcv, Pred>::type::complete(C, Args &&...args) noexcept { _op->complete(C{}, std::forward<Args>(args)...); }

		template<typename Snd, typename Pred>
		class sender<Snd, Pred>::type
		{
			friend recurse_until_t;

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

			template<decays_to<type> T, typename Rcv>
			constexpr static auto connect(T &&s, Rcv &&rcv) { return operation_t<Rcv>{std::forward<T>(s)._snd, std::forward<Rcv>(rcv), std::forward<T>(s)._pred}; }

			template<typename Snd2, typename Pred2>
			constexpr type(Snd2 &&snd, Pred2 &&pred) noexcept(std::is_nothrow_constructible_v<Snd, Snd2> && std::is_nothrow_constructible_v<Pred, Pred2>) : _snd(std::forward<Snd2>(snd)), _pred(std::forward<Pred2>(pred)) {}

		public:
			friend constexpr env_of_t<Snd> tag_invoke(get_env_t, const type &s) noexcept(detail::nothrow_callable<get_env_t, const Snd &>) { return get_env(s._snd); }
			template<decays_to<type> T, typename Env>
			friend constexpr signs_t tag_invoke(get_completion_signatures_t, T &&, Env) { return {}; }

			template<decays_to<type> T, typename Rcv> requires sender_to<Snd, receiver_t<Rcv>> && receiver_of<Rcv, signs_t>
			friend constexpr operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(std::is_nothrow_constructible_v<operation_t<Rcv>, copy_cvref_t<T, Snd>, Rcv, copy_cvref_t<T, Pred>>)
			{
				return connect(std::forward<T>(s), std::move(rcv));
			}

		private:
			ROD_NO_UNIQUE_ADDRESS Pred _pred;
			ROD_NO_UNIQUE_ADDRESS Snd _snd;
		};

		class recurse_until_t
		{
			template<typename Snd, typename Pred>
			using sender_t = typename sender<std::decay_t<Snd>, std::decay_t<Pred>>::type;
			template<typename Pred>
			using back_adaptor = detail::back_adaptor<recurse_until_t, std::decay_t<Pred>>;

		public:
			template<rod::sender Snd, movable_value Pred> requires tag_invocable<recurse_until_t, Snd, Pred>
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd, Pred &&pred) const noexcept(nothrow_tag_invocable<recurse_until_t, Snd, Pred>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<Pred>(pred));
			}
			template<rod::sender Snd, movable_value Pred> requires(!tag_invocable<recurse_until_t, Snd, Pred> && std::copy_constructible<std::decay_t<Snd>>)
			[[nodiscard]] constexpr sender_t<Snd, Pred> operator()(Snd &&snd, Pred &&pred) const noexcept(std::is_nothrow_constructible_v<sender_t<Snd, Pred>, Pred, Snd>)
			{
				return sender_t<Snd, Pred>{std::forward<Snd>(snd), std::forward<Pred>(pred)};
			}

			template<movable_value Pred>
			[[nodiscard]] constexpr back_adaptor<Pred> operator()(Pred &&pred) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Pred>, recurse_until_t, Pred>)
			{
				return {*this, std::forward<Pred>(pred)};
			}
		};
		class recurse_t
		{
			using back_adaptor = detail::back_adaptor<recurse_t>;

			struct always_true { constexpr bool operator()(auto &&...) const noexcept { return true; } };

		public:
			template<rod::sender Snd> requires tag_invocable<recurse_t, Snd>
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd) const noexcept(nothrow_tag_invocable<recurse_until_t, Snd>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd));
			}
			template<rod::sender Snd> requires(!tag_invocable<recurse_t, Snd> && std::copy_constructible<std::decay_t<Snd>>)
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd) const noexcept(detail::nothrow_callable<recurse_until_t, Snd, always_true>)
			{
				return recurse_until_t{}(std::forward<Snd>(snd), always_true{});
			}

			[[nodiscard]] constexpr back_adaptor operator()() const noexcept { return back_adaptor{}; }
		};
	}

	using _recurse::recurse_t;

	/** Customization point object used to create an operation that will recursively re-start itself until stopped.
	 * @param snd Input sender to use for starting the recursive operation. If omitted, creates a pipe-able sender adaptor.
	 * @return Sender completing via the error and stopped channels if \a snd completes with error or a stop request.
	 * @warning Using `recurse` with a sender that completes immediately upon the call to `start` (such as `rod::execute`) can result in stack overflow. */
	inline constexpr auto recurse = recurse_t{};

	using _recurse::recurse_until_t;

	/** Customization point object used to create an operation that will recursively re-start itself until stopped.
	 * @param snd Input sender to use for starting the recursive operation. If omitted, creates a pipe-able sender adaptor.
	 * @param pred Predicate invoked with value channel results of \a snd returning `false` to stop the recursive operation.
	 * @return Sender completing via the value channel when recursion is stopped using \a pred, and via the error and stopped channels if \a snd completes with error or a stop request.
	 * @warning Using `recurse` with a sender that completes immediately upon the call to `start` (such as `rod::execute`) can result in stack overflow. */
	inline constexpr auto recurse_until = recurse_until_t{};
}
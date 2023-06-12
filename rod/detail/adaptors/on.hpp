/*
 * Created by switchblade on 2023-05-06.
 */

#pragma once

#include "../queries/scheduler.hpp"
#include "../concepts.hpp"

namespace rod
{
	namespace _on
	{
		class on_t;

		template<typename, typename, typename>
		struct receiver_ref { class type; };
		template<typename, typename, typename>
		struct receiver { class type; };

		template<typename, typename, typename>
		struct operation { class type; };
		template<typename, typename>
		struct sender { class type; };
		template<typename, typename>
		struct env { class type; };

		template<typename Sch, typename Env>
		class env<Sch, Env>::type
		{
			template<typename, typename, typename>
			friend struct receiver_ref;

			template<typename Env2>
			constexpr type(Env2 &&env) noexcept(std::is_nothrow_constructible_v<Env, Env2>) : _env(std::forward<Env2>(env)) {}

		public:
			template<decays_to<type> E>
			friend constexpr Sch tag_invoke(get_scheduler_t, E &&e) noexcept { return get_scheduler(e._env); }
			template<is_forwarding_query Q, decays_to<type> E, typename... Args> requires detail::callable<Q, Env, Args...>
			friend constexpr decltype(auto) tag_invoke(Q, E &&e, Args &&...args) noexcept(detail::nothrow_callable<Q, Env, Args...>)
			{
				return Q{}(std::forward<E>(e)._env, std::forward<Args>(args)...);
			}

		private:
			ROD_NO_UNIQUE_ADDRESS Env _env;
		};

		template<typename Sch, typename Snd, typename Rcv>
		class operation<Sch, Snd, Rcv>::type
		{
			friend receiver_ref<Sch, Snd, Rcv>::type;
			friend receiver<Sch, Snd, Rcv>::type;
			friend sender<Sch, Snd>::type;

		private:
			using receiver_t = typename receiver<Sch, Snd, Rcv>::type;
			using state_t = std::variant<connect_result_t<schedule_result_t<Sch>, receiver_t>, connect_result_t<Snd, typename receiver_ref<Sch, Snd, Rcv>::type>>;

			template<typename Sch2, typename Snd2>
			constexpr type(Sch2 &&sch, Snd2 &&snd, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<Sch, Sch2> && std::is_nothrow_constructible_v<Snd, Snd2> && std::is_nothrow_move_constructible_v<Rcv>)
					: _sch(std::forward<Sch2>(sch)), _snd(std::forward<Snd2>(snd)), _rcv(std::forward<Rcv>(rcv)), _state(std::in_place_index<0>, connect()) {}

		public:
			type(type &&) = delete;
			type &operator=(type &&) = delete;

			friend constexpr void tag_invoke(start_t, type &op) noexcept { op.start(); }

		private:
			constexpr auto connect() noexcept { return detail::implicit_eval{[&]() { return rod::connect(schedule(_sch), receiver_t{this}); }}; }
			constexpr void start() noexcept { return rod::start(std::get<0>(_state)); }

			ROD_NO_UNIQUE_ADDRESS Sch _sch;
			ROD_NO_UNIQUE_ADDRESS Snd _snd;
			ROD_NO_UNIQUE_ADDRESS Rcv _rcv;
			state_t _state;
		};

		template<typename Sch, typename Snd, typename Rcv>
		class receiver_ref<Sch, Snd, Rcv>::type
		{
			friend receiver<Sch, Snd, Rcv>::type;

		public:
			using is_receiver = std::true_type;

		private:
			using operation_t = typename operation<Sch, Snd, Rcv>::type;
			using env_t = typename env<Sch, env_of_t<Rcv>>::type;

			constexpr type(operation_t *op) noexcept : _op(op) {}

		public:
			friend constexpr env_t tag_invoke(get_env_t, const type &r) noexcept(detail::nothrow_callable<get_env_t, const Rcv &> && std::is_nothrow_constructible_v<env_t, env_of_t<Rcv>>) { return r.get_env(); }
			template<detail::completion_channel C, typename... Args> requires detail::callable<C, Rcv, Args...>
			friend constexpr void tag_invoke(C, type &&r, Args &&...args) noexcept { r.template complete<C>(std::forward<Args>(args)...); }

		private:
			constexpr auto get_env() const { return env_t{rod::get_env(_op->_rcv)}; }
			template<typename C, typename... Args>
			constexpr void complete(Args &&...args) noexcept { C{}(std::move(_op->_rcv), std::forward<Args>(args)...); }

			operation_t *_op = {};
		};
		template<typename Sch, typename Snd, typename Rcv>
		class receiver<Sch, Snd, Rcv>::type
		{
			friend operation<Sch, Snd, Rcv>::type;

		public:
			using is_receiver = std::true_type;

		private:
			using receiver_ref_t = typename receiver_ref<Sch, Snd, Rcv>::type;
			using operation_t = typename operation<Sch, Snd, Rcv>::type;

			constexpr type(operation_t *op) noexcept : _op(op) {}

		public:
			friend constexpr env_of_t<Rcv> tag_invoke(get_env_t, const type &r) noexcept(detail::nothrow_callable<get_env_t, const Rcv &>) { return r.get_env(); }
			template<typename C, typename... Args> requires(requires (type &&r, Args &&...args) { r.complete(C{}, std::forward<Args>(args)...); })
			friend constexpr void tag_invoke(C, type &&r, Args &&...args) noexcept { r.complete(C{}, std::forward<Args>(args)...); }

		private:
			constexpr auto get_env() const { return rod::get_env(_op->_rcv); }
			template<typename C, typename... Args> requires(!decays_to<C, set_value_t> && detail::callable<C, Rcv, Args...>)
			constexpr void complete(C, Args &&...args) { C{}(std::move(_op->_rcv), std::forward<Args>(args)...); }
			template<typename C> requires decays_to<C, set_value_t>
			constexpr void complete(C)
			{
				constexpr bool nothrow_start = detail::nothrow_callable<start_t, std::add_lvalue_reference<connect_result_t<Snd, receiver_ref_t>>>;
				constexpr bool nothrow_connect = detail::nothrow_callable<connect_t, Snd, receiver_ref_t>;

				auto *op = _op;
				const auto start = [op]()
				{
					/* Use a conversion wrapper to allow emplacement of non-movable types. */
					const auto conv = [=]() { return connect(std::move(op->_snd), receiver_ref_t{op}); };
					rod::start(op->_state.template emplace<1>(detail::implicit_eval{conv}));
				};

				if constexpr (!(nothrow_connect && nothrow_start))
					try { start(); } catch (...) { set_error(std::move(op->_rcv), std::current_exception()); }
				else
					start();
			}

			operation_t *_op = {};
		};

		template<typename Sch, typename Snd>
		class sender<Sch, Snd>::type
		{
			friend on_t;

		public:
			using is_sender = std::true_type;

		private:
			template<typename Rcv>
			using receiver_ref_t = typename receiver_ref<Sch, Snd, Rcv>::type;
			template<typename Rcv>
			using operation_t = typename operation<Sch, Snd, Rcv>::type;
			template<typename Rcv>
			using receiver_t = typename receiver<Sch, Snd, Rcv>::type;
			template<typename Env>
			using env_t = typename env<Sch, Env>::type;

			template<typename...>
			using empty_signs_t = completion_signatures<>;
			template<typename T, typename Env>
			using signs_t = make_completion_signatures<schedule_result_t<Sch>, Env, make_completion_signatures<copy_cvref_t<T, Snd>, env_t<Env>, completion_signatures<set_error_t(std::exception_ptr)>>, empty_signs_t>;

			template<decays_to<type> T, typename Rcv>
			constexpr static auto connect(T &&s, Rcv &&rcv) { return operation_t<Rcv>{std::forward<T>(s)._sch, std::forward<T>(s)._snd, std::forward<Rcv>(rcv)}; }

			template<typename Sch2, typename Snd2>
			constexpr type(Sch2 &&sch, Snd2 &&snd) noexcept(std::is_nothrow_constructible_v<Sch, Sch2> && std::is_nothrow_constructible_v<Snd, Snd2>) : _sch(std::forward<Sch2>(sch)), _snd(std::forward<Snd2>(snd)) {}

		public:
			friend constexpr env_of_t<Snd> tag_invoke(get_env_t, const type &s) noexcept(detail::nothrow_callable<get_env_t, const Snd &>) { return get_env(s._snd); }
			template<decays_to<type> T, typename Env>
			friend constexpr signs_t<T, Env> tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }

			template<decays_to<type> T, typename Rcv>
			friend constexpr operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(std::is_nothrow_constructible_v<operation_t<Rcv>, copy_cvref_t<T, Sch>, copy_cvref_t<T, Snd>, Rcv>)
			{
				static_assert(std::constructible_from<Sch, copy_cvref_t<T, Sch>>);
				static_assert(std::constructible_from<Snd, copy_cvref_t<T, Snd>>);
				static_assert(sender_to<schedule_result_t<Sch>, receiver_t<Rcv>>);
				static_assert(sender_to<Snd, receiver_ref_t<Rcv>>);
				return connect(std::forward<T>(s), std::move(rcv));
			}

		private:
			ROD_NO_UNIQUE_ADDRESS Sch _sch;
			ROD_NO_UNIQUE_ADDRESS Snd _snd;
		};

		class on_t
		{
			template<typename Sch, typename Snd>
			using sender_t = typename sender<std::decay_t<Sch>, std::decay_t<Snd>>::type;

		public:
			template<rod::scheduler Sch, rod::sender Snd> requires tag_invocable<on_t, Sch, Snd>
			[[nodiscard]] constexpr rod::sender auto operator()(Sch &&sch, Snd &&snd) const noexcept(nothrow_tag_invocable<on_t, Sch, Snd>)
			{
				return tag_invoke(*this, std::forward<Sch>(sch), std::forward<Snd>(snd));
			}
			template<rod::scheduler Sch, rod::sender Snd> requires(!tag_invocable<on_t, Sch, Snd>)
			[[nodiscard]] constexpr sender_t<Sch, Snd> operator()(Sch &&sch, Snd &&snd) const noexcept(std::is_nothrow_constructible_v<sender_t<Sch, Snd>, Sch, Snd>)
			{
				return sender_t<Sch, Snd>{std::forward<Sch>(sch), std::forward<Snd>(snd)};
			}
		};
	}

	using _on::on_t;

	/** Sender adaptor used to adapt an input sender into a sender that will start on associated execution context belonging to a scheduler.
	 * @param sch Scheduler to adapt the input sender to.
	 * @param snd Sender to adapt to \a sch scheduler. If omitted, creates a pipe-able sender adaptor.
	 * @return Sender adaptor for \a snd that starts on \a sch. */
	inline constexpr auto on = _on::on_t{};
}

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
		class env<Sch, Env>::type : empty_base<Env>
		{
			using env_base = empty_base<Env>;

		public:
			template<typename Env2>
			constexpr explicit type(Env2 &&env) noexcept(std::is_nothrow_constructible_v<Env, Env2>) : env_base(std::forward<Env2>(env)) {}

			template<decays_to<type> E>
			friend constexpr Sch tag_invoke(get_scheduler_t, E &&e) noexcept { return get_scheduler(e.env_base::value()); }
			template<is_forwarding_query Q, decays_to<type> E, typename... Args> requires _detail::callable<Q, Env, Args...>
			friend constexpr decltype(auto) tag_invoke(Q, E &&e, Args &&...args) noexcept(_detail::nothrow_callable<Q, Env, Args...>)
			{
				return Q{}(std::forward<E>(e).env_base::value(), std::forward<Args>(args)...);
			}
		};

		template<typename Sch, typename Snd, typename Rcv>
		class receiver_ref<Sch, Snd, Rcv>::type
		{
		public:
			using is_receiver = std::true_type;

		private:
			using operation_t = typename operation<Sch, Snd, Rcv>::type;
			using env_t = typename env<Sch, env_of_t<Rcv>>::type;

		public:
			constexpr explicit type(operation_t *op) noexcept : _op(op) {}

			friend env_t tag_invoke(get_env_t, const type &r) noexcept(_detail::nothrow_callable<get_env_t, const Rcv &> && std::is_nothrow_constructible_v<env_t, env_of_t<Rcv>>) { return r.get_env(); }
			template<_detail::completion_channel C, typename... Args> requires _detail::callable<C, Rcv, Args...>
			friend void tag_invoke(C, type &&r, Args &&...args) noexcept { r.complete(C{}, std::forward<Args>(args)...); }

		private:
			inline env_t get_env() const;
			template<typename C, typename... Args>
			inline void complete(C, Args &&...args) noexcept;

			operation_t *_op = {};
		};
		template<typename Sch, typename Snd, typename Rcv>
		class receiver<Sch, Snd, Rcv>::type
		{
		public:
			using is_receiver = std::true_type;

		private:
			using receiver_ref_t = typename receiver_ref<Sch, Snd, Rcv>::type;
			using operation_t = typename operation<Sch, Snd, Rcv>::type;

		public:
			constexpr explicit type(operation_t *op) noexcept : _op(op) {}

			friend env_of_t<Rcv> tag_invoke(get_env_t, const type &r) noexcept(_detail::nothrow_callable<get_env_t, const Rcv &>) { return r.get_env(); }

			template<_detail::completion_channel C> requires decays_to<C, set_value_t>
			friend void tag_invoke(C, type &&r) noexcept { r.complete_value(); }
			template<_detail::completion_channel C, typename... Args> requires(!decays_to<C, set_value_t> && _detail::callable<C, Rcv, Args...>)
			friend void tag_invoke(C, type &&r, Args &&...args) noexcept { r.complete_forward(C{}, std::forward<Args>(args)...); }

		private:
			inline env_of_t<Rcv> get_env() const;
			inline void complete_value() noexcept;
			template<typename C, typename... Args>
			inline void complete_forward(C, Args &&...args) noexcept;

			operation_t *_op = {};
		};

		template<typename Sch, typename Snd, typename Rcv>
		class operation<Sch, Snd, Rcv>::type : empty_base<Sch>, empty_base<Snd>, empty_base<Rcv>
		{
			friend class receiver_ref<Sch, Snd, Rcv>::type;
			friend class receiver<Sch, Snd, Rcv>::type;

			using sch_base = empty_base<Sch>;
			using snd_base = empty_base<Snd>;
			using rcv_base = empty_base<Rcv>;

			using receiver_t = typename receiver<Sch, Snd, Rcv>::type;
			using state_t = std::variant<connect_result_t<schedule_result_t<Sch>, receiver_t>, connect_result_t<Snd, typename receiver_ref<Sch, Snd, Rcv>::type>>;

		public:
			type(type &&) = delete;
			type &operator=(type &&) = delete;

			template<typename Sch2, typename Snd2>
			constexpr explicit type(Sch2 &&sch, Snd2 &&snd, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<Sch, Sch2> && std::is_nothrow_constructible_v<Snd, Snd2> && std::is_nothrow_move_constructible_v<Rcv>)
					: sch_base(std::forward<Sch2>(sch)), snd_base(std::forward<Snd2>(snd)), rcv_base(std::forward<Rcv>(rcv)), _state(std::in_place_index<0>, connect()) {}

			friend constexpr void tag_invoke(start_t, type &op) noexcept { op.start(); }

		private:
			constexpr auto connect() noexcept { return _detail::eval_t{[&]() { return rod::connect(schedule(sch_base::value()), receiver_t{this}); }}; }
			constexpr void start() noexcept { return rod::start(std::get<0>(_state)); }
			state_t _state;
		};

		template<typename Sch, typename Snd, typename Rcv>
		env_of_t<Rcv> receiver<Sch, Snd, Rcv>::type::get_env() const { return rod::get_env(_op->rcv_base::value()); }
		template<typename Sch, typename Snd, typename Rcv>
		typename env<Sch, env_of_t<Rcv>>::type receiver_ref<Sch, Snd, Rcv>::type::get_env() const { return typename env<Sch, env_of_t<Rcv>>::type{rod::get_env(_op->rcv_base::value())}; }

		template<typename Sch, typename Snd, typename Rcv>
		void receiver<Sch, Snd, Rcv>::type::complete_value() noexcept
		{
			constexpr bool nothrow_start = _detail::nothrow_callable<start_t, std::add_lvalue_reference<connect_result_t<Snd, receiver_ref_t>>>;
			constexpr bool nothrow_connect = _detail::nothrow_callable<connect_t, Snd, receiver_ref_t>;

			auto *op = _op;
			const auto start = [op]()
			{
				/* Use a conversion wrapper to allow emplacement of non-movable types. */
				const auto conv = [=]() { return connect(std::move(op->snd_base::value()), receiver_ref_t{op}); };
				rod::start(op->_state.template emplace<1>(_detail::eval_t{conv}));
			};

			if constexpr (!(nothrow_connect && nothrow_start))
				try { start(); } catch (...) { set_error(std::move(op->rcv_base::value()), std::current_exception()); }
			else
				start();
		}
		template<typename Sch, typename Snd, typename Rcv>
		template<typename C, typename... Args>
		void receiver<Sch, Snd, Rcv>::type::complete_forward(C, Args &&... args) noexcept { C{}(std::move(_op->rcv_base::value()), std::forward<Args>(args)...); }
		template<typename Sch, typename Snd, typename Rcv>
		template<typename C, typename... Args>
		void receiver_ref<Sch, Snd, Rcv>::type::complete(C, Args &&... args) noexcept { C{}(std::move(_op->rcv_base::value()), std::forward<Args>(args)...); }

		template<typename Sch, typename Snd>
		class sender<Sch, Snd>::type : empty_base<Sch>, empty_base<Snd>
		{
			using sch_base = empty_base<Sch>;
			using snd_base = empty_base<Snd>;

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

		public:
			template<typename Sch2, typename Snd2>
			constexpr explicit type(Sch2 &&sch, Snd2 &&snd) noexcept(std::is_nothrow_constructible_v<Sch, Sch2> && std::is_nothrow_constructible_v<Snd, Snd2>) : sch_base(std::forward<Sch2>(sch)), snd_base(std::forward<Snd2>(snd)) {}

			friend constexpr env_of_t<Snd> tag_invoke(get_env_t, const type &s) noexcept(_detail::nothrow_callable<get_env_t, const Snd &>) { return get_env(s.snd_base::value()); }
			template<decays_to<type> T, typename Env>
			friend constexpr signs_t<T, Env> tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }

			template<decays_to<type> T, rod::receiver Rcv> requires sender_to<schedule_result_t<Sch>, receiver_t<Rcv>> && sender_to<Snd, receiver_ref_t<Rcv>>
			friend constexpr operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(std::is_nothrow_constructible_v<operation_t<Rcv>, copy_cvref_t<T, Sch>, copy_cvref_t<T, Snd>, Rcv>)
			{
				static_assert(std::constructible_from<Sch, copy_cvref_t<T, Sch>>);
				static_assert(std::constructible_from<Snd, copy_cvref_t<T, Snd>>);
				return operation_t<Rcv>{std::forward<T>(s).sch_base::value(), std::forward<T>(s).snd_base::value(), std::move(rcv)};
			}
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

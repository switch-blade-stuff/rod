/*
 * Created by switchblade on 2023-05-06.
 */

#pragma once

#include "../queries/scheduler.hpp"
#include "../concepts.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	namespace _on
	{
		template<typename Sch, typename Snd, typename Rcv>
		struct operation { struct type; };
		template<typename Sch, typename Snd, typename Rcv>
		struct receiver_ref { struct type; };
		template<typename Sch, typename Snd, typename Rcv>
		struct receiver { struct type; };

		template<typename Sch, typename Snd>
		struct sender { struct type; };
		template<typename Sch, typename Env>
		struct env { struct type; };

		template<typename Sch, typename Env>
		struct env<Sch, Env>::type
		{
			template<decays_to<type> E>
			friend constexpr Sch tag_invoke(get_scheduler_t, E &&e) noexcept { return get_scheduler(e._env); }
			template<is_forwarding_query Q, decays_to<type> E, typename... Args> requires detail::callable<Q, Env, Args...>
			friend constexpr decltype(auto) tag_invoke(Q, E &&e, Args &&...args) noexcept(detail::nothrow_callable<Q, Env, Args...>)
			{
				return Q{}(std::forward<E>(e)._env, std::forward<Args>(args)...);
			}

			[[ROD_NO_UNIQUE_ADDRESS]] Env _env;
		};

		template<typename Sch, typename Snd, typename Rcv>
		struct operation<Sch, Snd, Rcv>::type
		{
			using _receiver_t = typename receiver<Sch, Snd, Rcv>::type;
			using _receiver_ref_t = typename receiver_ref<Sch, Snd, Rcv>::type;
			using _state_t = std::variant<connect_result_t<schedule_result_t<Sch>, _receiver_t>, connect_result_t<Snd, _receiver_ref_t>>;

			type(type &&) = delete;
			type &operator=(type &&) = delete;

			template<typename Sch2, typename Snd2, typename Rcv2>
			constexpr type(Sch2 &&sch, Snd2 &&snd, Rcv2 &&rcv) noexcept(std::is_nothrow_constructible_v<Sch, Sch2> &&
			                                                            std::is_nothrow_constructible_v<Snd, Snd2> &&
			                                                            std::is_nothrow_constructible_v<Rcv, Rcv2>)
					: _sch(std::forward<Sch2>(sch)), _snd(std::forward<Snd2>(snd)), _rcv(std::forward<Rcv2>(rcv)),
					  _state{std::in_place_index<0>, detail::implicit_eval{[this] { return _do_connect(); }}} {}

			friend constexpr void tag_invoke(start_t, type &op) noexcept { start(std::get<0>(op._state)); }

			[[nodiscard]] constexpr auto _do_connect() { return connect(schedule(_sch), _receiver_t{this}); }

			[[ROD_NO_UNIQUE_ADDRESS]] Sch _sch;
			[[ROD_NO_UNIQUE_ADDRESS]] Snd _snd;
			[[ROD_NO_UNIQUE_ADDRESS]] Rcv _rcv;
			_state_t _state;
		};

		template<typename Sch, typename Snd, typename Rcv>
		struct receiver_ref<Sch, Snd, Rcv>::type
		{
			using is_receiver = std::true_type;

			using _operation_t = typename operation<Sch, Snd, Rcv>::type;
			using _env_t = typename env<Sch, env_of_t<Rcv>>::type;

			friend constexpr _env_t tag_invoke(get_env_t, const type &r) noexcept(detail::nothrow_callable<get_env_t, const Rcv &>) { return _env_t{get_env(r._op->_rcv)}; }
			template<detail::completion_channel C, typename... Args> requires detail::callable<C, Rcv, Args...>
			friend constexpr void tag_invoke(C, type &&r, Args &&...args) noexcept { C{}(std::move(r._op->_rcv), std::forward<Args>(args)...); }

			_operation_t *_op = {};
		};

		template<typename Sch, typename Snd, typename Rcv>
		struct receiver<Sch, Snd, Rcv>::type
		{
			using is_receiver = std::true_type;

			using _receiver_ref_t = typename receiver_ref<Sch, Snd, Rcv>::type;
			using _operation_t = typename operation<Sch, Snd, Rcv>::type;

			friend constexpr env_of_t<Rcv> tag_invoke(get_env_t, const type &r) noexcept(detail::nothrow_callable<get_env_t, const Rcv &>) { return get_env(r._op->_rcv); }

			template<detail::completion_channel C, typename... Args> requires(!std::same_as<C, set_value_t> && detail::callable<C, Rcv, Args...>)
			friend constexpr void tag_invoke(C, type &&r, Args &&...args) noexcept { C{}(std::move(r._op->_rcv), std::forward<Args>(args)...); }
			template<typename... Args>
			friend constexpr void tag_invoke(set_value_t, type &&r) noexcept
			{
				constexpr bool nothrow_start = detail::nothrow_callable<start_t, std::add_lvalue_reference<connect_result_t<Snd, _receiver_ref_t>>>;
				constexpr bool nothrow_connect = detail::nothrow_callable<connect_t, Snd, _receiver_ref_t>;

				auto *op = r._op;
				const auto do_start = [&]()
				{
					/* Use a conversion wrapper to allow emplacement of non-movable types. */
					const auto conv = [op]() { return connect(std::move(op->_snd), _receiver_ref_t{op}); };
					start(op->_state.template emplace<1>(detail::implicit_eval{conv}));
				};

				if constexpr (!(nothrow_connect && nothrow_start))
					try { do_start(); } catch (...) { set_error(std::move(op->_rcv), std::current_exception()); }
				else
					do_start();
			}

			_operation_t *_op = {};
		};

		template<typename Sch, typename Snd>
		struct sender<Sch, Snd>::type
		{
			using is_sender = std::true_type;

			template<typename Rcv>
			using _receiver_ref_t = typename receiver_ref<Sch, Snd, Rcv>::type;
			template<typename Rcv>
			using _operation_t = typename operation<Sch, Snd, Rcv>::type;
			template<typename Rcv>
			using _receiver_t = typename receiver<Sch, Snd, Rcv>::type;
			template<typename Env>
			using _env_t = typename env<Sch, Env>::type;

			template<typename...>
			using _empty_signs_t = completion_signatures<>;
			template<typename T, typename Env>
			using _signs_t = make_completion_signatures<schedule_result_t<Sch>, Env, make_completion_signatures<copy_cvref_t<T, Snd>, _env_t<Env>,
								completion_signatures<set_error_t(std::exception_ptr)>>, _empty_signs_t>;

			friend constexpr env_of_t<Snd> tag_invoke(get_env_t, const type &s) noexcept(detail::nothrow_callable<get_env_t, const Snd &>) { return get_env(s._snd); }
			template<decays_to<type> T, typename Env>
			friend constexpr _signs_t<T, Env> tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }

			template<decays_to<type> T, rod::receiver Rcv>
			friend constexpr _operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(std::is_nothrow_constructible_v<_operation_t<Rcv>, copy_cvref_t<T, Sch>, copy_cvref_t<T, Snd>, Rcv>)
			{
				static_assert(std::constructible_from<Sch, copy_cvref_t<T, Sch>>);
				static_assert(std::constructible_from<Snd, copy_cvref_t<T, Snd>>);
				static_assert(sender_to<schedule_result_t<Sch>, _receiver_t<Rcv>>);
				static_assert(sender_to<Snd, _receiver_ref_t<Rcv>>);
				return _operation_t<Rcv>{std::forward<T>(s)._sch, std::forward<T>(s)._snd, std::move(rcv)};
			}

			[[ROD_NO_UNIQUE_ADDRESS]] Sch _sch;
			[[ROD_NO_UNIQUE_ADDRESS]] Snd _snd;
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
ROD_TOPLEVEL_NAMESPACE_CLOSE

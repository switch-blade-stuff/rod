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
			template<is_forwarding_query Tag, detail::decays_to<type> E, typename... Args>
			friend constexpr decltype(auto) tag_invoke(Tag t, E &&e, Args &&...args) noexcept { return t(e._env, std::forward<Args>(args)...); }
			template<detail::decays_to<type> E>
			friend constexpr Sch tag_invoke(get_scheduler_t, E &&e) noexcept { return get_scheduler(e._env); }

			[[ROD_NO_UNIQUE_ADDRESS]] Env _env;
		};

		template<typename Sch, typename Snd, typename Rcv>
		struct operation<Sch, Snd, Rcv>::type
		{
			using _receiver_t = typename receiver<Sch, Snd, Rcv>::type;
			using _receiver_ref_t = typename receiver_ref<Sch, Snd, Rcv>::type;
			using _state_t = std::variant<connect_result_t<schedule_result_t<Sch>, _receiver_t>, connect_result_t<Snd, _receiver_ref_t>>;

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
			[[ROD_NO_UNIQUE_ADDRESS]] _state_t _state;
		};

		template<typename Sch, typename Snd, typename Rcv>
		struct receiver_ref<Sch, Snd, Rcv>::type
		{
			using is_receiver = std::true_type;

			using _operation_t = typename operation<Sch, Snd, Rcv>::type;
			using _env_t = typename env<Sch, env_of_t<Rcv>>::type;

			template<detail::decays_to<type> T> requires detail::callable<get_env_t, copy_cvref_t<T, Rcv>>
			friend constexpr _env_t tag_invoke(get_env_t, T &&r) noexcept(detail::nothrow_callable<get_env_t, copy_cvref_t<T, Rcv>>)
			{
				return {get_env(static_cast<copy_cvref_t<T, Rcv>>(r._op->_rcv))};
			}

			template<detail::decays_to<type> T, typename... Args> requires detail::callable<set_value_t, copy_cvref_t<T, Rcv>, Args...>
			friend constexpr void tag_invoke(set_value_t, T &&r, Args &&...args) noexcept(detail::nothrow_callable<set_value_t, copy_cvref_t<T, Rcv>, Args...>)
			{
				set_value(static_cast<copy_cvref_t<T, Rcv>>(r._op->_rcv), std::forward<Args>(args)...);
			}
			template<detail::decays_to<type> T, typename Err> requires detail::callable<set_error_t, copy_cvref_t<T, Rcv>, Err>
			friend constexpr void tag_invoke(set_error_t, T &&r, Err &&err) noexcept(detail::nothrow_callable<set_error_t, copy_cvref_t<T, Rcv>, Err>)
			{
				set_error(static_cast<copy_cvref_t<T, Rcv>>(r._op->_rcv), std::forward<Err>(err));
			}
			template<detail::decays_to<type> T> requires detail::callable<set_stopped_t, copy_cvref_t<T, Rcv>>
			friend constexpr void tag_invoke(set_stopped_t, T &&r) noexcept(detail::nothrow_callable<set_stopped_t, copy_cvref_t<T, Rcv>>)
			{
				set_stopped(static_cast<copy_cvref_t<T, Rcv>>(r._op->_rcv));
			}

			_operation_t *_op = {};
		};

		template<typename Sch, typename Snd, typename Rcv>
		struct receiver<Sch, Snd, Rcv>::type
		{
			using is_receiver = std::true_type;

			using _receiver_ref_t = typename receiver_ref<Sch, Snd, Rcv>::type;
			using _operation_t = typename operation<Sch, Snd, Rcv>::type;

			template<detail::decays_to<type> T> requires detail::callable<get_env_t, copy_cvref_t<T, Rcv>>
			friend constexpr decltype(auto) tag_invoke(get_env_t, T &&r) noexcept(detail::nothrow_callable<get_env_t, copy_cvref_t<T, Rcv>>)
			{
				return get_env(static_cast<copy_cvref_t<T, Rcv>>(r._op->_rcv));
			}

			template<detail::decays_to<type> T, typename Err> requires detail::callable<set_error_t, copy_cvref_t<T, Rcv>, Err>
			friend constexpr void tag_invoke(set_error_t, T &&r, Err &&err) noexcept(detail::nothrow_callable<set_error_t, copy_cvref_t<T, Rcv>, Err>)
			{
				set_error(static_cast<copy_cvref_t<T, Rcv>>(r._op->_rcv), std::forward<Err>(err));
			}
			template<detail::decays_to<type> T> requires detail::callable<set_stopped_t, copy_cvref_t<T, Rcv>>
			friend constexpr void tag_invoke(set_stopped_t, T &&r) noexcept(detail::nothrow_callable<set_stopped_t, copy_cvref_t<T, Rcv>>)
			{
				set_stopped(static_cast<copy_cvref_t<T, Rcv>>(r._op->_rcv));
			}

			template<detail::decays_to<type> T>
			friend constexpr void tag_invoke(set_value_t, T &&r) noexcept
			{
				auto *op = r._op;
				try
				{
					/* Use a conversion wrapper to allow emplacement of non-movable types. */
					const auto conv = [op]() { return connect(std::move(op->_snd), _receiver_ref_t{op}); };
					start(op->_state.template emplace<1>(detail::implicit_eval{conv}));
				}
				catch (...) { set_error(std::move(op->_rcv), std::current_exception()); }
			}

			_operation_t *_op = {};
		};

		template<typename Sch, typename Snd>
		struct sender<Sch, Snd>::type
		{
			using is_sender = std::true_type;

			template<typename Rcv>
			using _receiver_ref_t = typename receiver_ref<Sch, Snd, std::decay_t<Rcv>>::type;
			template<typename Rcv>
			using _operation_t = typename operation<Sch, Snd, std::decay_t<Rcv>>::type;
			template<typename Rcv>
			using _receiver_t = typename receiver<Sch, Snd, std::decay_t<Rcv>>::type;
			template<typename Env>
			using _env_t = typename env<Sch, Env>::type;

			template<typename...>
			using _empty_signs = completion_signatures<>;
			template<typename T, typename Env>
			using _signs = make_completion_signatures<copy_cvref_t<T, Snd>, _env_t<Env>,
			                                          make_completion_signatures<schedule_result_t<Sch>, Env,
			                                                                     completion_signatures<set_error_t(std::exception_ptr)>,
			                                                                     _empty_signs>>;

			template<detail::decays_to<type> T, typename Env>
			friend constexpr _signs<T, Env> tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }

			template<detail::decays_to<type> T, rod::receiver Rcv>
			requires std::constructible_from<Sch, copy_cvref_t<T, Sch>> && std::constructible_from<Snd, copy_cvref_t<T, Snd>> &&
			         sender_to<schedule_result_t<Sch>, _receiver_t<Rcv>> && sender_to<Snd, _receiver_ref_t<Rcv>>
			friend constexpr _operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<_operation_t<Rcv>, copy_cvref_t<T, Sch>, copy_cvref_t<T, Snd>, Rcv>)
			{
				return _operation_t<Rcv>{std::forward<T>(s)._sch, std::forward<T>(s)._snd, std::forward<Rcv>(rcv)};
			}

			friend constexpr decltype(auto) tag_invoke(get_env_t, const type &s) noexcept(detail::nothrow_callable<get_env_t, const Snd &>) { return get_env(s._snd); }

			[[ROD_NO_UNIQUE_ADDRESS]] Sch _sch;
			[[ROD_NO_UNIQUE_ADDRESS]] Snd _snd;
		};

		class on_t
		{
			template<typename Sch, typename Snd>
			using sender_t = typename sender<std::decay_t<Sch>, std::decay_t<Snd>>::type;

		public:
			template<rod::scheduler Sch, rod::sender Snd> requires tag_invocable<on_t, Sch, Snd>
			[[nodiscard]] constexpr rod::sender decltype(auto) operator()(Sch &&sch, Snd &&snd) const noexcept(nothrow_tag_invocable<on_t, Sch, Snd>)
			{
				return tag_invoke(*this, std::forward<Sch>(sch), std::forward<Snd>(snd));
			}
			template<rod::scheduler Sch, rod::sender Snd>
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
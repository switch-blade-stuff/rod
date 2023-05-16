/*
 * Created by switchblade on 2023-05-06.
 */

#pragma once

#include <tuple>

#include "../concepts.hpp"

namespace rod
{
	namespace _schedule_from
	{
		/* Build a variant of form
		 * std::variant<std::monostate,
		 *      std::tuple<std::set_stopped_t>,
		 *      std::tuple<std::set_value_t, Values0...>,
		 *      ...
		 *      std::tuple<std::set_value_t, ValuesN...>,
		 *      std::tuple<std::set_error_t, Error0>,
		 *      ...
		 *      std::tuple<std::set_error_t, ErrorN>,
		 * > */
		template<typename S, typename E, typename Tag, typename... Ts>
		struct tagged_signs
		{
			template<typename... Us>
			using tagged_tuple = detail::decayed_tuple<Tag, Us...>;
			using gather_tag = detail::gather_signatures_t<Tag, S, E, tagged_tuple, type_list_t>;

			using type = detail::concat_tuples_t<gather_tag, typename tagged_signs<S, E, Ts...>::type>;
		};
		template<typename S, typename E, typename... Ts>
		using tagged_signs_t = typename tagged_signs<S, E, Ts...>::type;
		template<typename... Ts>
		using monostate_variant = std::variant<std::monostate, Ts...>;
		template <typename S, typename E>
		using variant_for_t = detail::apply_tuple_list_t<monostate_variant, tagged_signs_t<S, E, set_value_t, set_error_t, set_stopped_t>>;

		template<typename, typename, typename>
		struct receiver1 { struct type; };
		template<typename, typename, typename>
		struct receiver2 { struct type; };
		template<typename, typename, typename>
		struct operation { struct type; };
		template<typename, typename>
		struct sender { struct type; };
		template<typename>
		struct env { struct type; };

		template<typename Sch>
		struct env<Sch>::type
		{
			friend constexpr Sch tag_invoke(get_completion_scheduler_t<set_error_t>, const env &) = delete;
			template<typename T> requires(std::same_as<T, set_value_t> || std::same_as<T, set_stopped_t>)
			friend constexpr Sch tag_invoke(get_completion_scheduler_t<T>, const env &e) noexcept(std::is_nothrow_copy_constructible_v<Sch>) { return e._sch; }

			template<detail::decays_to<type> T, is_forwarding_query Q, typename... Args> requires detail::callable<Q, env_of_t<Sch>, Args...>
			friend constexpr decltype(auto) tag_invoke(Q, T &&t, Args &&...args) noexcept(detail::nothrow_callable<Q, env_of_t<Sch>, Args...>)
			{
				return Q{}(get_env(t._sch), std::forward<Args>(args)...);
			}

			[[ROD_NO_UNIQUE_ADDRESS]] Sch _sch;
		};

		template<typename Sch, typename Snd, typename Rcv>
		struct receiver1<Sch, Snd, Rcv>::type
		{
			using is_receiver = std::true_type;
			using _operation_t = typename operation<Sch, Snd, Rcv>::type;

			friend constexpr env_of_t<Rcv> tag_invoke(get_env_t, const type &r) noexcept(detail::nothrow_callable<get_env_t, const Rcv &>) { return get_env(r._op->_rcv); }
			template<detail::decays_to<type> T, detail::completion_channel C, typename... Args> requires detail::callable<C, Rcv, std::decay_t<Args>...>
			friend constexpr void tag_invoke(C, T &&r, Args &&...args) noexcept { r.template _complete<C>(std::forward<Args>(args)...); }

			template<typename C, typename... Args>
			constexpr void _complete(Args &&...args) noexcept
			{
				const auto complete_channel = [&]() noexcept((std::is_nothrow_constructible_v<std::decay_t<Args>, Args> && ...))
				{
					_op->_data.template emplace<detail::decayed_tuple<C, Args...>>(C{}, std::forward<Args>(args)...);
					start(_op->_state2);
				};
				detail::rcv_try_invoke(std::move(_op->_rcv), complete_channel, C{}, *this, std::forward<Args>(args)...);
			}

			_operation_t *_op;
		};
		template<typename Sch, typename Snd, typename Rcv>
		struct receiver2<Sch, Snd, Rcv>::type
		{
			using is_receiver = std::true_type;
			using _operation_t = typename operation<Sch, Snd, Rcv>::type;

			friend constexpr env_of_t<Rcv> tag_invoke(get_env_t, const type &r) noexcept(detail::nothrow_callable<get_env_t, const Rcv &>) { return get_env(r._op->_rcv); }

			template<detail::decays_to<type> T>
			friend constexpr void tag_invoke(set_value_t, T &&r) noexcept { r._op->_complete(); }
			template<detail::decays_to<type> T, typename C, typename... Args> requires(std::same_as<C, set_error_t> || std::same_as<C, set_stopped_t>) && detail::callable<C, Rcv, Args...>
			friend constexpr void tag_invoke(C, T &&r, Args &&...args) noexcept { C{}(std::move(r._op->_rcv), std::forward<Args>(args)...); }

			_operation_t *_op;
		};

		template<typename Sch, typename Snd, typename Rcv>
		struct operation<Sch, Snd, Rcv>::type
		{
			using _rcv1_t = typename receiver1<Sch, Snd, Rcv>::type;
			using _rcv2_t = typename receiver2<Sch, Snd, Rcv>::type;

			using _state1_t = connect_result_t<Snd, _rcv1_t>;
			using _state2_t = connect_result_t<schedule_result_t<Sch>, _rcv2_t>;
			using _data_t = variant_for_t<Snd, env_of_t<Rcv>>;

			constexpr type(Sch sch, Snd &&snd, Rcv &&rcv) : _sch(std::move(sch)), _rcv(std::move(rcv)), _state1(connect(std::move(snd), _rcv1_t{this})), _state2(connect(schedule(this->_sch), _rcv2_t{this})) {}

			friend constexpr void tag_invoke(start_t, type &op) noexcept { start(op._state1); }

			constexpr void _complete() noexcept
			{
				std::visit([&]<typename T>(T &t)
				{
					if constexpr (!std::same_as<T, std::monostate>)
					    std::apply([&]<typename... Args>(auto tag, Args &&...args) { tag(std::move(_rcv), std::move(args)...); }, t);
					else
						std::terminate();
				}, _data);
			}

			[[ROD_NO_UNIQUE_ADDRESS]] Sch _sch;
			[[ROD_NO_UNIQUE_ADDRESS]] Rcv _rcv;
			_state1_t _state1;
			_state2_t _state2;
			_data_t _data;
		};

		template<typename Sch, typename Snd>
		struct sender<Sch, Snd>::type
		{
			using is_sender = std::true_type;

			template<typename T, typename R>
			using _receiver_t = typename receiver1<Sch, copy_cvref_t<T, Snd>, R>::type;
			template<typename T, typename R>
			using _operation_t = typename operation<Sch, copy_cvref_t<T, Snd>, R>::type;
			using _env_t = typename env<Sch>::type;

			template<typename... Ts>
			using _value_signs = completion_signatures<set_value_t(std::decay_t<Ts> &&...)>;
			template<typename T>
			using _error_signs = completion_signatures<set_error_t(std::decay_t<T> &&)>;
			template<typename...>
			using _empty_signs = completion_signatures<>;

			template<typename T, typename E>
			using _throwing_signs = std::conditional_t<
					value_types_of_t<copy_cvref_t<T, Snd>, E, detail::all_nothrow_decay_copyable, std::conjunction>::value &&
					error_types_of_t<copy_cvref_t<T, Snd>, E, detail::all_nothrow_decay_copyable>::value,
					completion_signatures<>, completion_signatures<set_error_t(std::exception_ptr)>>;

			template<typename T, typename E>
			using _signs = make_completion_signatures<copy_cvref_t<T, Snd>, E,
							make_completion_signatures<schedule_result_t<Sch>, E, _throwing_signs<T, E>, _empty_signs>,
							                          _value_signs, _error_signs>;

			friend constexpr const _env_t &tag_invoke(get_env_t, const type &s) noexcept { return s._env; }
			template<typename T, typename E> requires detail::decays_to<T, type>
			friend constexpr _signs<T, E> tag_invoke(get_completion_signatures_t, T &&, E &&) noexcept { return {}; }

			template<detail::decays_to<type> T, rod::receiver Rcv> requires sender_to<copy_cvref_t<T, Snd>, _receiver_t<T, Rcv>>
			friend constexpr _operation_t<T, Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv)
			{
				return {s._env._sch, std::forward<T>(s)._snd, std::forward<Rcv>(rcv)};
			}

			[[ROD_NO_UNIQUE_ADDRESS]] _env_t _env;
			[[ROD_NO_UNIQUE_ADDRESS]] Snd _snd;
		};

		class schedule_from_t
		{
			template<typename Sch, typename Snd>
			using sender_t = typename sender<std::decay_t<Sch>, std::decay_t<Snd>>::type;

		public:
			template<scheduler Sch, rod::sender Snd> requires tag_invocable<schedule_from_t, Sch, Snd>
			[[nodiscard]] constexpr rod::sender decltype(auto) operator()(Sch &&sch, Snd &&snd) const noexcept(nothrow_tag_invocable<schedule_from_t, Sch, Snd>)
			{
				return tag_invoke(*this, std::forward<Sch>(sch), std::forward<Snd>(snd));
			}
			template<scheduler Sch, rod::sender Snd> requires(!tag_invocable<schedule_from_t, Sch, Snd>)
			[[nodiscard]] constexpr sender_t<Sch, Snd> operator()(Sch &&sch, Snd &&snd) const noexcept(std::is_nothrow_constructible_v<sender_t<Sch, Snd>, Sch, Snd>)
			{
				return sender_t<Sch, Snd>{std::forward<Sch>(sch), std::forward<Snd>(snd)};
			}
		};
	}

	using _schedule_from::schedule_from_t;

	/** Customization point object used to schedule tasks bound by completion of a sender onto a scheduler's execution context.
	 * @warning This CPO is meant for customization of schedulers, use `transfer` instead. */
	inline constexpr auto schedule_from = schedule_from_t{};
}
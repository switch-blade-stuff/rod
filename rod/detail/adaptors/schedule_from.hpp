/*
 * Created by switchblade on 2023-05-06.
 */

#pragma once

#include <tuple>

#include "../queries/scheduler.hpp"

namespace rod
{
	namespace _schedule_from
	{
		class schedule_from_t;

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
			using tagged_tuple = _detail::decayed_tuple<Tag, Us...>;
			using gather_tag = _detail::gather_signatures_t<Tag, S, E, tagged_tuple, type_list_t>;

			using type = _detail::concat_tuples_t<gather_tag, typename tagged_signs<S, E, Ts...>::type>;
		};
		template<typename S, typename E, typename Tag>
		struct tagged_signs<S, E, Tag>
		{
			template<typename... Us>
			using tagged_tuple = _detail::decayed_tuple<Tag, Us...>;
			using type = _detail::gather_signatures_t<Tag, S, E, tagged_tuple, type_list_t>;
		};
		template<typename S, typename E, typename... Ts>
		using tagged_signs_t = typename tagged_signs<S, E, Ts...>::type;
		template<typename... Ts>
		using monostate_variant = std::variant<std::monostate, Ts...>;
		template <typename S, typename E>
		using variant_for_t = _detail::apply_tuple_list_t<monostate_variant, tagged_signs_t<S, E, set_value_t, set_error_t, set_stopped_t>>;

		template<typename, typename, typename>
		struct receiver1 { class type; };
		template<typename, typename, typename>
		struct receiver2 { class type; };
		template<typename, typename, typename>
		struct operation { class type; };
		template<typename, typename>
		struct sender { class type; };
		template<typename, typename>
		struct env { class type; };

		template<typename Sch, typename Snd>
		class env<Sch, Snd>::type : empty_base<Sch>, empty_base<Snd>
		{
			template<typename, typename, typename>
			friend struct operation;

			using sch_base = empty_base<Sch>;
			using snd_base = empty_base<Snd>;

		public:
			template<typename Sch2, typename Snd2>
			constexpr explicit type(Sch2 &&sch, Snd2 &&snd) noexcept(std::is_nothrow_constructible_v<Sch, Sch2> && std::is_nothrow_constructible_v<Snd, Snd2>) : sch_base(std::forward<Sch2>(sch)), snd_base(std::forward<Snd2>(snd)) {}

			template<decays_to_same<type> E>
			friend constexpr Sch tag_invoke(get_completion_scheduler_t<set_error_t>, E &&) = delete;
			template<decays_to_same<type> E, one_of<set_value_t, set_stopped_t> T>
			friend constexpr Sch tag_invoke(get_completion_scheduler_t<T>, E &&e) noexcept(std::is_nothrow_copy_constructible_v<Sch>) { return e.sch_base::value(); }

			template<is_forwarding_query Q, decays_to_same<type> E, typename... Args> requires _detail::callable<Q, env_of_t<Snd>, Args...>
			friend constexpr decltype(auto) tag_invoke(Q, E &&e, Args &&...args) noexcept(_detail::nothrow_callable<Q, env_of_t<Snd>, Args...>)
			{
				return Q{}(get_env(e.snd_base::value()), std::forward<Args>(args)...);
			}
		};

		template<typename Sch, typename Snd, typename Rcv>
		class receiver1<Sch, Snd, Rcv>::type
		{
			using operation_t = typename operation<Sch, Snd, Rcv>::type;

		public:
			using is_receiver = std::true_type;

		public:
			constexpr explicit type(operation_t *op) noexcept : _op(op) {}

			friend env_of_t<Rcv> tag_invoke(get_env_t, const type &r) noexcept(_detail::nothrow_callable<get_env_t, const Rcv &>) { return r.get_env(); }
			template<_detail::completion_channel C, typename... Args> requires _detail::callable<C, Rcv, std::decay_t<Args>...>
			friend void tag_invoke(C, type &&r, Args &&...args) noexcept { r.complete(C{}, std::forward<Args>(args)...); }

		private:
			inline env_of_t<Rcv> get_env() const;
			template<typename C, typename... Args>
			inline void complete(C, Args &&...args) noexcept;

			operation_t *_op;
		};
		template<typename Sch, typename Snd, typename Rcv>
		class receiver2<Sch, Snd, Rcv>::type
		{
			using operation_t = typename operation<Sch, Snd, Rcv>::type;

		public:
			using is_receiver = std::true_type;

		public:
			constexpr explicit type(operation_t *op) noexcept : _op(op) {}

			friend env_of_t<Rcv> tag_invoke(get_env_t, const type &r) noexcept(_detail::nothrow_callable<get_env_t, const Rcv &>) { return r.get_env(); }

			template<_detail::completion_channel C> requires decays_to_same<C, set_value_t>
			friend void tag_invoke(C, type &&r) noexcept { r.complete_value(); }
			template<_detail::completion_channel C, typename... Args> requires(!decays_to_same<C, set_value_t> && _detail::callable<C, Rcv, Args...>)
			friend void tag_invoke(C, type &&r, Args &&...args) noexcept { r.complete_forward(C{}, std::forward<Args>(args)...); }

		private:
			inline env_of_t<Rcv> get_env() const;
			inline void complete_value() noexcept;
			template<typename C, typename... Args>
			inline void complete_forward(C, Args &&...args) noexcept;

			operation_t *_op;
		};

		template<typename Sch, typename Snd, typename Rcv>
		class operation<Sch, Snd, Rcv>::type : empty_base<Sch>, empty_base<Rcv>
		{
			friend class receiver1<Sch, Snd, Rcv>::type;
			friend class receiver2<Sch, Snd, Rcv>::type;

			using sch_base = empty_base<Sch>;
			using rcv_base = empty_base<Rcv>;

			using rcv1_t = typename receiver1<Sch, Snd, Rcv>::type;
			using rcv2_t = typename receiver2<Sch, Snd, Rcv>::type;
			using env_t = typename env<Sch, Snd>::type;

			using state1_t = connect_result_t<Snd, rcv1_t>;
			using state2_t = connect_result_t<schedule_result_t<Sch>, rcv2_t>;
			using data_t = variant_for_t<Snd, env_of_t<Rcv>>;

		public:
			type(type &&) = delete;
			type &operator=(type &&) = delete;

			template<decays_to_same<env_t> Env>
			constexpr explicit type(Env &&env, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<Sch, copy_cvref_t<Env, Sch>> && noexcept(connect(std::forward<Env>(env).snd_base::value(), rcv1_t(this))) && noexcept(connect(schedule(sch_base::value()), rcv2_t(this))))
					: sch_base(std::forward<Env>(env).sch_base::value()), rcv_base(std::forward<Rcv>(rcv)), _state1(connect(std::forward<Env>(env).snd_base::value(), rcv1_t(this))), _state2(connect(schedule(sch_base::value()), rcv2_t(this))) {}

			friend constexpr void tag_invoke(start_t, type &op) noexcept { start(op._state1); }

		private:
			constexpr void complete() noexcept
			{
				std::visit([&]<typename T>(T &t)
				{
					if constexpr (!std::same_as<T, std::monostate>)
					    std::apply([&](auto tag, auto &&...args) { tag(std::move(rcv_base::value()), std::move(args)...); }, t);
					else
						std::terminate();
				}, _data);
			}

			state1_t _state1;
			state2_t _state2;
			data_t _data;
		};

		template<typename Sch, typename Snd, typename Rcv>
		env_of_t<Rcv> receiver1<Sch, Snd, Rcv>::type::get_env() const { return rod::get_env(_op->rcv_base::value()); }
		template<typename Sch, typename Snd, typename Rcv>
		env_of_t<Rcv> receiver2<Sch, Snd, Rcv>::type::get_env() const { return rod::get_env(_op->rcv_base::value()); }

		template<typename Sch, typename Snd, typename Rcv>
		template<typename C, typename... Args>
		void receiver1<Sch, Snd, Rcv>::type::complete(C, Args &&... args) noexcept
		{
			const auto complete_selected = [&]() noexcept((std::is_nothrow_constructible_v<std::decay_t<Args>, Args> && ...))
			{
				_op->_data.template emplace<_detail::decayed_tuple<C, Args...>>(C{}, std::forward<Args>(args)...);
				start(_op->_state2);
			};
			_detail::rcv_try_invoke(std::move(_op->rcv_base::value()), complete_selected);
		}
		template<typename Sch, typename Snd, typename Rcv>
		void receiver2<Sch, Snd, Rcv>::type::complete_value() noexcept { _op->complete(); }
		template<typename Sch, typename Snd, typename Rcv>
		template<typename C, typename... Args>
		void receiver2<Sch, Snd, Rcv>::type::complete_forward(C, Args &&... args) noexcept { C{}(std::move(_op->rcv_base::value()), std::forward<Args>(args)...); }

		template<typename Sch, typename Snd>
		class sender<Sch, Snd>::type : empty_base<typename env<Sch, Snd>::type>
		{
		public:
			using is_sender = std::true_type;

		private:
			template<typename T, typename Rcv>
			using operation_t = typename operation<Sch, copy_cvref_t<T, Snd>, Rcv>::type;
			template<typename T, typename Rcv>
			using receiver_t = typename receiver1<Sch, copy_cvref_t<T, Snd>, Rcv>::type;
			using env_t = typename env<Sch, Snd>::type;
			using env_base = empty_base<env_t>;

			template<typename... Ts>
			using value_signs_t = completion_signatures<set_value_t(std::decay_t<Ts> &&...)>;
			template<typename T>
			using error_signs_t = completion_signatures<set_error_t(std::decay_t<T> &&)>;
			template<typename...>
			using empty_signs_t = completion_signatures<>;

			template<typename T, typename E>
			using throwing_signs_t = std::conditional_t<
					value_types_of_t<copy_cvref_t<T, Snd>, E, _detail::all_nothrow_decay_copyable, std::conjunction>::value &&
					error_types_of_t<copy_cvref_t<T, Snd>, E, _detail::all_nothrow_decay_copyable>::value,
					completion_signatures<>, completion_signatures<set_error_t(std::exception_ptr)>>;

			template<typename T, typename E>
			using signs_t = make_completion_signatures<copy_cvref_t<T, Snd>, E, make_completion_signatures<schedule_result_t<Sch>, E, throwing_signs_t<T, E>, empty_signs_t>, value_signs_t, error_signs_t>;

		public:
			template<typename Sch2, typename Snd2>
			constexpr explicit type(Sch2 &&sch, Snd2 &&snd) noexcept(std::is_nothrow_constructible_v<env_t, Sch2, Snd2>) : env_base(std::forward<Sch2>(sch), std::forward<Snd2>(snd)) {}

			friend constexpr const env_t &tag_invoke(get_env_t, const type &s) noexcept { return s.env_base::value(); }
			template<typename T, typename Env> requires decays_to_same<T, type>
			friend constexpr signs_t<T, Env> tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }

			template<decays_to_same<type> T, rod::receiver Rcv> requires sender_to<copy_cvref_t<T, Snd>, receiver_t<T, Rcv>>
			friend constexpr operation_t<T, Rcv> tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(std::is_nothrow_constructible_v<operation_t<T, Rcv>, copy_cvref_t<T, Sch>, copy_cvref_t<T, Snd>, Rcv>)
			{
				return operation_t<T, Rcv>{std::forward<T>(s).env_base::value(), std::move(rcv)};
			}
		};

		class schedule_from_t
		{
			template<typename Sch, typename Snd>
			using sender_t = typename sender<std::decay_t<Sch>, std::decay_t<Snd>>::type;

		public:
			template<scheduler Sch, rod::sender Snd> requires tag_invocable<schedule_from_t, Sch, Snd>
			[[nodiscard]] constexpr rod::sender auto operator()(Sch &&sch, Snd &&snd) const noexcept(nothrow_tag_invocable<schedule_from_t, Sch, Snd>)
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

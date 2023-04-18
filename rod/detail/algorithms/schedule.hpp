/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include "../queries/signatures.hpp"
#include "../concepts.hpp"

namespace rod
{
	inline namespace _schedule
	{
		struct schedule_t
		{
			template<typename S> requires tag_invocable<schedule_t, S> && sender<tag_invoke_result_t<schedule_t, S>>
			[[nodiscard]] constexpr decltype(auto) operator()(S &&sch) const noexcept(nothrow_tag_invocable<schedule_t, S>)
			{
				return tag_invoke(*this, std::forward<S>(sch));
			}
		};
	}

	/** Customization point object used to obtain a sender from the passed scheduler.
	 * @param sch Scheduler to create a sender from.
	 * @return Sender created from \a sch. */
	inline constexpr auto schedule = schedule_t{};
	/** Alias for the sender type of scheduler `S` obtained via a call to `schedule(S)`. */
	template<typename S>
	using schedule_result_t = decltype(schedule(std::declval<S>()));

	/** Concept used to define a scheduler type \a S that can be used to schedule work on it's associated execution context. */
	template<typename S>
	concept scheduler = queryable<S> && requires(S &&s, decltype(get_env(schedule(std::forward<S>(s)))) env)
	{
		{ schedule(std::forward<S>(s)) } -> sender;
		tag_invocable<get_completion_scheduler_t<set_value_t>, decltype(env)>;
		std::same_as<tag_invoke_result_t<get_completion_scheduler_t<set_value_t>, decltype(env)>, std::remove_cvref_t<S>>;
	} && std::equality_comparable<std::remove_cvref_t<S>> && std::copy_constructible<std::remove_cvref_t<S>>;

	inline namespace _schedule_from
	{
		class schedule_from_t
		{
			template<typename, typename>
			class sender;
			template<typename Sch, typename Snd, typename R>
			class operation;

			template<typename Sch>
			class env : detail::ebo_helper<Sch>
			{
				friend class schedule_from_t;
				template<typename, typename>
				friend class sender;

				template<typename T>
				constexpr env(T &&s) : detail::ebo_helper<Sch>(std::forward<T>(s)) {}

			public:
				template<typename T> requires (std::same_as<T, set_value_t> || std::same_as<T, set_error_t>)
				friend constexpr Sch tag_invoke(get_completion_scheduler_t<T>, const env &e) noexcept(std::is_nothrow_copy_constructible_v<Sch>) { return e.sch(); }

			private:
				[[nodiscard]] constexpr auto &sch() noexcept { return detail::ebo_helper<Sch>::value(); }
				[[nodiscard]] constexpr auto &sch() const noexcept { return detail::ebo_helper<Sch>::value(); }
			};

			template<typename S, typename E, typename Tag>
			struct tagged_signs
			{
				template<typename... Ts>
				using tagged_tuple = typename detail::bind_front<detail::decayed_tuple, Tag>::template type<Ts...>;
				using type = detail::gather_signatures_t<Tag, S, E, tagged_tuple, detail::variant_or_empty>;
			};
			template<typename S, typename E, typename Tag>
			using tagged_signs_t = typename tagged_signs<S, E, Tag>::type;

			template<typename... Ts>
			using null_variant = typename detail::bind_front<std::variant, std::monostate>::template type<Ts...>;
			template <typename S, typename E>
			using variant_for_t = detail::concat_on_t<null_variant, tagged_signs_t<S, E, set_value_t>, tagged_signs_t<S, E, set_error_t>, tagged_signs_t<S, E, set_stopped_t>>;

			template<typename Sch, typename Snd, typename R>
			class receiver1
			{
				template<typename, typename, typename>
				friend class operation;

				template<typename C, typename... Args>
				static constexpr void complete_channel(C, receiver1 &&r, Args &&...args) noexcept((std::is_nothrow_constructible_v<std::decay_t<Args>, Args> && ...))
				{
					r.m_op->data().template emplace<detail::decayed_tuple<C, Args...>>(C{}, std::forward<Args>(args)...);
					start(r.m_op->snd_state);
				}

				constexpr receiver1(operation<Sch, Snd, R> *op) noexcept : m_op(op) {}

			public:
				template<detail::completion_signature C, typename... Args> requires tag_invocable<C, R, std::decay_t<Args>...>
				friend constexpr void tag_invoke(C c, receiver1 &&r, Args &&...args) noexcept
				{
					detail::rcv_try_invoke(std::move(r.m_op->rcv()), complete_channel<C, Args...>, std::move(c), std::forward<receiver1>(r), std::forward<Args>(args)...);
				}

				friend constexpr env_of_t<R> tag_invoke(get_env_t, const receiver1 &r) { return get_env(r.m_op->rcv()); }

			private:
				operation<Sch, Snd, R> *m_op;
			};
			template<typename Sch, typename Snd, typename R>
			class receiver2
			{
				constexpr receiver2(operation<Sch, Snd, R> *op) noexcept : m_op(op) {}

			public:
				template<typename C, typename... Args> requires std::invocable<C, R, Args...> && (std::same_as<C, set_value_t> || std::same_as<C, set_error_t>)
				friend constexpr void tag_invoke(C, receiver2 &&r, Args &&...args) noexcept { C{}(std::move(r.m_op->rcv()), std::forward<Args>(args)...); }
				template<std::same_as<set_value_t> C>
				friend constexpr void tag_invoke(C, receiver2 &&r) noexcept { r.m_op->complete(); }

				friend constexpr env_of_t<R> tag_invoke(get_env_t, const receiver2 &r) { return get_env(r.m_op->rcv()); }

			private:
				operation<Sch, Snd, R> *m_op;
			};

			template<typename Sch, typename Snd, typename R>
			class operation : detail::ebo_helper<Sch>, detail::ebo_helper<R>, detail::ebo_helper<variant_for_t<Snd, env_of_t<R>>>
			{
				using data_base = detail::ebo_helper<variant_for_t<Snd, env_of_t<R>>>;
				using sch_base = detail::ebo_helper<Sch>;
				using rcv_base = detail::ebo_helper<R>;

				using rcv1_t = receiver1<Sch, Snd, R>;
				using rcv2_t = receiver2<Sch, Snd, R>;

				using sch_state_t = connect_result_t<schedule_result_t<Sch>, rcv1_t>;
				using snd_state_t = connect_result_t<Snd, rcv2_t>;

				constexpr operation(Sch sch, Snd &&snd, R &&rcv) : sch_base(std::move(sch)), rcv_base(std::move(rcv)), m_snd_state(connect(std::forward<Snd>(snd), rcv1_t{this})), m_sch_state(connect(schedule(sch), rcv2_t{this})) {}

			public:
				operation(operation &&) = delete;
				operation &operator=(operation &&) = delete;

				friend void tag_invoke(start_t, operation &op) noexcept { start(op.state1); }

			private:
				[[nodiscard]] constexpr auto &rcv() noexcept { return rcv_base::value(); }
				[[nodiscard]] constexpr auto &data() noexcept { return data_base::value(); }

				void complete() noexcept
				{
					std::visit([&]<typename T>(T &t)
					{
					    if constexpr (!std::same_as<T, std::monostate>)
					        std::apply([&]<typename... Args>(auto tag, Args &&...args) { tag(std::move(rcv()), std::move(args)...); }, t);
					    else
					        std::terminate();
					}, data());
				}

				sch_state_t m_sch_state;
				snd_state_t m_snd_state;
			};

			template<typename Sch, typename S>
			class sender : env<Sch>, detail::ebo_helper<S>
			{
				friend class schedule_from_t;

				using snd_base = detail::ebo_helper<S>;
				using env_base = env<Sch>;

				template<typename...>
				using empty_sign = completion_signatures<>;
				template<typename T, typename E>
				using signs = make_completion_signatures<detail::copy_cvref_t<T, S>, E, make_completion_signatures<schedule_result_t<Sch>, E, completion_signatures<set_error_t(std::exception_ptr)>, empty_sign>>;

				template<typename T, typename R>
				using receiver_t = receiver1<Sch, detail::copy_cvref_t<T, S>, R>;

				template<typename T0, typename T1>
				constexpr sender(T0 &&sch, T1 &&snd) : env_base(std::forward<T0>(sch)), snd_base(std::forward<T1>(snd)) {}

			public:
				template<typename T, typename E> requires std::same_as<std::remove_cvref_t<T>, sender>
				friend constexpr signs<T, E> tag_invoke(get_completion_signatures_t, T &&, E &&) noexcept { return {}; };
				friend constexpr const auto &tag_invoke(get_env_t, const sender &s) noexcept { return s.env(); }

				template<typename T, receiver R> requires std::same_as<std::remove_cvref_t<T>, sender> && sender_to<detail::copy_cvref_t<T, S>, receiver_t<T, R>>
				friend constexpr operation<Sch, detail::copy_cvref_t<T, S>, R> tag_invoke(connect_t, T &&s, R &&r)
				{
					return {s.env().sch(), std::move(s.snd()), std::forward<R>(r)};
				}

			private:
				[[nodiscard]] constexpr auto &snd() noexcept { return snd_base::value(); }
				[[nodiscard]] constexpr auto &env() noexcept { return env_base::value(); }
				[[nodiscard]] constexpr auto &env() const noexcept { return env_base::value(); }
			};

		public:
			template<scheduler Sch, rod::sender S> requires tag_invocable<schedule_from_t, Sch, S>
			[[nodiscard]] constexpr decltype(auto) operator()(Sch &&sch, S &&s) const noexcept(nothrow_tag_invocable<schedule_from_t, Sch, S>)
			{
				return tag_invoke(*this, std::forward<Sch>(sch), std::forward<S>(s));
			}
			template<scheduler Sch, rod::sender S> requires (!tag_invocable<schedule_from_t, Sch, S>)
			[[nodiscard]] constexpr sender<std::decay_t<Sch>, std::decay_t<S>> operator()(Sch &&sch, S &&s) const
			{
				return {std::forward<Sch>(sch), std::forward<S>(s)};
			}
		};
	}

	/** Customization point object used to schedule tasks bound by completion of a sender onto a scheduler's execution context.
	 * @warning This CPO is meant for customization of schedulers, use `transfer` instead. */
	inline constexpr auto schedule_from = schedule_from_t{};
}

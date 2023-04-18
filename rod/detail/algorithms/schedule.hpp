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

	namespace _schedule_from
	{
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

		struct schedule_from_t;

		template<typename, typename, typename>
		struct receiver1 { struct type; };
		template<typename, typename, typename>
		struct receiver2 { struct type; };
		template<typename, typename, typename>
		struct operation { class type; };
		template<typename, typename>
		struct sender { class type; };
		template<typename>
		struct env { struct type; };

		template<typename Sch>
		struct env<Sch>::type : detail::ebo_helper<Sch>
		{
			using detail::ebo_helper<Sch>::ebo_helper;

			template<typename T> requires (std::same_as<T, set_value_t> || std::same_as<T, set_stopped_t>)
			friend constexpr Sch tag_invoke(get_completion_scheduler_t<T>, const env &e) noexcept(std::is_nothrow_copy_constructible_v<Sch>) { return e.detail::ebo_helper<Sch>::value(); }
		};

		template<typename Sch, typename Snd, typename R>
		struct receiver1<Sch, Snd, R>::type
		{
			friend constexpr env_of_t<R> tag_invoke(get_env_t, const type &r) { return get_env(r.op->rcv()); }
			template<detail::completion_signature C, typename... Args> requires tag_invocable<C, R, std::decay_t<Args>...>
			friend constexpr void tag_invoke(C c, type &&r, Args &&...args) noexcept
			{
				const auto complete_channel = [&]() noexcept((std::is_nothrow_constructible_v<std::decay_t<Args>, Args> && ...))
				{
					r.op->data().template emplace<detail::decayed_tuple<C, Args...>>(C{}, std::forward<Args>(args)...);
					start(r.op->snd_state);
				};
				detail::rcv_try_invoke(std::move(r.op->rcv()), complete_channel, std::move(c), std::forward<type>(r), std::forward<Args>(args)...);
			}

			typename operation<Sch, Snd, R>::type *op;
		};
		template<typename Sch, typename Snd, typename R>
		struct receiver2<Sch, Snd, R>::type
		{
			friend constexpr env_of_t<R> tag_invoke(get_env_t, const type &r) { return get_env(r.op->rcv()); }
			template<typename C, typename... Args> requires std::invocable<C, R, Args...> && (std::same_as<C, set_value_t> || std::same_as<C, set_stopped_t>)
			friend constexpr void tag_invoke(C, type &&r, Args &&...args) noexcept { C{}(std::move(r.op->rcv()), std::forward<Args>(args)...); }
			template<std::same_as<set_value_t> C>
			friend constexpr void tag_invoke(C, type &&r) noexcept { r.op->complete(); }

			typename operation<Sch, Snd, R>::type *op;
		};

		template<typename Sch, typename Snd, typename R>
		class operation<Sch, Snd, R>::type : detail::ebo_helper<Sch>, detail::ebo_helper<R>, detail::ebo_helper<variant_for_t<Snd, env_of_t<R>>>
		{
			friend class sender<Sch, std::remove_cvref_t<Snd>>::type;

			using data_base = detail::ebo_helper<variant_for_t<Snd, env_of_t<R>>>;
			using sch_base = detail::ebo_helper<Sch>;
			using rcv_base = detail::ebo_helper<R>;

			using rcv1_t = typename receiver1<Sch, Snd, R>::type;
			using rcv2_t = typename receiver2<Sch, Snd, R>::type;

			using state1_t = connect_result_t<schedule_result_t<Sch>, rcv1_t>;
			using state2_t = connect_result_t<Snd, rcv2_t>;

			constexpr type(Sch sch, Snd &&snd, R &&rcv) : sch_base(std::move(sch)), rcv_base(std::move(rcv)), m_state2(connect(std::forward<Snd>(snd), rcv1_t{this})), m_state1(connect(schedule(sch), rcv2_t{this})) {}

		public:
			type(type &&) = delete;
			type &operator=(type &&) = delete;

			friend void tag_invoke(start_t, type &op) noexcept { start(op.state1); }

		private:
			void complete() noexcept
			{
				std::visit([&]<typename T>(T &t)
				{
					if constexpr (!std::same_as<T, std::monostate>)
					    std::apply([&]<typename... Args>(auto tag, Args &&...args) { tag(std::move(this->rcv_base::value()), std::move(args)...); }, t);
					else
						std::terminate();
				}, this->data_base::value());
			}

			state1_t m_state1;
			state2_t m_state2;
		};

		template<typename Sch, typename S>
		class sender<Sch, S>::type : env<Sch>::type, detail::ebo_helper<S>
		{
			friend struct schedule_from_t;

			using env_base = typename env<Sch>::type;
			using snd_base = detail::ebo_helper<S>;

			template<typename...>
			using empty_sign = completion_signatures<>;
			template<typename T, typename E>
			using signs = make_completion_signatures<detail::copy_cvref_t<T, S>, E, make_completion_signatures<schedule_result_t<Sch>, E, completion_signatures<set_error_t(std::exception_ptr)>, empty_sign>>;

			template<typename T, typename R>
			using receiver_t = typename receiver1<Sch, detail::copy_cvref_t<T, S>, R>::type;

			template<typename T0, typename T1>
			constexpr type(T0 &&sch, T1 &&snd) : env_base(std::forward<T0>(sch)), snd_base(std::forward<T1>(snd)) {}

		public:
			friend constexpr const auto &tag_invoke(get_env_t, const type &s) noexcept { return s.env(); }
			template<typename T, typename E> requires std::same_as<std::remove_cvref_t<T>, type>
			friend constexpr signs<T, E> tag_invoke(get_completion_signatures_t, T &&, E &&) noexcept { return {}; };

			template<typename T, receiver R> requires std::same_as<std::remove_cvref_t<T>, type> && sender_to<detail::copy_cvref_t<T, S>, receiver_t<T, R>>
			friend constexpr typename operation<Sch, detail::copy_cvref_t<T, S>, R>::type tag_invoke(connect_t, T &&s, R &&r)
			{
				return {s.env().sch(), std::move(s.snd()), std::forward<R>(r)};
			}

		private:
			[[nodiscard]] constexpr auto &snd() noexcept { return snd_base::value(); }
			[[nodiscard]] constexpr auto &env() noexcept { return env_base::value(); }
			[[nodiscard]] constexpr auto &env() const noexcept { return env_base::value(); }
		};

		struct schedule_from_t
		{
			template<scheduler Sch, rod::sender S> requires tag_invocable<schedule_from_t, Sch, S>
			[[nodiscard]] constexpr decltype(auto) operator()(Sch &&sch, S &&s) const noexcept(nothrow_tag_invocable<schedule_from_t, Sch, S>)
			{
				return tag_invoke(*this, std::forward<Sch>(sch), std::forward<S>(s));
			}
			template<scheduler Sch, rod::sender S> requires (!tag_invocable<schedule_from_t, Sch, S>)
			[[nodiscard]] constexpr typename sender<std::decay_t<Sch>, std::decay_t<S>>::type operator()(Sch &&sch, S &&s) const
			{
				return {std::forward<Sch>(sch), std::forward<S>(s)};
			}
		};
	}

	using _schedule_from::schedule_from_t;

	/** Customization point object used to schedule tasks bound by completion of a sender onto a scheduler's execution context.
	 * @warning This CPO is meant for customization of schedulers, use `transfer` instead. */
	inline constexpr auto schedule_from = schedule_from_t{};
}

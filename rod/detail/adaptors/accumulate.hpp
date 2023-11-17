/*
 * Created by switchblade on 2023-10-09.
 */

#pragma once

#include <algorithm>
#include <optional>
#include <vector>

#include "../../stop_token.hpp"
#include "../async_base.hpp"
#include "transfer.hpp"

namespace rod
{
	namespace _accumulate
	{
		template<typename State, typename Res, typename Acc, typename Rcv>
		struct operation_base { struct type; };
		template<typename Rng, typename Res, typename Acc, typename Rcv>
		struct operation { class type; };
		template<typename Rng, typename Res, typename Acc, typename Rcv>
		struct receiver { class type; };
		template<typename Rng, typename Res, typename Acc>
		struct sender { class type; };
		template<typename Env>
		struct env { class type; };

		enum class status_t : std::uint8_t
		{
			running = 0,
			stopped = 1,
			error = 2,
		};

		struct stop_trigger
		{
			void operator()() const noexcept { _src.request_stop(); }
			in_place_stop_source &_src;
		};

		template<typename Env>
		class env<Env>::type : empty_base<Env>
		{
			using env_base = empty_base<Env>;

		public:
			template<typename Env2>
			constexpr explicit type(Env2 &&env, in_place_stop_token tok) noexcept(std::is_nothrow_constructible_v<Env, Env2>) : env_base(std::forward<Env2>(env)), _tok(tok) {}

			template<decays_to_same<type> E>
			friend constexpr in_place_stop_token tag_invoke(get_stop_token_t, E &&e) noexcept { return e._tok; }
			template<is_forwarding_query Q, decays_to_same<type> E, typename... Args> requires _detail::callable<Q, Env, Args...>
			friend constexpr decltype(auto) tag_invoke(Q, E &&e, Args &&...args) noexcept(_detail::nothrow_callable<Q, Env, Args...>)
			{
				return Q{}(std::forward<E>(e).env_base::value(), std::forward<Args>(args)...);
			}

		private:
			in_place_stop_token _tok;
		};

		template<typename T>
		using env_for_t = typename env<env_of_t<T>>::type;

		template<typename State, typename Res, typename Acc, typename Rcv>
		struct operation_base<State, Res, Acc, Rcv>::type : empty_base<State>, empty_base<Acc>, empty_base<Rcv>
		{
			using stop_cb_t = std::optional<stop_callback_for_t<stop_token_of_t<env_of_t<Rcv> &>, stop_trigger>>;
			using state_base = empty_base<State>;
			using acc_base = empty_base<Acc>;
			using rcv_base = empty_base<Rcv>;

			template<typename Res2, typename Acc2>
			constexpr type(Res2 &&res, Acc2 &&acc, Rcv &&rcv, std::size_t count) noexcept(std::is_nothrow_constructible_v<State, std::tuple<set_value_t, Res>> && std::is_nothrow_constructible_v<Acc, Acc2> && std::is_nothrow_move_constructible_v<Rcv>)
					: state_base(std::tuple<set_value_t, Res>(set_value_t{}, std::forward<Res2>(res))), acc_base(std::forward<Acc2>(acc)), rcv_base(std::forward<Rcv>(rcv)), count(count) {}

			template<typename... Args>
			void set_value(Args &&...args) noexcept
			{
				if (status.load(std::memory_order_acquire) != status_t::running)
					return;

				if constexpr (!noexcept(emplace_value(std::forward<Args>(args)...)))
					try { emplace_value(std::forward<Args>(args)...); } catch (...) { set_error(std::current_exception()); }
				else
					emplace_value(std::forward<Args>(args)...);
			}
			template<typename Err>
			void set_error(Err &&err) noexcept
			{
				if (status.exchange(status_t::error, std::memory_order_acq_rel) == status_t::error)
					return;

				stop_src.request_stop();
				if constexpr (!noexcept(emplace_error(std::forward<Err>(err))))
					try { emplace_error(std::forward<Err>(err)); } catch (...) { emplace_error(std::current_exception()); }
				else
					emplace_error(std::forward<Err>(err));
			}
			void set_stopped() noexcept
			{
				if (auto expected = status_t::running; status.compare_exchange_strong(expected, status_t::stopped, std::memory_order_acq_rel))
				{
					stop_src.request_stop();
					state_base::value().template emplace<std::tuple<set_stopped_t>>(set_stopped_t{});
				}
			}

			void submit() noexcept
			{
				if (count.fetch_sub(1, std::memory_order_acq_rel) == 1)
					complete();
			}
			void complete() noexcept
			{
				stop_cb.reset();

				/* By this point all children have submitted. */
				std::visit([&](auto &tpl) { std::apply([&](auto c, auto &...args) { c(std::move(rcv_base::value()), std::move(args)...); }, tpl); }, state_base::value());
			}

			template<typename... Args>
			constexpr void emplace_value(Args &&...args) noexcept(_detail::nothrow_decay_copyable<Res>::value && std::is_nothrow_invocable_v<Acc, std::add_lvalue_reference_t<Res>, Args...>)
			{
				static_assert(std::invocable<Acc, std::add_lvalue_reference_t<Res>, Args...>, "Accumulator must be invocable with result type and source senders' value types");
				static_assert(std::assignable_from<std::add_lvalue_reference_t<Res>, std::invoke_result_t<Acc, std::add_lvalue_reference_t<Res>, Args...>>, "Result type must be assignable from the accumuilator's return type");

				decltype(auto) value = std::get<1>(std::get<std::tuple<set_value_t, Res>>(state_base::value()));
				value = std::invoke(acc_base::value(), value, std::forward<Args>(args)...);
			}
			template<typename Err>
			constexpr void emplace_error(Err &&err) noexcept(_detail::nothrow_decay_copyable<Err>::value)
			{
				static_assert(requires { state_base::value().template emplace<std::tuple<set_error_t, std::decay_t<Err>>>(set_error_t{}, std::forward<Err>(err)); });
				state_base::value().template emplace<std::tuple<set_error_t, std::decay_t<Err>>>(set_error_t{}, std::forward<Err>(err));
			}

			std::atomic<status_t> status = status_t::running;
			std::atomic<std::size_t> count;

			in_place_stop_source stop_src = {};
			stop_cb_t stop_cb = {};
		};

		template<typename... Errs>
		using make_error_state = std::variant<std::tuple<set_error_t, std::decay_t<Errs>>...>;
		template<typename... Errs>
		using make_error_signs = completion_signatures<set_error_t(std::decay_t<Errs>)...>;

		template<typename Snd, typename Res, typename Acc, typename Env>
		using make_sender_state = unique_tuple_t<_detail::concat_tuples<std::variant<std::tuple<set_value_t, Res>, std::tuple<set_stopped_t>>, error_types_of_t<Snd, Env, make_error_state>>>;
		template<typename Snd, typename Res, typename Acc, typename Env>
		using make_sender_signs = unique_tuple_t<_detail::concat_tuples<completion_signatures<set_value_t(Res), set_stopped_t()>, error_types_of_t<Snd, Env, make_error_signs>>>;

		template<typename Snd, typename Res, typename Acc, typename Env>
		using is_non_throwing_accum = value_types_of_t<Snd, Env, _detail::bind_front<std::is_nothrow_invocable, Acc, Res>::template type, std::conjunction>;
		template<typename Snd, typename Res, typename Acc, typename Env>
		using is_non_throwing_state = _detail::apply_tuple_list_t<_detail::all_nothrow_decay_copyable, make_sender_state<Snd, Res, Acc, Env>>;

		template<typename Snd, typename Res, typename Acc, typename Env>
		using make_accum_state = std::conditional_t<is_non_throwing_accum<Res, Acc, Snd, Env>::value && is_non_throwing_state<Res, Acc, Snd, Env>::value, std::variant<>, std::variant<std::tuple<set_error_t, std::exception_ptr>>>;
		template<typename Snd, typename Res, typename Acc, typename Env>
		using make_accum_signs = std::conditional_t<is_non_throwing_accum<Res, Acc, Snd, Env>::value && is_non_throwing_state<Res, Acc, Snd, Env>::value, completion_signatures<>, completion_signatures<set_error_t(std::exception_ptr)>>;

		template<typename Snd, typename Res, typename Acc, typename Env>
		using make_state = unique_tuple_t<_detail::concat_tuples<make_sender_state<Snd, Res, Acc, Env>, make_accum_state<Snd, Res, Acc, Env>>>;
		template<typename Snd, typename Res, typename Acc, typename Env>
		using make_signs = unique_tuple_t<_detail::concat_tuples<make_sender_signs<Snd, Res, Acc, Env>, make_accum_signs<Snd, Res, Acc, Env>>>;;

		template<typename Rng, typename Res, typename Acc, typename Rcv>
		class receiver<Rng, Res, Acc, Rcv>::type
		{
			using operation_base = typename operation_base<make_state<std::ranges::range_value_t<Rng>, Res, Acc, Rcv>, Res, Acc, Rcv>::type;

		public:
			constexpr explicit type(operation_base *op) noexcept : _op(op) {}

			friend constexpr env_for_t<Rcv> tag_invoke(get_env_t, const type &r) noexcept(_detail::nothrow_callable<get_env_t, const Rcv &>) { return env_for_t<Rcv>(get_env(r._op->rcv_base::value()), r._op->stop_src.get_token()); }

			template<typename... Args>
			friend void tag_invoke(set_value_t, type &&r, Args &&...args) noexcept
			{
				r._op->set_value(std::forward<Args>(args)...);
				r._op->submit();
			}
			template<typename Err>
			friend void tag_invoke(set_error_t, type &&r, Err &&err) noexcept
			{
				r._op->set_error(std::forward<Err>(err));
				r._op->submit();
			}
			friend void tag_invoke(set_stopped_t, type &&r) noexcept
			{
				r._op->set_stopped();
				r._op->submit();
			}

		private:
			operation_base *_op = {};
		};

		template<typename Rng, typename T>
		struct select_allocator { using type = std::allocator<T>; };
		template<typename Rng, typename T>
		struct select_range { using type = std::vector<T, typename select_allocator<Rng, T>::type>; };

		template<typename Rng>
		concept has_allocator = requires(Rng rng) { typename std::decay_t<Rng>::allocator_type; rng.get_allocator(); };
		template<typename Rng>
		concept is_tuple_like = requires(Rng rng) { typename std::tuple_size<std::decay_t<Rng>>; std::get<0>(rng); };

		template<typename Rng, typename T> requires has_allocator<Rng>
		struct select_allocator<Rng, T> { using type = typename std::allocator_traits<typename std::decay_t<Rng>::allocator_type>::template rebind_alloc<T>; };
		template<typename Rng, typename T> requires is_tuple_like<Rng>
		struct select_range<Rng, T> { using type = std::array<T, std::tuple_size_v<std::decay_t<Rng>>>; };

		template<typename Rng, typename Res, typename Acc, typename Rcv>
		class operation<Rng, Res, Acc, Rcv>::type : operation_base<make_state<std::ranges::range_value_t<Rng>, Res, Acc, Rcv>, Res, Acc, Rcv>::type
		{
			using operation_base = typename operation_base<make_state<std::ranges::range_value_t<Rng>, Res, Acc, Rcv>, Res, Acc, Rcv>::type;
			using receiver_t = typename receiver<Rng, Res, Acc, Rcv>::type;

			using sender_state = typename select_range<Rng, connect_result_t<std::ranges::range_value_t<Rng>, receiver_t>>::type;
			using result_state = make_state<std::ranges::range_value_t<Rng>, Res, Acc, Rcv>;

			template<typename Rng2>
			static constexpr sender_state connect_rng(operation_base *op, Rng2 &&rng)
			{
				auto state = sender_state();
				state.reserve(std::ranges::size(rng));
				for (decltype(auto) snd : rng)
				{
					const auto connect_snd = [&]() { return _detail::eval_t{[&]() { return connect(std::forward<decltype(snd)>(snd), receiver_t(op)); }}; };
					state.emplace_back(connect_snd());
				}
				return state;
			}
			template<std::size_t... Is, typename Rng2>
			static constexpr sender_state connect_tpl(operation_base *op, Rng2 &&rng, std::index_sequence<Is>...)
			{
				const auto connect_snd = [&]<std::size_t I>(std::in_place_index_t<I>) { return _detail::eval_t{[&]() { return connect(std::get<I>(std::forward<Rng2>(rng)), op); }}; };
				return sender_state{connect_snd(std::in_place_index_t<Is>())...};
			}
			template<typename Rng2>
			static constexpr sender_state connect_all(operation_base *op, Rng2 &&rng) noexcept(_detail::nothrow_callable<connect_t, copy_cvref_t<Rng2, std::ranges::range_value_t<Rng>>, receiver_t> && is_tuple_like<Rng>)
			{
				if constexpr (is_tuple_like<Rng>)
					return connect_tpl(op, std::forward<Rng2>(rng), std::make_index_sequence<std::tuple_size_v<Rng>>());
				else
					return connect_rng(op, std::forward<Rng2>(rng));
			}

		public:
			type(type &&) = delete;
			type &operator=(type &&) = delete;

			template<typename Rng2, typename Res2, typename Acc2>
			constexpr explicit type(Rng2 &&rng, Res2 &&res, Acc2 &&acc, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<operation_base, Res2, Acc2, Rcv, std::size_t> && noexcept(connect_all(this, std::forward<Rng2>(rng))))
					: operation_base(std::forward<Res2>(res), std::forward<Acc2>(acc), std::forward<Rcv>(rcv), std::ranges::size(rng)), _state(connect_all(this, std::forward<Rng2>(rng))) {}

			friend void tag_invoke(start_t, type &op) noexcept
			{
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					op.stop_cb.emplace(get_stop_token(get_env(op.rcv_base::value())), stop_trigger{op.stop_src});

				if (op.stop_src.stop_requested()) [[unlikely]]
					set_stopped(std::move(op.rcv_base::value()));
				else if (!std::ranges::empty(op._state))
					std::ranges::for_each(op._state, [](auto &st) noexcept { start(st); });
				else
					op.complete();
			}

		private:
			sender_state _state;
		};

		template<typename Rng, typename Res, typename Acc>
		class sender<Rng, Res, Acc>::type : empty_base<Rng>, empty_base<Res>, empty_base<Acc>
		{
			using rng_base = empty_base<Rng>;
			using res_base = empty_base<Res>;
			using acc_base = empty_base<Acc>;

			template<typename T, typename Env>
			using signs_t = make_signs<copy_cvref_t<T, std::ranges::range_value_t<Rng>>, Res, Acc, Env>;
			template<typename T, typename Rcv>
			using operation_t = typename operation<copy_cvref_t<T, Rng>, Res, Acc, Rcv>::type;
			template<typename T, typename Rcv>
			using receiver_t = typename receiver<copy_cvref_t<T, Rng>, Res, Acc, Rcv>::type;

		public:
			using is_sender = std::true_type;

		public:
			template<typename Rng2, typename Res2, typename Acc2>
			constexpr explicit type(Rng2 &&rng, Res2 &&res, Acc2 &&acc) noexcept(std::is_nothrow_constructible_v<Rng, Rng2> && std::is_nothrow_constructible_v<Res, Res2> && std::is_nothrow_constructible_v<Acc, Acc2>)
					: rng_base(std::forward<Rng2>(rng)), res_base(std::forward<Res2>(res)), acc_base(std::forward<Acc2>(acc)) {}

		public:
			friend constexpr empty_env tag_invoke(get_env_t, const type &s) noexcept { return {}; }
			template<decays_to_same<type> T, typename Env>
			friend constexpr signs_t<T, Env> tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }

			template<decays_to_same<type> T, rod::receiver Rcv> requires receiver_of<Rcv, signs_t<T, env_of_t<Rcv>>>
			friend constexpr operation_t<T, Rcv> tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(std::is_nothrow_constructible_v<operation_t<T, Rcv>, copy_cvref_t<T, Rng>, Rcv>)
			{
				return operation_t<T, Rcv>(std::forward<T>(s).rng_base::value(), std::forward<T>(s).res_base::value(), std::forward<T>(s).acc_base::value(), std::move(rcv));
			}
		};

		struct accumulate_t
		{
			template<typename Rng, typename Res, typename Acc>
			using sender_t = typename sender<Rng, std::decay_t<Res>, std::decay_t<Acc>>::type;

		public:
			template<std::ranges::sized_range Rng, movable_value Res, movable_value Acc> requires tag_invocable<accumulate_t, Rng, Res, Acc>
			[[nodiscard]] constexpr rod::sender auto operator()(Rng &&rng, Res &&res, Acc &&acc) const noexcept(nothrow_tag_invocable<accumulate_t, Rng, Res, Acc>)
			{
				return tag_invoke(*this, std::forward<Rng>(rng), std::forward<Res>(res), std::forward<Acc>(acc));
			}
			template<std::ranges::sized_range Rng, movable_value Res, movable_value Acc> requires(!tag_invocable<accumulate_t, Rng, Res, Acc>)
			[[nodiscard]] constexpr sender_t<Rng, Res, Acc> operator()(Rng &&rng, Res &&res, Acc &&acc) const noexcept(std::is_nothrow_constructible_v<sender_t<Rng, Res, Acc>, Rng, Res, Acc>)
			{
				return sender_t<Rng, Res, Acc>(std::forward<Rng>(rng), std::forward<Res>(res), std::forward<Acc>(acc));
			}
		};
	}

	using _accumulate::accumulate_t;

	/* TODO: Document usage. */
	inline constexpr auto accumulate = accumulate_t{};
}
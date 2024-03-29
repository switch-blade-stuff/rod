/*
 * Created by switchblade on 2023-05-16.
 */

#pragma once

#include <optional>

#include "../../stop_token.hpp"
#include "../async_base.hpp"

#include "into_variant.hpp"
#include "transfer.hpp"

namespace rod
{
	namespace _when_all
	{
		class when_all_t;

		struct no_error {};
		template<typename C, typename Rcv>
		struct complete_for
		{
			constexpr complete_for(C, Rcv &rcv) noexcept : rcv(rcv) {}

			template<typename... Args> requires _detail::callable<C, Rcv, Args...>
			constexpr void operator()(Args &...args) const noexcept { C{}(std::move(rcv), std::move(args)...); }

			Rcv &rcv;
		};

		constexpr void complete_value(auto &rcv, auto &vals) noexcept
		{
			/* Need to transform a tuple reference into a tuple of references. */
			constexpr auto into_ref = []<typename... Args>(Args &...args) noexcept
			{
				return std::tuple<Args & ...>{args...};
			};

			std::apply([&](auto &...vs) noexcept
			{
				auto refs = std::tuple_cat(std::apply(into_ref, *vs)...);
				std::apply(complete_for{set_value, rcv}, refs);
			}, vals);
		}
		constexpr void complete_error(auto &rcv, auto &errs) noexcept
		{
			const auto visitor = [&]<typename T>(T &err) noexcept
			{
				if constexpr (!std::same_as<T, no_error>)
					complete_for{set_error, rcv}(err);
				else
					std::terminate();
			};
			std::visit(visitor, errs);
		}
		constexpr void complete_stopped(auto &rcv) noexcept { complete_for{set_stopped, rcv}(); }

		enum class state_t : std::uint8_t
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

		template<typename, typename, typename...>
		struct operation;
		template<typename, typename...>
		struct sender;

		template<std::size_t... Is, typename Rcv, typename... Snds>
		struct operation<std::index_sequence<Is...>, Rcv, Snds...> { class type; };
		template<std::size_t... Is, typename... Snds>
		struct sender<std::index_sequence<Is...>, Snds...> { class type; };

		template<typename, typename, typename>
		struct operation_base { struct type; };
		template<std::size_t, typename, typename, typename>
		struct receiver { class type; };
		template<typename>
		struct env { class type; };

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

		template<typename Rcv, typename Vals, typename Errs>
		struct operation_base<Rcv, Vals, Errs>::type : empty_base<Errs>, empty_base<Vals>, empty_base<Rcv>
		{
			using stop_cb_t = std::optional<stop_callback_for_t<stop_token_of_t<env_of_t<Rcv> &>, stop_trigger>>;
			using errs_base = empty_base<Errs>;
			using vals_base = empty_base<Vals>;
			using rcv_base = empty_base<Rcv>;

			constexpr type(Rcv &&rcv, std::size_t count) noexcept(std::is_nothrow_move_constructible_v<Rcv>) : rcv_base(std::forward<Rcv>(rcv)), count(count) {}

			template<std::size_t I, typename... Args>
			void set_value(Args &&...args) noexcept
			{
				if (state.load(std::memory_order_acquire) != state_t::running)
					return;

				if constexpr (!noexcept(emplace_value<I>(std::forward<Args>(args)...)))
					try { emplace_value<I>(std::forward<Args>(args)...); } catch (...) { set_error(std::current_exception()); }
				else
					emplace_value<I>(std::forward<Args>(args)...);
			}
			template<typename Err>
			void set_error(Err &&err) noexcept
			{
				if (state.exchange(state_t::error, std::memory_order_acq_rel) == state_t::error)
					return;

				stop_src.request_stop();
				if constexpr (!noexcept(emplace_error(std::forward<Err>(err))))
					try { emplace_error(std::forward<Err>(err)); } catch (...) { emplace_error(std::current_exception()); }
				else
					emplace_error(std::forward<Err>(err));
			}
			void set_stopped() noexcept
			{
				auto expected = state_t::running;
				if (state.compare_exchange_strong(expected, state_t::stopped, std::memory_order_acq_rel))
					stop_src.request_stop();
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
				switch (state.load(std::memory_order_relaxed))
				{
				default:
				case state_t::stopped:
					complete_stopped(rcv_base::value());
					break;
				case state_t::running:
					complete_value(rcv_base::value(), vals_base::value());
					break;
				case state_t::error:
					complete_error(rcv_base::value(), errs_base::value());
					break;
				}
			}

			template<std::size_t I, typename... Args>
			constexpr void emplace_value(Args &&...args) noexcept((_detail::nothrow_decay_copyable<Args>::value && ...))
			{
				static_assert(requires { std::get<I>(vals_base::value()).emplace(std::forward<Args>(args)...); });
				std::get<I>(vals_base::value()).emplace(std::forward<Args>(args)...);
			}
			template<typename Err>
			constexpr void emplace_error(Err &&err) noexcept(_detail::nothrow_decay_copyable<Err>::value)
			{
				static_assert(requires { errs_base::value().template emplace<std::decay_t<Err>>(std::forward<Err>(err)); });
				errs_base::value().template emplace<std::decay_t<Err>>(std::forward<Err>(err));
			}

			std::atomic<state_t> state = state_t::running;
			std::atomic<std::size_t> count;

			in_place_stop_source stop_src = {};
			stop_cb_t stop_cb = {};
		};

		template<std::size_t I, typename Rcv, typename Vals, typename Errs>
		class receiver<I, Rcv, Vals, Errs>::type
		{
			using operation_base = typename operation_base<Rcv, Vals, Errs>::type;

		public:
			using is_receiver = std::true_type;

		public:
			constexpr explicit type(operation_base *op) noexcept : _op(op) {}

			friend constexpr env_for_t<Rcv> tag_invoke(get_env_t, const type &r) noexcept(_detail::nothrow_callable<get_env_t, const Rcv &>) { return env_for_t<Rcv>(get_env(r._op->rcv_base::value()), r._op->stop_src.get_token()); }

			template<typename... Args>
			friend void tag_invoke(set_value_t, type &&r, Args &&...args) noexcept
			{
				r._op->template set_value<I>(std::forward<Args>(args)...);
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

		template<typename... Ts>
		using make_val_tuple = std::optional<_detail::decayed_tuple<Ts...>>;
		template<typename S, typename E>
		using sender_values = _detail::gather_signatures_t<set_value_t, S, E, make_val_tuple, std::tuple>;
		template<typename S, typename E>
		using sender_errors = unique_tuple_t<_detail::gather_signatures_t<set_error_t, S, E, std::type_identity_t, type_list_t>>;

		template<typename... Ts>
		using nullable_variant = unique_tuple_t<std::variant<no_error, std::decay_t<Ts>...>>;
		template<typename E, typename... Snds>
		using error_data = _detail::apply_tuple_list_t<nullable_variant, _detail::concat_tuples_t<sender_errors<Snds, E>...>>;
		template<typename E, typename... Snds>
		using value_data = _detail::concat_tuples_t<sender_values<Snds, E>...>;

		template<std::size_t... Is, typename Rcv, typename... Snds>
		class operation<std::index_sequence<Is...>, Rcv, Snds...>::type : operation_base<Rcv, value_data<env_for_t<Rcv>, Snds...>, error_data<env_for_t<Rcv>, Snds...>>::type
		{
			using value_data_t = value_data<env_for_t<Rcv>, Snds...>;
			using error_data_t = error_data<env_for_t<Rcv>, Snds...>;

			using operation_base = typename operation_base<Rcv, value_data_t, error_data_t>::type;
			template<std::size_t I>
			using receiver_t = typename receiver<I, Rcv, value_data_t, error_data_t>::type;
			using rcv_base = typename operation_base::rcv_base;

		public:
			type(type &&) = delete;
			type &operator=(type &&) = delete;

			template<typename Snds2>
			constexpr explicit type(Snds2 &&snd, Rcv rcv) noexcept(std::is_nothrow_move_constructible_v<Rcv> && std::conjunction_v<std::is_nothrow_constructible<Snds, copy_cvref_t<Snds2, Snds>>...>)
					: operation_base(std::move(rcv), sizeof...(Is)), _state{_detail::eval_t{[&]() { return connect(std::get<Is>(std::forward<Snds2>(snd)), receiver_t<Is>{this}); }}...} {}

			friend void tag_invoke(start_t, type &op) noexcept
			{
				if constexpr (_detail::stoppable_env<env_of_t<Rcv>>)
					op.stop_cb.emplace(get_stop_token(get_env(op.rcv_base::value())), stop_trigger{op.stop_src});

				if (op.stop_src.stop_requested()) [[unlikely]]
					set_stopped(std::move(op.rcv_base::value()));
				else if constexpr (sizeof...(Snds) != 0)
					std::apply([](auto &...ops) noexcept { (start(ops), ...); }, op._state);
				else
					op.complete();
			}

		private:
			_detail::decayed_tuple<connect_result_t<Snds, receiver_t<Is>>...> _state;
		};

		template<std::size_t... Is, typename... Snds>
		class sender<std::index_sequence<Is...>, Snds...>::type : std::tuple<Snds...>
		{
		public:
			using is_sender = std::true_type;

		private:
			template<typename T, typename Rcv>
			using value_data_t = value_data<env_for_t<Rcv>, copy_cvref_t<T, Snds>...>;
			template<typename T, typename Rcv>
			using error_data_t = error_data<env_for_t<Rcv>, copy_cvref_t<T, Snds>...>;

			template<typename T, typename Rcv>
			using operation_t = typename operation<std::index_sequence<Is...>, Rcv, copy_cvref_t<T, Snds>...>::type;
			template<std::size_t I, typename T, typename Rcv>
			using receiver_t = typename receiver<I, Rcv, value_data_t<T, Rcv>, error_data_t<T, Rcv>>::type;

			template<typename T, typename E>
			using has_throwing = std::negation<_detail::nothrow_decay_copyable<value_data<E, copy_cvref_t<T, Snds>...>>>;
			template<typename T, typename E>
			using error_signs_t = _detail::concat_tuples_t<_detail::gather_signatures_t<set_error_t, copy_cvref_t<T, Snds>, E,
					_detail::bind_front<_detail::make_signature_t, set_error_t>::template type, completion_signatures>...,
					std::conditional_t<has_throwing<T, E>::value, completion_signatures<set_error_t(std::exception_ptr)>, completion_signatures<>>>;
			template<typename T, typename E>
			using value_signs_t = completion_signatures<_detail::apply_tuple_list_t<
					_detail::bind_front<_detail::make_signature_t, set_value_t>::template type,
					_detail::concat_tuples_t<value_types_of_t<copy_cvref_t<T, Snds>, E, std::type_identity_t, type_list_t>...>>>;
			template<typename T, typename E>
			using signs_t = unique_tuple_t<_detail::concat_tuples_t<value_signs_t<T, E>, error_signs_t<T, E>, completion_signatures<set_stopped_t()>>>;

		public:
			template<typename... Args> requires std::constructible_from<std::tuple<Snds...>, Args...>
			constexpr explicit type(std::in_place_t, Args &&...args) noexcept(std::is_nothrow_constructible_v<std::tuple<Snds...>, Args...>) : std::tuple<Snds...>(std::forward<Args>(args)...) {}

			friend constexpr empty_env tag_invoke(get_env_t, const type &) noexcept { return {}; }
			template<decays_to_same<type> T, typename E>
			friend constexpr signs_t<T, E> tag_invoke(get_completion_signatures_t, T &&, E) noexcept { return {}; }

			template<decays_to_same<type> T, rod::receiver Rcv> requires(sender_to<copy_cvref_t<T, Snds>, receiver_t<Is, T, Rcv>> && ...)
			friend constexpr operation_t<T, Rcv> tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(std::is_nothrow_constructible_v<operation_t<T, Rcv>, copy_cvref_t<T, std::tuple<Snds...>>, Rcv>)
			{
				return operation_t<T, Rcv>{static_cast<copy_cvref_t<T, std::tuple<Snds...>>>(s), std::move(rcv)};
			}
		};

		class when_all_t
		{
			template<typename... Snds>
			using sender_t = typename sender<std::index_sequence_for<Snds...>, std::decay_t<Snds>...>::type;

		public:
			template<rod::sender... Snds> requires tag_invocable<when_all_t, Snds...>
			[[nodiscard]] constexpr rod::sender auto operator()(Snds &&...snds) const noexcept(nothrow_tag_invocable<when_all_t, Snds...>) { return tag_invoke(*this, std::forward<Snds>(snds)...); }
			template<rod::sender... Snds> requires(!tag_invocable<when_all_t, Snds...>)
			[[nodiscard]] constexpr sender_t<Snds...> operator()(Snds &&...snds) const noexcept(std::is_nothrow_constructible_v<sender_t<Snds...>, Snds...>) { return sender_t<Snds...>{std::in_place, std::forward<Snds>(snds)...}; }
		};

		struct when_all_with_variant_t
		{
			template<rod::sender... Snds> requires tag_invocable<when_all_with_variant_t, Snds...>
			[[nodiscard]] constexpr rod::sender auto operator()(Snds &&...snds) const noexcept(nothrow_tag_invocable<when_all_with_variant_t, Snds...>)
			{
				return tag_invoke(*this, std::forward<Snds>(snds)...);
			}
			template<rod::sender... Snds> requires(!tag_invocable<when_all_with_variant_t, Snds...> && (_detail::callable<into_variant_t, Snds> && ...))
			[[nodiscard]] constexpr rod::sender auto operator()(Snds &&...snds) const noexcept(
			_detail::nothrow_callable<when_all_t, std::invoke_result_t<into_variant_t, Snds>...> && (_detail::nothrow_callable<into_variant_t, Snds> && ...))
			{
				return when_all_t{}(into_variant(std::forward<Snds>(snds))...);
			}
		};

		struct transfer_when_all_t
		{
			template<rod::scheduler Sch, rod::sender... Snds> requires tag_invocable<transfer_when_all_t, Sch, Snds...>
			[[nodiscard]] constexpr rod::sender auto operator()(Sch &&sch, Snds &&...snds) const noexcept(nothrow_tag_invocable<transfer_when_all_t, Sch, Snds...>)
			{
				return tag_invoke(*this, std::forward<Sch>(sch), std::forward<Snds>(snds)...);
			}
			template<rod::scheduler Sch, rod::sender... Snds> requires(!tag_invocable<transfer_when_all_t, Sch, Snds...>)
			[[nodiscard]] constexpr rod::sender auto operator()(Sch &&sch, Snds &&...snds) const noexcept(_detail::nothrow_callable<when_all_t, Snds...> && _detail::nothrow_callable<transfer_t, std::invoke_result_t<when_all_t, Snds...>, Sch>)
			{
				return transfer(when_all_t{}(std::forward<Snds>(snds)...), std::forward<Sch>(sch));
			}
		};

		struct transfer_when_all_with_variant_t
		{
			template<rod::scheduler Sch, rod::sender... Snds> requires tag_invocable<transfer_when_all_with_variant_t, Sch, Snds...>
			[[nodiscard]] constexpr rod::sender auto operator()(Sch &&sch, Snds &&...snds) const noexcept(nothrow_tag_invocable<transfer_when_all_with_variant_t, Sch, Snds...>)
			{
				return tag_invoke(*this, std::forward<Sch>(sch), std::forward<Snds>(snds)...);
			}
			template<rod::scheduler Sch, rod::sender... Snds> requires(!tag_invocable<transfer_when_all_with_variant_t, Sch, Snds...>)
			[[nodiscard]] constexpr rod::sender auto operator()(Sch &&sch, Snds &&...snds) const noexcept(
			_detail::nothrow_callable<transfer_when_all_t, Sch, std::invoke_result_t<into_variant_t, Snds>...> && (_detail::nothrow_callable<into_variant_t, Snds> && ...))
			{
				return transfer_when_all_t{}(std::forward<Sch>(sch), into_variant(std::forward<Snds>(snds))...);
			}
		};
	}

	using _when_all::when_all_t;

	/** Customization point object used to adapt a group of child senders into a sender that only completes once all children complete
	 * via the value channel, or once at least one of the children completes via the error or stop channels.
	 * @param snd Parameter pack of child senders.
	 * @return Sender representing grouped state of it's children. */
	inline constexpr auto when_all = when_all_t{};

	using _when_all::transfer_when_all_t;

	/** Customization point object used to adapt a group of child senders into a sender that only completes once all children complete
	 * via the value channel, or once at least one of the children completes via the error or stop channels, and then transfer the resulting
	 * sender to complete on a different scheduler.
	 * @param sch Scheduler that will be used for completion of the grouped sender.
	 * @param snd Parameter pack of child senders.
	 * @return Sender representing grouped state of it's children completing on \a sch. */
	inline constexpr auto transfer_when_all = transfer_when_all_t{};

	using _when_all::when_all_with_variant_t;

	/** Customization point object used to adapt a group of child senders into a variant sender that only completes once all children complete
	 * via the value channel, or once at least one of the children completes via the error or stop channels.
	 * @param snd Parameter pack of child senders.
	 * @return Variant sender representing grouped state of it's children. */
	inline constexpr auto when_all_with_variant = when_all_with_variant_t{};

	using _when_all::transfer_when_all_with_variant_t;

	/** Customization point object used to adapt a group of child senders into a variant sender that only completes once all children complete
	 * via the value channel, or once at least one of the children completes via the error or stop channels, and then transfer the resulting
	 * variant sender to complete on a different scheduler.
	 * @param sch Scheduler that will be used for completion of the grouped sender.
	 * @param snd Parameter pack of child senders.
	 * @return Variant sender representing grouped state of it's children completing on \a sch. */
	inline constexpr auto transfer_when_all_with_variant = transfer_when_all_with_variant_t{};
}

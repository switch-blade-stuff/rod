/*
 * Created by switchblade on 2023-05-16.
 */

#pragma once

#include <optional>

#include "../../stop_token.hpp"
#include "../concepts.hpp"

#include "into_variant.hpp"
#include "transfer.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	namespace _when_all
	{
		struct no_error {};
		template<typename C, typename Rcv>
		struct complete_for
		{
			constexpr complete_for(C, Rcv &rcv) noexcept : rcv(rcv) {}

			template<typename... Args>
			constexpr void operator()(Args &...args) const noexcept
			{
				static_assert(detail::callable<C, Rcv, Args...>);
				C{}(std::move(rcv), std::move(args)...);
			}

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

		template<typename, typename, typename>
		struct operation_base { struct type; };
		template<std::size_t, typename, typename, typename>
		struct receiver { struct type; };
		template<typename>
		struct env { struct type; };

		template<typename Env>
		struct env<Env>::type
		{
			template<detail::decays_to<type> E>
			friend constexpr auto tag_invoke(get_stop_token_t, E &&e) noexcept { return e._token; }

			template<is_forwarding_query Q, detail::decays_to<type> E, typename... Args> requires detail::callable<Q, Env, Args...>
			friend constexpr decltype(auto) tag_invoke(Q, E &&e, Args &&...args) noexcept(detail::nothrow_callable<Q, Env, Args...>)
			{
				return Q{}(std::forward<E>(e)._env, std::forward<Args>(args)...);
			}

			[[ROD_NO_UNIQUE_ADDRESS]] Env _env;
			in_place_stop_token _token;
		};

		template<typename T>
		using env_for_t = typename env<env_of_t<T>>::type;

		template<typename Rcv, typename Vals, typename Errs>
		struct operation_base<Rcv, Vals, Errs>::type
		{
			using _stop_cb_t = std::optional<stop_callback_for_t<stop_token_of_t<env_of_t<Rcv> &>, stop_trigger>>;

			template<std::size_t I, typename... Args>
			void _set_value(Args &&...args) noexcept
			{
				if (_state.load(std::memory_order_acquire) != state_t::running)
					return;

				if constexpr ((detail::nothrow_decay_copyable<Args>::value && ...))
					_emplace_value<I>(std::forward<Args>(args)...);
				else
				{
					try { _emplace_value<I>(std::forward<Args>(args)...); }
					catch (...) { _set_error(std::current_exception()); }
				}
			}
			template<typename Err>
			void _set_error(Err &&err) noexcept
			{
				if (_state.exchange(state_t::error, std::memory_order_acq_rel) == state_t::error)
					return;

				_stop_src.request_stop();
				if constexpr (detail::nothrow_decay_copyable<Err>::value)
					_emplace_error(std::forward<Err>(err));
				else
				{
					try { _emplace_error(std::forward<Err>(err)); }
					catch (...) { _emplace_error(std::current_exception()); }
				}
			}
			void _set_stopped() noexcept
			{
				auto expected = state_t::running;
				if (_state.compare_exchange_strong(expected, state_t::stopped, std::memory_order_acq_rel))
					_stop_src.request_stop();
			}

			void _submit() noexcept { if (--_count == 0) _complete(); }
			void _complete() noexcept
			{
				_stop_cb.reset();

				/* By this point all children have submitted. */
				switch (_state.load(std::memory_order_relaxed))
				{
					case state_t::stopped:
						complete_stopped(_rcv);
						break;
					case state_t::running:
						complete_value(_rcv, _vals);
						break;
					case state_t::error:
						complete_error(_rcv, _errs);
						break;
				}
			}

			template<std::size_t I, typename... Args>
			constexpr void _emplace_value(Args &&...args) noexcept((detail::nothrow_decay_copyable<Args>::value && ...))
			{
				static_assert(requires { std::get<I>(_vals).emplace(std::forward<Args>(args)...); });
				std::get<I>(_vals).emplace(std::forward<Args>(args)...);
			}
			template<typename Err>
			constexpr void _emplace_error(Err &&err) noexcept(detail::nothrow_decay_copyable<Err>::value)
			{
				static_assert(requires { _errs.template emplace<std::decay_t<Err>>(std::forward<Err>(err)); });
				_errs.template emplace<std::decay_t<Err>>(std::forward<Err>(err));
			}

			[[ROD_NO_UNIQUE_ADDRESS]] Rcv _rcv;
			[[ROD_NO_UNIQUE_ADDRESS]] Vals _vals = {};
			[[ROD_NO_UNIQUE_ADDRESS]] Errs _errs = no_error{};

			std::atomic<std::size_t> _count;
			std::atomic<state_t> _state;

			in_place_stop_source _stop_src = {};
			_stop_cb_t _stop_cb = {};
		};

		template<std::size_t I, typename Rcv, typename Vals, typename Errs>
		struct receiver<I, Rcv, Vals, Errs>::type
		{
			using is_receiver = std::true_type;

			using _operation_base_t = typename operation_base<Rcv, Vals, Errs>::type;
			using _env_t = env_for_t<Rcv>;

			friend constexpr _env_t tag_invoke(get_env_t, const type &r) noexcept(detail::nothrow_callable<get_env_t, const Rcv &>) { return _env_t{get_env(r._op->_rcv), r._op->_stop_src.get_token()}; }

			template<typename... Args>
			friend void tag_invoke(set_value_t, type &&r, Args &&...args) noexcept
			{
				r._op->template _set_value<I>(std::forward<Args>(args)...);
				r._op->_submit();
			}
			template<typename Err>
			friend void tag_invoke(set_error_t, type &&r, Err &&err) noexcept
			{
				r._op->_set_error(std::forward<Err>(err));
				r._op->_submit();
			}
			friend void tag_invoke(set_stopped_t, type &&r) noexcept
			{
				r._op->_set_stopped();
				r._op->_submit();
			}

			_operation_base_t *_op = {};
		};

		template<typename... Ts>
		using make_val_tuple = std::optional<detail::decayed_tuple<Ts...>>;
		template<typename S, typename E>
		using sender_values = detail::gather_signatures_t<set_value_t, S, E, make_val_tuple, std::tuple>;
		template<typename S, typename E>
		using sender_errors = unique_tuple_t<detail::gather_signatures_t<set_error_t, S, E, std::type_identity_t, type_list_t>>;

		template<typename... Ts>
		using nullable_variant = unique_tuple_t<std::variant<no_error, std::decay_t<Ts>...>>;
		template<typename E, typename... Snds>
		using error_data = detail::apply_tuple_list_t<nullable_variant, detail::concat_tuples_t<sender_errors<Snds, E>...>>;
		template<typename E, typename... Snds>
		using value_data = detail::concat_tuples_t<sender_values<Snds, E>...>;

		template<typename, typename, typename...>
		struct operation;
		template<std::size_t... Is, typename Rcv, typename... Snds>
		struct operation<std::index_sequence<Is...>, Rcv, Snds...> { struct type; };

		template<std::size_t... Is, typename Rcv, typename... Snds>
		struct operation<std::index_sequence<Is...>, Rcv, Snds...>::type : operation_base<Rcv, value_data<env_for_t<Rcv>, Snds...>, error_data<env_for_t<Rcv>, Snds...>>::type
		{
			using _value_data_t = value_data<env_for_t<Rcv>, Snds...>;
			using _error_data_t = error_data<env_for_t<Rcv>, Snds...>;

			template<std::size_t I>
			using _receiver_t = typename receiver<I, Rcv, _value_data_t, _error_data_t>::type;
			using _state_data = detail::decayed_tuple<connect_result_t<Snds, _receiver_t<Is>>...>;
			using _operation_base_t = typename operation_base<Rcv, _value_data_t, _error_data_t>::type;

			type(type &&) = delete;
			type &operator=(type &&) = delete;

			template<typename SndData>
			type(SndData &&snd, Rcv rcv) : _operation_base_t{._rcv = std::move(rcv), ._count = {sizeof...(Is)}, ._state = {state_t::running}},
			                               _states{detail::implicit_eval{[&]() { return _connect<Is>(std::forward<SndData>(snd)); }}...} {}

			friend void tag_invoke(start_t, type &op) noexcept
			{
				op._stop_cb.emplace(get_stop_token(get_env(op._rcv)), stop_trigger{op._stop_src});

				if (op._stop_src.stop_requested())
					[[unlikely]] set_stopped(std::move(op._rcv));
				else if constexpr (sizeof...(Snds) != 0)
					std::apply([](auto &...ops) noexcept { (start(ops), ...); }, op._states);
				else
					op._complete();
			}

			template<std::size_t I, typename SndData>
			constexpr decltype(auto) _connect(SndData &&data) { return connect(std::get<I>(std::forward<SndData>(data)), _receiver_t<I>{this}); }

			_state_data _states;
		};

		template<typename, typename...>
		struct sender;
		template<std::size_t... Is, typename... Snds>
		struct sender<std::index_sequence<Is...>, Snds...> { struct type; };

		template<std::size_t... Is, typename... Snds>
		struct sender<std::index_sequence<Is...>, Snds...>::type
		{
			using is_sender = std::true_type;

			template<typename T, typename Rcv>
			using _value_data_t = value_data<env_for_t<Rcv>, copy_cvref_t<T, Snds>...>;
			template<typename T, typename Rcv>
			using _error_data_t = error_data<env_for_t<Rcv>, copy_cvref_t<T, Snds>...>;

			template<typename T, typename Rcv>
			using _operation_t = typename operation<std::index_sequence<Is...>, Rcv, copy_cvref_t<T, Snds>...>::type;
			template<std::size_t I, typename T, typename Rcv>
			using _receiver_t = typename receiver<I, Rcv, _value_data_t<T, Rcv>, _error_data_t<T, Rcv>>::type;

			template<typename T, typename E>
			using _has_throwing = std::negation<detail::nothrow_decay_copyable<value_data<E, copy_cvref_t<T, Snds>...>>>;
			template<typename T, typename E>
			using _error_signs_t = detail::concat_tuples_t<detail::gather_signatures_t<set_error_t, copy_cvref_t<T, Snds>, E,
					detail::bind_front<detail::make_signature_t, set_error_t>::template type, completion_signatures>...,
					std::conditional_t<_has_throwing<T, E>::value, completion_signatures<set_error_t(std::exception_ptr)>, completion_signatures<>>>;
			template<typename T, typename E>
			using _value_signs_t = completion_signatures<detail::apply_tuple_list_t<
					detail::bind_front<detail::make_signature_t, set_value_t>::template type,
					detail::concat_tuples_t<value_types_of_t<copy_cvref_t<T, Snds>, E, std::type_identity_t, type_list_t>...>>>;
			template<typename T, typename E>
			using _signs_t = unique_tuple_t<detail::concat_tuples_t<_value_signs_t<T, E>, _error_signs_t<T, E>, completion_signatures<set_stopped_t()>>>;

			friend constexpr empty_env tag_invoke(get_env_t, const type &) noexcept { return {}; }
			template<detail::decays_to<type> T, typename E>
			friend constexpr _signs_t<T, E> tag_invoke(get_completion_signatures_t, T &&, E) noexcept { return {}; }

			template<detail::decays_to<type> T, typename Rcv>
			friend _operation_t<T, Rcv> tag_invoke(connect_t, T &&s, Rcv rcv)
			{
				static_assert((sender_to<copy_cvref_t<T, Snds>, _receiver_t<Is, T, Rcv>> && ...));
				return _operation_t<T, Rcv>{std::forward<T>(s)._snds, std::move(rcv)};
			}

			[[ROD_NO_UNIQUE_ADDRESS]] std::tuple<Snds...> _snds;
		};

		class when_all_t
		{
			template<typename... Snds>
			using sender_t = typename sender<std::index_sequence_for<Snds...>, std::decay_t<Snds>...>::type;

		public:
			template<rod::sender... Snds> requires tag_invocable<when_all_t, Snds...>
			[[nodiscard]] constexpr rod::sender auto operator()(Snds &&...snds) const noexcept(nothrow_tag_invocable<when_all_t, Snds...>)
			{
				return tag_invoke(*this, std::forward<Snds>(snds)...);
			}
			template<rod::sender... Snds> requires(!tag_invocable<when_all_t, Snds...>)
			[[nodiscard]] constexpr sender_t<Snds...> operator()(Snds &&...snds) const noexcept(std::is_nothrow_constructible_v<sender_t<Snds...>, Snds...>)
			{
				return sender_t<Snds...>{{std::forward<Snds>(snds)...}};
			}
		};

		struct when_all_with_variant_t
		{
			template<rod::sender... Snds> requires tag_invocable<when_all_with_variant_t, Snds...>
			[[nodiscard]] constexpr rod::sender auto operator()(Snds &&...snds) const noexcept(nothrow_tag_invocable<when_all_with_variant_t, Snds...>)
			{
				return tag_invoke(*this, std::forward<Snds>(snds)...);
			}
			template<rod::sender... Snds> requires(!tag_invocable<when_all_with_variant_t, Snds...> && (detail::callable<into_variant_t, Snds> && ...))
			[[nodiscard]] constexpr rod::sender auto operator()(Snds &&...snds) const noexcept(
			detail::nothrow_callable<when_all_t, std::invoke_result_t<into_variant_t, Snds>...> && (detail::nothrow_callable<into_variant_t, Snds> && ...))
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
			[[nodiscard]] constexpr rod::sender auto operator()(Sch &&sch, Snds &&...snds) const noexcept(
			detail::nothrow_callable<when_all_t, Snds...> && detail::nothrow_callable<transfer_t, std::invoke_result_t<when_all_t, Snds...>, Sch>)
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
			detail::nothrow_callable<transfer_when_all_t, Sch, std::invoke_result_t<into_variant_t, Snds>...> && (detail::nothrow_callable<into_variant_t, Snds> && ...))
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
	using _when_all::when_all_with_variant_t;
	using _when_all::transfer_when_all_with_variant_t;
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

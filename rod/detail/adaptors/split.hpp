/*
 * Created by switchblade on 2023-05-15.
 */

#pragma once

#include <optional>
#include <atomic>

#include "../../stop_token.hpp"
#include "../shared_ref.hpp"
#include "../concepts.hpp"
#include "closure.hpp"

namespace rod
{
	namespace _split
	{
		struct split_sender_tag {};

		struct stop_trigger
		{
			void operator()() noexcept { _src.request_stop(); }
			in_place_stop_source &_src;
		};

		template<typename, typename>
		struct shared_state { struct type; };
		template<typename, typename>
		struct receiver { struct type; };
		template<typename>
		struct env { struct type; };

		struct operation_base
		{
			using notify_func = void(operation_base *) noexcept;

			notify_func *_notify = {};
			operation_base *_next = {};
		};
		template<typename Env>
		struct env<Env>::type
		{
			template<detail::decays_to<type> E>
			friend constexpr auto tag_invoke(get_stop_token_t, const E &&e) noexcept { return e._token; }

			template<is_forwarding_query Q, detail::decays_to<type> E, typename... Args>
			friend constexpr decltype(auto) tag_invoke(Q, E &&e, Args &&...args) noexcept(detail::nothrow_callable<Q, Env, Args...>)
			{
				static_assert(detail::callable<Q, Env, Args...>);
				return Q{}(*e._env, std::forward<Args>(args)...);
			}

			in_place_stop_token _token;
			Env *_env = {};
		};

		template<typename Snd, typename Env>
		struct receiver<Snd, Env>::type
		{
			using is_receiver = std::true_type;

			friend auto tag_invoke(get_env_t, const type &r) noexcept { return r._get_env(); }
			template<detail::completion_channel C, typename... Args>
			friend void tag_invoke(C c, type &&r, Args &&...args) noexcept { r._complete(c, std::forward<Args>(args)...); }

			inline typename env<Env>::type _get_env() const noexcept;
			template<detail::completion_channel C, typename... Args>
			inline void _complete(C, Args &&...) noexcept;

			typename shared_state<Snd, Env>::type &_state;
		};

		template<typename, typename, typename>
		struct operation { struct type; };
		template<typename, typename>
		struct sender { struct type; };

		template<typename Snd, typename Env>
		struct shared_state<Snd, Env>::type : public detail::shared_base
		{
			using _receiver_t = typename receiver<Snd, Env>::type;
			using _state_t = connect_result_t<Snd, _receiver_t>;
			using _env_t = typename env<Env>::type;

			using _values_list = detail::gather_signatures_t<set_value_t, Snd, _env_t, detail::bind_front<detail::decayed_tuple, set_value_t>::template type, type_list_t>;
			using _errors_list = detail::gather_signatures_t<set_error_t, Snd, _env_t, detail::bind_front<detail::decayed_tuple, set_error_t>::template type, type_list_t>;
			template<typename... Ts>
			using _bind_data = typename detail::bind_front<std::variant, std::tuple<set_stopped_t>, std::tuple<set_error_t, std::exception_ptr>>::template type<Ts...>;
			using _data_t = unique_tuple_t<detail::apply_tuple_list_t<_bind_data, detail::concat_tuples_t<_values_list, _errors_list>>>;

			type(Snd &&snd) : _env(get_env(snd)), _state(connect(std::forward<Snd>(snd), _receiver_t{*this})) {}

			void notify() noexcept
			{
				/* Notify queued dependants and replace the queue pointer with `this` sentinel. */
				const auto ptr = _queue.exchange(this, std::memory_order_acq_rel);
				for (auto *state = static_cast<operation_base *>(ptr); state != nullptr;)
				{
					const auto next = state->_next;
					state->_notify(state);
					state = next;
				}
			}

			in_place_stop_source _stop_src;

			[[ROD_NO_UNIQUE_ADDRESS]] Env _env;
			_state_t _state;

			std::atomic<void *> _queue = {};
			_data_t _data = {};
		};

		template<typename Snd, typename Env>
		typename env<Env>::type receiver<Snd, Env>::type::_get_env() const noexcept
		{
			return {_state._stop_src.get_token(), &_state._env};
		}
		template<typename Snd, typename Env>
		template<detail::completion_channel C, typename... Args>
		void receiver<Snd, Env>::type::_complete(C c, Args &&...args) noexcept
		{
			auto &state = _state;
			try { state._data.template emplace<detail::decayed_tuple<C, Args...>>(c, std::forward<Args>(args)...); }
			catch (...) { state._data.template emplace<std::tuple<set_error_t, std::exception_ptr>>(set_error, std::current_exception()); }
			state.notify();
		}

		template<typename Snd, typename Env, typename Rcv>
		struct operation<Snd, Env, Rcv>::type : public operation_base
		{
			using _stop_cb_t = std::optional<stop_callback_for_t<stop_token_of_t<env_of_t<Rcv> &>, stop_trigger>>;
			using _shared_state_t = typename shared_state<Snd, Env>::type;

			static void _bind_notify(operation_base *ptr) noexcept
			{
				auto *op = static_cast<type *>(ptr);
				op->_stop_cb.reset();
				std::visit([&](const auto &val) noexcept
				{
					std::apply([&](auto t, const auto &...args) noexcept { t(std::move(op->_rcv), args...); }, val);
				}, op->_state->_data);
			}

			type(type &&) = delete;
			type &operator=(type &&) = delete;

			constexpr type(Rcv &&rcv, detail::shared_handle<_shared_state_t> handle) noexcept(std::is_nothrow_move_constructible_v<Rcv>)
					: operation_base{_bind_notify}, _rcv(std::forward<Rcv>(rcv)), _state(std::move(handle)) {}

			friend constexpr void tag_invoke(start_t, type &op) noexcept
			{
				const auto state = op._state.get();
				auto &queue = state->_queue;

				/* If the queue pointer is not a sentinel, initialize the stop callback. */
				auto *ptr = queue.load(std::memory_order_acquire);
				if (ptr != state) op._stop_cb.emplace(get_stop_token(get_env(op._rcv)), stop_trigger{state->_stop_src});

				/* Attempt to enqueue this operation to the shared state queue. */
				do
				{
					/* If the queue pointer is a sentinel, notify ourselves. */
					if (ptr == state)
					{
						op._notify(&op);
						return;
					}
					op._next = static_cast<operation_base *>(ptr);
				}
				while (!queue.compare_exchange_weak(ptr, &op, std::memory_order_release, std::memory_order_acquire));

				/* Queue is empty. */
				if (ptr == nullptr)
				{
					/* If stop has been requested, notify the shared state.
					 * Otherwise, start the child operation. */
					if (!state->_stop_src.stop_requested())
						start(state->_state);
					else
						state->notify();
				}
			}

			[[ROD_NO_UNIQUE_ADDRESS]] Rcv _rcv;

			detail::shared_handle<_shared_state_t> _state;
			_stop_cb_t _stop_cb = {};
		};

		template<typename Snd, typename Env>
		struct sender<Snd, Env>::type : split_sender_tag
		{
			using is_sender = std::true_type;

			template<typename Rcv>
			using _operation_t = typename operation<Snd, Env, Rcv>::type;
			using _shared_state_t = typename shared_state<Snd, Env>::type;
			using _env_t = typename env<Env>::type;

			template<typename... Ts>
			using _value_signs_t = completion_signatures<set_value_t(const std::decay_t<Ts> &...)>;
			template<typename Err>
			using _error_signs_t = completion_signatures<set_error_t(const std::decay_t<Err> &)>;
			template<typename T>
			using _signs_t = make_completion_signatures<copy_cvref_t<T, Snd>, _env_t, completion_signatures<set_error_t(const std::exception_ptr &), set_stopped_t()>, _value_signs_t, _error_signs_t>;

			explicit type(Snd &&snd) : _state(new _shared_state_t{std::forward<Snd>(snd)}) {}

			template<detail::decays_to<type> T, typename E>
			friend constexpr _signs_t<T> tag_invoke(get_completion_signatures_t, T &&, E) noexcept { return {}; }
			template<detail::decays_to<type> T, receiver_of<_signs_t<T>> Rcv>
			friend _operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(std::is_nothrow_move_constructible_v<Rcv>) { return _operation_t<Rcv>{std::move(rcv), s._state}; }

			detail::shared_handle<_shared_state_t> _state;
		};

		class split_t
		{
			template<typename Snd>
			using value_completion = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Snd>
			using sender_t = typename sender<std::decay_t<Snd>, std::decay_t<env_of_t<Snd>>>::type;
			using back_adaptor = detail::back_adaptor<split_t>;

		public:
			template<rod::sender Snd> requires detail::tag_invocable_with_completion_scheduler<split_t, set_value_t, Snd, Snd>
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd) const noexcept(nothrow_tag_invocable<split_t, value_completion<Snd>, Snd>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd));
			}
			template<rod::sender Snd> requires(!detail::tag_invocable_with_completion_scheduler<split_t, set_value_t, Snd, Snd> && tag_invocable<split_t, Snd>)
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd) const noexcept(nothrow_tag_invocable<split_t, Snd>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd));
			}

			/* split(_split::sender::type) should not split. */
			template<rod::sender Snd> requires(!detail::tag_invocable_with_completion_scheduler<split_t, set_value_t, Snd> && !tag_invocable<split_t, Snd> && !std::derived_from<std::decay_t<Snd>, split_sender_tag>)
			[[nodiscard]] sender_t<Snd> operator()(Snd &&snd) const { return sender_t<Snd>{std::forward<Snd>(snd)}; }
			template<typename Snd> requires(std::derived_from<std::decay_t<Snd>, split_sender_tag>)
			[[nodiscard]] constexpr auto operator()(Snd &&snd) const noexcept { return std::forward<Snd>(snd); }

			[[nodiscard]] constexpr back_adaptor operator()() const noexcept { return {}; }
		};
	}

	using _split::split_t;

	/** Customization point object used to adapt a sender into a sender that can be connected multiple times.
	 * @param snd Sender to adapt. If omitted, creates a pipe-able sender adaptor.
	 * @return Sender wrapper for \a snd that can be connected multiple times.
	 * @note Splitting a sender requires dynamic allocation of shared state. */
	inline constexpr auto split = split_t{};
}

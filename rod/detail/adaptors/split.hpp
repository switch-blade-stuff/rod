/*
 * Created by switchblade on 2023-05-15.
 */

#pragma once

#include <optional>

#include "../../stop_token.hpp"
#include "../shared_ref.hpp"
#include "../concepts.hpp"
#include "closure.hpp"

namespace rod
{
	namespace _split
	{
		class split_t;

		struct sender_tag {};
		struct stop_trigger
		{
			void operator()() const noexcept { _src.request_stop(); }
			in_place_stop_source &_src;
		};

		template<typename, typename>
		struct shared_state { struct type; };
		template<typename, typename>
		struct receiver { class type; };
		template<typename>
		struct env { class type; };

		struct operation_base
		{
			using notify_func = void(operation_base *) noexcept;

			operation_base *next = {};
			notify_func *notify = {};
		};
		template<typename Env>
		class env<Env>::type
		{
		public:
			constexpr explicit type(in_place_stop_token tok, Env *env) noexcept : _tok(tok), _env(env) {}

			template<decays_to<type> E>
			friend constexpr auto tag_invoke(get_stop_token_t, const E &&e) noexcept { return e._tok; }

			template<is_forwarding_query Q, decays_to<type> E, typename... Args> requires _detail::callable<Q, Env, Args...>
			friend constexpr decltype(auto) tag_invoke(Q, E &&e, Args &&...args) noexcept(_detail::nothrow_callable<Q, Env, Args...>)
			{
				return Q{}(*e._env, std::forward<Args>(args)...);
			}

		private:
			in_place_stop_token _tok;
			Env *_env = {};
		};

		template<typename Snd, typename Env>
		class receiver<Snd, Env>::type
		{
		public:
			using is_receiver = std::true_type;

		private:
			using shared_state_t = typename shared_state<Snd, Env>::type;
			using env_t = typename env<Env>::type;

		public:
			constexpr explicit type(shared_state_t *state) noexcept : _state(state) {}

			friend env_t tag_invoke(get_env_t, const type &r) noexcept { return r.get_env(); }
			template<_detail::completion_channel C, typename... Args>
			friend void tag_invoke(C c, type &&r, Args &&...args) noexcept { r.complete(c, std::forward<Args>(args)...); }

		private:
			inline env_t get_env() const noexcept;
			template<_detail::completion_channel C, typename... Args>
			inline void complete(C, Args &&...) noexcept;

			shared_state_t *_state;
		};

		template<typename, typename, typename>
		struct operation { class type; };
		template<typename, typename>
		struct sender { class type; };

		template<typename Snd, typename Env>
		struct shared_state<Snd, Env>::type : public _detail::shared_base
		{
			using receiver_t = typename receiver<Snd, Env>::type;
			using state_t = connect_result_t<Snd, receiver_t>;
			using env_t = typename env<Env>::type;

			using values_list = _detail::gather_signatures_t<set_value_t, Snd, env_t, _detail::bind_front<_detail::decayed_tuple, set_value_t>::template type, type_list_t>;
			using errors_list = _detail::gather_signatures_t<set_error_t, Snd, env_t, _detail::bind_front<_detail::decayed_tuple, set_error_t>::template type, type_list_t>;
			template<typename... Ts>
			using bind_data = typename _detail::bind_front<std::variant, std::tuple<set_stopped_t>, std::tuple<set_error_t, std::exception_ptr>>::template type<Ts...>;
			using data_t = unique_tuple_t<_detail::apply_tuple_list_t<bind_data, _detail::concat_tuples_t<values_list, errors_list>>>;

			constexpr type(Snd &&snd) : env(get_env(snd)), state(connect(std::forward<Snd>(snd), receiver_t{this})) {}

			void notify_all() noexcept
			{
				/* Notify queued dependants and replace the queue pointer with `this` sentinel. */
				const auto ptr = queue.exchange(this, std::memory_order_acq_rel);
				for (auto *op = static_cast<operation_base *>(ptr); op != nullptr;)
				{
					const auto next = op->next;
					op->notify(op);
					op = next;
				}
			}

			in_place_stop_source stop_src;
			ROD_NO_UNIQUE_ADDRESS Env env;
			state_t state;

			data_t data = std::tuple<set_stopped_t>{};
			std::atomic<void *> queue = {};
		};

		template<typename Snd, typename Env>
		typename env<Env>::type receiver<Snd, Env>::type::get_env() const noexcept
		{
			return typename env<Env>::type{_state->stop_src.get_token(), &_state->env};
		}
		template<typename Snd, typename Env>
		template<_detail::completion_channel C, typename... Args>
		void receiver<Snd, Env>::type::complete(C c, Args &&...args) noexcept
		{
			auto &state = *_state;
			try { state.data.template emplace<_detail::decayed_tuple<C, Args...>>(c, std::forward<Args>(args)...); }
			catch (...) { state.data.template emplace<std::tuple<set_error_t, std::exception_ptr>>(set_error, std::current_exception()); }
			state.notify_all();
		}

		template<typename Snd, typename Env, typename Rcv>
		class operation<Snd, Env, Rcv>::type : operation_base
		{
			using stop_cb_t = std::optional<stop_callback_for_t<stop_token_of_t<env_of_t<Rcv> &>, stop_trigger>>;
			using shared_state_t = typename shared_state<Snd, Env>::type;

			constexpr static void notify_complete(operation_base *ptr) noexcept
			{
				auto *op = static_cast<type *>(ptr);
				op->_stop_cb.reset();
				std::visit([&](const auto &val) noexcept
				{
					std::apply([&](auto t, const auto &...args) noexcept { t(std::move(op->_rcv), args...); }, val);
				}, op->_state->data);
			}

		public:
			type(type &&) = delete;
			type &operator=(type &&) = delete;

			constexpr explicit type(Rcv &&rcv, _detail::shared_handle<shared_state_t> handle) noexcept(std::is_nothrow_move_constructible_v<Rcv>)
					: operation_base{{}, notify_complete}, _state(std::move(handle)), _rcv(std::forward<Rcv>(rcv)) {}

			friend constexpr void tag_invoke(start_t, type &op) noexcept { op.start(); }

		private:
			constexpr void start() noexcept
			{
				const auto state = _state.get();
				auto &queue = state->queue;

				/* If the queue pointer is not a sentinel, initialize the stop callback. */
				auto *ptr = queue.load(std::memory_order_acquire);
				if (ptr != state) _stop_cb.emplace(get_stop_token(get_env(_rcv)), stop_trigger{state->stop_src});

				/* Attempt to enqueue this operation to the shared state queue. */
				do
				{
					/* If the queue pointer is a sentinel, notify ourselves. */
					if (ptr == state)
					{
						notify(this);
						return;
					}
					next = static_cast<operation_base *>(ptr);
				}
				while (!queue.compare_exchange_weak(ptr, this, std::memory_order_release, std::memory_order_acquire));

				/* Queue is empty. */
				if (ptr == nullptr)
				{
					/* If stop has been requested, notify the shared state.
					 * Otherwise, start the child operation. */
					if (!state->stop_src.stop_requested())
						rod::start(state->state);
					else
						state->notify_all();
				}
			}

			_detail::shared_handle<shared_state_t> _state;
			ROD_NO_UNIQUE_ADDRESS Rcv _rcv;
			stop_cb_t _stop_cb = {};
		};

		template<typename Snd, typename Env>
		class sender<Snd, Env>::type : public sender_tag
		{
		public:
			using is_sender = std::true_type;

		private:
			using shared_state_t = typename shared_state<Snd, Env>::type;
			template<typename Rcv>
			using operation_t = typename operation<Snd, Env, Rcv>::type;
			using env_t = typename env<Env>::type;

			template<typename... Ts>
			using value_signs_t = completion_signatures<set_value_t(const std::decay_t<Ts> &...)>;
			template<typename Err>
			using error_signs_t = completion_signatures<set_error_t(const std::decay_t<Err> &)>;
			template<typename T>
			using signs_t = make_completion_signatures<copy_cvref_t<T, Snd>, env_t, completion_signatures<set_error_t(const std::exception_ptr &), set_stopped_t()>, value_signs_t, error_signs_t>;

		public:
			constexpr explicit type(Snd &&snd) : _state(new shared_state_t{std::forward<Snd>(snd)}) {}

			template<decays_to<type> T, typename E>
			friend constexpr signs_t<T> tag_invoke(get_completion_signatures_t, T &&, E) noexcept { return {}; }
			template<decays_to<type> T, receiver_of<signs_t<T>> Rcv>
			friend constexpr operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(std::is_nothrow_constructible_v<operation_t<Rcv>, Rcv, _detail::shared_handle<shared_state_t>>) { return operation_t<Rcv>{std::move(rcv), s._state}; }

		private:
			_detail::shared_handle<shared_state_t> _state;
		};

		class split_t
		{
			template<typename Snd>
			using value_scheduler = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Snd>
			using sender_t = typename sender<std::decay_t<Snd>, env_of_t<std::decay_t<Snd>>>::type;
			using back_adaptor = _detail::back_adaptor<split_t>;

		public:
			template<rod::sender Snd> requires _detail::tag_invocable_with_completion_scheduler<split_t, set_value_t, Snd, Snd>
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd) const noexcept(nothrow_tag_invocable<split_t, value_scheduler<Snd>, Snd>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd));
			}
			template<rod::sender Snd> requires(!_detail::tag_invocable_with_completion_scheduler<split_t, set_value_t, Snd, Snd> && tag_invocable<split_t, Snd>)
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd) const noexcept(nothrow_tag_invocable<split_t, Snd>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd));
			}

			/* split(_split::sender::type) should not split. */
			template<rod::sender Snd> requires(!_detail::tag_invocable_with_completion_scheduler<split_t, set_value_t, Snd> && !tag_invocable<split_t, Snd> && !std::derived_from<std::decay_t<Snd>, sender_tag>)
			[[nodiscard]] sender_t<Snd> operator()(Snd &&snd) const { return sender_t<Snd>{std::forward<Snd>(snd)}; }
			template<typename Snd> requires(std::derived_from<std::decay_t<Snd>, sender_tag>)
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd) const noexcept { return std::forward<Snd>(snd); }

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

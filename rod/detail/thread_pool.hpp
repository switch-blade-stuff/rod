/*
 * Created by switch_blade on 2023-06-10.
 */

#pragma once

#include <span>

#include "../scheduling.hpp"

namespace rod
{
	namespace _thread_pool
	{
		class thread_pool;
		class scheduler;
		class sender;

		template<typename, typename, typename, typename>
		struct bulk_operation { class type; };
		template<typename, typename, typename, typename, typename>
		struct bulk_receiver { class type; };
		template<typename, typename, typename>
		struct bulk_sender { class type; };

		template<typename>
		struct operation { class type; };

		struct env { thread_pool *_pool; };

		struct operation_base
		{
			using notify_func_t = void (*)(operation_base *, std::size_t) noexcept;

			void notify(std::size_t id) { std::exchange(notify_func, {})(this, id); }

			notify_func_t notify_func;
			operation_base *next = {};
		};
		struct bulk_task_base : operation_base
		{
			constexpr bulk_task_base(const bulk_task_base &) noexcept = default;
			constexpr bulk_task_base(auto *notify, auto *state) noexcept : operation_base{notify}, state(state) {}

			void *state;
		};

		template<typename Fn, typename Shape, typename... Args>
		concept bulk_nothrow = _detail::nothrow_callable<Fn, Shape, Args &...> && std::is_nothrow_constructible_v<_detail::decayed_tuple<Args...>, Args...>;

		template<typename Snd, typename Rcv, typename Shape, typename Fn, typename ThrowTag>
		struct bulk_shared_state : empty_base<value_types_of_t<Snd, env_of_t<Rcv>, _detail::decayed_tuple, _detail::variant_or_empty>>, empty_base<Rcv>, empty_base<Fn>
		{
			static constexpr std::pair<Shape, Shape> split_tasks(Shape n, std::size_t pos, std::size_t size) noexcept
			{
				const auto div = n / size;
				const auto rem = n % size;
				const auto i = pos < rem ? (div + 1) * pos : (div + 1) * rem + (pos - rem) * div;
				return std::make_pair(i, div + i + static_cast<Shape>(pos < rem));
			}

			using data_t = value_types_of_t<Snd, env_of_t<Rcv>, _detail::decayed_tuple, _detail::variant_or_empty>;
			using data_base = empty_base<data_t>;
			using func_base = empty_base<Fn>;
			using rcv_base = empty_base<Rcv>;

			static void notify_task(operation_base *ptr, std::size_t id) noexcept
			{
				auto &state = *static_cast<bulk_shared_state *>(static_cast<bulk_task_base *>(ptr)->state);
				if constexpr (ThrowTag::value)
					try { state.invoke(id); } catch (...) { state.set_exception(); }
				else
					state.invoke(id);
				state.complete();
			}

			template<typename Fn2>
			constexpr bulk_shared_state(thread_pool *pool, Rcv rcv, Shape shape, Fn2 &&fn) noexcept(std::is_nothrow_move_constructible_v<Rcv> && std::is_nothrow_constructible_v<Fn, Fn2>)
					: rcv_base(std::move(rcv)), func_base(std::forward<Fn2>(fn)), pool(pool), shape(shape), tasks(required_threads(), bulk_task_base{notify_task, this}) {}

			template<typename... Args> requires(!std::same_as<data_t, _detail::empty_variant<>>)
			constexpr void start_bulk(Args &&...args) noexcept
			{
				const auto emplace = [&]() { data_base::value().template emplace<_detail::decayed_tuple<Args...>>(std::forward<Args>(args)...); };

				if constexpr (ThrowTag::value)
					try { emplace(); } catch (...) { set_error(std::move(rcv_base::value()), std::current_exception()); }
				else
					emplace();

				if (shape == Shape{0})
					apply([&](auto &...vs) { set_value(std::move(rcv_base::value()), std::move(vs)...); });
				else
					start();
			}
			template<typename... Args> requires std::same_as<data_t, _detail::empty_variant<>>
			constexpr void start_bulk(Args &&...) noexcept
			{
				if (shape == Shape{0})
					set_value(std::move(rcv_base::value()));
				else
					start();
			}

			template<typename F> requires(!std::same_as<data_t, _detail::empty_variant<>>)
			constexpr void apply(F &&f) noexcept { std::visit([&](auto &tpl) { std::apply([&](auto &...args) { std::invoke(f, args...); }, tpl); }, data_base::value()); }
			template<typename F> requires std::same_as<data_t, _detail::empty_variant<>>
			constexpr void apply(F &&f) noexcept { std::invoke(f); }

			constexpr void complete() noexcept
			{
				if (done.fetch_add(1, std::memory_order_acq_rel) + 1 < required_threads())
					return;

				if (!has_error.test(std::memory_order_acquire))
					apply([&](auto &...args) { set_value(std::move(rcv_base::value()), std::move(args)...); });
				else
					set_error(std::move(rcv_base::value()), std::move(err));
			}
			constexpr void set_exception() noexcept
			{
				if (!has_error.test_and_set(std::memory_order_acq_rel))
					err = std::current_exception();
			}
			constexpr void invoke(std::size_t id) noexcept(ThrowTag::value)
			{
				if (has_error.test(std::memory_order_acquire))
					return;

				apply([&](auto &...args)
				{
					auto [i, n] = split_tasks(shape, id, required_threads());
					for (; i != n; ++i) std::invoke(func_base::value(), i, args...);
				});
			}

			constexpr std::size_t required_threads() const noexcept;
			inline void start() noexcept;

			thread_pool *pool;
			Shape shape;

			std::vector<bulk_task_base> tasks;
			std::atomic<std::size_t> done = 0;

			std::atomic_flag has_error = {};
			std::exception_ptr err = {};
		};
		template<typename Snd, typename Rcv, typename Shape, typename Fn, typename ThrowTag>
		class bulk_receiver<Snd, Rcv, Shape, Fn, ThrowTag>::type
		{
			using shared_state_t = bulk_shared_state<Snd, Rcv, Shape, Fn, ThrowTag>;

		public:
			using is_receiver = std::true_type;

		public:
			constexpr explicit type(shared_state_t *state) noexcept : _state(state) {}

			friend constexpr env_of_t<Rcv> tag_invoke(get_env_t, const type &r) noexcept { return get_env(r._state->rcv_base::value()); }

			template<std::same_as<set_value_t> C, typename... Args>
			friend void tag_invoke(C, type &&r, Args &&...args) noexcept { r._state->start_bulk(std::forward<Args>(args)...); }
			template<_detail::completion_channel C, typename... Args> requires(!std::same_as<C, set_value_t>)
			friend void tag_invoke(C, type &&r, Args &&...args) noexcept { C{}(std::move(r._state->rcv_base::value()), std::forward<Args>(args)...); }

		private:
			shared_state_t *_state;
		};

		template<typename Rcv>
		class operation<Rcv>::type : operation_base, empty_base<Rcv>
		{
			static void notify_complete(operation_base *p, std::size_t) noexcept
			{
				auto &rcv = static_cast<type *>(p)->empty_base<Rcv>::value();
				if (rod::get_stop_token(get_env(rcv)).stop_requested())
					set_stopped(std::move(rcv));
				else
					set_value(std::move(rcv));
			}

		public:
			type() = delete;
			type(const type &) = delete;

			constexpr explicit type(thread_pool *pool, Rcv &&rcv) noexcept(std::is_nothrow_move_constructible_v<Rcv>) : operation_base{notify_complete}, empty_base<Rcv>(std::forward<Rcv>(rcv)), _pool(pool) {}

			friend void tag_invoke(start_t, type &op) noexcept { op.start(); }

		private:
			inline void start() noexcept;

			thread_pool *_pool;
		};
		template<typename Snd, typename Rcv, typename Shape, typename Fn>
		class bulk_operation<Snd, Rcv, Shape, Fn>::type
		{
			template<typename... Ts>
			using test_nothrow = std::bool_constant<bulk_nothrow<Fn, Shape, Ts...>>;
			using is_throwing = std::negation<value_types_of_t<Snd, env_of_t<Rcv>, test_nothrow, std::conjunction>>;

			using receiver_t = typename bulk_receiver<Snd, Rcv, Shape, Fn, is_throwing>::type;
			using shared_state_t = bulk_shared_state<Snd, Rcv, Shape, Fn, is_throwing>;
			using connect_state_t = connect_result_t<Snd, receiver_t>;

		public:
			type(type &&) = delete;
			type &operator=(type &&) = delete;

			template<typename Snd2, typename Fn2>
			constexpr explicit type(thread_pool *pool, Snd2 &&snd, Rcv &&rcv, Shape shape, Fn2 &&fn) noexcept(std::is_nothrow_constructible_v<shared_state_t, thread_pool *, Rcv, Shape, Fn2> && _detail::nothrow_callable<connect_t, Snd, receiver_t>)
					: _shared_state(pool, std::forward<Rcv>(rcv), shape, std::forward<Fn2>(fn)), _connect_state{connect(std::forward<Snd2>(snd), receiver_t{&_shared_state})} {}

			friend constexpr void tag_invoke(start_t, type &op) noexcept { start(op._connect_state); }

		private:
			shared_state_t _shared_state;
			connect_state_t _connect_state;
		};

		/** Thread pool execution context. */
		class thread_pool
		{
			template<typename, typename, typename, typename, typename>
			friend struct bulk_shared_state;
			template<typename>
			friend struct operation;

			struct worker_t
			{
				using task_queue_t = _detail::atomic_queue<operation_base, &operation_base::next>;

				void stop() noexcept
				{
					if (thread.joinable())
					{
						queue.terminate();
						queue.notify_all();
					}
				}
				void start(thread_pool *pool, std::size_t id) noexcept
				{
					thread = std::jthread{[=]() { pool->worker_main(id); }};
				}

				void push(operation_base *node) noexcept
				{
					queue.push(node);
					queue.notify_one();
				}
				bool try_push(operation_base *node) noexcept
				{
					if (queue.empty())
					{
						push(node);
						return true;
					}
					return false;
				}

				std::jthread thread;
				task_queue_t queue;
			};

		public:
			/** Initializes thread pool with a default number of threads. */
			ROD_API_PUBLIC thread_pool();
			/** Initializes thread pool with `size` threads. */
			ROD_API_PUBLIC thread_pool(std::size_t size);
			ROD_API_PUBLIC ~thread_pool();

			/** Changes the internal state to stopped and terminates worker threads.
			 * @note After a call to `finish` the thread pool will no longer be dispatching scheduled operations. */
			ROD_API_PUBLIC void finish() noexcept;

			/** Returns a scheduler used to schedule work to be executed on the thread pool. */
			[[nodiscard]] constexpr scheduler get_scheduler() noexcept;
			/** Returns the number of worker threads managed by the thread pool. */
			[[nodiscard]] std::size_t size() const noexcept { return _workers.size(); }

			/** Returns copy of the stop source associated with the thread pool. */
			[[nodiscard]] constexpr in_place_stop_source &get_stop_source() noexcept { return _stop_src; }
			/** Returns a stop token of the stop source associated with the thread pool. */
			[[nodiscard]] constexpr in_place_stop_token get_stop_token() const noexcept { return _stop_src.get_token(); }
			/** Sends a stop request to the stop source associated with the thread pool. */
			void request_stop() { _stop_src.request_stop(); }

		protected:
			void stop_all() noexcept { for (auto &worker: _workers) { worker.stop(); }}

			ROD_API_PUBLIC void worker_main(std::size_t id) noexcept;
			ROD_API_PUBLIC void schedule(operation_base *node) noexcept;
			ROD_API_PUBLIC void schedule_bulk(std::span<bulk_task_base> tasks) noexcept;

			std::atomic<std::size_t> _next = {};
			std::vector<worker_t> _workers;
			in_place_stop_source _stop_src;
		};

		class sender
		{
		public:
			using is_sender = std::true_type;

		private:
			using signs_t = completion_signatures<set_value_t(), set_stopped_t()>;
			template<typename Rcv>
			using operation_t = typename operation<Rcv>::type;

		public:
			constexpr explicit sender(thread_pool *pool) noexcept : _pool(pool) {}

			friend constexpr env tag_invoke(get_env_t, const sender &s) noexcept { return {s._pool}; }
			template<decays_to<sender> T, typename E>
			friend constexpr signs_t tag_invoke(get_completion_signatures_t, T &&, E) { return {}; }

			template<decays_to<sender> T, receiver_of<signs_t> Rcv>
			friend constexpr operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(_detail::nothrow_decay_copyable<Rcv>::value) { return operation_t<Rcv>{s._pool, std::move(rcv)}; }

		private:
			thread_pool *_pool;
		};
		template<typename Snd, typename Shape, typename Fn>
		class bulk_sender<Snd, Shape, Fn>::type : empty_base<Snd>, empty_base<Fn>
		{
			friend class scheduler;

		public:
			using is_sender = std::true_type;

		private:
			template<typename... Ts>
			using test_nothrow = std::bool_constant<bulk_nothrow<Fn, Shape, Ts...>>;
			template<typename T, typename Env>
			using nothrow_completions = value_types_of_t<copy_cvref_t<T, Snd>, Env, test_nothrow, std::conjunction>;

			template<typename... Ts>
			using value_signs_t = completion_signatures<set_value_t(std::decay_t<Ts>...)>;
			template<typename T, typename Env>
			using error_signs_t = std::conditional_t<nothrow_completions<T, Env>::value, completion_signatures<>, completion_signatures<set_error_t(std::exception_ptr)>>;
			template<typename T, typename Env>
			using signs_t = make_completion_signatures<copy_cvref_t<T, Snd>, Env, error_signs_t<T, Env>, value_signs_t>;

			template<typename T, typename Rcv>
			using operation_t = typename bulk_operation<copy_cvref_t<T, Snd>, std::decay_t<Rcv>, Shape, Fn>::type;

			using snd_base = empty_base<Snd>;
			using func_base = empty_base<Fn>;

		public:
			template<typename Snd2, typename Fn2>
			constexpr explicit type(thread_pool *pool, Snd2 &&snd, Shape shape, Fn2 &&fn) noexcept(std::is_nothrow_constructible_v<Snd, Snd2> && std::is_nothrow_constructible_v<Fn, Fn2>)
					: snd_base(std::forward<Snd2>(snd)), func_base(std::forward<Fn2>(fn)), _pool(pool), _shape(shape) {}

			friend constexpr env_of_t<const Snd &> tag_invoke(get_env_t, const type &s) noexcept { return get_env(s.snd_base::value()); }
			template<decays_to<type> T, typename Env>
			friend constexpr signs_t<T, Env> tag_invoke(get_completion_signatures_t, T &&, Env &&) noexcept { return {}; }

			template<decays_to<type> T, rod::receiver Rcv> requires receiver_of<Rcv, signs_t<T, env_of_t<const Snd &>>>
			friend constexpr operation_t<T, Rcv> tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(std::is_nothrow_constructible_v<operation_t<T, Rcv>, thread_pool *, copy_cvref_t<T, Snd>, Rcv, Shape, copy_cvref_t<T, Fn>>)
			{
				return operation_t<T, Rcv>{s._pool,  std::forward<T>(s).snd_base::value(), std::move(rcv), s._shape, std::forward<T>(s).func_base::value()};
			}

		private:
			thread_pool *_pool;
			Shape _shape;
		};

		class scheduler
		{
			template<typename Snd, typename Shape, typename Fn>
			using bulk_sender_t = typename bulk_sender<std::decay_t<Snd>, Shape, std::decay_t<Fn>>::type;

		public:
			constexpr explicit scheduler(thread_pool *pool) noexcept : _pool(pool) {}

			[[nodiscard]] friend constexpr bool operator==(const scheduler &, const scheduler &) noexcept = default;

			friend constexpr bool tag_invoke(execute_may_block_caller_t, const scheduler &) noexcept { return false; }
			friend constexpr auto tag_invoke(get_forward_progress_guarantee_t, const scheduler &) noexcept { return forward_progress_guarantee::parallel; }

			template<decays_to<scheduler> T>
			friend constexpr auto tag_invoke(schedule_t, T &&s) noexcept { return s.schedule(); }
			template<decays_to<scheduler> T, typename Snd, typename Shape, typename Fn>
			friend constexpr auto tag_invoke(bulk_t, T &&s, Snd &&snd, Shape shape, Fn &&fn) noexcept { return s.schedule_bulk(std::forward<Snd>(snd), shape, std::forward<Fn>(fn)); }

		private:
			auto schedule() noexcept { return sender{_pool}; }
			template<typename Snd, typename Shape, typename Fn>
			auto schedule_bulk(Snd &&snd, Shape shape, Fn &&fn) noexcept { return bulk_sender_t<Snd, Shape, Fn>{_pool, std::forward<Snd>(snd), shape, std::forward<Fn>(fn)}; }

			thread_pool *_pool;
		};

		template<typename Rcv>
		void operation<Rcv>::type::start() noexcept { _pool->schedule(this); }

		template<typename Snd, typename Rcv, typename Shape, typename Fn, typename ThrowTag>
		void bulk_shared_state<Snd, Rcv, Shape, Fn, ThrowTag>::start() noexcept { pool->schedule_bulk(tasks); }
		template<typename Snd, typename Rcv, typename Shape, typename Fn, typename ThrowTag>
		constexpr std::size_t bulk_shared_state<Snd, Rcv, Shape, Fn, ThrowTag>::required_threads() const noexcept { return std::min(shape, static_cast<Shape>(pool->size())); }

		constexpr in_place_stop_token tag_invoke(get_stop_token_t, const env &s) noexcept { return s._pool->get_stop_token(); }
		template<typename T>
		constexpr scheduler tag_invoke(get_completion_scheduler_t<T>, const env &e) noexcept { return e._pool->get_scheduler(); }

		constexpr scheduler thread_pool::get_scheduler() noexcept { return scheduler{this}; }
	}

	using _thread_pool::thread_pool;
}
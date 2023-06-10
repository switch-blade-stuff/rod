/*
 * Created by switch_blade on 2023-06-10.
 */

#pragma once

#include "../scheduling.hpp"

namespace rod::_thread_pool
{
	class basic_thread_pool;
	class operation_base;
	class scheduler;

	template<typename, typename, typename, typename, typename>
	struct bulk_shared_state { struct type; };
	template<typename, typename, typename, typename>
	struct bulk_operation { class type; };
	template<typename, typename, typename, typename, typename>
	struct bulk_receiver { class type; };
	template<typename, typename, typename>
	struct bulk_sender { class type; };

	template<typename>
	struct operation { class type; };

	class sender;

	struct env { basic_thread_pool *_pool; };

	class operation_base
	{
		friend class basic_thread_pool;

		using notify_func_t = void (*)(operation_base *, std::size_t) noexcept;

	protected:
		constexpr operation_base(notify_func_t notify = {}) noexcept : _notify_func(notify) {}

	private:
		void notify(std::size_t worker) noexcept { _notify_func(this, worker); }

	protected:
		notify_func_t _notify_func;
		operation_base *_next = {};
	};

	template<typename Fn, typename Shape, typename... Args>
	concept bulk_nothrow = detail::nothrow_callable<Fn, Shape, Args &...> && std::is_nothrow_constructible_v<detail::decayed_tuple<Args...>, Args...>;

	template<typename Snd, typename Rcv, typename Shape, typename Fn, typename ThrowTag>
	struct bulk_shared_state<Snd, Rcv, Shape, Fn, ThrowTag>::type
	{
		static constexpr std::pair<Shape, Shape> split_tasks(Shape n, std::size_t pos, std::size_t size) noexcept
		{
			const auto div = n / size;
			const auto rem = n % size;
			const auto i = pos < rem ? (div + 1) * pos : (div + 1) * rem + (pos - rem) * div;
			return {i, div + i + pos < rem};
		}

		struct task_t : operation_base
		{
			static void notify_complete(operation_base *ptr, std::size_t id) noexcept
			{
				auto &state = *static_cast<task_t *>(ptr)->state;
				const auto threads = state.required_threads();

				const auto do_invoke = [&](auto &...args)
				{
					auto [i, n] = split_tasks(state.shape, id, threads);
					for (; i != n; ++i) state.fn(i, args...);
				};

				if constexpr (!ThrowTag::value)
					state.apply(do_invoke);
				else if (!state.can_invoke.test(std::memory_order_acquire))
				{
					try { state.apply(do_invoke); }
					catch (...)
					{
						state.can_invoke.test_and_set(std::memory_order_acq_rel);
						state.err = std::current_exception();
					}
				}
				if (state.done.fetch_add(1, std::memory_order_acq_rel) == threads - 1)
				{
					if constexpr (ThrowTag::value)
						if (state.err) [[unlikely]]
						{
							set_error(std::move(state.rcv), std::move(state.err));
							return;
						}
					state.apply([&](auto &...args) { set_value(std::move(state._rcv), std::move(args)...); });
				}
			}

			constexpr task_t(const task_t &) noexcept = default;
			constexpr task_t(type *state) noexcept : operation_base(notify_complete), state(state) {}

			type *state;
		};

		using data_t = value_types_of_t<Snd, env_of_t<Rcv>, detail::decayed_tuple, detail::variant_or_empty>;

		type(basic_thread_pool *pool, Rcv rcv, Shape shape, Fn fn) : pool(pool), rcv(std::move(rcv)), fn(std::move(fn)), shape(shape), tasks(required_threads(), task_t{this}) {}

		std::size_t required_threads() const noexcept { return std::min(shape, static_cast<Shape>(pool->size())); }
		template<typename F>
		constexpr void apply(F &&f) { std::visit([&](auto &tpl) { std::apply([&](auto &...args) { f(args...); }, tpl); }, data); }

		inline void start() noexcept;

		basic_thread_pool *pool;

		ROD_NO_UNIQUE_ADDRESS data_t data;
		ROD_NO_UNIQUE_ADDRESS Rcv rcv;
		ROD_NO_UNIQUE_ADDRESS Fn fn;
		Shape shape;

		std::atomic<std::size_t> done = {};
		std::atomic_flag can_invoke = {};
		std::exception_ptr err = {};
		std::vector<task_t> tasks;
	};
	template<typename Snd, typename Rcv, typename Shape, typename Fn, typename ThrowTag>
	class bulk_receiver<Snd, Rcv, Shape, Fn, ThrowTag>::type
	{
		template<typename, typename, typename, typename>
		friend struct bulk_operation;

		using shared_state_t = bulk_shared_state<Snd, Rcv, Shape, Fn, ThrowTag>;

		constexpr type(shared_state_t *state) noexcept : _state(state) {}

	public:
		friend constexpr env_of_t<Rcv> tag_invoke(get_env_t, const type &r) noexcept { return get_env(r._state->rcv); }

		template<std::same_as<set_value_t> C, typename... Args>
		friend void tag_invoke(C, type &&r, Args &&...args) noexcept
		{
			using tuple_t = detail::decayed_tuple<Args...>;
			auto &state = *r._state;

			if constexpr (!ThrowTag::value)
				state.data.template emplace<tuple_t>(std::forward<Args>(args)...);
			else
			{
				try { state.data.template emplace<tuple_t>(std::forward<Args>(args)...); }
				catch (...) { set_error(std::move(state.rcv), std::current_exception()); }
			}

			if (state.shape == Shape{0})
				state.apply([&](auto &...vs) { set_value(std::move(state.rcv), std::move(vs)...); });
			else
				state.start();
		}
		template<detail::completion_channel C, typename... Args>
		requires(!std::same_as<C, set_value_t>)
		friend void tag_invoke(C, type &&r, Args &&...args) noexcept { C{}(std::move(r._state->rcv), std::forward<Args>(args)...); }

	private:
		shared_state_t *_state;
	};

	template<typename Rcv>
	class operation<Rcv>::type : operation_base
	{
		friend class sender;

		static void notify_complete(operation_base *p, std::size_t) noexcept
		{
			auto &rcv = static_cast<type *>(p)->_rcv;
			if (rod::get_stop_token(get_env(rcv)).stop_requested())
				set_stopped(std::move(rcv));
			else
				set_value(std::move(rcv));
		}

		constexpr type(basic_thread_pool *pool, Rcv rcv) noexcept(std::is_nothrow_move_constructible_v<Rcv>) : operation_base(notify_complete), _rcv(std::move(rcv)), _pool(pool) {}

	public:
		type() = delete;
		type(const type &) = delete;

		friend void tag_invoke(start_t, type &op) noexcept { op.start(); }

	private:
		inline void start() noexcept;

		ROD_NO_UNIQUE_ADDRESS Rcv _rcv;
		basic_thread_pool *_pool;
	};
	template<typename Snd, typename Rcv, typename Shape, typename Fn>
	class bulk_operation<Snd, Rcv, Shape, Fn>::type
	{
		template<typename, typename, typename>
		friend struct bulk_sender;

		template<typename... Ts>
		using test_nothrow = std::bool_constant<bulk_nothrow<Fn, Shape, Ts...>>;
		using is_throwing = std::negation<value_types_of_t<Snd, env_of_t<Rcv>, test_nothrow, std::conjunction>>;

		using shared_state_t = typename bulk_shared_state<Snd, Rcv, Shape, Fn, is_throwing>::type;
		using receiver_t = typename bulk_receiver<Snd, Rcv, Shape, Fn, is_throwing>::type;
		using connect_state_t = connect_result_t<Snd, receiver_t>;

		template<typename Snd0, typename Fn0>
		type(basic_thread_pool *pool, Snd0 &&snd, Rcv rcv, Shape shape, Fn0 &&fn) : _shared_state(pool, std::move(rcv), shape, std::forward<Fn0>(fn)), _connect_state{connect(std::forward<Snd0>(snd), receiver_t{&_shared_state})} {}

	public:
		type(type &&) = delete;
		type &operator=(type &&) = delete;

		friend constexpr void tag_invoke(start_t, type &op) noexcept { start(op._connect_state); }

	private:
		connect_state_t _connect_state;
		shared_state_t _shared_state;
	};

	class basic_thread_pool
	{
		template<typename, typename, typename, typename, typename>
		friend struct bulk_shared_state;
		template<typename>
		friend struct operation;

		struct worker_t
		{
			using task_queue_t = detail::atomic_queue<operation_base, &operation_base::_next>;

			void start(basic_thread_pool *pool, std::size_t id) noexcept { thread = std::thread{[=]() { pool->worker_main(id); }}; }

			void push(operation_base *node) noexcept
			{
				queue.push(node);
				queue.notify_one();
			}
			bool try_push(operation_base *node) noexcept
			{
				if (void *old = nullptr; queue.head.compare_exchange_strong(old, node, std::memory_order_acq_rel))
				{
					queue.notify_one();
					return true;
				}
				return false;
			}

			void join() noexcept
			{
				if (thread.joinable())
					thread.join();
			}
			void stop() noexcept
			{
				if (thread.joinable())
				{
					queue.terminate();
					queue.notify_one();
				}
			}
			
			task_queue_t queue;
			std::thread thread;
		};

	public:
		/** Initializes thread pool with a default number of threads. */
		ROD_PUBLIC basic_thread_pool();
		/** Initializes thread pool with `size` threads. */
		ROD_PUBLIC basic_thread_pool(std::size_t size);
		ROD_PUBLIC ~basic_thread_pool();

		/** Changes the internal state to stopped and terminates worker threads.
		 * @note After a call to `finish` the thread pool will no longer be dispatching scheduled operations. */
		ROD_PUBLIC void finish() noexcept;

		/** Returns a scheduler used to schedule work to be executed on the thread pool. */
		[[nodiscard]] constexpr scheduler get_scheduler() noexcept;
		/** Returns the number of worker threads managed by the thread pool. */
		[[nodiscard]] std::size_t size() const noexcept { return _workers.size(); }

	protected:
		void stop_all() noexcept { for (auto &worker : _workers) { worker.stop(); } }
		void join_all() noexcept { for (auto &worker : _workers) { worker.join(); } }

		ROD_PUBLIC void worker_main(std::size_t id) noexcept;
		ROD_PUBLIC void schedule(operation_base *node) noexcept;
		ROD_PUBLIC void schedule_bulk(operation_base *nodes, std::size_t n) noexcept;

		std::atomic<std::size_t> _next = {};
		std::vector<worker_t> _workers;
	};

	class sender
	{
		friend class scheduler;

	public:
		using is_sender = std::true_type;

	private:
		using signs_t = completion_signatures<set_value_t(), set_stopped_t()>;
		template<typename Rcv>
		using operation_t = typename operation<Rcv>::type;

		constexpr sender(basic_thread_pool *pool) noexcept : _pool(pool) {}

	public:
		friend constexpr env tag_invoke(get_env_t, const sender &s) noexcept { return {s._pool}; }
		template<decays_to<sender> T, typename E>
		friend constexpr signs_t tag_invoke(get_completion_signatures_t, T &&, E) { return {}; }

		template<decays_to<sender> T, typename Rcv>
		friend constexpr operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(detail::nothrow_decay_copyable<Rcv>::value) { return {s._pool, std::move(rcv)}; }

	private:
		basic_thread_pool *_pool;
	};
	template<typename Snd, typename Shape, typename Fn>
	class bulk_sender<Snd, Shape, Fn>::type
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

		template<typename Snd0, typename Fn0>
		constexpr type(basic_thread_pool *pool, Snd0 &&snd, Shape shape, Fn0 &&fn) noexcept(std::is_nothrow_constructible_v<Snd, Snd0> && std::is_nothrow_constructible_v<Fn, Fn0>)
				: _snd(std::forward<Snd0>(snd)), _fn(std::forward<Fn0>(fn)), _pool(pool), _shape(shape) {}

	public:
		friend constexpr env_of_t<const Snd &> tag_invoke(get_env_t, const type &s) noexcept { return get_env(s._snd); }
		template<decays_to<type> T, typename Env>
		friend constexpr signs_t<T, Env> tag_invoke(get_completion_signatures_t, T &&, Env &&) noexcept { return {}; }

		template<decays_to<type> T, typename Rcv> requires receiver_of<Rcv, signs_t<T, env_of_t<Rcv>>>
		friend operation_t<T, Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv)
		{
			return {s._pool, std::forward<T>(s)._snd, std::forward<Rcv>(rcv), s._shape, std::forward<T>(s)._fn};
		}

	private:
		ROD_NO_UNIQUE_ADDRESS Snd _snd;
		ROD_NO_UNIQUE_ADDRESS Fn _fn;
		basic_thread_pool *_pool;
		Shape _shape;
	};

	class scheduler
	{
		friend class basic_thread_pool;

		template<typename Snd, typename Shape, typename Fn>
		using bulk_sender_t = typename bulk_sender<std::decay_t<Snd>, Shape, Fn>::type;

	public:
		constexpr scheduler(basic_thread_pool *pool) noexcept : _pool(pool) {}

		constexpr bool operator==(const scheduler &) const noexcept = default;

		friend constexpr bool tag_invoke(execute_may_block_caller_t, const scheduler &) noexcept { return false; }
		friend constexpr auto tag_invoke(get_forward_progress_guarantee_t, const scheduler &) noexcept { return forward_progress_guarantee::parallel; }

		template<decays_to<scheduler> T>
		friend constexpr sender tag_invoke(schedule_t, T &&s) noexcept { return s.schedule(); }
		template<decays_to<scheduler> T, typename Snd, typename Shape, typename Fn>
		friend constexpr bulk_sender_t<Snd, Shape, Fn> tag_invoke(bulk_t, T &&s, Snd &&snd, Shape shape, Fn fn) noexcept { return s.schedule_bulk(std::forward<Snd>(snd), shape, std::forward<Fn>(fn)); }

	private:
		auto schedule() noexcept { return sender{_pool}; }
		template<typename Snd, typename Shape, typename Fn>
		auto schedule_bulk(Snd &&snd, Shape shape, Fn &&fn) noexcept { return bulk_sender_t<Snd, Shape, Fn>{std::forward<Snd>(snd), shape, std::forward<Fn>(fn)}; }

		basic_thread_pool *_pool;
	};

	template<typename Rcv>
	void operation<Rcv>::type::start() noexcept { _pool->schedule(this); }
	template<typename Snd, typename Rcv, typename Shape, typename Fn, typename ThrowTag>
	void bulk_shared_state<Snd, Rcv, Shape, Fn, ThrowTag>::type::start() noexcept { pool->schedule_bulk(tasks.data(), required_threads()); }

	constexpr scheduler basic_thread_pool::get_scheduler() noexcept { return {this}; }
}
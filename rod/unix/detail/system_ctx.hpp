/*
 * Created by switchblade on 2023-06-01.
 */

#pragma once

#include "../../detail/adaptors/read_some.hpp"
#include "../../detail/adaptors/write_some.hpp"

#if defined(ROD_HAS_EPOLL) || defined(ROD_HAS_LIBURING)

#include <utility>
#include <thread>
#include <chrono>

#include "../../detail/priority_queue.hpp"
#include "../../detail/atomic_queue.hpp"
#include "../../detail/basic_queue.hpp"

#include "../monotonic_clock.hpp"
#include "file.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::_system_ctx
{
	enum class io_id { read, write, read_at, write_at, /*rcv_data, rcv_err, send_data*/ };
	enum flags_t { stop_possible = 1, stop_requested = 4, dispatched = 8 };

	using clock = monotonic_clock;
	using time_point = typename clock::time_point;

	template<typename Env>
	concept stoppable_env = stoppable_token<stop_token_of_t<Env &>>;

	template<typename Ctx, typename... Args>
	struct operation_base
	{
		using _notify_func_t = void (*)(operation_base *);

		operation_base(operation_base &&) = delete;
		operation_base &operator=(operation_base &&) = delete;

		constexpr operation_base() noexcept = default;

		void _notify(Args... args) noexcept { std::exchange(_notify_func, {})(this, std::move(args)...); }

		_notify_func_t _notify_func = {};
		operation_base *_next = {};
	};
	template<typename Ctx, typename... Args>
	struct timer_node : operation_base<Ctx>
	{
		constexpr timer_node(Ctx &ctx, time_point timeout, bool can_stop) noexcept : _flags(can_stop ? flags_t::stop_possible : 0), _timeout(timeout), _ctx(ctx) {}

		[[nodiscard]] bool stop_possible() const noexcept { return _flags.load(std::memory_order_relaxed) & flags_t::stop_possible; }

		timer_node *_timer_prev = {};
		timer_node *_timer_next = {};
		std::atomic<int> _flags = {};
		time_point _timeout = {};
		Ctx &_ctx;
	};

	struct iov_buff
	{
		void *data;
		std::size_t size;
	};

	template<typename Op = void, typename Ctx = void>
	struct io_cmd;
	template<>
	struct io_cmd<void>
	{
		constexpr io_cmd(int fd, void *data, std::size_t size, std::ptrdiff_t off = -1) noexcept : fd(fd), buff{data, size}, off(off) {}

		detail::basic_descriptor fd;
		iov_buff buff;
		union
		{
			std::ptrdiff_t off;
			std::ptrdiff_t ret;
		};
	};

	template<>
	struct io_cmd<schedule_read_some_t> : io_cmd<>
	{
		constexpr static auto id = io_id::read;

		template<decays_to<std::byte> T>
		io_cmd(int fd, std::span<T> s) noexcept : io_cmd<>(fd, (void *) s.data(), s.size()) {}
	};
	template<>
	struct io_cmd<schedule_write_some_t> : io_cmd<>
	{
		constexpr static auto id = io_id::write;

		template<decays_to<std::byte> T>
		io_cmd(int fd, std::span<T> s) noexcept : io_cmd<>(fd, (void *) s.data(), s.size()) {}
	};
	template<>
	struct io_cmd<schedule_read_some_at_t> : io_cmd<>
	{
		constexpr static auto id = io_id::read_at;

		template<decays_to<std::byte> T>
		io_cmd(int fd, std::ptrdiff_t pos, std::span<T> s) noexcept : io_cmd<>(fd, (void *) s.data(), s.size(), pos) {}
	};
	template<>
	struct io_cmd<schedule_write_some_at_t> : io_cmd<>
	{
		constexpr static auto id = io_id::write_at;

		template<decays_to<std::byte> T>
		io_cmd(int fd, std::ptrdiff_t pos, std::span<T> s) noexcept : io_cmd<>(fd, (void *) s.data(), s.size(), pos) {}
	};

	template<typename Ctx>
	struct scheduler { struct type; };

	template<typename Ctx>
	struct sender { struct type; };
	template<typename Ctx>
	struct timer_sender { struct type; };
	template<typename Op, typename Ctx>
	struct io_sender { struct type; };

	template<typename Ctx, typename Rcv>
	struct operation { struct type; };
	template<typename Ctx, typename Rcv>
	struct timer_operation { struct type; };
	template<typename Op, typename Ctx, typename Rcv>
	struct io_operation { struct type; };

	/* IO operations inherit from `operation_base` twice in order to
	 * enable cancellation in the middle of a queued IO operation. */
	template<typename Ctx>
	struct complete_base : operation_base<Ctx> {};
	template<typename Ctx>
	struct stop_base : operation_base<Ctx> {};

	template<typename Ctx>
	struct env { Ctx *_ctx; };

	template<typename, auto>
	struct stop_cb {};
	template<typename Env, typename Op, void (Op::*Stop)() noexcept> requires stoppable_env<Env>
	struct stop_cb<Env, Stop>
	{
		struct callback
		{
			void operator()() const noexcept { (op->*Stop)(); }
			Op *op;
		};

		constexpr void init(auto &&env, Op *ptr) noexcept { data.emplace(get_stop_token(env), callback{ptr}); }
		constexpr void reset() noexcept { data.reset(); }

		std::optional<stop_callback_for_t<stop_token_of_t<Env &>, callback>> data;
	};

	template<typename Ctx, typename Rcv>
	struct operation<Ctx, Rcv>::type : operation_base<Ctx>
	{
		static void _bind_notify(operation_base<Ctx> *ptr) noexcept
		{
			auto &op = *static_cast<type *>(ptr);

			/* Stop the top-level receiver if necessary. */
			if constexpr (stoppable_env<env_of_t<Rcv>>)
				if (get_stop_token(get_env(op._rcv)).stop_requested())
				{
					set_stopped(std::move(op._rcv));
					return;
				}

			/* Complete the top-level receiver. */
			set_value(std::move(op._rcv));
		}

		template<typename Rcv1>
		constexpr type(Ctx &ctx, Rcv1 &&rcv) : _rcv(std::forward<Rcv1>(rcv)), _ctx(ctx) { operation_base<Ctx>::_notify_func = _bind_notify; }

		friend void tag_invoke(start_t, type &op) noexcept { op._start(); }

		void _start() noexcept
		{
			std::error_code err = {};
			_ctx.schedule(this, err);
			if (err) [[unlikely]] set_error(std::move(_rcv), err);
		}

		[[ROD_NO_UNIQUE_ADDRESS]] Rcv _rcv;
		Ctx &_ctx;
	};
	template<typename Ctx, typename Rcv>
	struct timer_operation<Ctx, Rcv>::type : timer_node<Ctx>
	{
		static void _notify_value(operation_base<Ctx> *ptr) noexcept { static_cast<type *>(ptr)->_complete_value(); }
		static void _notify_stopped(operation_base<Ctx> *ptr) noexcept { static_cast<type *>(ptr)->_complete_stopped(); }

		constexpr type(Ctx &ctx, time_point timeout, Rcv rcv) noexcept(std::is_nothrow_move_constructible_v<Rcv>) : timer_node<Ctx>(ctx, timeout, get_stop_token(get_env(rcv)).stop_possible()), _rcv(std::move(rcv)) {}

		friend void tag_invoke(start_t, type &op) noexcept { op._start(); }

		void _complete_value() noexcept
		{
			/* Handle spontaneous stop requests. */
			if constexpr (stoppable_env<env_of_t<Rcv>>)
			{
				_stop_cb.reset();
				if (get_stop_token(get_env(_rcv)).stop_requested())
				{
					_complete_stopped();
					return;
				}
			}
			set_value(std::move(_rcv));
		}
		void _complete_stopped() noexcept
		{
			if constexpr (stoppable_env<env_of_t<Rcv>>)
				set_stopped(std::move(_rcv));
			else
				std::terminate();
		}

		void _start() noexcept
		{
			if (timer_node<Ctx>::_ctx.is_consumer_thread())
				_start_consumer();
			else
			{
				std::error_code err = {};
				operation_base<Ctx>::_notify_func = [](operation_base<Ctx> *p) noexcept { static_cast<type *>(p)->_start_consumer(); };
				timer_node<Ctx>::_ctx.schedule_producer(this, err);
				if (err) [[unlikely]] set_error(std::move(_rcv), err);
			}
		}
		void _start_consumer() noexcept
		{
			/* Bail if a stop has already been requested. */
			if constexpr (stoppable_env<env_of_t<Rcv>>)
				if (get_stop_token(get_env(_rcv)).stop_requested())
				{
					operation_base<Ctx>::_notify_func = _notify_stopped;
					timer_node<Ctx>::_ctx.schedule_consumer(this);
					return;
				}

			operation_base<Ctx>::_notify_func = _notify_value;
			timer_node<Ctx>::_ctx.add_timer(this);

			/* Initialize the stop callback for stoppable environments. */
			if constexpr (stoppable_env<env_of_t<Rcv>>)
				_stop_cb.init(get_env(_rcv), this);
		}

		void _request_stop() noexcept
		{
			if constexpr (!stoppable_env<env_of_t<Rcv>>)
				std::terminate();
			else if (timer_node<Ctx>::_ctx.is_consumer_thread())
			{
				_stop_cb.reset();
				operation_base<Ctx>::_notify_func = _notify_stopped;
				if (!(timer_node<Ctx>::_flags.load(std::memory_order_relaxed) & flags_t::dispatched))
				{
					/* Timer has not yet been dispatched, schedule stop completion. */
					timer_node<Ctx>::_ctx.del_timer(this);
					timer_node<Ctx>::_ctx.schedule_consumer(this);
				}
			}
			else if (!(timer_node<Ctx>::_flags.fetch_or(flags_t::stop_requested, std::memory_order_acq_rel) & flags_t::dispatched))
			{
				/* Timer has not yet been dispatched, schedule stop completion on producer thread. */
				operation_base<Ctx>::_notify_func = [](operation_base<Ctx> *ptr) noexcept
				{
					const auto op = static_cast<type *>(ptr);
					op->_stop_cb.reset();

					/* Make sure to erase the timer if it has not already been dispatched. */
					if (!(op->_flags.load(std::memory_order_relaxed) & flags_t::dispatched))
						op->_ctx.del_timer(op);

					op->_complete_stopped();
				};

				std::error_code err = {};
				timer_node<Ctx>::_ctx.schedule_producer(this, err);
				if (err) [[unlikely]] std::terminate();
			}
		}

		using _stop_cb_t = stop_cb<env_of_t<Rcv>, &type::_request_stop>;

		[[ROD_NO_UNIQUE_ADDRESS]] _stop_cb_t _stop_cb;
		[[ROD_NO_UNIQUE_ADDRESS]] Rcv _rcv;
	};

	template<typename Ctx>
	struct sender<Ctx>::type
	{
		using is_sender = std::true_type;

		template<typename Rcv>
		using _operation_t = typename operation<Ctx, std::decay_t<Rcv>>::type;

		template<typename Env>
		using _stop_signs_t = std::conditional_t<stoppable_env<Env>, completion_signatures<set_stopped_t()>, completion_signatures<>>;
		template<typename Env>
		using _signs_t = detail::concat_tuples_t<completion_signatures<set_value_t(), set_error_t(std::error_code)>, _stop_signs_t<Env>>;

		friend constexpr env<Ctx> tag_invoke(get_env_t, const type &s) noexcept { return {&s._ctx}; }
		template<decays_to<type> T, typename Env>
		friend constexpr _signs_t<Env> tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }

		template<decays_to<type> T, typename Rcv>
		friend constexpr _operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<std::decay_t<Rcv>, Rcv>)
		{
			static_assert(receiver_of<Rcv, _signs_t<env_of_t<Rcv>>>);
			return _operation_t<Rcv>{s._ctx, std::forward<Rcv>(rcv)};
		}

		Ctx &_ctx;
	};
	template<typename Ctx>
	struct timer_sender<Ctx>::type
	{
		using is_sender = std::true_type;

		template<typename Rcv>
		using _operation_t = typename timer_operation<Ctx, std::decay_t<Rcv>>::type;

		template<typename Env>
		using _stop_signs_t = std::conditional_t<stoppable_env<Env>, completion_signatures<set_stopped_t()>, completion_signatures<>>;
		template<typename Env>
		using _signs_t = detail::concat_tuples_t<completion_signatures<set_value_t(), set_error_t(std::error_code)>, _stop_signs_t<Env>>;

		friend constexpr env<Ctx> tag_invoke(get_env_t, const type &s) noexcept { return {&s._ctx}; }
		template<decays_to<type> T, typename Env>
		friend constexpr _signs_t<Env> tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }

		template<decays_to<type> T, typename Rcv>
		friend constexpr _operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<std::decay_t<Rcv>, Rcv>)
		{
			static_assert(receiver_of<Rcv, _signs_t<env_of_t<Rcv>>>);
			return _operation_t<Rcv>{s._ctx, s._tp, std::forward<Rcv>(rcv)};
		}

		time_point _tp;
		Ctx &_ctx;
	};

	template<typename Op, typename Ctx>
	struct io_sender<Op, Ctx>::type
	{
		using is_sender = std::true_type;
		using _signs_t = completion_signatures<set_value_t(std::size_t), set_error_t(std::error_code), set_stopped_t()>;
		template<typename Rcv>
		using _operation_t = typename io_operation<Op, Ctx, std::decay_t<Rcv>>::type;

		template<typename... Args>
		type(Ctx &ctx, Args &&...args) noexcept : _cmd(std::forward<Args>(args)...), _ctx(ctx) {}

		friend env<Ctx> tag_invoke(get_env_t, const type &s) noexcept { return {&s._ctx}; }
		template<decays_to<type> T, typename Env>
		friend _signs_t tag_invoke(get_completion_signatures_t, T &&, Env) noexcept { return {}; }
		template<decays_to<type> T, typename Rcv>
		friend _operation_t<Rcv> tag_invoke(connect_t, T &&s, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<std::decay_t<Rcv>, Rcv>)
		{
			static_assert(receiver_of<Rcv, _signs_t>);
			return _operation_t<Rcv>{s._ctx, std::forward<Rcv>(rcv), s._cmd};
		}

		io_cmd<Op, Ctx> _cmd;
		Ctx &_ctx;
	};

	template<typename Ctx>
	struct scheduler<Ctx>::type
	{
		using _sender_t = typename _system_ctx::sender<Ctx>::type;
		using _timer_sender_t = typename _system_ctx::timer_sender<Ctx>::type;
		template<typename Op>
		using _io_sender_t = typename io_sender<Op, Ctx>::type;

		friend constexpr auto tag_invoke(get_forward_progress_guarantee_t, const type &) noexcept { return forward_progress_guarantee::weakly_parallel; }
		friend constexpr bool tag_invoke(execute_may_block_caller_t, const type &) noexcept { return false; }

		template<decays_to<type> T>
		friend constexpr auto tag_invoke(schedule_t, T &&s) noexcept { return _sender_t{*s._ctx}; }
		template<decays_to<type> T, decays_to<time_point> TP>
		friend constexpr auto tag_invoke(schedule_at_t, T &&s, TP &&tp) noexcept { return _timer_sender_t{std::forward<TP>(tp), *s._ctx}; }
		template<decays_to<type> T, typename Dur>
		friend constexpr auto tag_invoke(schedule_in_t, T &&s, Dur &&dur) noexcept { return schedule_at(std::forward<T>(s), s.now() + dur); }

		template<typename Buff>
		friend auto tag_invoke(schedule_read_some_t, type sch, int fd, Buff &&buff) noexcept
		{
			return _io_sender_t<schedule_read_some_t>{*sch._ctx, fd, buff};
		}
		template<typename Buff>
		friend auto tag_invoke(schedule_write_some_t, type sch, int fd, Buff &&buff) noexcept
		{
			return _io_sender_t<schedule_write_some_t>{*sch._ctx, fd, buff};
		}
		template<std::convertible_to<std::ptrdiff_t> Pos, typename Buff>
		friend auto tag_invoke(schedule_read_some_at_t, type sch, int fd, Pos pos, Buff &&buff) noexcept
		{
			return _io_sender_t<schedule_read_some_at_t>{*sch._ctx, fd, static_cast<std::ptrdiff_t>(pos), buff};
		}
		template<std::convertible_to<std::ptrdiff_t> Pos, typename Buff>
		friend auto tag_invoke(schedule_write_some_at_t, type sch, int fd, Pos pos, Buff &&buff) noexcept
		{
			return _io_sender_t<schedule_write_some_at_t>{*sch._ctx, fd, static_cast<std::ptrdiff_t>(pos), buff};
		}

		template<typename Snd, typename Dst>
		friend decltype(auto) tag_invoke(async_read_some_t, type sch, Snd &&snd, int fd, Dst &&dst)
		{
			return let_value(std::forward<Snd>(snd), [sch, fd, dst = std::forward<Dst>(dst)]()
			{
				return schedule_read_some(sch, fd, std::move(dst));
			});
		}
		template<typename Snd, typename Src>
		friend decltype(auto) tag_invoke(async_write_some_t, type sch, Snd &&snd, int fd, Src &&src)
		{
			return let_value(std::forward<Snd>(snd), [sch, fd, src = std::forward<Src>(src)]()
			{
				return schedule_write_some(sch, fd, std::move(src));
			});
		}
		template<typename Snd, std::convertible_to<std::ptrdiff_t> Pos, typename Dst>
		friend decltype(auto) tag_invoke(async_read_some_at_t, type sch, Snd &&snd, int fd, Pos pos, Dst &&dst)
		{
			return let_value(std::forward<Snd>(snd), [sch, fd, pos, dst = std::forward<Dst>(dst)]()
			{
				return schedule_read_some_at(sch, fd, pos, std::move(dst));
			});
		}
		template<typename Snd, std::convertible_to<std::ptrdiff_t> Pos, typename Src>
		friend decltype(auto) tag_invoke(async_write_some_at_t, type sch, Snd &&snd, int fd, Pos pos, Src &&src)
		{
			return let_value(std::forward<Snd>(snd), [sch, fd, pos, src = std::forward<Src>(src)]()
			{
				return schedule_write_some_at(sch, fd, pos, std::move(src));
			});
		}

		/** Returns the current time point of the clock used by the context. */
		[[nodiscard]] time_point now() const noexcept { return clock::now(); }

		constexpr bool operator==(const type &) const noexcept = default;

		Ctx *_ctx;
	};

	template<typename Ctx>
	constexpr auto tag_invoke(get_stop_token_t, const env<Ctx> &e) noexcept { return e._ctx->get_stop_token(); }
	template<typename Ctx, typename T>
	constexpr auto tag_invoke(get_completion_scheduler_t<T>, const env<Ctx> &e) noexcept { return typename scheduler<Ctx>::type{e._ctx}; }
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

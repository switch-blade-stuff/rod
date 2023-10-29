/*
 * Created by switchblade on 2023-09-29.
 */

#include "run_loop.hpp"

namespace rod::_run_loop
{
	bool run_loop::run_once(thread_context &ctx)
	{
		while (!ctx.local_queue.empty())
		{
			auto *node = ctx.local_queue.pop_front();
			node->_notify_func(node);
		}
		ctx.lock = std::unique_lock(_mtx);
		return !(_is_stopping && is_idle());
	}
	bool run_loop::poll(thread_context &ctx, bool block)
	{
		auto g = std::unique_lock(std::move(ctx.lock));

		/* Acquire tasks from the producer queue. */
		while (!_task_queue.empty())
		{
			auto *node = _task_queue.pop_front();
			ctx.local_queue.push_back(node);
		}

		/* Dispatch elapsed timers. */
		if (!_timer_queue.empty())
			for (const auto now = clock::now(); !_timer_queue.empty();)
			{
				if (_timer_queue.front()->_tp > now)
				{
					_next_timeout = _timer_queue.front()->_tp;
					break;
				}
				auto *node = _timer_queue.pop_front();
				ctx.local_queue.push_back(node);
			}

		if (ctx.local_queue.empty() && block)
		{
			if (const auto wait_pred = [&]() noexcept { return !is_idle() || _is_stopping; }; !_timer_queue.empty())
				_cnd.wait_until(ctx.lock, _next_timeout, wait_pred);
			else
				_cnd.wait(ctx.lock, wait_pred);
			return true;
		}
		return false;
	}

	void run_loop::run()
	{
		thread_context ctx;
		while (run_once(ctx))
			poll(ctx);
	}
	void run_loop::finish()
	{
		const auto g = std::unique_lock{_mtx};
		_is_stopping = true;
		_cnd.notify_all();
	}

	void run_loop::schedule(operation_base *node) noexcept
	{
		const auto g = std::unique_lock{_mtx};
		_task_queue.push_back(node);
		_cnd.notify_one();
	}
	void run_loop::schedule_timer(timer_operation_base *node) noexcept
	{
		const auto g = std::unique_lock{_mtx};
		_timer_queue.insert(node);
		_cnd.notify_one();
	}
}
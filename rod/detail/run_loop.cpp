/*
 * Created by switchblade on 2023-09-29.
 */

#include "run_loop.hpp"

namespace rod::_run_loop
{
	std::size_t run_loop::run_once(thread_context &ctx)
	{
		std::size_t notified = 0;
		for (; !ctx._local_queue.empty(); ++notified)
		{
			auto *node = ctx._local_queue.pop_front();
			node->_notify_func(node);
		}
		return notified;
	}
	std::size_t run_loop::poll(thread_context &ctx, bool block)
	{
		auto g = std::unique_lock(_mtx);
		std::size_t acquired = 0;

		/* Acquire tasks from the producer queue. */
		for (; !_task_queue.empty(); ++acquired)
		{
			auto *node = _task_queue.pop_front();
			ctx._local_queue.push_back(node);
		}

		/* Dispatch elapsed timers. */
		if (!_timer_queue.empty())
			for (const auto now = clock::now(); !_timer_queue.empty(); ++acquired)
			{
				if (_timer_queue.front()->_tp > now)
				{
					_next_timeout = _timer_queue.front()->_tp;
					break;
				}
				auto *node = _timer_queue.pop_front();
				ctx._local_queue.push_back(node);
			}

		if (ctx._local_queue.empty() && block)
		{
			if (const auto wait_condition = [&]() noexcept { return !_task_queue.empty() || !_timer_queue.empty() || !ctx.active() || !active(); }; !_timer_queue.empty())
				_cnd.wait_until(g, _next_timeout, wait_condition);
			else
				_cnd.wait(g, wait_condition);
		}
		return acquired;
	}

	void run_loop::run_ctx(thread_context &ctx)
	{
		for (std::size_t pending = 0;;)
		{
			/* Skip execution if there are no scheduled local queue operations. */
			if (pending != 0) [[likely]]
				pending -= run_once(ctx);
			if (!ctx.active() || !active())
				break;

			/* Block only if there are no scheduled local queue operations, pending tasks or timers. */
			pending += poll(ctx, pending == 0);
		}
	}
	void run_loop::pop_ctx(thread_context *ctx)
	{
		const auto g = std::unique_lock(_mtx);
		if ((*ctx->_this_ptr = ctx->_next_node) != nullptr)
			ctx->_next_node->_this_ptr = ctx->_this_ptr;
	}
	void run_loop::push_ctx(thread_context *ctx)
	{
		const auto g = std::unique_lock(_mtx);
		if (auto old = std::exchange(_ctx_queue, ctx); old != nullptr)
		{
			old->_this_ptr = &ctx->_next_node;
			ctx->_next_node = old;
		}
		ctx->_this_ptr = &_ctx_queue;
	}

	void run_loop::schedule(operation_base *node)
	{
		{
			const auto g = std::unique_lock(_mtx);
			_task_queue.push_back(node);
		}
		_cnd.notify_one();
	}
	void run_loop::schedule_timer(timer_operation_base *node)
	{
		{
			const auto g = std::unique_lock(_mtx);
			_timer_queue.insert(node);
		}
		_cnd.notify_one();
	}
}
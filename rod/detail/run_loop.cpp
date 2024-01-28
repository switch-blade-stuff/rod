/*
 * Created by switchblade on 2023-09-29.
 */

#include "run_loop.hpp"

namespace rod::_run_loop
{
	void run_loop::acquire_elapsed_timers() noexcept
	{
		for (const auto now = clock::now();;)
		{
			if (_timer_queue.empty())
			{
				_next_timeout = {};
				break;
			}
			if (_timer_queue.front()->_tp > now)
			{
				_next_timeout = _timer_queue.front()->_tp;
				break;
			}
			_consumer_queue.push_back(_timer_queue.pop_front());
		}
	}
	void run_loop::acquire_producer_queue() noexcept
	{
		if (_producer_queue.empty())
			return;
		_consumer_queue.merge_back(std::move(_producer_queue));
	}

	std::size_t run_loop::run_once()
	{
		const auto g = std::unique_lock(_consumer_lock);
		std::size_t notified = 0;

		for (auto queue = std::move(_consumer_queue); !queue.empty(); ++notified)
			queue.pop_front()->notify();

		return notified;
	}
	std::size_t run_loop::poll(bool block)
	{
		std::lock(_consumer_lock, _mtx);
		auto g_tid = std::unique_lock(_consumer_lock, std::adopt_lock);
		auto g_mtx = std::unique_lock(_mtx, std::adopt_lock);

		if (!has_pending() && block) [[unlikely]]
			for (;;)
			{
				const auto wait_condition = [&]() { return has_pending() || !active(); };
				auto old_timeout = _next_timeout;
				bool has_timeout = false;

				if (!_timer_queue.empty())
					 has_timeout = !_cnd.wait_until(g_mtx, old_timeout, wait_condition);
				else
					_cnd.wait(g_mtx, wait_condition);

				if (has_timeout && _next_timeout < old_timeout)
					old_timeout = _next_timeout;
				else
					break;
			}

		acquire_elapsed_timers();
		acquire_producer_queue();
		return _consumer_queue.size;
	}

	void run_loop::schedule(operation_base *node)
	{
		if (is_consumer_thread())
		{
			_consumer_queue.push_back(node);
			return;
		}

		{
			const auto g = std::unique_lock(_mtx);
			_producer_queue.push_back(node);
		}
		_cnd.notify_one();
	}
	void run_loop::schedule_timer(timer_operation_base *node)
	{
		if (is_consumer_thread() && node->_tp <= clock::now())
		{
			_consumer_queue.push_back(node);
			return;
		}

		{
			const auto g = std::unique_lock(_mtx);
			if (node->_tp <= _next_timeout)
				_next_timeout = node->_tp;
			_timer_queue.insert(node);
		}
		_cnd.notify_one();
	}
}
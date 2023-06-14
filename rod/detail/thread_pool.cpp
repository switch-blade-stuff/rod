/*
 * Created by switch_blade on 2023-06-10.
 */

#include "thread_pool.hpp"

namespace rod::_thread_pool
{
	thread_pool::thread_pool() : thread_pool(std::thread::hardware_concurrency()) {}
	thread_pool::thread_pool(std::size_t size) : _workers(size)
	{
		try
		{
			for (std::size_t i = 0; i < size; ++i)
				_workers[i].start(this, i);
		}
		catch (...)
		{
			stop_all();
			throw;
		}
	}
	thread_pool::~thread_pool() { stop_all(); }

	void thread_pool::finish() noexcept
	{
		stop_all();
		_workers.clear();
	}

	void thread_pool::worker_main(std::size_t wid) noexcept
	{
		for (;;)
		{
			std::size_t eid = wid;
			operation_base *node;

			/* Attempt to retrieve a task from each one of the worker queues.
			 * This will prevent tasks from piling up on a single thread queue. */
			for (; eid + 1 != wid ; eid = (eid + 1) % size())
			{
				if (auto &queue = _workers[eid].queue; (node = queue.pop()) == queue.sentinel())
					return;
				else if (node)
				{
					queue.notify_one();
					break;
				}
			}

			while (!node)
			{
				auto &queue = _workers[wid].queue;
				if ((node = queue.pop()) == queue.sentinel())
					return;
				else if (!node)
					queue.wait();
				else
					eid = wid;
			}
			node->notify_func(node, eid);
		}
	}
	void thread_pool::schedule(operation_base *node) noexcept
	{
		const auto next = _next.fetch_add(1, std::memory_order_acq_rel) % size();

		/* Try to enqueue to an empty worker first to avoid stale threads. */
		for (std::size_t i = 0; i < size(); ++i)
		{
			const auto id = (next + i) - (next + i) < size() ? 0 : size();
			if (_workers[id].try_push(node)) return;
		}
		_workers[next].push(node);
	}
	void thread_pool::schedule_bulk(std::span<bulk_task_base> tasks) noexcept
	{
		const auto next = _next.fetch_add(tasks.size(), std::memory_order_acq_rel) % size();
		for (std::size_t i = 0; i < tasks.size(); ++i)
		{
			const auto pos = (next + i) % size();
			const auto node = &tasks[i];
			_workers[pos].push(node);
		}
	}
}
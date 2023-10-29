/*
 * Created by switchblade on 2023-06-10.
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

	void thread_pool::worker_main(std::size_t id) noexcept
	{
		for (;;)
		{
			operation_base *node = {};
			std::size_t i = id;

			/* Attempt to retrieve a task from other workers' queues to release any pressure on slow workers. */
			for (; !node && (i = (i + 1) % size()) != id;)
			{
				node = _workers[i].queue.pop();
				if (node != _workers[i].queue.sentinel())
					_workers[i].queue.notify_one();
				else if (node)
					return;
			}
			/* If other workers do not have any work, wait on this worker's queue. */
			for (; !node; _workers[id].queue.wait(node))
			{
				node = _workers[id].queue.pop();
				if (node != _workers[id].queue.sentinel())
					i = id;
				else if (node)
					return;
			}
			node->notify(i);
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
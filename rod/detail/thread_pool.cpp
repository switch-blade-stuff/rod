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

	void thread_pool::worker_main(std::size_t id) noexcept
	{
		for (operation_base *node;;)
		{
			/* Attempt to retrieve a task from each one of the worker queues.
			 * This will prevent tasks from piling up on a single thread queue. */
			for (std::size_t i = id; i + 1 != id ; i = (i + 1) % size())
			{
				auto &worker = _workers[i];
				if ((node = worker.try_pop()))
					break;
			}

			if (node || (node = _workers[id].pop()))
				node->notify(id);
			else
				break;
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
	void thread_pool::schedule_bulk(operation_base *nodes, std::size_t n) noexcept
	{
		for (std::size_t i = 0; i < n; ++i) _workers[i % size()].push(nodes + i);
	}
}
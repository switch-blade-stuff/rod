/*
 * Created by switch_blade on 2023-06-10.
 */

#include "thread_pool.hpp"

namespace rod::_thread_pool
{
	basic_thread_pool::basic_thread_pool() : basic_thread_pool(std::thread::hardware_concurrency()) {}
	basic_thread_pool::basic_thread_pool(std::size_t size) : _workers(size)
	{
		try
		{
			for (std::size_t i = 0; i < size; ++i)
				_workers[i].start(this, i);
		}
		catch (...)
		{
			stop_all();
			join_all();
			throw;
		}
	}
	basic_thread_pool::~basic_thread_pool()
	{
		stop_all();
		join_all();
	}

	void basic_thread_pool::finish() noexcept
	{
		stop_all();
		join_all();
		_workers.clear();
	}

	void basic_thread_pool::worker_main(std::size_t id) noexcept
	{
		for (operation_base *node;;)
		{
			/* Attempt to retrieve a task from each one of the worker queues.
			 * This will prevent tasks from piling up on a single thread queue. */
			for (std::size_t i = id; i + 1 != id ; i = (i + 1) % size())
			{
				auto &queue = _workers[i].queue;
				if (node = queue.pop(); node == queue.sentinel())
					return;
				else if (node)
					break;
			}

			if (!node)
				_workers[id].queue.wait();
			else
				node->notify(id);
		}
	}
	void basic_thread_pool::schedule(operation_base *node) noexcept
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
	void basic_thread_pool::schedule_bulk(operation_base *nodes, std::size_t n) noexcept
	{
		for (std::size_t i = 0; i < n; ++i) _workers[i % size()].push(nodes + i);
	}
}
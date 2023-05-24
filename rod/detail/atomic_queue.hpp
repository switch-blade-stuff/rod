/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include <atomic>

#include "basic_queue.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	template<typename Node, auto *Node::*Next>
	struct atomic_queue
	{
		void terminate(bool notify = true) noexcept
		{
			head.store(sentinel(), std::memory_order_release);
			if (notify) notify_all();
		}
		bool try_terminate(bool notify = true) noexcept
		{
			if (void *old = nullptr; head.compare_exchange_strong(old, sentinel(), std::memory_order_acq_rel))
			{
				if (notify) notify_all();
				return true;
			}
			return false;
		}

		bool push(Node *node, bool notify = true) noexcept
		{
			for (auto ptr = head.load(std::memory_order_relaxed);;)
			{
				node->*Next = (ptr == sentinel() ? nullptr : static_cast<Node *>(ptr));
				if (head.compare_exchange_weak(ptr, node, std::memory_order_acq_rel, std::memory_order_relaxed))
				{
					if (notify) notify_one();
					return ptr == sentinel();
				}
			}
		}
		[[nodiscard]] Node *pop(bool block = true, bool notify = true) noexcept
		{
			for (auto ptr = head.load(std::memory_order_relaxed);;)
			{
				if (ptr == sentinel() || (!ptr && !block))
					return nullptr;
				else if (!ptr)
				{
					wait(ptr);
					ptr = head.load(std::memory_order_relaxed);
					continue;
				}

				const auto node = static_cast<Node *>(ptr);
				if (head.compare_exchange_weak(ptr, node->*Next, std::memory_order_acq_rel, std::memory_order_relaxed))
				{
					if (notify) notify_one();
					node->*Next = {};
					return node;
				}
			}
		}

		void notify_one() noexcept { head.notify_one(); }
		void notify_all() noexcept { head.notify_all(); }
		void wait(void *old = nullptr) noexcept { head.wait(old); }
		[[nodiscard]] void *sentinel() const noexcept { return const_cast<atomic_queue *>(this); }

		/* Convert to a basic queue with `head` becoming the tail of the new queue. */
		[[nodiscard]] explicit operator basic_queue<Node, Next>() && noexcept
		{
			auto node = static_cast<Node *>(head.exchange({}, std::memory_order_acq_rel));
			basic_queue<Node, Next> result;
			result.tail = node;
			while (node)
			{
				const auto next = std::exchange(node->*Next, result.head);
				result.head = std::exchange(node, next);
			}
			return result;
		}

		std::atomic<void *> head = {};
	};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include <atomic>

#include "basic_queue.hpp"

namespace rod::_detail
{
	template<typename Node, Node *Node::*Next>
	struct atomic_queue
	{
		void activate() noexcept { head.store(nullptr, std::memory_order_release); }
		bool try_activate() noexcept
		{
			void *old = sentinel();
			return head.compare_exchange_strong(old, nullptr, std::memory_order_acq_rel);
		}

		void terminate() noexcept { head.store(sentinel(), std::memory_order_release); }
		bool try_terminate() noexcept
		{
			void *old = nullptr;
			return head.compare_exchange_strong(old, sentinel(), std::memory_order_acq_rel);
		}

		[[nodiscard]] bool empty() const noexcept { return head.load(std::memory_order_acquire) == nullptr; }
		[[nodiscard]] bool active() const noexcept { return head.load(std::memory_order_acquire) != sentinel(); }

		[[nodiscard]] Node *front() const noexcept { return static_cast<Node *>(head.load(std::memory_order_acquire)); }

		bool push(Node *node) noexcept
		{
			for (auto ptr = head.load(std::memory_order_relaxed);;)
			{
				node->*Next = (ptr == sentinel() ? nullptr : static_cast<Node *>(ptr));
				if (head.compare_exchange_weak(ptr, node, std::memory_order_acq_rel, std::memory_order_relaxed))
					return ptr == sentinel();
			}
		}
		[[nodiscard]] Node *pop() noexcept
		{
			for (auto ptr = head.load(std::memory_order_relaxed);;)
			{
				if (const auto node = static_cast<Node *>(ptr); !ptr || ptr == sentinel())
					return node;
				else if (head.compare_exchange_weak(ptr, node->*Next, std::memory_order_acq_rel, std::memory_order_relaxed))
				{
					node->*Next = {};
					return node;
				}
			}
		}

		void notify_one() noexcept { head.notify_one(); }
		void notify_all() noexcept { head.notify_all(); }
		void wait(void *old = nullptr) noexcept { head.wait(old); }
		[[nodiscard]] void *sentinel() const noexcept { return const_cast<atomic_queue *>(this); }

		[[nodiscard]] operator basic_queue<Node, Next>() && noexcept
		{
			auto front = static_cast<Node *>(head.exchange({}, std::memory_order_acq_rel));
			auto queue = basic_queue<Node, Next>();

			for (Node *node = front, *next; node != nullptr; node = next)
			{
				next = std::exchange(node->*Next, nullptr);
				queue.push_back(node);
			}
			return queue;
		}

		std::atomic<void *> head = {};
	};
}

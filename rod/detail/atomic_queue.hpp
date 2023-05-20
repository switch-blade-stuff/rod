/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include <atomic>

#include "../utility.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::detail
{
	inline constexpr struct next_t
	{
		template<typename V>
		constexpr decltype(auto) operator()(auto &node, V &&val) const { return tag_invoke(*this, node, std::forward<V>(val)); }
		constexpr decltype(auto) operator()(auto &node) const { return tag_invoke(*this, node); }
	} node_next = {};

	template<typename Node>
	struct atomic_queue
	{
		void terminate() noexcept
		{
			head.store(sentinel(), std::memory_order_release);
			notify_all();
		}
		bool push(Node *node) noexcept
		{
			for (auto ptr = head.load(std::memory_order_relaxed);;)
			{
				if (ptr == sentinel()) return false;

				node_next(*node, static_cast<Node *>(ptr));
				if (head.compare_exchange_weak(ptr, node, std::memory_order_acq_rel, std::memory_order_relaxed))
				{
					notify_one();
					return true;
				}
			}
		}
		[[nodiscard]] Node *pop() noexcept
		{
			for (auto ptr = head.load(std::memory_order_relaxed);;)
			{
				if (ptr == sentinel())
					return nullptr;
				else if (!ptr)
				{
					wait(ptr);
					ptr = head.load(std::memory_order_relaxed);
					continue;
				}

				const auto next = node_next(*static_cast<Node *>(ptr));
				if (head.compare_exchange_weak(ptr, next, std::memory_order_acq_rel, std::memory_order_relaxed))
				{
					head.notify_one();
					return static_cast<Node *>(ptr);
				}
			}
		}

		void notify_one() noexcept { head.notify_one(); }
		void notify_all() noexcept { head.notify_all(); }
		void wait(void *old = nullptr) noexcept { head.wait(old); }
		[[nodiscard]] void *sentinel() const noexcept { return const_cast<atomic_queue *>(this); }

		std::atomic<void *> head = {};
	};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

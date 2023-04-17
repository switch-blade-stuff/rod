/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include <atomic>

#include "utility.hpp"

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
		void notify_one() noexcept { head.notify_one(); }
		void notify_all() noexcept { head.notify_all(); }
		void wait(void *old = nullptr) noexcept { head.wait(old); }

		void terminate() noexcept
		{
			head = this;
			notify_all();
		}
		bool push(Node *node) noexcept
		{
			for (auto candidate = head.load(std::memory_order_relaxed);;)
			{
				if (candidate == this) return false;

				node_next(*node, static_cast<Node *>(candidate));
				if (head.compare_exchange_weak(candidate, node, std::memory_order_acq_rel))
				{
					notify_one();
					return true;
				}
			}
		}
		[[nodiscard]] Node *pop() noexcept
		{
			for (auto candidate = head.load(std::memory_order_relaxed);;)
			{
				if (candidate == this)
					return nullptr;
				else if (!candidate)
				{
					wait(candidate);
					continue;
				}

				const auto next = node_next(*static_cast<Node *>(candidate));
				if (head.compare_exchange_weak(candidate, next, std::memory_order_acq_rel))
					return static_cast<Node *>(candidate);
			}
		}

		std::atomic<void *> head = {};
	};
}

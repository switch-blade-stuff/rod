/*
 * Created by switchblade on 2023-05-21.
 */

#pragma once

#include <functional>
#include <utility>

#include "config.hpp"

namespace rod::_detail
{
	/* Priority queue implemented as a sorted list. While a more complex data structure will give better
	 * insert time complexity, it's also going to complicate implementation. Plus the queue is only used
	 * for sorting timer nodes, and it is not likely that there will be a lot of them at any given time. */
	template<typename Node, typename Cmp, Node *Node::*Prev, Node *Node::*Next>
	struct priority_queue
	{
		constexpr priority_queue() noexcept = default;
		constexpr priority_queue(priority_queue &&other) noexcept { swap(other); }
		constexpr priority_queue &operator=(priority_queue &&other) noexcept { return (swap(other), *this); }

		[[nodiscard]] constexpr bool empty() const noexcept { return !head; }
		[[nodiscard]] constexpr Node *front() const noexcept { return head; }
		[[nodiscard]] constexpr Node *pop_front() noexcept
		{
			const auto node = std::exchange(head, head->*Next);
			if (head) head->*Prev = {};
			node->*Next = {};
			node->*Prev = {};
			return node;
		}

		constexpr Node *insert(Node *node) noexcept
		{
			if (!head)
				head = node;
			else if (Cmp{}(*node, *head))
			{
				node->*Next = head;
				head->*Prev = node;
				head = node;
			}
			else
			{
				auto prev = head;
				while (prev->*Next && !Cmp{}(*node, *(prev->*Next)))
					prev = prev->*Next;

				const auto next = prev->*Next;
				if (next) next->*Prev = node;
				node->*Next = next;
				prev->*Next = node;
				node->*Prev = prev;
			}
			return node;
		}
		constexpr void erase(Node *node) noexcept
		{
			const auto next = std::exchange(node->*Next, {});
			const auto prev = std::exchange(node->*Prev, {});
			if (next)
				next->*Prev = prev;
			if (prev)
				prev->*Next = next;
			else
				head = next;
		}

		constexpr void swap(priority_queue &other) noexcept { std::swap(head, other.head); }

		Node *head = {};
	};
}

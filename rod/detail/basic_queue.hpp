/*
 * Created by switchblade on 2023-05-21.
 */

#pragma once

#include <utility>

#include "config.hpp"

namespace rod::detail
{
	template<typename Node, Node *Node::*Next>
	struct basic_queue
	{
		constexpr basic_queue() noexcept = default;
		constexpr basic_queue(basic_queue &&other) noexcept { swap(other); }
		constexpr basic_queue &operator=(basic_queue &&other) noexcept { return (swap(other), *this); }

		void push_back(Node *node) noexcept
		{
			if (tail)
				tail->*Next = node;
			else
				head = node;
			tail = node;
		}
		void push_front(Node *node) noexcept
		{
			node->*Next = std::exchange(head, node);
			if (!tail) tail = node;
		}
		[[nodiscard]] Node *pop_front() noexcept
		{
			const auto node = std::exchange(head, head->*Next);
			if (!head) tail = {};
			node->*Next = {};
			return node;
		}

		void merge_back(basic_queue &&other) noexcept
		{
			if (other.empty()) return;
			if (const auto node = std::exchange(other.head, {}); tail)
				tail->*Next = node;
			else
				head = node;
			tail = std::exchange(other.tail, {});
		}
		void merge_front(basic_queue &&other) noexcept
		{
			if (other.empty()) return;
			other.tail->*Next = std::exchange(head, std::exchange(other.head, {}));
			if (!tail) tail = std::exchange(other.tail, {});
		}

		[[nodiscard]] constexpr bool empty() const noexcept { return !head; }

		constexpr void swap(basic_queue &other) noexcept
		{
			std::swap(head, other.head);
			std::swap(tail, other.tail);
		}

		Node *head = {};
		Node *tail = {};
	};
}

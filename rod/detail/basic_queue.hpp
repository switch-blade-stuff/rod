/*
 * Created by switchblade on 2023-05-21.
 */

#pragma once

#include <utility>

#include "config.hpp"

namespace rod::_detail
{
	template<typename Node, Node *Node::*Next, Node *Node::*Prev = nullptr>
	struct basic_queue
	{
		constexpr basic_queue() noexcept = default;
		constexpr basic_queue(basic_queue &&other) noexcept { swap(other); }
		constexpr basic_queue &operator=(basic_queue &&other) noexcept { return (swap(other), *this); }

		[[nodiscard]] constexpr bool empty() const noexcept { return !head; }

		[[nodiscard]] constexpr Node *back() const noexcept { return tail; }
		[[nodiscard]] constexpr Node *front() const noexcept { return head; }

		Node *erase(Node *node) noexcept requires(Prev != nullptr)
		{
			const auto next = std::exchange(node->*Next, nullptr);
			const auto prev = std::exchange(node->*Prev, nullptr);

			if (prev != nullptr)
				prev->*Next = next;
			if (next != nullptr)
				next->*Prev = prev;
			if (node == head)
				head = next;
			if (node == tail)
				tail = prev;
			size -= 1;
			return next;
		}

		[[nodiscard]] Node *pop_back() noexcept requires(Prev != nullptr)
		{
			const auto node = std::exchange(tail, tail->*Prev);
			if (tail != nullptr)
				tail->*Next = nullptr;
			else
				head = nullptr;
			size -= 1;

			node->*Prev = nullptr;
			return node;
		}
		[[nodiscard]] Node *pop_front() noexcept requires(Prev != nullptr)
		{
			const auto node = std::exchange(head, head->*Next);
			if (head != nullptr)
				head->*Prev = nullptr;
			else
				tail = nullptr;
			size -= 1;

			node->*Next = nullptr;
			return node;
		}
		[[nodiscard]] Node *pop_front() noexcept requires(Prev == nullptr)
		{
			const auto node = std::exchange(head, head->*Next);
			if (head == nullptr)
				tail = nullptr;
			size -= 1;

			node->*Next = nullptr;
			return node;
		}

		void push_back(Node *node) noexcept requires(Prev != nullptr)
		{
			if (tail != nullptr)
			{
				tail->*Next = node;
				node->*Prev = tail;
				tail = node;
			}
			else
			{
				head = node;
				tail = node;
			}
			size += 1;
		}
		void push_back(Node *node) noexcept requires(Prev == nullptr)
		{
			if (tail != nullptr)
				tail->*Next = node;
			else
				head = node;
			tail = node;
			size += 1;
		}

		void push_front(Node *node) noexcept requires(Prev != nullptr)
		{
			if (head != nullptr)
			{
				node->*Next = head;
				head->*Prev = node;
				head = node;
			}
			else
			{
				head = node;
				tail = node;
			}
			size += 1;
		}
		void push_front(Node *node) noexcept requires(Prev == nullptr)
		{
			node->*Next = std::exchange(head, node);
			if (tail == nullptr)
				tail = node;
			size += 1;
		}

		void merge_back(basic_queue other) noexcept requires(Prev != nullptr)
		{
			if (other.empty())
				return;

			if (tail != nullptr)
			{
				tail->*Next = other.head;
				other.head->*Prev = tail;
				tail = other.tail;
			}
			else
			{
				head = other.head;
				tail = other.tail;
			}
			size += other.size;
		}
		void merge_back(basic_queue other) noexcept requires(Prev == nullptr)
		{
			if (other.empty())
				return;

			if (tail != nullptr)
			{
				tail->*Next = other.head;
				tail = other.tail;
			}
			else
			{
				head = other.head;
				tail = other.tail;
			}
			size += other.size;
		}

		void merge_front(basic_queue other) noexcept requires(Prev != nullptr)
		{
			if (other.empty())
				return;

			if (head != nullptr)
			{
				head->*Prev = other.tail;
				other.tail->*Next = head;
				head = other.head;
			}
			else
			{
				head = other.head;
				tail = other.tail;
			}
			size += other.size;
		}
		void merge_front(basic_queue other) noexcept requires(Prev == nullptr)
		{
			if (other.empty())
				return;

			if (head != nullptr)
			{
				other.tail->*Next = head;
				head = other.head;
			}
			else
			{
				head = other.head;
				tail = other.tail;
			}
			size += other.size;
		}

		constexpr void swap(basic_queue &other) noexcept
		{
			std::swap(head, other.head);
			std::swap(tail, other.tail);
			std::swap(size, other.size);
		}

		Node *head = nullptr;
		Node *tail = nullptr;
		std::size_t size = 0;
	};
}

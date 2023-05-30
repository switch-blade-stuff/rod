/*
 * Created by switchblade on 2023-04-15.
 */

#pragma once

#include <functional>
#include <cassert>

#include "detail/config.hpp"
#include "packed_pair.hpp"
#include "generator.hpp"
#include "delegate.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	template<std::move_constructible, typename>
	class basic_signal;
	template<typename>
	class sink;

	/** Ordered list of listeners of type \a Func, representing the private half of the signal-sink interface.
	 *
	 * @tparam Func Underlying invocable type of the signal's listeners.
	 * @tparam Alloc Allocator type used to allocate internal storage. */
	template<std::move_constructible Func, typename Alloc = std::allocator<Func>>
	class basic_signal
	{
		template<typename>
		friend class sink;

	public:
		using size_type = typename std::allocator_traits<Alloc>::size_type;
		using allocator_type = Alloc;
		using value_type = Func;

	private:
		constexpr static auto npos = std::numeric_limits<size_type>::max();

		struct node_t
		{
			constexpr node_t(const node_t &other) noexcept(std::is_nothrow_copy_constructible_v<value_type>) : next(other.next), prev(other.prev), value(other.value) {}
			constexpr node_t(node_t &&other) noexcept(std::is_nothrow_move_constructible_v<value_type>) : next(other.next), prev(other.prev), value(std::move(other.value)) {}
			template<typename... Args>
			constexpr explicit node_t(Args &&...args) noexcept(std::is_nothrow_constructible_v<value_type, Args...>) : value(std::forward<Args>(args)...) {}

			size_type next = npos;
			size_type prev = npos;
			std::optional<value_type> value;
		};

		using storage_t = std::vector<node_t, typename std::allocator_traits<Alloc>::template rebind_alloc<node_t>>;

	public:
		basic_signal(const basic_signal &) = delete;
		basic_signal &operator=(const basic_signal &) = delete;

		/** Initializes an empty signal. */
		constexpr basic_signal() noexcept = default;
		/** Initializes an empty signal using the specified allocator. */
		constexpr explicit basic_signal(const allocator_type &alloc) noexcept : m_nodes(alloc) {}

		constexpr basic_signal(basic_signal &&other) noexcept(std::is_nothrow_move_constructible_v<storage_t>) : m_nodes(std::move(other.m_nodes))
		{
			std::swap(m_head, other.m_head);
			std::swap(m_slot, other.m_slot);
		}
		constexpr basic_signal &operator=(basic_signal &&other) noexcept(std::is_nothrow_move_assignable_v<storage_t>)
		{
			if (this != &other)
			{
				m_nodes = std::move(other.m_nodes);
				m_head = std::exchange(other.m_head, npos);
				m_slot = std::exchange(other.m_slot, npos);
			}
			return *this;
		}

		/** Checks if the signal is empty (has no associated listeners). */
		[[nodiscard]] constexpr bool empty() const noexcept { return m_head == npos; }
		/** Returns copy of the signal's allocator. */
		[[nodiscard]] constexpr allocator_type get_allocator() noexcept { return allocator_type{m_nodes.get_allocator()}; }

		/** Invokes all associated listeners with \a args. */
		template<typename... Args>
		constexpr void emit(Args &&...args) requires std::invocable<Func, Args...>
		{
			for (auto pos = m_head; pos != npos;)
			{
				auto &node = m_nodes[pos];
				pos = node.next;

				std::invoke(*node.value, args...);
			}
		}
		/** Invokes all associated listeners with \a args, and accumulates results using functor \a acc.
		 * If \a acc returns a `non-void` value convertible to `bool`, stops execution when the result evaluates to `false`. */
		template<typename A, typename... Args, typename R = std::invoke_result_t<Func, Args...>>
		constexpr void accumulate(A acc, Args &&...args) requires(!std::same_as<R, void> && std::invocable<Func, Args...> && std::invocable<A, R>)
		{
			for (auto pos = m_head; pos != npos;)
			{
				auto &node = m_nodes[pos];
				pos = node.next;

				const auto invoke = [&]() -> decltype(auto) { return std::invoke(acc, std::invoke(*node.value, args...)); };
				if constexpr (!std::convertible_to<std::invoke_result_t<A, R>, bool>)
					invoke();
				else if (!static_cast<bool>(invoke()))
					break;
			}
		}

#ifdef ROD_HAS_COROUTINES
		/** Returns a generator coroutine used to invoke & yield results of the associated listeners using arguments \a Args. */
		template<typename... Args, typename R = std::invoke_result_t<Func, Args...>>
		[[nodiscard]] generator<R> generate(Args ...args) requires(!std::same_as<R, void> && std::invocable<Func, Args...>)
		{
			for (auto pos = m_head; pos != npos;)
			{
				auto &node = m_nodes[pos];
				pos = node.next;

				co_yield std::invoke(*node.value, args...);
			}
		}
#endif

		constexpr void swap(basic_signal &other) noexcept(std::is_nothrow_swappable_v<storage_t>) { swap(*this, other); }
		friend constexpr void swap(basic_signal &a, basic_signal &b) noexcept(std::is_nothrow_swappable_v<storage_t>)
		{
			std::swap(a.m_nodes, b.m_nodes);
			std::swap(a.m_head, b.m_head);
			std::swap(a.m_slot, b.m_slot);
		}

	private:
		template<typename... Args>
		constexpr size_type emplace(Args &&...args)
		{
			/* If m_slot == npos, node array is contiguous and next free is m_last + 1 or 0 if m_last == npos. */
			size_type result;
			if (m_slot != npos)
			{
				result = m_slot;
				m_slot = m_nodes[result].next;
				m_nodes[result].value.emplace(std::forward<Args>(args)...);
			}
			else
			{
				result = m_nodes.size();
				m_nodes.emplace_back(std::forward<Args>(args)...);
			}
			if (m_head != npos)
			{
				m_nodes[m_head].prev = result;
				m_nodes[result].next = m_head;
			}
			return (m_head = result);
		}
		constexpr void erase(size_type pos) noexcept(std::is_nothrow_destructible_v<value_type>)
		{
			assert(pos < m_nodes.size());

			auto &node = m_nodes[pos];
			if (m_head == pos) m_head = node.next;
			if (node.prev != npos) m_nodes[node.prev].next = node.next;
			if (node.next != npos) m_nodes[node.next].prev = node.prev;

			node.value.reset();
			node.next = m_slot;
			node.prev = npos;
			m_slot = pos;
		}

		storage_t m_nodes = {};
		size_type m_head = npos;
		size_type m_slot = npos;
	};

	/** Alias of `basic_signal` that uses `delegate` as it's listener type. */
	template<typename Sign>
	using delegate_signal = basic_signal<delegate<Sign>>;
	/** Alias of `basic_signal` that uses `std::function` as it's listener type. */
	template<typename Sign>
	using function_signal = basic_signal<std::function<Sign>>;

	/** @brief Thin handle of an associated signal instance, representing the public half of the signal-sink interface.
	 *
	 * Sinks provide access to the range-like API of a signal, enabling the user to
	 * iterate over listeners, insert listeners into, and erase listeners from from
	 * the associated signal's listener queue.
	 *
	 * Iterators pointing to listener objects are guaranteed to always remain valid until the listener is
	 * erased from the signal queue. Stability of pointers and references to listeners is unspecified.
	 *
	 * @tparam Signal Signal type associated with the sink. */
	template<typename Signal>
	class sink
	{
		static_assert(instance_of<Signal, basic_signal>);

	public:
		using value_type = typename Signal::value_type;
		using size_type = typename Signal::size_type;

		/** Sentinel value used to indicate an invalid index. */
		constexpr static auto npos = Signal::npos;

	public:
		/** Initializes a sink for signal \a signal. */
		constexpr sink(Signal &signal) noexcept : m_signal(&signal) {}

		/** Inserts a copy-constructed listener into the associated signal's queue and returns an index to the inserted element. */
		constexpr size_type insert(const value_type &value) const requires std::copy_constructible<value_type> { return emplace(value); }
		/** @copydoc insert */
		constexpr size_type operator+=(const value_type &value) const requires std::copy_constructible<value_type> { return insert(value); }

		/** Inserts a move-constructed listener into the associated signal's queue and returns an index to the inserted element. */
		constexpr size_type insert(value_type &&value) const requires std::move_constructible<value_type> { return emplace(std::forward<value_type>(value)); }
		/** @copydoc insert */
		constexpr size_type operator+=(value_type &&value) const requires std::move_constructible<value_type> { return insert(std::forward<value_type>(value)); }

		/** Inserts an in-place constructed listener into the associated signal's queue and returns an index to the inserted element. */
		template<typename... Args>
		constexpr size_type emplace(Args &&...args) const requires std::constructible_from<value_type, Args...> { return m_signal->emplace(std::forward<Args>(args)...); }

		/** Removes listener at index \a idx. */
		constexpr void erase(size_type idx) const noexcept(std::is_nothrow_destructible_v<value_type>) { m_signal->erase(idx); }
		/** @copydoc erase */
		constexpr void operator-=(size_type idx) const noexcept(std::is_nothrow_destructible_v<value_type>) { erase(idx); }

		[[nodiscard]] constexpr bool operator==(const sink &) const noexcept = default;

	private:
		Signal *m_signal;
	};

	template<typename Signal>
	sink(Signal &) -> sink<Signal>;
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

/*
 * Created by switchblade on 2023-04-15.
 */

#pragma once

#include <functional>
#include <optional>
#include <cassert>
#include <limits>

#include "detail/config.hpp"
#include "packed_pair.hpp"
#include "generator.hpp"
#include "delegate.hpp"

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
		constexpr explicit basic_signal(const allocator_type &alloc) noexcept : _nodes(alloc) {}

		constexpr basic_signal(basic_signal &&other) noexcept(std::is_nothrow_move_constructible_v<storage_t>) : _nodes(std::move(other._nodes))
		{
			std::swap(_head, other._head);
			std::swap(_slot, other._slot);
		}
		constexpr basic_signal &operator=(basic_signal &&other) noexcept(std::is_nothrow_move_assignable_v<storage_t>)
		{
			if (this != &other)
			{
				_nodes = std::move(other._nodes);
				_head = std::exchange(other._head, npos);
				_slot = std::exchange(other._slot, npos);
			}
			return *this;
		}

		/** Checks if the signal is empty (has no associated listeners). */
		[[nodiscard]] constexpr bool empty() const noexcept { return _head == npos; }
		/** Returns copy of the signal's allocator. */
		[[nodiscard]] constexpr allocator_type get_allocator() noexcept { return allocator_type{_nodes.get_allocator()}; }

		/** Invokes all associated listeners with \a args. */
		template<typename... Args>
		constexpr void emit(Args &&...args) requires std::invocable<Func, Args...>
		{
			for (auto pos = _head; pos != npos;)
			{
				auto &node = _nodes[pos];
				pos = node.next;

				std::invoke(*node.value, args...);
			}
		}
		/** Invokes all associated listeners with \a args, and accumulates results using functor \a acc.
		 * If \a acc returns a `non-void` value convertible to `bool`, stops execution when the result evaluates to `false`. */
		template<typename A, typename... Args, typename R = std::invoke_result_t<Func, Args...>>
		constexpr void accumulate(A acc, Args &&...args) requires(!std::same_as<R, void> && std::invocable<Func, Args...> && std::invocable<A, R>)
		{
			for (auto pos = _head; pos != npos;)
			{
				auto &node = _nodes[pos];
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
			for (auto pos = _head; pos != npos;)
			{
				auto &node = _nodes[pos];
				pos = node.next;

				co_yield std::invoke(*node.value, args...);
			}
		}
#endif

		constexpr void swap(basic_signal &other) noexcept(std::is_nothrow_swappable_v<storage_t>)
		{
			std::swap(_nodes, other._nodes);
			std::swap(_head, other._head);
			std::swap(_slot, other._slot);
		}
		template<typename F>
		friend constexpr void swap(basic_signal<F> &a, basic_signal<F> &b) noexcept(noexcept(a.swap(b))) { a.swap(b); }

	private:
		template<typename... Args>
		constexpr size_type emplace(Args &&...args)
		{
			/* If _slot == npos, node array is contiguous and next free is _last + 1 or 0 if _last == npos. */
			size_type result;
			if (_slot != npos)
			{
				result = _slot;
				_slot = _nodes[result].next;
				_nodes[result].value.emplace(std::forward<Args>(args)...);
			}
			else
			{
				result = _nodes.size();
				_nodes.emplace_back(std::forward<Args>(args)...);
			}
			if (_head != npos)
			{
				_nodes[_head].prev = result;
				_nodes[result].next = _head;
			}
			return (_head = result);
		}
		constexpr void erase(size_type pos)
		{
			assert(pos < _nodes.size());

			auto &node = _nodes[pos];
			if (_head == pos) _head = node.next;
			if (node.prev != npos) _nodes[node.prev].next = node.next;
			if (node.next != npos) _nodes[node.next].prev = node.prev;

			node.value.reset();
			node.next = _slot;
			node.prev = npos;
			_slot = pos;
		}

		storage_t _nodes = {};
		size_type _head = npos;
		size_type _slot = npos;
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
	 * For ABI compatibility purposes, all sink types are equivalent to the following (exposition-only) struct:
	 * @code{cpp}
	 * struct sink { void *internal; };
	 * @endcode
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
		constexpr sink(Signal &signal) noexcept : _signal(&signal) {}

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
		constexpr size_type emplace(Args &&...args) const requires std::constructible_from<value_type, Args...> { return _signal->emplace(std::forward<Args>(args)...); }

		/** Removes listener at index \a idx. */
		constexpr void erase(size_type idx) const noexcept(std::is_nothrow_destructible_v<value_type>) { _signal->erase(idx); }
		/** @copydoc erase */
		constexpr void operator-=(size_type idx) const noexcept(std::is_nothrow_destructible_v<value_type>) { erase(idx); }

		[[nodiscard]] constexpr bool operator==(const sink &) const noexcept = default;

	private:
		Signal *_signal;
	};

	template<typename Signal>
	sink(Signal &) -> sink<Signal>;
}

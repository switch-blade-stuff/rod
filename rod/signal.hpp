/*
 * Created by switchblade on 2023-04-15.
 */

#pragma once

#include <functional>
#include <list>

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

		using storage_t = std::list<Func, Alloc>;

	public:
		using allocator_type = typename storage_t::allocator_type;
		using size_type = typename storage_t::size_type;

	public:
		/** Checks if the signal's listener queue is empty. */
		[[nodiscard]] constexpr bool empty() const noexcept { return m_data.empty(); }
		/** Returns the current size of the signal's listener queue. */
		[[nodiscard]] constexpr size_type size() const noexcept { return m_data.size(); }
		/** Returns copy of the signal's allocator. */
		[[nodiscard]] constexpr allocator_type get_allocator() noexcept { return m_data.get_allocator(); }

		/** Resets the signal to initial (no listeners) state. */
		constexpr void reset() { m_data.clear(); }

		/** Invokes all associated listeners with \a args. */
		template<typename... Args>
		constexpr void emit(Args &&...args) requires std::invocable<Func, Args...>
		{
			for (auto &&target: m_data) std::invoke(target, args...);
		}
		/** Invokes all associated listeners with \a args, and accumulates results using functor \a acc.
		 * If \a acc returns a `non-void` value convertible to `bool`, stops execution when the result evaluates to `false`. */
		template<typename A, typename... Args, typename R = std::invoke_result_t<Func, Args...>>
		constexpr void accumulate(A acc, Args &&...args) requires(!std::same_as<R, void> && std::invocable<Func, Args...> && std::invocable<A, R>)
		{
			for (auto &&target: m_data)
			{
				const auto next = [&]() -> decltype(auto) { return std::invoke(acc, std::invoke(target, args...)); };
				if constexpr (std::convertible_to<std::invoke_result_t<A, R>, bool>)
				{
					const auto do_continue = static_cast<bool>(next());
					if (!do_continue) break;
				}
				else
					next();
			}
		}

#ifdef ROD_HAS_COROUTINES
		/** Returns a generator coroutine used to invoke & yield results of the associated listeners using arguments \a Args. */
		template<typename... Args, typename R = std::invoke_result_t<Func, Args...>>
		[[nodiscard]] generator<R> generate(Args ...args) requires(!std::same_as<R, void> && std::invocable<Func, Args...>)
		{
			for (auto &&target: m_data) co_yield std::invoke(target, args...);
		}
#endif

		constexpr void swap(basic_signal &other) noexcept(std::is_nothrow_swappable_v<storage_t>) { m_data.swap(other.m_data); }
		friend constexpr void swap(basic_signal &a, basic_signal &b) noexcept(std::is_nothrow_swappable_v<storage_t>) { a.swap(b); }

	private:
		storage_t m_data = {};
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
		using storage_t = typename Signal::storage_t;

	public:
		using value_type = typename storage_t::value_type;

		using reference = typename storage_t::const_reference;
		using const_reference = typename storage_t::const_reference;
		using pointer = typename storage_t::const_pointer;
		using const_pointer = typename storage_t::const_pointer;

		using iterator = typename storage_t::const_iterator;
		using reverse_iterator = typename storage_t::const_reverse_iterator;

		using size_type = typename storage_t::size_type;
		using difference_type = typename storage_t::difference_type;

	public:
		/** Initializes a sink for signal \a signal. */
		constexpr sink(Signal &signal) noexcept : m_signal(&signal) {}

		/** Checks if the associated signal's listener queue is empty. */
		[[nodiscard]] constexpr bool empty() const noexcept { return m_signal->empty(); }
		/** Returns the current size of the associated signal's listener queue. */
		[[nodiscard]] constexpr size_type size() const noexcept { return m_signal->size(); }

		/** Returns bidirectional iterator to the first listener of the associated signal. */
		[[nodiscard]] constexpr iterator begin() const noexcept { return m_signal->m_data.cbegin(); }
		/** @copydoc begin */
		[[nodiscard]] constexpr iterator cbegin() const noexcept { return m_signal->m_data.cbegin(); }
		/** Returns bidirectional iterator one past the last listener of the associated signal. */
		[[nodiscard]] constexpr iterator end() const noexcept { return m_signal->m_data.cend(); }
		/** @copydoc end */
		[[nodiscard]] constexpr iterator cend() const noexcept { return m_signal->m_data.cend(); }

		/** Returns reverse bidirectional iterator to the first listener of the associated signal. */
		[[nodiscard]] constexpr reverse_iterator rbegin() const noexcept { return m_signal->m_data.crbegin(); }
		/** @copydoc rbegin */
		[[nodiscard]] constexpr reverse_iterator crbegin() const noexcept { return m_signal->m_data.crbegin(); }
		/** Returns reverse bidirectional iterator one past the last listener of the associated signal. */
		[[nodiscard]] constexpr reverse_iterator rend() const noexcept { return m_signal->m_data.crend(); }
		/** @copydoc rend */
		[[nodiscard]] constexpr reverse_iterator crend() const noexcept { return m_signal->m_data.crend(); }

		/** Inserts a listener at the end of the associated signal's queue. */
		void push_back(value_type &&value) const { m_signal->m_data.push_back(std::forward<value_type>(value)); }
		/** Inserts a listener at the start of the associated signal's queue. */
		void push_front(value_type &&value) const { m_signal->m_data.push_front(std::forward<value_type>(value)); }
		/** Inserts a listener into the associated signal's queue after the listener at \a pos and returns an iterator pointing to the inserted element. */
		iterator insert(iterator pos, value_type &&value) const { return m_signal->m_data.insert(pos, std::forward<value_type>(value)); }

		/** Inserts an in-place constructed listener at the end of the associated signal's queue. */
		template<typename... Args>
		void emplace_back(Args &&...args) const { return m_signal->m_data.emplace_back(std::forward<Args>(args)...); }
		/** Inserts an in-place constructed listener at the start of the associated signal's queue. */
		template<typename... Args>
		void emplace_front(Args &&...args) const { return m_signal->m_data.emplace_front(std::forward<Args>(args)...); }
		/** Inserts an in-place constructed listener into the associated signal's queue after the listener at \a pos and returns an iterator pointing to the inserted element. */
		template<typename... Args>
		iterator emplace(iterator pos, Args &&...args) const { return m_signal->m_data.emplace(pos, std::forward<Args>(args)...); }

		/** Removes listener located at \a pos from the associated signal's queue and returns iterator to the listener after the erased one or an end iterator. */
		iterator erase(iterator pos) const { return m_signal->m_data.erase(pos); }
		/** Removes all listeners within the range `[first, last)` and returns iterator to the listener after the erased sequence or an end iterator. */
		iterator erase(iterator first, iterator last) const { return m_signal->m_data.erase(first, last); }

		/** Appends a listener to the associated signal's queue. */
		sink operator+=(value_type &&value) const { return (push_back(std::forward<value_type>(value)), *this); }
		/** Removes listener located at \a pos from the associated signal's queue. */
		sink operator-=(iterator pos) const { return (erase(pos), *this); }

		[[nodiscard]] constexpr bool operator==(const sink &) const noexcept = default;

	private:
		Signal *m_signal;
	};

	template<typename Signal>
	sink(Signal &) -> sink<Signal>;
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

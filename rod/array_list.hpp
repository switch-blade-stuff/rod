/*
 * Created by switch_blade on 2023-07-30.
 */

#pragma once

#include <cassert>

#include "utility.hpp"

namespace rod
{
	template<typename T, typename Alloc>
	class array_list;

	namespace detail
	{
		template<typename Alloc>
		struct array_list_header
		{
			using difference_type = typename std::allocator_traits<Alloc>::difference_type;

			constexpr array_list_header() noexcept = default;
			constexpr array_list_header(array_list_header &&other) noexcept : next_off(other.next_off), prev_off(other.prev_off) {}

			constexpr auto next() noexcept { return this + next_off; }
			constexpr auto next() const noexcept { return this + next_off; }
			constexpr void next(const array_list_header *node) noexcept { next_off = node - this; }

			constexpr auto prev() noexcept { return this - prev_off; }
			constexpr auto prev() const noexcept { return this - prev_off; }
			constexpr void prev(const array_list_header *node) noexcept { prev_off = this - node; }

			constexpr void swap(array_list_header &other)
			{
				std::swap(next_off, other.next_off);
				std::swap(prev_off, other.prev_off);
			}

			difference_type next_off = {};
			difference_type prev_off = {};
		};

		template<typename T, typename Alloc>
		struct array_list_node : array_list_header<Alloc>
		{
			template<typename... Args>
			constexpr explicit array_list_node(Args &&...args) : value(std::forward<Args>(args)...) {}

			[[nodiscard]] constexpr auto *get() noexcept { return &value; }
			[[nodiscard]] constexpr auto *get() const noexcept { return &value; }

			T value;
		};
		template<typename T, typename Alloc> requires(std::is_object_v<T> && !std::is_final_v<T>)
		struct array_list_node<T, Alloc> : array_list_header<Alloc>, T
		{
			template<typename... Args>
			constexpr explicit array_list_node(Args &&...args) : T(std::forward<Args>(args)...) {}

			[[nodiscard]] constexpr auto *get() noexcept { return static_cast<T *>(this); }
			[[nodiscard]] constexpr auto *get() const noexcept { return static_cast<const T *>(this); }
		};

		template<typename T, typename Alloc>
		using select_allocator_pointer = std::conditional<std::is_const_v<T>, typename std::allocator_traits<Alloc>::const_pointer, typename std::allocator_traits<Alloc>::pointer>;
		template<typename T, typename Alloc>
		using select_allocator_pointer_t = typename select_allocator_pointer<T, Alloc>::type;

		template<typename T, typename Alloc>
		class array_list_iterator
		{
			template<typename, typename>
			friend class array_list_iterator;
			template<typename, typename>
			friend class rod::array_list;

		public:
			using value_type = typename std::allocator_traits<Alloc>::value_type;
			using pointer = select_allocator_pointer_t<T, Alloc>;
			using reference = std::add_lvalue_reference_t<T>;

			using size_type = typename std::allocator_traits<Alloc>::size_type;
			using difference_type = typename std::allocator_traits<Alloc>::difference_type;
			using iterator_category = std::bidirectional_iterator_tag;

		private:
			using header_t = detail::array_list_header<Alloc>;
			using node_t = detail::array_list_node<value_type, Alloc>;

			using header_pointer = select_allocator_pointer_t<copy_cv_t<T, header_t>, typename std::allocator_traits<Alloc>::template rebind_traits<header_t>>;
			using node_pointer = select_allocator_pointer_t<copy_cv_t<T, node_t>, typename std::allocator_traits<Alloc>::template rebind_traits<node_t>>;

			constexpr array_list_iterator(header_pointer node) noexcept : _node(node_pointer(node)) {}

		public:
			constexpr array_list_iterator() noexcept = default;
			template<std::same_as<std::add_const_t<T>> U>
			constexpr array_list_iterator(const array_list_iterator<U, Alloc> &other) noexcept : _node(other._node) {}

			constexpr array_list_iterator &operator++() noexcept
			{
				_node = node_pointer(_node->next());
				return *this;
			}
			constexpr array_list_iterator operator++(int) noexcept
			{
				auto tmp = *this;
				operator++();
				return tmp;
			}

			constexpr array_list_iterator &operator--() noexcept
			{
				_node = node_pointer(_node->prev());
				return *this;
			}
			constexpr array_list_iterator operator--(int) noexcept
			{
				auto tmp = *this;
				operator--();
				return tmp;
			}

			[[nodiscard]] constexpr auto get() noexcept { return _node->get(); }
			[[nodiscard]] constexpr auto operator->() noexcept { return get(); }
			[[nodiscard]] constexpr auto operator*() noexcept { return *get(); }

			[[nodiscard]] constexpr bool operator==(const array_list_iterator &other) noexcept { return _node == other._node; }
			[[nodiscard]] constexpr bool operator!=(const array_list_iterator &other) noexcept { return _node != other._node; }

			constexpr void swap(array_list_iterator &other) noexcept { std::swap(_node, other._node); }
			friend constexpr void swap(array_list_iterator &a, array_list_iterator &b) noexcept { a.swap(b); }

		private:
			node_pointer _node = {};
		};
	}

	/** Contiguous-storage list container.
	 * @tparam T Value type stored by the list.
	 * @tparam Alloc Allocator used to allocate list buffer. */
	template<typename T, typename Alloc = std::allocator<T>>
	class array_list : detail::array_list_header<Alloc>
	{
		using header_t = detail::array_list_header<Alloc>;
		using node_t = detail::array_list_node<T, Alloc>;

		using header_pointer = typename std::allocator_traits<Alloc>::template rebind_traits<header_t>::pointer;
		using const_header_pointer = typename std::allocator_traits<Alloc>::template rebind_traits<header_t>::const_pointer;

		using node_pointer = typename std::allocator_traits<Alloc>::template rebind_traits<node_t>::pointer;
		using const_node_pointer = typename std::allocator_traits<Alloc>::template rebind_traits<node_t>::const_pointer;

		using node_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<node_t>;

	public:
		using value_type = T;
		using allocator_type = Alloc;

		using iterator = detail::array_list_iterator<value_type, allocator_type>;
		using const_iterator = detail::array_list_iterator<std::add_const_t<value_type>, allocator_type>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		using reference = typename iterator::reference;
		using const_reference = typename const_iterator::reference;
		using pointer = typename iterator::pointer;
		using const_pointer = typename const_iterator::pointer;

		using size_type = typename std::allocator_traits<allocator_type>::size_type;
		using difference_type = typename std::allocator_traits<allocator_type>::difference_type;

	public:
		/** Initializes an empty list. */
		constexpr array_list() noexcept(std::is_nothrow_default_constructible_v<node_allocator>) = default;
		/** Initializes an empty list using allocator \a alloc. */
		constexpr array_list(const allocator_type &alloc) noexcept(std::is_nothrow_constructible_v<node_allocator, const allocator_type &>) : _alloc(alloc) {}

		/** Copy-constructs a list from \a other. */
		constexpr array_list(const array_list &other) : array_list(other, std::allocator_traits<allocator_type>::select_on_container_copy_construction(other.get_allocator())) {}
		/** Copy-constructs a list from \a other with allocator \a alloc. */
		constexpr array_list(const array_list &other, const allocator_type &alloc) : _alloc(alloc)
		{
			reserve(other.size());
			copy_data(other);
		}

		/** Move-constructs a list from \a other. */
		constexpr array_list(array_list &&other) noexcept(std::is_nothrow_move_constructible_v<node_allocator>) : _alloc(std::move(other._alloc)) { swap_data(other); }
		/** Move-constructs a list from \a other with allocator \a alloc. */
		constexpr array_list(array_list &&other, const allocator_type &alloc) noexcept(std::allocator_traits<node_allocator>::is_always_equal::value && std::is_nothrow_constructible_v<node_allocator, const allocator_type &>) : _alloc(alloc)
		{
			if (std::allocator_traits<node_allocator>::is_always_equal::value || _alloc == other._alloc)
				swap_data(other);
			else
			{
				reserve(other.size());
				move_data(other);
			}
		}

		/** Initializes a list with \a n default-constructed elements. */
		constexpr array_list(size_type n) : array_list(n, node_allocator{}) {}
		/** Initializes a list with \a n default-constructed elements using allocator \a alloc. */
		constexpr array_list(size_type n, const allocator_type &alloc) : _alloc(alloc) { resize(n); }

		/** Initializes a list with \a n elements copy-constructed from \a val. */
		constexpr array_list(size_type n, const_reference val) : array_list(n, val, node_allocator{}) {}
		/** Initializes a list with \a n elements copy-constructed from \a val using allocator \a alloc. */
		constexpr array_list(size_type n, const_reference val, const allocator_type &alloc) : _alloc(alloc) { resize(n, val); }

		/** Initializes a list with elements in range [\a first, \a last). */
		template<std::forward_iterator I, std::sentinel_for<I> S>
		constexpr array_list(I first, S last) requires std::constructible_from<value_type, std::iter_value_t<T>> { insert(first, last); }
		/** Initializes a list with elements in range [\a first, \a last) using allocator \a alloc. */
		template<std::forward_iterator I, std::sentinel_for<I> S>
		constexpr array_list(I first, S last, const allocator_type &alloc) requires std::constructible_from<value_type, std::iter_value_t<T>> : _alloc(alloc) { insert(first, last); }

		/** Initializes a list with elements from initializer list \a il. */
		template<typename U = T>
		constexpr array_list(std::initializer_list<U> il) requires std::constructible_from<value_type, std::add_const_t<U>> { insert(il); }
		/** Initializes a list with elements from initializer list \a il using allocator \a alloc. */
		template<typename U = T>
		constexpr array_list(std::initializer_list<U> il, const allocator_type &alloc) requires std::constructible_from<value_type, std::add_const_t<U>> : _alloc(alloc) { insert(il); }

		constexpr array_list &operator=(const array_list &other)
		{
			if (this != &other)
			{
				if (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value)
				{
					auto old_alloc = std::move(_alloc);
					_alloc = other._alloc;

					if (!std::allocator_traits<node_allocator>::is_always_equal::value && _alloc != old_alloc) [[unlikely]]
						destroy_data(old_alloc);
				}
				assign(other.begin(), other.end());
			}
			return *this;
		}
		constexpr array_list &operator=(array_list &&other) noexcept(std::allocator_traits<node_allocator>::is_always_equal::value && std::is_nothrow_destructible_v<value_type>)
		{
			if (this != &other)
			{
				if (std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
				{
					auto old_alloc = std::move(_alloc);
					_alloc = std::move(other._alloc);

					if (!std::allocator_traits<node_allocator>::is_always_equal::value && old_alloc != other._alloc) [[unlikely]]
						destroy_data(old_alloc);

					swap_data(other);
				}
				else if (!std::allocator_traits<node_allocator>::is_always_equal::value && _alloc != other._alloc)
					assign(std::move_iterator(other.begin()), std::move_iterator(other.end()));
				else
					swap_data(other);
			}
			return *this;
		}

		constexpr ~array_list() { destroy_data(_alloc); }

		/** Replaces contents of the list with \a n elements copy-constructed from \a val. */
		constexpr void assign(size_type n, const_reference val)
		{
			auto dst = begin();
			for (size_type i = 0; dst != end() && i < n; ++dst, ++i)
			{
				if constexpr (std::is_copy_assignable_v<value_type>)
					*dst = val;
				else
				{
					std::destroy_at(std::to_address(dst));
					std::construct_at(std::to_address(dst), val);
				}
			}

			reserve(n);
			while (size() < n)
				push_back(val);
			while (size() > n)
				pop_back();
		}
		/** Replaces contents of the list with elements in range [\a first, \a last). */
		template<std::forward_iterator I, std::sentinel_for<I> S>
		constexpr void assign(I first, S last) requires std::assignable_from<value_type, std::iter_value_t<T>> || std::constructible_from<value_type, std::iter_value_t<T>>
		{
			auto dst = begin();
			for (; dst != end() && first != last; ++dst, ++first)
			{
				if constexpr (std::is_assignable_v<value_type, decltype(*first)>)
					*dst = *first;
				else
				{
					std::destroy_at(std::to_address(dst));
					std::construct_at(std::to_address(dst), *first);
				}
			}

			if constexpr (std::random_access_iterator<I>)				
				reserve(static_cast<size_type>(last - first));

			for (; first != last; ++first)
				emplace_back(*first);
			for (; dst != end();)
				dst = erase(dst);
		}
		/** Replaces contents of the list with elements from initializer list \a il. */
		template<typename U = T>
		constexpr void assign(std::initializer_list<U> il) requires std::assignable_from<value_type, std::add_const_t<U>> || std::constructible_from<value_type, std::add_const_t<U>>
		{
			assign(il.begin(), il.end());
		}

		/** Returns iterator to the first element of the list. */
		[[nodiscard]] constexpr iterator begin() noexcept { return header_t::next(); }
		/** Returns const iterator to the first element of the list. */
		[[nodiscard]] constexpr const_iterator begin() const noexcept { return cbegin(); }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_iterator cbegin() const noexcept { return header_t::next(); }

		/** Returns iterator one past the last element of the list. */
		[[nodiscard]] constexpr iterator end() noexcept { return this; }
		/** Returns const iterator one past the last element of the list. */
		[[nodiscard]] constexpr const_iterator end() const noexcept { return cend(); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_iterator cend() const noexcept { return this; }

		/** Returns reference to the first element of the list. */
		[[nodiscard]] constexpr reference front() noexcept { return *node_pointer(header_t::next())->get(); }
		/** Returns const reference to the first element of the list. */
		[[nodiscard]] constexpr const_reference front() const noexcept { return *const_node_pointer(header_t::next())->get(); }

		/** Returns reference to the last element of the list. */
		[[nodiscard]] constexpr reference back() noexcept { return *node_pointer(header_t::prev())->get(); }
		/** Returns const reference to the last element of the list. */
		[[nodiscard]] constexpr const_reference back() const noexcept { return *const_node_pointer(header_t::prev())->get(); }

		/** Checks if the list is empty. */
		[[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }
		/** Returns the current size of the list. */
		[[nodiscard]] constexpr size_type size() const noexcept { return _list_size; }
		/** Returns the current capacity of the list. */
		[[nodiscard]] constexpr size_type capacity() const noexcept { return _data_size; }
		/** Returns the maximum possible size of the list. */
		[[nodiscard]] constexpr size_type max_size() const noexcept { return std::numeric_limits<size_type>::max(); }

		/** Reserves space for at least \a n elements. */
		constexpr void reserve(size_type n)
		{
			if (n <= _data_size)
				return;

			const auto new_size = std::max(_data_size * 2, n);
			const auto new_data = std::allocator_traits<node_allocator>::allocate(_alloc, new_size);

			try
			{
				for (header_pointer pos = header_pointer(header_t::next()), next; pos != this; pos = next)
				{
					const auto src = node_pointer(pos);
					const auto dst = new_data + src - _data;
					next = src->next();

					std::construct_at(dst, std::move(*src));
					std::destroy_at(src);
				}

				std::allocator_traits<node_allocator>::deallocate(_alloc, _data, _data_size);
				_data_size = new_size;
				_data = new_data;
			}
			catch (...)
			{
				std::allocator_traits<node_allocator>::deallocate(_alloc, new_data, new_size);
				throw;
			}
		}

		/** Resizes the list to contain \a n elements. New elements are default-initialized. */
		constexpr void resize(size_type n)
		{
			reserve(n);
			while (_list_size < n)
				emplace_at(_list_size);
			while (_list_size > n)
				destroy_at(_list_size);
		}
		/** Resizes the list to contain \a n elements. New elements are copy-initialized from \a val. */
		constexpr void resize(size_type n, const_reference val)
		{
			reserve(n);
			while (_list_size < n)
				emplace_at(_list_size, val);
			while (_list_size > n)
				destroy_at(_list_size);
		}

		/** Erases all elements of the list. */
		constexpr void clear() noexcept(std::is_nothrow_destructible_v<value_type>)
		{
			for (auto pos = begin(); pos != end(); ++pos)
				std::destroy_at(pos.get());

			header_t::next(this);
			header_t::prev(this);
			_free = nullptr;
			_list_size = {};
		}

		/** Inserts a copy-constructed element at the end of the list.
		 * @param val Value passed to the copy constructor of the new element.
		 * @return Iterator to the inserted element. */
		constexpr iterator push_back(const_reference val) { return insert(cend(), val); }
		/** Inserts a copy-constructed element at the start of the list.
		 * @param val Value passed to the copy constructor of the new element.
		 * @return Iterator to the inserted element. */
		constexpr iterator push_front(const_reference val) { return insert(cbegin(), val); }

		/** Inserts a move-constructed element at the end of the list.
		 * @param val Value passed to the move constructor of the new element.
		 * @return Iterator to the inserted element. */
		constexpr iterator push_back(value_type &&val) { return insert(cend(), std::forward<value_type>(val)); }
		/** Inserts a move-constructed element at the start of the list.
		 * @param val Value passed to the move constructor of the new element.
		 * @return Iterator to the inserted element. */
		constexpr iterator push_front(value_type &&val) { return insert(cbegin(), std::forward<value_type>(val)); }

		/** Inserts a copy-constructed element at the specified position within the list.
		 * @param pos Position within the list at which to insert the element.
		 * @param val Value passed to the copy constructor of the new element.
		 * @return Iterator to the inserted element. */
		constexpr iterator insert(const_iterator pos, const_reference val) { return emplace(pos, val); }
		/** Inserts a move-constructed element at the specified position within the list.
		 * @param pos Position within the list at which to insert the element.
		 * @param val Value passed to the move constructor of the new element.
		 * @return Iterator to the inserted element. */
		constexpr iterator insert(const_iterator pos, value_type &&val) { return emplace(pos, std::forward<value_type>(val)); }

		/** Inserts elements in range [\a first, \a last) at the specified position within the list.
		 * @param pos Position within the list at which to insert the elements.
		 * @param first Iterator to the first element to be inserted.
		 * @param last Sentinel for the \a first iterator.
		 * @return Iterator to the first inserted element. */
		template<std::forward_iterator I, std::sentinel_for<I> S>
		constexpr iterator insert(const_iterator pos, I first, S last) requires std::constructible_from<value_type, std::iter_value_t<T>>
		{
			if constexpr (std::random_access_iterator<I>)
				reserve(static_cast<size_type>(last - first));

			for (; first != last; ++first, ++pos)
				pos = insert(pos, *first);
			return pos;
		}
		/** Inserts elements from initializer list \a il at the specified position within the list.
		 * @param pos Position within the list at which to insert the elements.
		 * @param il Initializer list containing elements to insert.
		 * @return Iterator to the first inserted element. */
		template<typename U = value_type>
		constexpr iterator insert(const_iterator pos, std::initializer_list<U> il) requires std::constructible_from<value_type, std::add_const_t<U>>
		{
			return insert(pos, il.begin(), il.end());
		}

		/** Inserts an in-place constructed element at the end of the list.
		 * @param pos Position within the list at which to insert the element.
		 * @param args Arguments passed to the constructor of the new element.
		 * @return Reference to the inserted element. */
		template<typename... Args>
		constexpr reference emplace_back(Args &&...args) requires std::constructible_from<value_type, Args...> { return emplace(cend(), std::forward<Args>(args)...); }
		/** Inserts an in-place constructed element at the start of the list.
		 * @param pos Position within the list at which to insert the element.
		 * @param args Arguments passed to the constructor of the new element.
		 * @return Reference to the inserted element. */
		template<typename... Args>
		constexpr reference emplace_front(Args &&...args) requires std::constructible_from<value_type, Args...> { return emplace(cbegin(), std::forward<Args>(args)...); }

		/** Inserts an in-place constructed element at the specified position within the list.
		 * @param pos Position within the list at which to insert the element.
		 * @param args Arguments passed to the constructor of the new element.
		 * @return Reference to the inserted element. */
		template<typename... Args>
		constexpr reference emplace(const_iterator pos, Args &&...args) requires std::constructible_from<value_type, Args...> { return emplace_at(pos._node - _data, std::forward<Args>(args)...); }

		/** Erases the last element of the list. */
		constexpr void pop_back() noexcept(std::is_nothrow_destructible_v<value_type>) { erase(header_t::prev()); }
		/** Erases the first element of the list. */
		constexpr void pop_front() noexcept(std::is_nothrow_destructible_v<value_type>) { erase(header_t::next()); }

		/** Erases the element located at \a pos.
		 * @param pos Position within the list at which to erase the element.
		 * @return Iterator to the element after the erased one, or `end()`. */
		constexpr iterator erase(const_iterator pos) noexcept(std::is_nothrow_destructible_v<value_type>) { return destroy_at(pos._node - _data); }
		/** Erases all elements in range [\a first, \a last).
		 * @param first Iterator to the first element to be erased.
		 * @param first Iterator one past the last element to be erased.
		 * @return Iterator to the element after the erased range, or `end()`. */
		constexpr iterator erase(const_iterator first, const_iterator last) noexcept(std::is_nothrow_destructible_v<value_type>)
		{
			auto next = end();
			for (; first != last; first = next)
				next = erase(first);
			return next;
		}

		/** Returns a copy of the list's allocator. */
		[[nodiscard]] constexpr allocator_type get_allocator() const noexcept(std::is_nothrow_constructible_v<allocator_type, const node_allocator &>) { return _alloc; }

		/** Compares `this` with \a other using `std::lexicographical_compare`. */
		[[nodiscard]] constexpr bool operator==(const array_list &other) const { return std::lexicographical_compare(begin(), end(), other.begin(), other.end(), std::equal_to<>{}); }
		/** Compares `this` with \a other using `std::lexicographical_compare_three_way`. */
		[[nodiscard]] constexpr auto operator<=>(const array_list &other) const { return std::lexicographical_compare_three_way(begin(), end(), other.begin(), other.end(), std::compare_three_way{}); }

		constexpr void swap(array_list &other) noexcept(std::allocator_traits<node_allocator>::is_always_equal::value || std::is_nothrow_swappable_v<node_allocator>)
		{
			if constexpr (std::allocator_traits<node_allocator>::propagate_on_container_swap::value)
				adl_swap(_alloc, other._alloc);
			else
				assert(_alloc == other._alloc);
			swap_data(other);
		}
		friend constexpr void swap(array_list &a, array_list &b) noexcept(std::allocator_traits<node_allocator>::is_always_equal::value || std::is_nothrow_swappable_v<node_allocator>) { a.swap(b); }

	private:
		constexpr header_pointer link_node(header_pointer node, header_pointer next)
		{
			if (next->prev())
			{
				next->prev()->next(node);
				node->prev(next->prev());
			}
			next->prev(node);
			node->next(next);

			_list_size += 1;
			return node;
		}
		constexpr header_pointer unlink_node(header_pointer node)
		{
			const auto next = node->next();
			const auto prev = node->prev();

			if (next)
				next->prev(prev);
			if (prev)
				prev->next(next);

			_list_size -= 1;
			return node;
		}

		template<typename... Args>
		constexpr header_pointer emplace_at(size_type pos, Args &&...args)
		{
			const auto node = acquire_node();
			try
			{
				std::construct_at(node->get(), std::forward<Args>(args)...);
				return link_node(node, _data + pos);
			}
			catch (...)
			{
				release_node(node);
				throw;
			}
		}
		constexpr header_pointer destroy_at(size_type pos)
		{
			const auto node = _data + pos;
			const auto next = unlink_node(node);

			std::destroy_at(node->get());
			release_node(node);
			return next;
		}

		constexpr void destroy_data(node_allocator &alloc)
		{
			clear();
			std::allocator_traits<node_allocator>::deallocate(alloc, _data, _data_size);
			_data = nullptr;
			_data_size = {};
		}
		constexpr void copy_data(const array_list &other)
		{
			for (decltype(auto) val : other)
				emplace_back(val);
		}
		constexpr void move_data(array_list &other)
		{
			for (decltype(auto) val : other)
				emplace_back(std::move(val));
		}
		constexpr void swap_data(array_list &other)
		{
			header_t::swap(other);
			std::swap(_data, other._data);
			std::swap(_free, other._free);
			std::swap(_data_size, other._data_size);
			std::swap(_list_size, other._list_size);
		}

		constexpr void reallocate()
		{
			const auto new_size = std::max(_data_size * 2, 1);
			const auto new_data = std::allocator_traits<node_allocator>::allocate(_alloc, new_size);

			try
			{
				for (size_type i = 0; i < _data_size; ++i)
				{
					const auto dst = new_data + i;
					const auto src = _data + i;

					std::construct_at(dst, std::move(*src));
					std::destroy_at(src);
				}

				std::allocator_traits<node_allocator>::deallocate(_alloc, _data, _data_size);
				_data_size = new_size;
				_data = new_data;
			}
			catch (...)
			{
				std::allocator_traits<node_allocator>::deallocate(_alloc, new_data, new_size);
				throw;
			}
		}
		constexpr auto acquire_node()
		{
			if (_free != nullptr)
				return std::exchange(_free, _free->next());

			if (_list_size >= _data_size)
				reallocate();
			return _data + _list_size;
		}
		constexpr void release_node(node_pointer node) noexcept
		{
			_free->prev(node);
			node->next(_free);
			_free = node;
		}

		ROD_NO_UNIQUE_ADDRESS node_allocator _alloc;
		node_pointer _data = {};
		node_pointer _free = {};
		size_type _data_size = {};
		size_type _list_size = {};
	};
}
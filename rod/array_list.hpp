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

	namespace _detail
	{
		struct array_list_header
		{
			constexpr array_list_header() noexcept = default;
			array_list_header(array_list_header &&other) noexcept
			{
				if (other.next())
				{
					other.next()->prev(this);
					next(other.next());
				}
				if (other.prev())
				{
					other.prev()->next(this);
					prev(other.prev());
				}
			}

			array_list_header *next() const noexcept { return reinterpret_cast<array_list_header *>(reinterpret_cast<std::intptr_t>(this) + next_off); }
			void next(const array_list_header *node) noexcept { next_off = reinterpret_cast<std::intptr_t>(node) - reinterpret_cast<std::intptr_t>(this); }

			array_list_header *prev() const noexcept { return reinterpret_cast<array_list_header *>(reinterpret_cast<std::intptr_t>(this) - prev_off); }
			void prev(const array_list_header *node) noexcept { prev_off = reinterpret_cast<std::intptr_t>(this) - reinterpret_cast<std::intptr_t>(node); }

			constexpr void swap(array_list_header &other)
			{
				std::swap(next_off, other.next_off);
				std::swap(prev_off, other.prev_off);
			}

			std::intptr_t next_off = {};
			std::intptr_t prev_off = {};
		};

		template<typename T, typename Alloc>
		struct array_list_node : array_list_header, empty_base<T>
		{
			template<typename... Args>
			constexpr explicit array_list_node(Args &&...args) : empty_base<T>(std::forward<Args>(args)...) {}

			[[nodiscard]] constexpr auto *get() noexcept { return empty_base<T>::get(); }
			[[nodiscard]] constexpr auto *get() const noexcept { return empty_base<T>::get(); }
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
			using header_t = _detail::array_list_header;
			using node_t = _detail::array_list_node<value_type, Alloc>;

			using header_pointer = select_allocator_pointer_t<copy_cv_t<T, header_t>, typename std::allocator_traits<Alloc>::template rebind_traits<header_t>>;
			using node_pointer = select_allocator_pointer_t<copy_cv_t<T, node_t>, typename std::allocator_traits<Alloc>::template rebind_traits<node_t>>;

			constexpr array_list_iterator(header_pointer node) noexcept : _node(node_pointer(node)) {}

		public:
			constexpr array_list_iterator() noexcept = default;
			template<typename U> requires(!std::same_as<T, U> && std::same_as<std::remove_const_t<T>, U>)
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

			[[nodiscard]] constexpr decltype(auto) get() const noexcept { return _node->get(); }
			[[nodiscard]] constexpr decltype(auto) operator->() const noexcept { return get(); }
			[[nodiscard]] constexpr decltype(auto) operator*() const noexcept { return *get(); }

			[[nodiscard]] friend constexpr bool operator==(const array_list_iterator &a, const array_list_iterator &b) noexcept = default;
			[[nodiscard]] friend constexpr bool operator!=(const array_list_iterator &a, const array_list_iterator &b) noexcept = default;

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
	class array_list : empty_base<typename std::allocator_traits<Alloc>::template rebind_alloc<_detail::array_list_node<T, Alloc>>>
	{
		using header_t = _detail::array_list_header;
		using node_t = _detail::array_list_node<T, Alloc>;

		using header_pointer = typename std::allocator_traits<Alloc>::template rebind_traits<header_t>::pointer;
		using const_header_pointer = typename std::allocator_traits<Alloc>::template rebind_traits<header_t>::const_pointer;

		using node_pointer = typename std::allocator_traits<Alloc>::template rebind_traits<node_t>::pointer;
		using const_node_pointer = typename std::allocator_traits<Alloc>::template rebind_traits<node_t>::const_pointer;

		using node_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<node_t>;
		using allocator_base = empty_base<node_allocator>;

	public:
		using value_type = T;
		using allocator_type = Alloc;

		using iterator = _detail::array_list_iterator<value_type, allocator_type>;
		using const_iterator = _detail::array_list_iterator<std::add_const_t<value_type>, allocator_type>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		using reference = typename iterator::reference;
		using const_reference = typename const_iterator::reference;
		using pointer = typename iterator::pointer;
		using const_pointer = typename const_iterator::pointer;

		using size_type = typename std::allocator_traits<allocator_type>::size_type;
		using difference_type = typename std::allocator_traits<allocator_type>::difference_type;

	private:
		constexpr static size_type apply_growth(size_type n) noexcept { return n + n / 2; }

	public:
		/** Initializes an empty list. */
		constexpr array_list() noexcept(std::is_nothrow_default_constructible_v<node_allocator>) : _header(), _data(nullptr), _free(end()._node) {}
		/** Initializes an empty list using allocator \a alloc. */
		constexpr array_list(const allocator_type &alloc) noexcept(std::is_nothrow_constructible_v<node_allocator, const allocator_type &>) : allocator_base(alloc), _header(), _data(nullptr), _free(end()._node) {}

		/** Copy-constructs a list from \a other. */
		constexpr array_list(const array_list &other) : array_list(other, std::allocator_traits<allocator_type>::select_on_container_copy_construction(other.get_allocator())) {}
		/** Copy-constructs a list from \a other with allocator \a alloc. */
		constexpr array_list(const array_list &other, const allocator_type &alloc) : allocator_base(alloc)
		{
			reserve(other.size());
			copy_data(other);
		}

		/** Move-constructs a list from \a other. */
		constexpr array_list(array_list &&other) noexcept(std::is_nothrow_move_constructible_v<node_allocator>) : allocator_base(std::move(other)) { swap_data(other); }
		/** Move-constructs a list from \a other with allocator \a alloc. */
		constexpr array_list(array_list &&other, const allocator_type &alloc) noexcept(std::allocator_traits<node_allocator>::is_always_equal::value && std::is_nothrow_constructible_v<node_allocator, const allocator_type &>) : allocator_base(alloc)
		{
			if (std::allocator_traits<node_allocator>::is_always_equal::value || allocator_base::value() == other.allocator_base::value())
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
		constexpr array_list(size_type n, const allocator_type &alloc) : allocator_base(alloc) { resize(n); }

		/** Initializes a list with \a n elements copy-constructed from \a val. */
		constexpr array_list(size_type n, const_reference val) : array_list(n, val, node_allocator{}) {}
		/** Initializes a list with \a n elements copy-constructed from \a val using allocator \a alloc. */
		constexpr array_list(size_type n, const_reference val, const allocator_type &alloc) : allocator_base(alloc) { resize(n, val); }

		/** Initializes a list with elements in range [\a first, \a last). */
		template<std::forward_iterator I, std::sentinel_for<I> S>
		constexpr array_list(I first, S last) requires std::constructible_from<value_type, std::iter_value_t<T>> { insert(first, last); }
		/** Initializes a list with elements in range [\a first, \a last) using allocator \a alloc. */
		template<std::forward_iterator I, std::sentinel_for<I> S>
		constexpr array_list(I first, S last, const allocator_type &alloc) requires std::constructible_from<value_type, std::iter_value_t<T>> : allocator_base(alloc) { insert(first, last); }

		/** Initializes a list with elements from initializer list \a il. */
		template<typename U = T>
		constexpr array_list(std::initializer_list<U> il) requires std::constructible_from<value_type, std::add_const_t<U>> { insert(il); }
		/** Initializes a list with elements from initializer list \a il using allocator \a alloc. */
		template<typename U = T>
		constexpr array_list(std::initializer_list<U> il, const allocator_type &alloc) requires std::constructible_from<value_type, std::add_const_t<U>> : allocator_base(alloc) { insert(il); }

		constexpr array_list &operator=(const array_list &other)
		{
			if (this != &other)
			{
				if (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value)
				{
					auto old_alloc = std::move(allocator_base::value());
					allocator_base::operator=(other.allocator_base::value());

					if (!std::allocator_traits<node_allocator>::is_always_equal::value && allocator_base::value() != old_alloc) [[unlikely]]
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
					auto old_alloc = std::move(allocator_base::value());
					allocator_base::operator=(std::move(other.allocator_base::value()));

					if (!std::allocator_traits<node_allocator>::is_always_equal::value && old_alloc != other.allocator_base::value()) [[unlikely]]
						destroy_data(old_alloc);

					swap_data(other);
				}
				else if (!std::allocator_traits<node_allocator>::is_always_equal::value && allocator_base::value() != other.allocator_base::value())
					assign(std::move_iterator(other.begin()), std::move_iterator(other.end()));
				else
					swap_data(other);
			}
			return *this;
		}

		constexpr ~array_list() { destroy_data(allocator_base::value()); }

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
		[[nodiscard]] iterator begin() noexcept { return _header.next(); }
		/** Returns const iterator to the first element of the list. */
		[[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }
		/** @copydoc begin */
		[[nodiscard]] const_iterator cbegin() const noexcept { return _header.next(); }

		/** Returns iterator one past the last element of the list. */
		[[nodiscard]] iterator end() noexcept { return &_header; }
		/** Returns const iterator one past the last element of the list. */
		[[nodiscard]] const_iterator end() const noexcept { return cend(); }
		/** @copydoc end */
		[[nodiscard]] const_iterator cend() const noexcept { return &_header; }

		/** Returns reverse iterator to the last element of the list. */
		[[nodiscard]] reverse_iterator rbegin() noexcept { return std::reverse_iterator(end()); }
		/** Returns const reverse iterator to the last of the list. */
		[[nodiscard]] const_reverse_iterator rbegin() const noexcept { return std::reverse_iterator(end()); }
		/** @copydoc rbegin */
		[[nodiscard]] const_reverse_iterator crbegin() const noexcept { return std::reverse_iterator(cend()); }

		/** Returns reverse iterator one past the first element of the list. */
		[[nodiscard]] reverse_iterator rend() noexcept { return std::reverse_iterator(begin()); }
		/** Returns const reverse iterator one past the first element of the list. */
		[[nodiscard]] const_reverse_iterator rend() const noexcept { return std::reverse_iterator(begin()); }
		/** @copydoc rend */
		[[nodiscard]] const_reverse_iterator crend() const noexcept { return std::reverse_iterator(cbegin()); }

		/** Returns reference to the first element of the list. */
		[[nodiscard]] reference front() noexcept { return *node_pointer(_header.next())->get(); }
		/** Returns const reference to the first element of the list. */
		[[nodiscard]] const_reference front() const noexcept { return *const_node_pointer(_header.next())->get(); }

		/** Returns reference to the last element of the list. */
		[[nodiscard]] reference back() noexcept { return *node_pointer(_header.prev())->get(); }
		/** Returns const reference to the last element of the list. */
		[[nodiscard]] const_reference back() const noexcept { return *const_node_pointer(_header.prev())->get(); }

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

			const auto new_size = std::max(apply_growth(_data_size), n);
			const auto new_data = std::allocator_traits<node_allocator>::allocate(allocator_base::value(), new_size);

			try
			{
				for (header_pointer pos = header_pointer(_header.next()), next; pos != &_header; pos = next)
				{
					const auto src = node_pointer(pos);
					const auto dst = new_data + src - _data;
					next = src->next();

					std::construct_at(dst, std::move(*src));
					std::destroy_at(src);
				}

				std::allocator_traits<node_allocator>::deallocate(allocator_base::value(), _data, _data_size);
				_data_size = new_size;
				_data = new_data;
			}
			catch (...)
			{
				std::allocator_traits<node_allocator>::deallocate(allocator_base::value(), new_data, new_size);
				throw;
			}
		}

		/** Resizes the list to contain \a n elements. New elements are default-initialized. */
		constexpr void resize(size_type n)
		{
			reserve(n);
			while (_list_size < n)
				emplace_back();
			while (_list_size > n)
				pop_back();
		}
		/** Resizes the list to contain \a n elements. New elements are copy-initialized from \a val. */
		constexpr void resize(size_type n, const_reference val)
		{
			reserve(n);
			while (_list_size < n)
				push_back(val);
			while (_list_size > n)
				pop_back();
		}

		/** Erases all elements of the list. */
		constexpr void clear() noexcept(std::is_nothrow_destructible_v<value_type>)
		{
			for (auto pos = begin(); pos != end(); ++pos)
				std::destroy_at(pos.get());

			_header.next(&_header);
			_header.prev(&_header);
			_free = end()._node;
			_list_size = {};
		}

		/** Inserts a copy-constructed element at the end of the list.
		 * @param val Value passed to the copy constructor of the new element.
		 * @return Iterator to the inserted element. */
		iterator push_back(const_reference val) { return insert(cend(), val); }
		/** Inserts a copy-constructed element at the start of the list.
		 * @param val Value passed to the copy constructor of the new element.
		 * @return Iterator to the inserted element. */
		iterator push_front(const_reference val) { return insert(cbegin(), val); }

		/** Inserts a move-constructed element at the end of the list.
		 * @param val Value passed to the move constructor of the new element.
		 * @return Iterator to the inserted element. */
		iterator push_back(value_type &&val) { return insert(cend(), std::forward<value_type>(val)); }
		/** Inserts a move-constructed element at the start of the list.
		 * @param val Value passed to the move constructor of the new element.
		 * @return Iterator to the inserted element. */
		iterator push_front(value_type &&val) { return insert(cbegin(), std::forward<value_type>(val)); }

		/** Inserts a copy-constructed element at the specified position within the list.
		 * @param pos Position within the list at which to insert the element.
		 * @param val Value passed to the copy constructor of the new element.
		 * @return Iterator to the inserted element. */
		iterator insert(const_iterator pos, const_reference val) { return emplace(pos, val); }
		/** Inserts a move-constructed element at the specified position within the list.
		 * @param pos Position within the list at which to insert the element.
		 * @param val Value passed to the move constructor of the new element.
		 * @return Iterator to the inserted element. */
		iterator insert(const_iterator pos, value_type &&val) { return emplace(pos, std::forward<value_type>(val)); }

		/** Inserts elements in range [\a first, \a last) at the specified position within the list.
		 * @param pos Position within the list at which to insert the elements.
		 * @param first Iterator to the first element to be inserted.
		 * @param last Sentinel for the \a first iterator.
		 * @return Iterator to the first inserted element. */
		template<std::forward_iterator I, std::sentinel_for<I> S>
		iterator insert(const_iterator pos, I first, S last) requires std::constructible_from<value_type, std::iter_value_t<T>>
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
		iterator insert(const_iterator pos, std::initializer_list<U> il) requires std::constructible_from<value_type, std::add_const_t<U>>
		{
			return insert(pos, il.begin(), il.end());
		}

		/** Inserts an in-place constructed element at the end of the list.
		 * @param pos Position within the list at which to insert the element.
		 * @param args Arguments passed to the constructor of the new element.
		 * @return Reference to the inserted element. */
		template<typename... Args>
		reference emplace_back(Args &&...args) requires std::constructible_from<value_type, Args...> { return emplace(cend(), std::forward<Args>(args)...); }
		/** Inserts an in-place constructed element at the start of the list.
		 * @param pos Position within the list at which to insert the element.
		 * @param args Arguments passed to the constructor of the new element.
		 * @return Reference to the inserted element. */
		template<typename... Args>
		reference emplace_front(Args &&...args) requires std::constructible_from<value_type, Args...> { return emplace(cbegin(), std::forward<Args>(args)...); }

		/** Inserts an in-place constructed element at the specified position within the list.
		 * @param pos Position within the list at which to insert the element.
		 * @param args Arguments passed to the constructor of the new element.
		 * @return Reference to the inserted element. */
		template<typename... Args>
		reference emplace(const_iterator pos, Args &&...args) requires std::constructible_from<value_type, Args...> { return *emplace_at(const_cast<node_pointer>(pos._node), std::forward<Args>(args)...); }

		/** Erases the last element of the list. */
		void pop_back() noexcept(std::is_nothrow_destructible_v<value_type>) { erase(_header.prev()); }
		/** Erases the first element of the list. */
		void pop_front() noexcept(std::is_nothrow_destructible_v<value_type>) { erase(_header.next()); }

		/** Erases the element located at \a pos.
		 * @param pos Position within the list at which to erase the element.
		 * @return Iterator to the element after the erased one, or `end()`. */
		iterator erase(const_iterator pos) noexcept(std::is_nothrow_destructible_v<value_type>) { return destroy_at(const_cast<node_pointer>(pos._node)); }
		/** Erases all elements in range [\a first, \a last).
		 * @param first Iterator to the first element to be erased.
		 * @param last Iterator one past the last element to be erased.
		 * @return Iterator to the element after the erased range, or `end()`. */
		iterator erase(const_iterator first, const_iterator last) noexcept(std::is_nothrow_destructible_v<value_type>)
		{
			auto next = end();
			for (; first != last; first = next)
				next = erase(first);
			return next;
		}

		/** Returns a copy of the list's allocator. */
		[[nodiscard]] constexpr allocator_type get_allocator() const noexcept(std::is_nothrow_constructible_v<allocator_type, const node_allocator &>) { return allocator_base::value(); }

		[[nodiscard]] friend constexpr bool operator==(const array_list &a, const array_list &b) noexcept { return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), std::equal_to<>{}); }
		[[nodiscard]] friend constexpr auto operator<=>(const array_list &a, const array_list &b) noexcept { return std::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end(), std::compare_three_way{}); }

		constexpr void swap(array_list &other) noexcept(std::allocator_traits<node_allocator>::is_always_equal::value || std::is_nothrow_swappable_v<node_allocator>)
		{
			if constexpr (std::allocator_traits<node_allocator>::propagate_on_container_swap::value)
				adl_swap(allocator_base::value(), other.allocator_base::value());
			else
				assert(allocator_base::value() == other.allocator_base::value());
			swap_data(other);
		}
		friend constexpr void swap(array_list &a, array_list &b) noexcept(std::allocator_traits<node_allocator>::is_always_equal::value || std::is_nothrow_swappable_v<node_allocator>) { a.swap(b); }

	private:
		template<typename... Args>
		constexpr iterator emplace_at(node_pointer next, Args &&...args)
		{
			const auto node = acquire_node();
			try
			{
				std::construct_at(node->get(), std::forward<Args>(args)...);

				next->prev()->next(node);
				node->prev(next->prev());
				next->prev(node);
				node->next(next);

				_list_size += 1;
				return next;
			}
			catch (...)
			{
				release_node(node);
				throw;
			}
		}
		constexpr iterator destroy_at(node_pointer node)
		{
			_list_size -= 1;
			std::destroy_at(node->get());
			return release_node(node);
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
			_header.swap(other);
			std::swap(_data, other._data);
			std::swap(_free, other._free);
			std::swap(_data_size, other._data_size);
			std::swap(_list_size, other._list_size);
		}

		constexpr node_pointer acquire_node()
		{
			if (_free != &_header)
			{
				auto next = node_pointer(_free->next());
				return std::exchange(_free, next);
			}

			if (_list_size >= _data_size)
			{
				const auto new_size = std::max<size_type>(apply_growth(_data_size), 1);
				const auto new_data = std::allocator_traits<node_allocator>::allocate(allocator_base::value(), new_size);

				try
				{
					for (size_type i = 0; i < _data_size; ++i)
					{
						const auto dst = new_data + i;
						const auto src = _data + i;

						std::construct_at(dst, std::move(*src));
						std::destroy_at(src);
					}

					std::allocator_traits<node_allocator>::deallocate(allocator_base::value(), _data, _data_size);
					_data_size = new_size;
					_data = new_data;
				}
				catch (...)
				{
					std::allocator_traits<node_allocator>::deallocate(allocator_base::value(), new_data, new_size);
					throw;
				}
			}
			return _data + _list_size;
		}
		constexpr node_pointer release_node(node_pointer node) noexcept
		{
			const auto next = node->next();
			const auto prev = node->prev();

			next->prev(prev);
			prev->next(next);
			node->next(std::exchange(_free, node));
			return static_cast<node_pointer>(next);
		}

		header_t _header;
		node_pointer _data;
		node_pointer _free;
		size_type _data_size = {};
		size_type _list_size = {};
	};

	template<typename I, typename S>
	array_list(I, S) -> array_list<std::iter_value_t<I>>;
	template<typename I, typename S, typename Alloc = std::allocator<std::iter_value_t<I>>>
	array_list(I, S, const Alloc &) -> array_list<std::iter_value_t<I>, Alloc>;

	template<typename T>
	array_list(std::initializer_list<T>) -> array_list<T>;
	template<typename T, typename Alloc = std::allocator<T>>
	array_list(std::initializer_list<T>, const Alloc &) -> array_list<T, Alloc>;

	static_assert(std::ranges::bidirectional_range<array_list<int>>);
}

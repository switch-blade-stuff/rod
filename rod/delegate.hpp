/*
 * Created by switchblade on 2023-04-06.
 */

#pragma once

#include <functional>
#include <array>
#include <bit>

#include "utility.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	namespace detail
	{
		template<typename T>
		struct delegate_traits : std::false_type {};
		template<typename R, typename... Args>
		struct delegate_traits<R(Args...)> : std::true_type
		{
			enum flags_t
			{
				is_local = 1,
				is_owned = 2,
				flags_bits = 2,
				flags_mask = 3,
			};

			using return_type = R;
			using arg_types = type_list_t<Args...>;
			using native_function = R(*)(void *, Args...);

			template<typename T>
			constexpr static bool is_invocable = std::is_invocable_r_v<R, T, Args...>;
			template<typename T>
			constexpr static bool is_const_invocable = std::is_invocable_r_v<R, std::add_const_t<T>, Args...>;
		};

		template<std::size_t Size>
		class alignas(std::uintptr_t) delegate_storage
		{
		public:
			[[nodiscard]] void *data() noexcept { return bytes(); }
			[[nodiscard]] const void *data() const noexcept { return bytes(); }
			[[nodiscard]] constexpr std::byte *bytes() noexcept { return _bytes.data(); }
			[[nodiscard]] constexpr const std::byte *bytes() const noexcept { return _bytes.data(); }

			constexpr void swap(delegate_storage &other) noexcept
			{
				/* std::swap creates a manual byte copy loop. */
				auto tmp = _bytes;
				_bytes = other._bytes;
				other._bytes = tmp;
			}
			template<std::size_t OtherSize> requires(OtherSize <= Size)
			constexpr void copy(const delegate_storage<OtherSize> &other) noexcept
			{
				std::copy_n(other.bytes(), OtherSize, bytes());
			}

		private:
			std::array<std::byte, Size> _bytes = {};
		};
		template<>
		class delegate_storage<0>
		{
		public:
			[[nodiscard]] void *data() noexcept { return nullptr; }
			[[nodiscard]] const void *data() const noexcept { return nullptr; }
			[[nodiscard]] constexpr std::byte *bytes() noexcept { return nullptr; }
			[[nodiscard]] constexpr const std::byte *bytes() const noexcept { return nullptr; }

			constexpr void swap(delegate_storage &) noexcept {}
			template<std::size_t OtherSize> requires(OtherSize <= 0)
			constexpr void copy(const delegate_storage<OtherSize> &) noexcept {}
		};

		template<typename T>
		struct delegate_heap_data;
		template<>
		struct delegate_heap_data<void>
		{
			constexpr delegate_heap_data *copy() const { return copy_func(this); }
			constexpr void destroy() { delete_func(this); }

			delegate_heap_data *(*copy_func)(const delegate_heap_data *);
			void (*delete_func)(delegate_heap_data *);
		};
		template<typename T>
		struct delegate_heap_data : delegate_heap_data<void>
		{
			template<typename... Args>
			delegate_heap_data(Args &&...args) : value(std::forward<Args>(args)...)
			{
				this->copy_func = [](const delegate_heap_data<void> *ptr) -> delegate_heap_data<void> *
				{
					return new delegate_heap_data(static_cast<const delegate_heap_data *>(ptr)->value);
				};
				this->delete_func = [](delegate_heap_data<void> *ptr)
				{
					delete static_cast<delegate_heap_data *>(ptr);
				};
			}

			[[ROD_NO_UNIQUE_ADDRESS]] T value;
		};

		/* Ugly mess to strip instance qualifiers from function signatures. Unfortunately, remove_cvref_t does not work for functions. */
		template<typename>
		struct strip_qualifiers;

		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...)> { using type = R(Args...); };
		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) const> { using type = R(Args...); };
		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) volatile> { using type = R(Args...); };
		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) const volatile> { using type = R(Args...); };

		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) &> { using type = R(Args...); };
		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) const &> { using type = R(Args...); };
		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) volatile &> { using type = R(Args...); };
		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) const volatile &> { using type = R(Args...); };

		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) &&> { using type = R(Args...); };
		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) const &&> { using type = R(Args...); };
		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) volatile &&> { using type = R(Args...); };
		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) const volatile &&> { using type = R(Args...); };

		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) noexcept> { using type = R(Args...) noexcept; };
		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) const noexcept> { using type = R(Args...) noexcept; };
		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) volatile noexcept> { using type = R(Args...) noexcept; };
		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) const volatile noexcept> { using type = R(Args...) noexcept; };

		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) & noexcept> { using type = R(Args...) noexcept; };
		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) const & noexcept> { using type = R(Args...) noexcept; };
		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) volatile & noexcept> { using type = R(Args...) noexcept; };
		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) const volatile & noexcept> { using type = R(Args...) noexcept; };

		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) && noexcept> { using type = R(Args...) noexcept; };
		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) const && noexcept> { using type = R(Args...) noexcept; };
		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) volatile && noexcept> { using type = R(Args...) noexcept; };
		template<typename R, typename... Args>
		struct strip_qualifiers<R(Args...) const volatile && noexcept> { using type = R(Args...) noexcept; };

		template<typename T>
		using strip_qualifiers_t = typename strip_qualifiers<T>::type;

		template<typename...>
		struct deduce_signature;
		template<typename R, typename T>
		struct deduce_signature<R T::*> { using type = R(); };
		template<typename F, typename T> requires std::is_function_v<F>
		struct deduce_signature<F T::*> { using type = F; };

		template<typename T>
		using deduce_signature_t = typename deduce_signature<std::remove_cv_t<T>>::type;

		template<typename, typename, auto>
		struct check_member : std::false_type {};
		template<typename R, typename... Args, typename T, auto F> requires std::is_member_pointer_v<decltype(F)>
		struct check_member<R(Args...), T, F> : std::is_invocable_r<R, decltype(F), T, Args...> {};
		template<typename R, typename... Args, typename T, auto F> requires std::is_member_pointer_v<decltype(F)>
		struct check_member<R(Args...) noexcept, T, F> : std::is_nothrow_invocable_r<R, decltype(F), T, Args...> {};
	}

	/** Utility used to specify a member function pointer for construction of `delegate`. */
	template<auto F> requires std::is_member_pointer_v<decltype(F)>
	struct bind_member_t {};
	/** Instance of `bind_member_t<F>` */
	template<auto F>
	inline constexpr auto bind_member = bind_member_t<F>{};

	/** @brief Versatile type-erased functor or functor reference.
	 *
	 * Delegate act as a lightweight versatile alternative to `std::function`.
	 * Unlike `std::function`, delegates can hold references to externally stored functors,
	 * can be directly constructed from a member function pointer and object instance,
	 * and can be used to invoke C-style APIs taking a callback and data pointer.
	 *
	 * @tparam F Function signature of the delegate.
	 * @tparam Buffer Size of the in-place storage buffer (default is 2 pointers). If set to `0`, non-empty functors will always be allocated on the heap. */
	template<typename F, std::size_t Buffer = sizeof(std::uintptr_t) * 2>
	class delegate : detail::delegate_storage<Buffer>
	{
		static_assert(detail::delegate_traits<F>::value, "First delegate template parameter must be a function signature");

		template<typename, std::size_t>
		friend class delegate;

		using storage_t = detail::delegate_storage<Buffer>;
		using traits_t = detail::delegate_traits<F>;
		using flags_t = typename traits_t::flags_t;

		template<typename T>
		constexpr static bool is_by_value = alignof(T) <= alignof(std::uintptr_t) && (std::is_empty_v<T> || sizeof(T) <= Buffer) &&
		                                    std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>;

		template<typename T>
		using heap_data = detail::delegate_heap_data<T>;

	public:
		/** Type of the native C-style invoke callback function. */
		using native_function_t = typename traits_t::native_function;

	private:
		template<std::size_t N>
		using buffer_data = heap_data<detail::delegate_storage<N>>;

		template<std::size_t Size>
		[[nodiscard]] static auto *make_heap_buffer()
		{
			auto *storage = new buffer_data<Size>();
			storage->copy_func = [](const heap_data<void> *ptr) -> heap_data<void> *
			{
				return new buffer_data<Size>(static_cast<const buffer_data<Size> *>(ptr)->value);
			};
			storage->delete_func = [](heap_data<void> *ptr)
			{
				delete static_cast<buffer_data<Size> *>(ptr);
			};
			return storage;
		}

	public:
		constexpr delegate() noexcept = default;

		delegate(const delegate &other) { copy_from(other); }
		delegate &operator=(const delegate &other)
		{
			if (this != &other)
			{
				destroy();
				copy_from(other);
			}
			return *this;
		}

		delegate(delegate &&other) noexcept { swap(other); }
		delegate &operator=(delegate &&other) noexcept
		{
			if (this != &other) swap(other);
			return *this;
		}

		/** Copy-constructs the delegate from a delegate of a different buffer size. */
		template<std::size_t OtherBuffer> requires(Buffer != OtherBuffer)
		delegate(const delegate<F, OtherBuffer> &other) { copy_from(other); }
		/** Copy-assigns the delegate from a delegate of a different buffer size. */
		template<std::size_t OtherBuffer> requires(Buffer != OtherBuffer)
		delegate &operator=(const delegate<F, OtherBuffer> &other)
		{
			destroy();
			copy_from(other);
			return *this;
		}

		/** Move-constructs the delegate from a delegate of a different buffer size. */
		template<std::size_t OtherBuffer> requires(Buffer != OtherBuffer)
		delegate(delegate<F, OtherBuffer> &&other) noexcept(OtherBuffer < Buffer) { move_from(other); }
		/** Move-assigns the delegate from a delegate of a different buffer size. */
		template<std::size_t OtherBuffer> requires(Buffer != OtherBuffer)
		delegate &operator=(delegate<F, OtherBuffer> &&other) noexcept(OtherBuffer < Buffer)
		{
			/* Use a placeholder with the largest buffer size to avoid allocations. */
			auto tmp = delegate<F, std::max(Buffer, OtherBuffer)>{std::move(*this)};
			move_from(other);
			return *this;
		}

		~delegate() { destroy(); }

		/** Initializes the delegate with a reference to a functor. */
		template<typename T>
		delegate(T &func) requires(!decays_to<T, delegate> && !std::is_function_v<T>) { init_ref(typename traits_t::arg_types{}, func); }
		/** Initializes the delegate with an in-place constructed functor. */
		template<typename T, typename U = std::decay_t<T>>
		delegate(T &&func) requires(!std::same_as<U, delegate> && !std::is_function_v<U>) { init_value<U>(typename traits_t::arg_types{}, std::forward<T>(func)); }
		/** Initializes the delegate with functor of type \a F constructed in-place from \a args. */
		template<typename T, typename... Args>
		delegate(std::in_place_type_t<T>, Args &&...args) { init_value<T>(typename traits_t::arg_types{}, std::forward<Args>(args)...); }

		/** Initializes the delegate from a function pointer. */
		template<typename R, typename... Args>
		delegate(R (*func)(Args...)) { init_func(typename traits_t::arg_types{}, func); }
		/** Initializes the delegate from an invoker callback and a data pointer. */
		delegate(native_function_t invoke, void *data) noexcept : _invoke(invoke), _data_flags(std::bit_cast<std::uintptr_t>(data)) {}
		/** Initializes the delegate from a member function and instance pointers. */
		template<typename T, auto Mem>
		delegate(bind_member_t<Mem>, T &&instance) { init_obj_mem<Mem>(typename traits_t::arg_types{}, std::forward<T>(instance)); }

		/** Checks if the delegate is empty. */
		[[nodiscard]] constexpr operator bool() const noexcept { return _invoke != nullptr; }

		/** Resets the delegate to an empty state. */
		void reset()
		{
			destroy();
			_invoke = {};
			_data_flags = {};
		}

		/** Returns pointer to the data of the delegate. */
		[[nodiscard]] void *data() const noexcept { return std::bit_cast<void *>(_data_flags & ~flags_t::flags_mask); }
		/** Returns pointer to the C-style invoker function of the delegate. */
		[[nodiscard]] native_function_t native_function() const noexcept { return _invoke; }

		/** Invokes the underlying functor with \a args. */
		template<typename... Args>
		auto invoke(Args &&...args) const -> std::invoke_result_t<native_function_t, void *, Args...> { return _invoke(data(), std::forward<Args>(args)...); }
		/** @copydoc invoke */
		template<typename... Args>
		auto operator()(Args &&...args) const -> std::invoke_result_t<native_function_t, void *, Args...> { return invoke(std::forward<Args>(args)...); }

		/** Exchanges the contents of this delegate with \a other. */
		void swap(delegate &other) noexcept
		{
			storage_t::swap(other);
			std::swap(_invoke, other._invoke);

			const auto a_data = this->make_swapped_data(other);
			const auto b_data = other.make_swapped_data(*this);
			this->_data_flags = a_data;
			other._data_flags = b_data;
		}

	private:
		constexpr flags_t flags(flags_t value) const noexcept
		{
			_data_flags = (_data_flags & ~flags_t::flags_mask) | value;
			return value;
		}
		[[nodiscard]] constexpr flags_t flags() const noexcept { return static_cast<flags_t>(_data_flags & flags_t::flags_mask); }

		[[nodiscard]] auto make_swapped_data(const delegate &other) const noexcept
		{
			if (other._data_flags & flags_t::is_local)
				return std::bit_cast<std::uintptr_t>(storage_t::data()) | other.flags();
			else
				return other._data_flags;
		}

		template<typename... Args, typename FR, typename... FArgs>
		void init_func(type_list_t<Args...>, FR (*func)(FArgs...)) requires traits_t::template is_invocable<FR (*)(FArgs...)>
		{
			_invoke = [](void *ptr, Args ...args) -> typename traits_t::return_type { return reinterpret_cast<FR (*)(FArgs...)>(ptr)(args...); };
			_data_flags = std::bit_cast<std::uintptr_t>(func);
		}

		template<typename T, typename... Args>
		void init_ref(type_list_t<Args...>, T &func) requires traits_t::template is_invocable<T>
		{
			_invoke = [](void *ptr, Args ...args) -> typename traits_t::return_type { return std::invoke(*static_cast<T *>(ptr), args...); };
			_data_flags = std::bit_cast<std::uintptr_t>(&func);
		}
		template<typename T, typename... Args, typename... TArgs>
		void init_value(type_list_t<Args...>, TArgs &&...args) requires traits_t::template is_invocable<T> && std::constructible_from<T, TArgs...>
		{
			if constexpr (is_by_value<T> && traits_t::template is_const_invocable<T>)
			{
				const auto ptr = static_cast<T *>(storage_t::data());
				new(std::launder(ptr)) T(std::forward<TArgs>(args)...);

				_invoke = [](void *ptr, Args ...args) -> typename traits_t::return_type { return std::invoke(*static_cast<const T *>(ptr), args...); };
				_data_flags = std::bit_cast<std::uintptr_t>(ptr) | flags_t::is_owned | flags_t::is_local;
			}
			else
			{
				const auto ptr = new heap_data<T>(std::forward<TArgs>(args)...);
				_invoke = [](void *ptr, Args ...args) -> typename traits_t::return_type { return std::invoke(static_cast<const heap_data<T> *>(ptr)->value, args...); };
				_data_flags = std::bit_cast<std::uintptr_t>(ptr) | flags_t::is_owned;
			}
		}

		template<auto Mem, typename... Args, typename T>
		void init_obj_mem(type_list_t<Args...>, T *instance) requires(detail::check_member<F, T *, Mem>::value && alignof(T) > flags_t::flags_bits)
		{
			_invoke = [](void *ptr, Args ...args) -> typename traits_t::return_type { return std::invoke(Mem, *static_cast<T *>(ptr), args...); };
			_data_flags = std::bit_cast<std::uintptr_t>(instance);
		}
		template<auto Mem, typename... Args, typename T, typename U = std::decay_t<T>>
		void init_obj_mem(type_list_t<Args...>, T &&instance) requires detail::check_member<F, std::remove_cvref_t<T>, Mem>::value
		{
			if constexpr (is_by_value<U>)
			{
				const auto ptr = static_cast<U *>(storage_t::data());
				new(std::launder(ptr)) U(std::forward<T>(instance));

				_invoke = [](void *ptr, Args ...args) -> typename traits_t::return_type { return std::invoke(Mem, *static_cast<const U *>(ptr), args...); };
				_data_flags = std::bit_cast<std::uintptr_t>(ptr) | flags_t::is_owned | flags_t::is_local;
			}
			else
			{
				const auto ptr = new heap_data<U>(std::forward<T>(instance));
				_invoke = [](void *ptr, Args ...args) -> typename traits_t::return_type { return std::invoke(Mem, static_cast<const heap_data<U> *>(ptr)->value, args...); };
				_data_flags = std::bit_cast<std::uintptr_t>(ptr) | flags_t::is_owned;
			}
		}

		template<std::size_t OtherBuffer>
		void move_from(delegate<F, OtherBuffer> &other) noexcept(OtherBuffer <= Buffer)
		{
			_invoke = std::exchange(other._invoke, {});

			/* Reference, heap-allocated, and small buffer delegates can be moved via thin copy. */
			if (const auto new_flags = other.flags(); !(new_flags & flags_t::is_owned))
				_data_flags = other._data_flags;
			else if ((new_flags & flags_t::is_local) && OtherBuffer <= Buffer)
			{
				_data_flags = std::bit_cast<std::uintptr_t>(storage_t::data()) | new_flags;
				storage_t::copy(other);
			}
			else
			{
				/* Allocate new object with default copy & fake deleter. */
				auto *buff = make_heap_buffer<OtherBuffer>();
				_data_flags = std::bit_cast<std::uintptr_t>(buff) | flags_t::is_owned;
				buff->value.copy(other);
			}
			other._data_flags = {};
		}
		template<std::size_t OtherBuffer>
		void copy_from(const delegate<F, OtherBuffer> &other)
		{
			_invoke = other._invoke;

			if (const auto new_flags = other.flags(); !new_flags)
				_data_flags = other._data_flags;
			else if (new_flags == flags_t::is_owned)
			{
				auto data = static_cast<heap_data<void> *>(other.data())->copy();
				_data_flags = std::bit_cast<std::uintptr_t>(data) | new_flags;
			}
			else if constexpr (OtherBuffer <= Buffer)
			{
				_data_flags = std::bit_cast<std::uintptr_t>(storage_t::data()) | new_flags;
				storage_t::copy(other);
			}
			else
			{
				auto *data = make_heap_buffer<OtherBuffer>(other);
				_data_flags = std::bit_cast<std::uintptr_t>(data) | flags_t::is_owned;
				data->value.copy(other);
			}
		}
		void destroy()
		{
			if (flags() == flags_t::is_owned)
			{
				static_cast<heap_data<void> *>(data())->destroy();
				_data_flags = {};
			}
		}

		native_function_t _invoke = {};
		std::uintptr_t _data_flags = {};
	};

	template<typename F, std::size_t N>
	void swap(delegate<F, N> &a, delegate<F, N> &b) noexcept { a.swap(b); }

	template<typename R, typename... Args>
	delegate(R (*)(Args...)) -> delegate<R(Args...)>;
	template<typename R, typename... Args>
	delegate(R (*)(void *, Args...), void *) -> delegate<R(Args...)>;

	template<typename F>
	delegate(F &&) -> delegate<detail::strip_qualifiers_t<detail::deduce_signature_t<decltype(&std::decay_t<F>::operator())>>>;
	template<auto Mem, typename T>
	delegate(bind_member_t<Mem>, T &&) -> delegate<detail::strip_qualifiers_t<detail::deduce_signature_t<decltype(Mem)>>>;

	/** Creates a delegate from a member pointer and an object instance pointer. */
	template<auto Mem, std::size_t Buffer = sizeof(std::uintptr_t) * 2, typename T>
	[[nodiscard]] inline auto member_delegate(T &&instance) -> delegate<detail::strip_qualifiers_t<detail::deduce_signature_t<decltype(Mem)>>, Buffer>
	{
		return {bind_member<Mem>, std::forward<T>(instance)};
	}
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

/*
 * Created by switchblade on 2023-04-06.
 */

#pragma once

#include <functional>
#include <bit>

#include "utility.hpp"

namespace rod
{
	namespace detail
	{
		template<typename T>
		struct delegate_traits : std::false_type {};
		template<typename R, typename... Args>
		struct delegate_traits<R(Args...)> : std::true_type
		{
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
			[[nodiscard]] constexpr std::byte *bytes() noexcept { return m_bytes.data(); }
			[[nodiscard]] constexpr const std::byte *bytes() const noexcept { return m_bytes.data(); }

			constexpr void swap(delegate_storage &other) noexcept { std::swap(m_bytes, other.m_bytes); }
			constexpr void copy(const delegate_storage &other) noexcept { m_bytes = other.m_bytes; }

		private:
			std::array<std::byte, Size> m_bytes = {};
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
			constexpr void copy(const delegate_storage &) noexcept {}
		};

		template<typename...>
		struct deduce_signature;
		template<typename R, typename T>
		struct deduce_signature<R T::*> { using type = R(); };
		template<typename F, typename T> requires std::is_function_v<F>
		struct deduce_signature<F T::*> { using type = F; };

		template<typename T>
		using deduce_signature_t = deduce_signature<std::remove_cv_t<T>>;

		template<typename, typename, auto>
		struct check_member : std::false_type {};
		template<typename R, typename... Args, typename T, auto F> requires std::is_member_pointer_v<decltype(F)>
		struct check_member<R(Args...), T, F> : std::bool_constant<requires(T i, Args...args){ std::invoke(F, i, args...); }> {};
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
	 * @tparam LocalBytes Size of the in-place storage buffer. If set to `0`, non-empty functors will always be allocated on the heap. */
	template<typename F, std::size_t LocalBytes = sizeof(std::uintptr_t) * 2> requires detail::delegate_traits<F>::value
	class delegate : detail::delegate_storage<LocalBytes>
	{
		using storage_t = detail::delegate_storage<LocalBytes>;
		using traits_t = detail::delegate_traits<F>;

		template<typename T>
		constexpr static bool is_by_value = alignof(T) <= alignof(std::uintptr_t) && (std::is_empty_v<T> || sizeof(T) <= LocalBytes) &&
		                                    std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>;

		template<typename T>
		struct heap_storage;
		template<>
		struct heap_storage<void>
		{
			constexpr heap_storage *copy() const { return copy_func(this); }
			constexpr void destroy() { delete_func(this); }

			heap_storage *(*copy_func)(const heap_storage *);
			void (*delete_func)(heap_storage *);
		};

		enum flags_t
		{
			is_local = 1,
			is_owned = 2,
			flags_bits = 2,
			flags_mask = 3,
		};

	public:
		/** Type of the native C-style invoke callback function. */
		using native_function_t = typename traits_t::native_function;

	private:
		template<typename T>
		struct heap_storage : heap_storage<void>
		{
			template<typename... Args>
			heap_storage(Args &&...args) : value(std::forward<Args>(args)...)
			{
				this->copy_func = [](const heap_storage<void> *ptr) -> heap_storage<void> *
				{
					return new heap_storage(static_cast<const heap_storage *>(ptr)->value);
				};
				this->delete_func = [](heap_storage<void> *ptr)
				{
					delete static_cast<heap_storage *>(ptr);
				};
			}

			[[ROD_NO_UNIQUE_ADDRESS]] T value;
		};

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

		~delegate() { destroy(); }

		/** Initializes the delegate with a reference to a functor. */
		template<typename T>
		delegate(T &func) requires (!detail::decays_to<T, delegate> && !std::is_function_v<T>) { init_ref(typename traits_t::arg_types{}, func); }
		/** Initializes the delegate with an in-place constructed functor. */
		template<typename T, typename U = std::decay_t<T>>
		delegate(T &&func) requires (!std::same_as<U, delegate> && !std::is_function_v<U>) { init_value<U>(typename traits_t::arg_types{}, std::forward<T>(func)); }
		/** Initializes the delegate with functor of type \a F constructed in-place from \a args. */
		template<typename T, typename... Args>
		delegate(std::in_place_type_t<T>, Args &&...args) { init_value<T>(typename traits_t::arg_types{}, std::forward<Args>(args)...); }

		/** Initializes the delegate from a function pointer. */
		template<typename R, typename... Args>
		delegate(R (*func)(Args...)) { init_func(typename traits_t::arg_types{}, func); }
		/** Initializes the delegate from an invoker callback and a data pointer. */
		delegate(native_function_t invoke, void *data) noexcept : m_invoke(invoke), m_data_flags(std::bit_cast<std::uintptr_t>(data)) {}
		/** Initializes the delegate from a member function and instance pointers. */
		template<typename T, auto Mem>
		delegate(bind_member_t<Mem>, T &&instance) { init_obj_mem<Mem>(typename traits_t::arg_types{}, std::forward<T>(instance)); }

		/** Checks if the delegate is empty. */
		[[nodiscard]] constexpr operator bool() const noexcept { return m_invoke != nullptr; }

		/** Resets the delegate to an empty state. */
		void reset()
		{
			destroy();
			m_invoke = {};
			m_data_flags = {};
		}

		/** Returns pointer to the data of the delegate. */
		[[nodiscard]] void *data() const noexcept { return std::bit_cast<void *>(m_data_flags & ~flags_mask); }
		/** Returns pointer to the C-style invoker function of the delegate. */
		[[nodiscard]] native_function_t native_function() const noexcept { return m_invoke; }

		/** Invokes the underlying functor with \a args. */
		template<typename... Args>
		auto invoke(Args &&...args) const -> std::invoke_result_t<native_function_t, void *, Args...> { return m_invoke(data(), std::forward<Args>(args)...); }
		/** @copydoc invoke */
		template<typename... Args>
		auto operator()(Args &&...args) const -> std::invoke_result_t<native_function_t, void *, Args...> { return invoke(std::forward<Args>(args)...); }

		/** Exchanges the contents of this delegate with \a other. */
		void swap(delegate &other) noexcept
		{
			storage_t::swap(other);
			std::swap(m_invoke, other.m_invoke);

			const auto a_data = this->make_swapped_data(other);
			const auto b_data = other.make_swapped_data(*this);
			this->m_data_flags = a_data;
			other.m_data_flags = b_data;
		}

	private:
		constexpr flags_t flags(flags_t value) const noexcept
		{
			m_data_flags = (m_data_flags & ~flags_mask) | value;
			return value;
		}
		[[nodiscard]] constexpr flags_t flags() const noexcept { return static_cast<flags_t>(m_data_flags & flags_mask); }

		[[nodiscard]] auto make_swapped_data(const delegate &other) const noexcept
		{
			if (other.m_data_flags & is_local)
				return std::bit_cast<std::uintptr_t>(storage_t::data()) | other.flags();
			else
				return other.m_data_flags;
		}

		template<typename... Args, typename FR, typename... FArgs>
		void init_func(type_list_t<Args...>, FR (*func)(FArgs...)) requires traits_t::template is_invocable<FR (*)(FArgs...)>
		{
			m_invoke = [](void *ptr, Args ...args) -> typename traits_t::return_type { return reinterpret_cast<FR (*)(FArgs...)>(ptr)(args...); };
			m_data_flags = std::bit_cast<std::uintptr_t>(func);
		}

		template<typename T, typename... Args>
		void init_ref(type_list_t<Args...>, T &func) requires traits_t::template is_invocable<T>
		{
			m_invoke = [](void *ptr, Args ...args) -> typename traits_t::return_type { return std::invoke(*static_cast<T *>(ptr), args...); };
			m_data_flags = std::bit_cast<std::uintptr_t>(&func);
		}
		template<typename T, typename... Args, typename... TArgs>
		void init_value(type_list_t<Args...>, TArgs &&...args) requires traits_t::template is_invocable<T> && std::constructible_from<T, TArgs...>
		{
			if constexpr (is_by_value<T> && traits_t::template is_const_invocable<T>)
			{
				const auto ptr = static_cast<T *>(storage_t::data());
				new(std::launder(ptr)) T(std::forward<TArgs>(args)...);

				m_invoke = [](void *ptr, Args ...args) -> typename traits_t::return_type { return std::invoke(*static_cast<const T *>(ptr), args...); };
				m_data_flags = std::bit_cast<std::uintptr_t>(ptr) | is_owned | is_local;
			}
			else
			{
				const auto ptr = new heap_storage<T>(std::forward<TArgs>(args)...);
				m_invoke = [](void *ptr, Args ...args) -> typename traits_t::return_type { return std::invoke(static_cast<const heap_storage<T> *>(ptr)->value, args...); };
				m_data_flags = std::bit_cast<std::uintptr_t>(ptr) | is_owned;
			}
		}

		template<auto Mem, typename... Args, typename T>
		void init_obj_mem(type_list_t<Args...>, T *instance) requires (detail::check_member<F, T *, Mem>::value && alignof(T) > flags_bits)
		{
			m_invoke = [](void *ptr, Args ...args) -> typename traits_t::return_type { return std::invoke(Mem, *static_cast<T *>(ptr), args...); };
			m_data_flags = std::bit_cast<std::uintptr_t>(instance);
		}
		template<auto Mem, typename... Args, typename T, typename U = std::decay_t<T>>
		void init_obj_mem(type_list_t<Args...>, T &&instance) requires detail::check_member<F, std::remove_cvref_t<T>, Mem>::value
		{
			if constexpr (is_by_value<U>)
			{
				const auto ptr = static_cast<U *>(storage_t::data());
				new(std::launder(ptr)) U(std::forward<T>(instance));

				m_invoke = [](void *ptr, Args ...args) -> typename traits_t::return_type { return std::invoke(Mem, *static_cast<const U *>(ptr), args...); };
				m_data_flags = std::bit_cast<std::uintptr_t>(ptr) | is_owned | is_local;
			}
			else
			{
				const auto ptr = new heap_storage<U>(std::forward<T>(instance));
				m_invoke = [](void *ptr, Args ...args) -> typename traits_t::return_type { return std::invoke(Mem, static_cast<const heap_storage<U> *>(ptr)->value, args...); };
				m_data_flags = std::bit_cast<std::uintptr_t>(ptr) | is_owned;
			}
		}

		void copy_from(const delegate &other)
		{
			storage_t::copy(other);
			m_invoke = other.m_invoke;

			if (const auto new_flags = other.flags(); !(new_flags & is_owned))
				m_data_flags = other.m_data_flags;
			else
			{
				void *new_data;
				if (!(new_flags & is_local))
					new_data = static_cast<heap_storage<void> *>(other.data())->copy();
				else
					new_data = storage_t::data();
				m_data_flags = std::bit_cast<std::uintptr_t>(new_data) | new_flags;
			}
		}
		void destroy()
		{
			if (flags() == is_owned)
			{
				static_cast<heap_storage<void> *>(data())->destroy();
				m_data_flags = {};
			}
		}

		native_function_t m_invoke = {};
		std::uintptr_t m_data_flags = {};
	};

	template<typename F, std::size_t N>
	void swap(delegate<F, N> &a, delegate<F, N> &b) noexcept { a.swap(b); }

	template<typename R, typename... Args>
	delegate(R (*)(Args...)) -> delegate<R(Args...)>;
	template<typename R, typename... Args>
	delegate(R (*)(void *, Args...), void *) -> delegate<R(Args...)>;

	template<typename F>
	delegate(F &&) -> delegate<typename detail::deduce_signature_t<decltype(&std::decay_t<F>::operator())>>;
	template<auto Mem, typename T>
	delegate(bind_member_t<Mem>, T &&) -> delegate<typename detail::deduce_signature_t<decltype(Mem)>>;

	/** Creates a delegate from a member pointer and an object instance pointer. */
	template<auto Mem, std::size_t LocalBytes = sizeof(std::uintptr_t) * 2, typename T>
	[[nodiscard]] inline auto member_delegate(T &&instance) -> delegate<typename detail::deduce_signature_t<decltype(Mem)>, LocalBytes>
	{
		return {bind_member<Mem>, std::forward<T>(instance)};
	}
}

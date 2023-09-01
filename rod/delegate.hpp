/*
 * Created by switchblade on 2023-04-06.
 */

#pragma once

#include <functional>
#include <array>
#include <bit>

#include "utility.hpp"

namespace rod
{
	namespace _detail
	{
		template<typename T>
		concept delegate_value_type = alignof(T) <= alignof(std::uintptr_t) && sizeof(T) <= sizeof(std::uintptr_t[3]) && std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>;

		template<typename T>
		struct delegate_traits : std::false_type {};
		template<typename R, typename... Args>
		struct delegate_traits<R(Args...)> : std::true_type
		{
			using return_type = R;
			using arg_types = type_list_t<Args...>;
			using native_function = R (*)(void *, Args...);

			template<typename T>
			constexpr static bool is_invocable = std::is_invocable_r_v<R, T, Args...>;
			template<typename T>
			constexpr static bool is_const_invocable = std::is_invocable_r_v<R, std::add_const_t<T>, Args...>;
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
	/** Instance of `rod::bind_member_t&lt;F&gt;` */
	template<auto F>
	inline constexpr auto bind_member = bind_member_t<F>{};

	/** @brief Versatile type-erased functor or functor reference.
	 *
	 * Delegate act as a lightweight versatile alternative to `std::function`.
	 * Unlike `std::function`, delegates can hold references to externally stored functors,
	 * can be directly constructed from a member function pointer and object instance,
	 * and can be used to invoke C-style APIs taking a callback and data pointer.
	 *
	 * For ABI compatibility and interop purposes, delegate types are equivalent to the following struct:
	 * @code{cpp}
	 * struct delegate
	 * {
	 * 	Ret (*invoke_func)(void *data, Args...args);
	 * 	uintptr_t flags;
	 *
	 * 	union
	 * 	{
	 * 		struct
	 * 		{
	 * 			void *(*copy_func)(const void *data);
	 * 			void (*delete_func)(void *data);
	 * 			void *data;
	 * 		} udata;
	 * 		struct
	 * 		{
	 * 			uintptr_t data[3];
	 * 		} local;
	 * 	};
	 * };
	 * @endcode
	 *
	 * Where `flags` is `0` or a combination the following flags:
	 * <table>
	 * <tr><th>Flag<th><th>Value</th><th>Description</th></tr>
	 * <tr><td>DELEGATE_FLAG_LOCAL<th><th>1</th><th>If set, data pointer is obtained from `local.data`, otherwise `udata.data`</th></tr>
	 * <tr><td>DELEGATE_FLAG_OWNED<th><th>2</th><th>If set, delegate takes ownership of user data. When `DELEGATE_FLAG_LOCAL` is not set, `udata.delete_func` is used to destroy the data.</th></tr>
	 * </table
	 *
	 * @tparam F Function signature of the delegate. */
	template<typename F>
	class delegate
	{
		static_assert(_detail::delegate_traits<F>::value, "Delegate template parameter must be a function signature");

		using traits_t = _detail::delegate_traits<F>;
		using args_t = typename traits_t::arg_types;
		using return_t = typename traits_t::return_type;

	public:
		/** Type of the native C-style invoke callback function. */
		using native_function_type = typename traits_t::native_function;

	private:
		enum flags_t : std::intptr_t
		{
			is_local = 1,
			is_owned = 2,
			flags_bits = 2,
			flags_mask = 3,
		};

		struct udata_storage
		{
			void *(*copy_func)(const void *);
			void (*delete_func)(void *);
			void *data;
		};
		struct local_storage
		{
			std::uintptr_t data[3];
		};

		template<auto Mem, typename T, typename... Args>
		static return_t invoke_mem(void *ptr, Args ...args) { return std::invoke(*reinterpret_cast<T *>(ptr), args...); }
		template<typename T, typename... Args>
		static return_t invoke_obj(void *ptr, Args ...args) { return std::invoke(*reinterpret_cast<T *>(ptr), args...); }

		template<typename T>
		static void delete_mem(void *ptr) {}
		template<typename T>
		static void delete_obj(void *ptr) { delete reinterpret_cast<T *>(ptr); }

		template<typename T>
		static void *copy_mem(const void *ptr) {}
		template<typename T>
		static void *copy_obj(const void *ptr) { return new T(*reinterpret_cast<T *>(ptr)); }

	public:
		constexpr delegate() noexcept : _invoke_func(), _flags(), _udata() {}

		delegate(const delegate &other) : delegate()
		{
			copy_from(other);
		}
		delegate &operator=(const delegate &other)
		{
			if (this != &other)
			{
				destroy_udata();
				copy_from(other);
			}
			return *this;
		}

		constexpr delegate(delegate &&other) noexcept : delegate() { swap(other); }
		constexpr delegate &operator=(delegate &&other) noexcept { return (swap(other), *this); }

		~delegate() { destroy_udata(); }

		/** Initializes the delegate with a function pointer. */
		template<typename R, typename... Args>
		delegate(R (*func)(Args...)) : delegate() { init_obj<R(*)(Args...)>(args_t{}, func); }

		/** Initializes the delegate with a pointer to an external functor. */
		template<typename T> requires(!decays_to_same<T, delegate>&& !std::is_function_v<T>)
		delegate(T *data) noexcept : delegate() { init_ptr(args_t{}, data); }
		/** Initializes the delegate with a by-value stored functor. */
		template<typename T> requires(!decays_to_same<T, delegate> && !std::is_function_v<std::decay_t<T>>)
		delegate(T &&func) : delegate() { init_obj<std::decay_t<T>>(args_t{}, std::forward<T>(func)); }
		/** Initializes the delegate with an in-place constructed functor of type \a T using arguments \a args. */
		template<typename T, typename... Args> requires decays_to_same<T, T> && std::constructible_from<T, Args...>
		delegate(std::in_place_type_t<T>, Args &&...args) : delegate() { init_obj<T>(args_t{}, std::forward<Args>(args)...); }

		/** Initializes the delegate from a pointer to member or member function and external instance pointer. */
		template<auto Mem, typename T>
		delegate(bind_member_t<Mem>, T *data) noexcept : delegate() { init_mem<Mem, T *>(args_t{}, data); }
		/** Initializes the delegate from a pointer to member or member function and a by-value stored instance. */
		template<auto Mem, typename T>
		delegate(bind_member_t<Mem>, T &&value) : delegate() { init_mem<Mem, std::decay_t<T>>(args_t{}, std::forward<T>(value)); }
		/** Initializes the delegate from a pointer to member or member function and an in-place constructed instance of type \a T using arguments \a args. */
		template<auto Mem, typename T, typename... Args> requires decays_to_same<T, T> && std::constructible_from<T, Args...>
		delegate(bind_member_t<Mem>, std::in_place_type_t<T>, Args &&...args) : delegate() { init_mem<Mem, std::decay_t<T>>(args_t{}, std::forward<Args>(args)...); }

		/** Initializes the delegate from an invoker callback and a user data pointer. */
		delegate(native_function_type invoke, void *data) noexcept : _invoke_func(invoke), _udata{.data = data}, _flags() {}
		/** Initializes the delegate from an invoker callback, user data pointer, copy and deleter functions. */
		delegate(native_function_type invoke, void *data, void *(*copy_fn)(const void *), void (*delete_fn)(void *)) noexcept : _invoke_func(invoke), _udata{copy_fn, delete_fn, data}, _flags(flags_t::is_owned) {}

		/** Initializes the delegate from a pointer to member or member function and object instance.
		 * @param instance Either a pointer to an instance or a value instance to invoke the pointer to member or member function on. */
		template<typename T, auto Mem>
		delegate(bind_member_t<Mem>, T &&instance) { init_mem<Mem, std::decay_t<T>>(args_t{}, std::forward<T>(instance)); }

		/** Checks if the delegate is empty. */
		[[nodiscard]] constexpr bool empty() const noexcept { return native_function() == nullptr; }
		/** @copydoc empty */
		[[nodiscard]] constexpr operator bool() const noexcept { return native_function() != nullptr; }

		/** Resets the delegate to an empty state. */
		void reset()
		{
			destroy_udata();
			_invoke_func = {};
			_flags = {};
		}

		/** Returns pointer to the data of the delegate.
		 * @note Returned pointer might be a const-casted pointer to internal storage. */
		[[nodiscard]] void *data() const noexcept { return (_flags & flags_t::is_local) ? const_cast<std::uintptr_t *>(_local.data) : _udata.data; }
		/** Returns pointer to the C-style invoker function of the delegate. */
		[[nodiscard]] native_function_type native_function() const noexcept { return _invoke_func; }

		/** Invokes the underlying functor with \a args. */
		template<typename... Args>
		auto invoke(Args &&...args) const -> std::invoke_result_t<native_function_type, void *, Args...> { return native_function()(data(), std::forward<Args>(args)...); }
		/** @copydoc invoke */
		template<typename... Args>
		auto operator()(Args &&...args) const -> std::invoke_result_t<native_function_type, void *, Args...> { return invoke(std::forward<Args>(args)...); }

		/** Exchanges the contents of this delegate with \a other. */
		constexpr void swap(delegate &other) noexcept
		{
			std::swap(_invoke_func, other._invoke_func);
			std::swap(_flags, other._flags);
			std::swap(_local, other._local);
		}

	private:
		template<typename T, typename... Args>
		void init_ptr(type_list_t<Args...>, T *ptr) noexcept requires std::is_invocable_r_v<return_t, T &, Args...>
		{
			_invoke_func = invoke_obj<T, Args...>;
			_udata.data = ptr;
		}
		template<typename T, typename... Args, typename... TArgs>
		void init_obj(type_list_t<Args...>, TArgs &&...args) requires std::is_invocable_r_v<return_t, T &, Args...>
		{
			if constexpr(_detail::delegate_value_type<T> && std::invocable<const T &, Args...>)
			{
				new (_local.data) T(std::forward<TArgs>(args)...);
				_invoke_func = invoke_obj<const T, Args...>;
				_flags = flags_t::is_owned | flags_t::is_local;
			}
			else
			{
				_invoke_func = invoke_obj<T, Args...>;
				_flags = flags_t::is_owned;

				_udata.data = new T(std::forward<TArgs>(args)...);
				_udata.delete_func = delete_obj<T>;
				_udata.copy_func = copy_obj<T>;
			}
		}
		template<auto Mem, typename T, typename... Args, typename... TArgs>
		void init_mem(type_list_t<Args...>, TArgs &&...args) requires std::is_invocable_r_v<return_t, decltype(Mem), T &, Args...>
		{
			if constexpr(_detail::delegate_value_type<T> && std::invocable<decltype(Mem), const T &, Args...>)
			{
				new (_local.data) T(std::forward<TArgs>(args)...);
				_invoke_func = invoke_mem<Mem, const T, Args...>;
				_flags = flags_t::is_owned | flags_t::is_local;
			}
			else
			{
				_invoke_func = invoke_mem<Mem, T, Args...>;
				_flags = flags_t::is_owned;

				_udata.data = new T(std::forward<TArgs>(args)...);
				_udata.delete_func = delete_obj<T>;
				_udata.copy_func = copy_obj<T>;
			}
		}

		void copy_from(const delegate &other)
		{
			_invoke_func = other._invoke_func;
			_flags = other._flags;
			_local = other._local;

			/* Copy the userdata object if required. */
			if (_flags == flags_t::is_owned && _udata.copy_func)
				_udata.data = _udata.copy_func(_udata.data);
		}
		void destroy_udata()
		{
			if (_flags == flags_t::is_owned && _udata.delete_func)
				_udata.delete_func(_udata.data);
		}

		native_function_type _invoke_func;
		std::uintptr_t _flags;

		union
		{
			local_storage _local;
			udata_storage _udata;
		};
	};

	template<typename F>
	constexpr void swap(delegate<F> &a, delegate<F> &b) noexcept { a.swap(b); }

	template<typename R, typename... Args>
	delegate(R (*)(Args...)) -> delegate<R(Args...)>;
	template<typename R, typename... Args>
	delegate(R (*)(void *, Args...), void *) -> delegate<R(Args...)>;
	template<typename R, typename... Args>
	delegate(R (*)(void *, Args...), void *, void *(*)(const void *), void (*)(void *)) -> delegate<R(Args...)>;

	template<typename T>
	delegate(T &&) -> delegate<_detail::strip_qualifiers_t<_detail::deduce_signature_t<decltype(&std::decay_t<T>::operator())>>>;
	template<typename T, typename... Args>
	delegate(std::in_place_type_t<T>, Args &&...) -> delegate<_detail::strip_qualifiers_t<_detail::deduce_signature_t<decltype(&std::decay_t<T>::operator())>>>;

	template<auto Mem, typename T>
	delegate(bind_member_t<Mem>, T &&) -> delegate<_detail::strip_qualifiers_t<_detail::deduce_signature_t<decltype(Mem)>>>;
	template<auto Mem, typename T, typename... Args>
	delegate(bind_member_t<Mem>, std::in_place_type_t<T>, Args &&...) -> delegate<_detail::strip_qualifiers_t<_detail::deduce_signature_t<decltype(Mem)>>>;

	/** Creates a delegate from a member pointer and an object instance. */
	template<auto Mem, typename T>
	[[nodiscard]] inline auto member_delegate(T &&instance) -> delegate<_detail::strip_qualifiers_t<_detail::deduce_signature_t<decltype(Mem)>>> { return {bind_member<Mem>, std::forward<T>(instance)}; }

	static_assert(alignof(delegate<void()>) == alignof(void *[5]));
	static_assert(sizeof(delegate<void()>) == sizeof(void *[5]));
}

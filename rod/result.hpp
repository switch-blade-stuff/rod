/*
 * Created by switchblade on 2023-06-26.
 */

#pragma once

#include "utility.hpp"

/* FIXME:
 * Once P1028R5 (SG14 status_code and standard error object) is finalized and made part of the
 * language (expected to be somewhere around C++26), transition to using std::status_code instead
 * of std::error_code and implement an std::error_code - based shim for older language versions. */

namespace rod
{
	namespace _result
	{
		template<typename Val, typename Err>
		class result;

		template<typename T>
		struct is_result : std::false_type {};
		template<typename Val, typename Err>
		struct is_result<result<Val, Err>> : std::true_type {};

		template<typename T, typename Val>
		struct is_result_with_value : std::false_type {};
		template<typename Val, typename Err>
		struct is_result_with_value<result<Val, Err>, Val> : std::true_type {};

		template<typename T, typename Err>
		struct is_result_with_error : std::false_type {};
		template<typename Val, typename Err>
		struct is_result_with_error<result<Val, Err>, Err> : std::true_type {};

		/** Exception thrown when an invalid state was requested from `rod::result`. */
		class ROD_API_PUBLIC bad_result_access : public std::runtime_error
		{
			template<typename, typename>
			friend class result;

			explicit bad_result_access(const char *msg) : std::runtime_error(msg) {}
			explicit bad_result_access(const std::string &msg) : std::runtime_error(msg) {}

		public:
			ROD_API_PUBLIC ~bad_result_access() override;
		};

		/** Tag used to select value-state constructor for `rod::result`. */
		struct in_place_value_t {};
		/** Tag used to select error-state constructor for `rod::result`. */
		struct in_place_error_t {};

		template<typename Val, typename Err>
		struct storage
		{
			constexpr storage() {}
			constexpr storage(const storage &) {}
			constexpr storage &operator=(const storage &) {}
			constexpr storage(storage &&) {}
			constexpr storage &operator=(storage &&) {}
			constexpr ~storage() {}

			union
			{
				Val value;
				Err error;
			};
		};
		template<typename Err>
		struct storage<void, Err>
		{
			constexpr storage() {}
			constexpr storage(const storage &) {}
			constexpr storage &operator=(const storage &) {}
			constexpr storage(storage &&) {}
			constexpr storage &operator=(storage &&) {}
			constexpr ~storage() {}

			union { Err error; };
		};

		/** Utility type used to return a value or error result.
		 *
		 * For the purposes of ABI compatibility and interop, result types are layout-compatible with the following struct:
		 * @code{cpp}
		 * struct result
		 * {
		 * 	union
		 * 	{
		 * 		Val value;
		 * 		Err error;
		 * 	};
		 * 	uint8_t state;
		 * };
		 * @endcode
		 *
		 * Where `state` is `0` for `value` and `1` for `error`.
		 *
		 * @tparam Val Value type stored by the result.
		 * @tparam Err Error type stored by the result. */
		template<typename Val = void, typename Err = std::error_code>
		class result
		{
			template<typename, typename>
			friend class result;

			static_assert(!std::is_void_v<Err>, "`result` does not support `void` errors, use `std::optional&lt;Val&gt;` instead");

		public:
			template<typename NewVal>
			using rebind_value = result<NewVal, Err>;
			template<typename NewErr>
			using rebind_error = result<Val, NewErr>;

			using value_type = Val;
			using error_type = Err;

		private:
			constexpr static std::int8_t is_value = 0;
			constexpr static std::int8_t is_error = 1;

			using storage_t = storage<Val, Err>;

			template<typename Val2, typename Err2>
			using is_convertible = std::conjunction<std::disjunction<std::is_void<Val>, std::is_convertible<Val2, Val>>, std::is_convertible<Err2, Err>>;
			template<typename Val2, typename Err2>
			using is_constructible = std::conjunction<std::disjunction<std::is_void<Val>, std::is_constructible<Val, Val2>>, std::is_constructible<Err, Err2>>;
			template<typename Val2, typename Err2>
			using is_assignable = std::conjunction<is_constructible<Val2, Err2>, std::disjunction<std::is_void<Val>, std::is_assignable<Val, Val2>>, std::is_assignable<Err, Err2>>;

			template<typename Val2, typename Err2>
			using is_nothrow_constructible = std::conjunction<std::disjunction<std::is_void<Val>, std::is_nothrow_constructible<Val, Val2>>, std::is_nothrow_constructible<Err, Err2>>;
			template<typename Val2, typename Err2>
			using is_nothrow_assignable = std::conjunction<is_nothrow_constructible<Val2, Err2>, std::disjunction<std::is_void<Val>, std::is_nothrow_assignable<Val, Val2>>, std::is_nothrow_assignable<Err, Err2>>;

			using is_default_constructible = std::disjunction<std::is_void<Val>, std::is_default_constructible<Val>>;
			using is_nothrow_default_constructible = std::disjunction<std::is_void<Val>, std::is_nothrow_default_constructible<Val>>;

			using is_copy_constructible = std::conjunction<std::disjunction<std::is_void<Val>, std::is_copy_constructible<Val>>, std::is_copy_constructible<Err>>;
			using is_move_constructible = std::conjunction<std::disjunction<std::is_void<Val>, std::is_move_constructible<Val>>, std::is_move_constructible<Err>>;
			using is_copy_assignable = std::conjunction<is_copy_constructible, std::disjunction<std::is_void<Val>, std::is_copy_assignable<Val>>, std::is_copy_assignable<Err>>;
			using is_move_assignable = std::conjunction<is_move_constructible, std::disjunction<std::is_void<Val>, std::is_move_assignable<Val>>, std::is_move_assignable<Err>>;

			using is_nothrow_copy_constructible = std::conjunction<std::disjunction<std::is_void<Val>, std::is_nothrow_copy_constructible<Val>>, std::is_nothrow_copy_constructible<Err>>;
			using is_nothrow_move_constructible = std::conjunction<std::disjunction<std::is_void<Val>, std::is_nothrow_move_constructible<Val>>, std::is_nothrow_move_constructible<Err>>;
			using is_nothrow_copy_assignable = std::conjunction<is_nothrow_copy_constructible, std::disjunction<std::is_void<Val>, std::is_nothrow_copy_assignable<Val>>, std::is_nothrow_copy_assignable<Err>>;
			using is_nothrow_move_assignable = std::conjunction<is_nothrow_move_constructible, std::disjunction<std::is_void<Val>, std::is_nothrow_move_assignable<Val>>, std::is_nothrow_move_assignable<Err>>;

			using is_nothrow_destructible = std::conjunction<std::disjunction<std::is_void<Val>, std::is_nothrow_destructible<Val>>, std::is_nothrow_destructible<Err>>;

			using is_swappable = std::conjunction<is_move_constructible, std::disjunction<std::is_void<Val>, std::is_swappable<Val>>, std::is_swappable<Err>>;
			using is_nothrow_swappable = std::conjunction<is_nothrow_move_constructible, std::disjunction<std::is_void<Val>, std::is_nothrow_swappable<Val>>, std::is_nothrow_swappable<Err>>;

			template<typename T, typename F>
			using is_value_invocable = std::conditional_t<std::is_void_v<Val>, std::is_invocable<F>, std::is_invocable<F, copy_cvref_t<T, Val>>>;
			template<typename T, typename F>
			using is_error_invocable = std::conjunction<std::is_invocable<F, copy_cvref_t<T, Err>>, std::negation<std::is_void<std::invoke_result_t<F, copy_cvref_t<T, Err>>>>>;

			template<typename T, typename F>
			using is_nothrow_value_invocable = std::conjunction<is_value_invocable<T, F>, std::conditional_t<std::is_void_v<Val>, std::is_nothrow_invocable<F>, std::is_nothrow_invocable<F, copy_cvref_t<T, Val>>>>;
			template<typename T, typename F>
			using is_nothrow_error_invocable = std::conjunction<is_error_invocable<T, F>, std::is_nothrow_invocable<F, copy_cvref_t<T, Err>>>;

			template<typename T, typename F>
			using transform_value_type = typename std::conditional_t<std::is_void_v<Val>, std::invoke_result<F>, std::invoke_result<F, copy_cvref_t<T, Val>>>::type;
			template<typename T, typename F>
			using transform_error_type = std::invoke_result_t<F, copy_cvref_t<T, Err>>;

			template<typename T, typename FVal, typename FErr>
			using transform_result = result<transform_value_type<T, FVal>, transform_error_type<T, FErr>>;
			template<typename T, typename F>
			using transform_value_result = result<transform_value_type<T, F>, Err>;
			template<typename T, typename F>
			using transform_error_result = result<Val, transform_error_type<T, F>>;

			template<typename T, typename F>
			using into_value_type = std::decay_t<typename std::conditional_t<std::is_void_v<Val>, std::invoke_result<F>, std::invoke_result<F, copy_cvref_t<T, Val>>>::type>;
			template<typename T, typename F>
			using into_error_type = std::decay_t<std::invoke_result_t<F, copy_cvref_t<T, Err>>>;

			template<typename T, typename FVal, typename FErr>
			using into_result = std::common_type_t<into_value_type<T, FVal>, into_error_type<T, FErr>>;

			template<typename F, typename... Args>
			static constexpr decltype(auto) dispatch_invoke(F &&func, Args &&...args) noexcept(std::is_nothrow_invocable_v<F, Args...>)
			{
				if constexpr (!std::is_void_v<std::invoke_result_t<F, Args...>>)
					return std::invoke(std::forward<F>(func), std::forward<Args>(args)...);
				else
					std::invoke(std::forward<F>(func), std::forward<Args>(args)...);
			}

		public:
			/** Initializes result with a default-constructed value state. */
			constexpr result() noexcept(is_nothrow_default_constructible::value) requires is_default_constructible::value
			{
				if constexpr (!std::is_void_v<Val>)
					new (&_storage.value) value_type();
				_state = is_value;
			}
			/** Initializes a value result using \a val. */
			template<typename U = Val, typename = std::enable_if<!decays_to_same<U, Err> && !std::is_void_v<Val>>>
			constexpr result(U &&val) noexcept(std::is_nothrow_constructible_v<Val, U>) requires std::constructible_from<Val, U> : result(in_place_value_t{}, std::forward<U>(val)) {}
			/** Initializes an error result using \a err. */
			template<typename U = Err, typename = std::enable_if<!decays_to_same<U, Val>>>
			constexpr result(U &&err) noexcept(std::is_nothrow_constructible_v<Err, U>) requires std::constructible_from<Err, U> : result(in_place_error_t{}, std::forward<U>(err)) {}

			/** Direct-initializes result's value from passed arguments.
			 * @param args Arguments passed to the value type's constructor. */
			template<typename T = Val, typename... Args, typename = std::enable_if_t<!std::is_void_v<T>>> requires std::constructible_from<Val, Args...>
			constexpr result(in_place_value_t, Args &&...args) noexcept(std::is_nothrow_constructible_v<Val, Args...>)
			{
				new (&_storage.value) value_type(std::forward<Args>(args)...);
				_state = is_value;
			}
			/** Direct-initializes result's value from passed initializer list and arguments.
			 * @param init Initializer list passed to the value type's constructor.
			 * @param args Arguments passed to the value type's constructor. */
			template<typename T = Val, typename U, typename... Args, typename = std::enable_if_t<!std::is_void_v<T>>> requires std::constructible_from<Val, std::initializer_list<U>, Args...>
			constexpr result(in_place_value_t, std::initializer_list<U> init, Args &&...args) noexcept(std::is_nothrow_constructible_v<Val, std::initializer_list<U>, Args...>)
			{
				new (&_storage.value) value_type(init, std::forward<Args>(args)...);
				_state = is_value;
			}

			/** Direct-initializes result's error from passed arguments.
			 * @param args Arguments passed to the error type's constructor. */
			template<typename... Args> requires std::constructible_from<Err, Args...>
			constexpr result(in_place_error_t, Args &&...args) noexcept(std::is_nothrow_constructible_v<Err, Args...>)
			{
				new (&_storage.error) error_type(std::forward<Args>(args)...);
				_state = is_error;
			}
			/** Direct-initializes result's error from passed initializer list and arguments.
			 * @param init Initializer list passed to the error type's constructor.
			 * @param args Arguments passed to the error type's constructor. */
			template<typename T, typename... Args> requires std::constructible_from<Err, std::initializer_list<T>, Args...>
			constexpr result(in_place_error_t, std::initializer_list<T> init, Args &&...args) noexcept(std::is_nothrow_constructible_v<Err, std::initializer_list<T>, Args...>)
			{
				new (&_storage.error) error_type(init, std::forward<Args>(args)...);
				_state = is_error;
			}

			template<typename Val2, typename Err2, typename = std::enable_if_t<!std::same_as<result, result<Val2, Err2>>>> requires is_constructible<const Val2 &, const Err2 &>::value
			constexpr explicit(!is_convertible<Val2, Err2>::value) result(const result<Val2, Err2> &other) noexcept(is_nothrow_constructible<const Val2 &, const Err2 &>::value)
			{
				if (other.has_value())
				{
					if constexpr (!std::is_void_v<Val>)
						new (&_storage.value) value_type(other._storage.value);
				}
				else if (other.has_error())
					new (&_storage.error) error_type(other._storage.error);
				_state = other._state;
			}
			template<typename Val2, typename Err2, typename = std::enable_if_t<!std::same_as<result, result<Val2, Err2>>>> requires is_constructible<Val2, Err2>::value
			constexpr explicit(!is_convertible<Val2, Err2>::value) result(result<Val2, Err2> &&other) noexcept(is_nothrow_constructible<Val2 &&, Err2 &&>::value)
			{
				if (other.has_value())
				{
					if constexpr (!std::is_void_v<Val>)
						new (&_storage.value) value_type(std::move(other._storage.value));
				}
				else if (other.has_error())
					new (&_storage.error) error_type(std::move(other._storage.error));
				_state = other._state;
			}

			template<typename Val2, typename Err2, typename = std::enable_if_t<!std::same_as<result, result<Val2, Err2>>>> requires is_assignable<const Val2 &, const Err2 &>::value
			constexpr result &operator=(const result<Val2, Err2> &other) noexcept(is_nothrow_assignable<const Val2 &, const Err2 &>::value)
			{
				if (_state != other._state)
					destroy();

				if (other.has_value())
				{
					if constexpr (!std::is_void_v<Val>)
						assign_value(other._storage.value);
				}
				else if (other.has_error())
					assign_error(other._storage.error);

				_state = other._state;
				return *this;
			}
			template<typename Val2, typename Err2, typename = std::enable_if_t<!std::same_as<result, result<Val2, Err2>>>> requires is_assignable<Val2, Err2>::value
			constexpr result &operator=(result<Val2, Err2> &&other) noexcept(is_nothrow_move_assignable::value) requires is_move_assignable::value
			{
				if (_state != other._state)
					destroy();

				if (other.has_value())
				{
					if constexpr (!std::is_void_v<Val>)
						assign_value(std::move(other._storage.value));
				}
				else if (other.has_error())
					assign_error(std::move(other._storage.error));

				_state = other._state;
				return *this;
			}

			constexpr result(const result &other) noexcept(is_nothrow_copy_constructible::value) requires is_copy_constructible::value : _state(other._state)
			{
				if (has_value())
				{
					if constexpr (!std::is_void_v<Val>)
						new (&_storage.value) value_type(other._storage.value);
				}
				else if (has_error())
					new (&_storage.error) error_type(other._storage.error);
			}
			constexpr result(result &&other) noexcept(is_nothrow_move_constructible::value) requires is_move_constructible::value : _state(other._state)
			{
				if (has_value())
				{
					if constexpr (!std::is_void_v<Val>)
						new (&_storage.value) value_type(std::move(other._storage.value));
				}
				else if (has_error())
					new (&_storage.error) error_type(std::move(other._storage.error));
			}

			constexpr result &operator=(const result &other) noexcept(is_nothrow_copy_assignable::value) requires is_copy_assignable::value
			{
				if (_state != other._state)
					destroy();

				if (other.has_value())
				{
					if constexpr (!std::is_void_v<Val>)
						assign_value(other._storage.value);
				}
				else if (other.has_error())
					assign_error(other._storage.error);

				_state = other._state;
				return *this;
			}
			constexpr result &operator=(result &&other) noexcept(is_nothrow_move_assignable::value) requires is_move_assignable::value
			{
				if (_state != other._state)
					destroy();

				if (other.has_value())
				{
					if constexpr (!std::is_void_v<Val>)
						assign_value(std::move(other._storage.value));
				}
				else if (other.has_error())
					assign_error(std::move(other._storage.error));

				_state = other._state;
				return *this;
			}

			constexpr ~result() noexcept(is_nothrow_destructible::value) { destroy(); }

			/** Emplaces an error to the result.
			 * @param args Arguments passed to the error.
			 * @return Reference to the contained error object. */
			template<typename... Args> requires std::constructible_from<Err, Args...>
			constexpr auto emplace_error(Args &&...args) noexcept(is_nothrow_destructible::value && std::is_nothrow_constructible_v<Err, Args...>) -> std::add_lvalue_reference_t<Err>
			{
				destroy();
				new (&_storage.error) error_type(std::forward<Args>(args)...);
				_state = is_error;
				return _storage.error;
			}
			/** Emplaces a value to the result.
			 * @param args Arguments passed to the value.
			 * @return Reference to the contained value object. */
			template<typename... Args, typename T = Val> requires std::constructible_from<Val, Args...>
			constexpr auto emplace_value(Args &&...args) noexcept(is_nothrow_destructible::value && std::is_nothrow_constructible_v<Val, Args...>) -> std::enable_if_t<!std::is_void_v<T>, std::add_lvalue_reference_t<T>>
			{
				destroy();
				new (&_storage.value) value_type(std::forward<Args>(args)...);
				_state = is_value;
				return _storage.value;
			}

			/** Checks if the result contains an error. */
			[[nodiscard]] constexpr bool has_error() const noexcept { return _state == is_error; }
			/** Checks if the result contains a value. */
			[[nodiscard]] constexpr bool has_value() const noexcept { return _state == is_value; }
			/** @copydoc has_value */
			[[nodiscard]] constexpr operator bool() const noexcept { return has_value(); }

			/** Returns pointer to the value of the result.
			 * @note If the result does not contain a value results in undefined behavior. */
			template<typename U = Val>
			[[nodiscard]] constexpr auto operator->() noexcept -> std::enable_if_t<std::negation_v<std::is_void<Val>>, U *>;
			/** @copydoc value */
			template<typename U = std::add_const_t<Val>>
			[[nodiscard]] constexpr auto operator->() const noexcept -> std::enable_if_t<std::negation_v<std::is_void<Val>>, U *>;

			/** Returns reference to the value of the result.
			 * @note If the result does not contain a value results in undefined behavior. */
			template<typename U = Val>
			[[nodiscard]] constexpr auto operator*() & noexcept -> std::enable_if_t<std::negation_v<std::is_void<Val>>, std::add_lvalue_reference_t<U>>;
			/** @copydoc value */
			template<typename U = Val>
			[[nodiscard]] constexpr auto operator*() && noexcept -> std::enable_if_t<std::negation_v<std::is_void<Val>>, std::add_rvalue_reference_t<U>>;
			/** @copydoc value */
			template<typename U = std::add_const_t<Val>>
			[[nodiscard]] constexpr auto operator*() const & noexcept -> std::enable_if_t<std::negation_v<std::is_void<Val>>, std::add_lvalue_reference_t<U>>;
			/** @copydoc value */
			template<typename U = std::add_const_t<Val>>
			[[nodiscard]] constexpr auto operator*() const && noexcept -> std::enable_if_t<std::negation_v<std::is_void<Val>>, std::add_rvalue_reference_t<U>>;

			/** Returns reference to the contained value of the result.
			 * @throw `rod::throw_exception(error())` if the result does not contain a value and  is ill-formed.
			 * @throw bad_result_access If the result does not contain a value and `rod::throw_exception(error())` is ill-formed. */
			constexpr auto value() & -> std::conditional_t<std::is_void_v<Val>, std::add_lvalue_reference_t<Val>, Val>
			{
				if (!has_value())
					throw_missing_value();
				else if constexpr (!std::is_void_v<Val>)
					return _storage.value;
			}
			/** @copydoc value */
			constexpr auto value() && -> std::conditional_t<std::is_void_v<Val>, std::add_rvalue_reference_t<Val>, Val>
			{
				if (!has_value())
					throw_missing_value();
				else if constexpr (!std::is_void_v<Val>)
					return std::move(_storage.value);
			}
			/** @copydoc value */
			constexpr auto value() const & -> std::conditional_t<std::is_void_v<Val>, std::add_lvalue_reference_t<std::add_const_t<Val>>, Val>
			{
				if (!has_value())
					throw_missing_value();
				else if constexpr (!std::is_void_v<Val>)
					return _storage.value;
			}
			/** @copydoc value */
			constexpr auto value() const && -> std::conditional_t<std::is_void_v<Val>, std::add_rvalue_reference_t<std::add_const_t<Val>>, Val>
			{
				if (!has_value())
					throw_missing_value();
				else if constexpr (!std::is_void_v<Val>)
					return std::move(_storage.value);
			}

			/** Returns the contained value or \a val if result does not contain a value. */
			template<typename Val2 = Val> requires std::constructible_from<Val, Val &>
			[[nodiscard]] constexpr Val value_or(Val2 &&val) & noexcept(std::is_nothrow_constructible_v<Val, std::add_lvalue_reference_t<Val>> && std::is_nothrow_constructible_v<Val, Val2>)
			{
				if (!has_value()) [[unlikely]]
					return Val2(std::forward<Val2>(val));
				else
					return _storage.value;
			}
			/** @copydoc value_or */
			template<typename Val2 = Val> requires std::constructible_from<Val, Val> && std::constructible_from<Val, Val2>
			[[nodiscard]] constexpr Val value_or(Val2 &&val) && noexcept(std::is_nothrow_constructible_v<Val, std::add_rvalue_reference_t<Val>> && std::is_nothrow_constructible_v<Val, Val2>)
			{
				if (!has_value()) [[unlikely]]
					return Val2(std::forward<Val2>(val));
				else
					return std::move(_storage.value);
			}
			/** @copydoc value_or */
			template<typename Val2 = Val> requires std::constructible_from<Val, std::add_const_t<Val> &> && std::constructible_from<Val, Val2>
			[[nodiscard]] constexpr Val value_or(Val2 &&val) const & noexcept(std::is_nothrow_constructible_v<Val, std::add_lvalue_reference_t<std::add_const_t<Val>>> && std::is_nothrow_constructible_v<Val, Val2>)
			{
				if (!has_value()) [[unlikely]]
					return Val2(std::forward<Val2>(val));
				else
					return _storage.value;
			}
			/** @copydoc value_or */
			template<typename Val2 = Val> requires std::constructible_from<Val, std::add_const_t<Val>> && std::constructible_from<Val, Val2>
			[[nodiscard]] constexpr Val value_or(Val2 &&val) const && noexcept(std::is_nothrow_constructible_v<Val, std::add_rvalue_reference_t<std::add_const_t<Val>>> && std::is_nothrow_constructible_v<Val, Val2>)
			{
				if (!has_value()) [[unlikely]]
					return Val2(std::forward<Val2>(val));
				else
					return std::move(_storage.value);
			}

			/** Returns reference to the error of the result.
			 * @throw bad_result_access if the result does not contain an error. */
			constexpr auto error() & -> std::add_lvalue_reference_t<Err>
			{
				if (has_error())
					return _storage.error;
				else
					throw_missing_error();
			}
			/** @copydoc error */
			constexpr auto error() && -> std::add_rvalue_reference_t<Err>
			{
				if (has_error())
					return std::move(_storage.error);
				else
					throw_missing_error();
			}
			/** @copydoc error */
			constexpr auto error() const & -> std::add_lvalue_reference_t<std::add_const_t<Err>>
			{
				if (has_error())
					return _storage.error;
				else
					throw_missing_error();
			}
			/** @copydoc error */
			constexpr auto error() const && -> std::add_rvalue_reference_t<std::add_const_t<Err>>
			{
				if (has_error())
					return std::move(_storage.error);
				else
					throw_missing_error();
			}

			/** Returns the error state or \a err if result does not contain a value. */
			template<typename Err2 = Err> requires std::constructible_from<Err, Err &> && std::constructible_from<Err, Err2>
			[[nodiscard]] constexpr Err error_or(Err2 &&err) & noexcept(std::is_nothrow_constructible_v<Err, std::add_lvalue_reference_t<Err>> && std::is_nothrow_constructible_v<Err, Err2>)
			{
				if (!has_error()) [[unlikely]]
					return Err(std::forward<Err2>(err));
				else
					return _storage.error;
			}
			/** @copydoc error_or */
			template<typename Err2 = Err> requires std::constructible_from<Err, Err> && std::constructible_from<Err, Err2>
			[[nodiscard]] constexpr Err error_or(Err2 &&err) && noexcept(std::is_nothrow_constructible_v<Err, std::add_rvalue_reference_t<Err>> && std::is_nothrow_constructible_v<Err, Err2>)
			{
				if (!has_error()) [[unlikely]]
					return Err(std::forward<Err2>(err));
				else
					return std::move(_storage.error);
			}
			/** @copydoc error_or */
			template<typename Err2 = Err> requires std::constructible_from<Err, std::add_const_t<Err> &> && std::constructible_from<Err, Err2>
			[[nodiscard]] constexpr Err error_or(Err2 &&err) const & noexcept(std::is_nothrow_constructible_v<Err, std::add_lvalue_reference_t<std::add_const_t<Err>>> && std::is_nothrow_constructible_v<Err, Err2>)
			{
				if (!has_error()) [[unlikely]]
					return Err(std::forward<Err2>(err));
				else
					return _storage.error;
			}
			/** @copydoc error_or */
			template<typename Err2 = Err> requires std::constructible_from<Err, std::add_const_t<Err>> && std::constructible_from<Err, Err2>
			[[nodiscard]] constexpr Err error_or(Err2 &&err) const && noexcept(std::is_nothrow_constructible_v<Err, std::add_rvalue_reference_t<std::add_const_t<Err>>> && std::is_nothrow_constructible_v<Err, Err2>)
			{
				if (!has_error()) [[unlikely]]
					return Err(std::forward<Err2>(err));
				else
					return std::move(_storage.error);
			}

			/** Invokes either \a func_val with the contained value or \a func_err with the contained error, and returns result of the expression as `rod::result`. */
			template<typename FVal, typename FErr> requires is_value_invocable<result &, FVal>::value && is_error_invocable<result &, FErr>::value
			constexpr auto transform(FVal &&func_val, FErr &&func_err) & noexcept(is_nothrow_value_invocable<result &, FVal>::value && is_nothrow_error_invocable<result &, FErr>::value) -> transform_result<result &, FVal, FErr>
			{
				if (has_value()) [[likely]]
				{
					if constexpr (!std::is_void_v<Val>)
						return dispatch_invoke(std::forward<FVal>(func_val), _storage.value);
					else
						return dispatch_invoke(std::forward<FVal>(func_val));
				}
				else if (has_error())
					return {in_place_error_t{}, std::invoke(std::forward<FErr>(func_err), _storage.error)};
				else
					return {};
			}
			/** @copydoc transform */
			template<typename FVal, typename FErr> requires is_value_invocable<result &&, FVal>::value && is_error_invocable<result &&, FErr>::value
			constexpr auto transform(FVal &&func_val, FErr &&func_err) && noexcept(is_nothrow_value_invocable<result &&, FVal>::value && is_nothrow_error_invocable<result &&, FErr>::value) -> transform_result<result &&, FVal, FErr>
			{
				if (has_value()) [[likely]]
				{
					if constexpr (!std::is_void_v<Val>)
						return dispatch_invoke(std::forward<FVal>(func_val), std::move(_storage.value));
					else
						return dispatch_invoke(std::forward<FVal>(func_val));
				}
				else if (has_error())
					return {in_place_error_t{}, std::invoke(std::forward<FErr>(func_err), std::move(_storage.error))};
				else
					return {};
			}
			/** @copydoc transform */
			template<typename FVal, typename FErr> requires is_value_invocable<const result &, FVal>::value && is_error_invocable<const result &, FErr>::value
			constexpr auto transform(FVal &&func_val, FErr &&func_err) const & noexcept(is_nothrow_value_invocable<const result &, FVal>::value && is_nothrow_error_invocable<const result &, FErr>::value) -> transform_result<const result &, FVal, FErr>
			{
				if (has_value()) [[likely]]
				{
					if constexpr (!std::is_void_v<Val>)
						return dispatch_invoke(std::forward<FVal>(func_val), _storage.value);
					else
						return dispatch_invoke(std::forward<FVal>(func_val));
				}
				else if (has_error())
					return {in_place_error_t{}, std::invoke(std::forward<FErr>(func_err), _storage.error)};
				else
					return {};
			}
			/** @copydoc transform */
			template<typename FVal, typename FErr> requires is_value_invocable<const result &&, FVal>::value && is_error_invocable<const result &&, FErr>::value
			constexpr auto transform(FVal &&func_val, FErr &&func_err) const && noexcept(is_nothrow_value_invocable<const result &&, FVal>::value && is_nothrow_error_invocable<const result &&, FErr>::value) -> transform_result<const result &&, FVal, FErr>
			{
				if (has_value()) [[likely]]
				{
					if constexpr (!std::is_void_v<Val>)
						return dispatch_invoke(std::forward<FVal>(func_val), std::move(_storage.value));
					else
						return dispatch_invoke(std::forward<FVal>(func_val));
				}
				else if (has_error())
					return {in_place_error_t{}, std::invoke(std::forward<FErr>(func_err), std::move(_storage.error))};
				else
					return {};
			}

			/** Invokes either \a func with the contained value or forwards the contained error, and returns result of the expression as `rod::result`. */
			template<typename F> requires is_value_invocable<result &, F>::value
			constexpr auto transform_value(F &&func) & noexcept(is_nothrow_value_invocable<result &, F>::value) -> transform_value_result<result &, F>
			{
				if (has_value()) [[likely]]
				{
					if constexpr (!std::is_void_v<Val>)
						return dispatch_invoke(std::forward<F>(func), _storage.value);
					else
						return dispatch_invoke(std::forward<F>(func));
				}
				else if (has_error())
					return {in_place_error_t{}, _storage.error};
				else
					return {};
			}
			/** @copydoc transform_value */
			template<typename F> requires is_value_invocable<result &&, F>::value
			constexpr auto transform_value(F &&func) && noexcept(is_nothrow_value_invocable<result &&, F>::value) -> transform_value_result<result &&, F>
			{
				if (has_value()) [[likely]]
				{
					if constexpr (!std::is_void_v<Val>)
						return dispatch_invoke(std::forward<F>(func), std::move(_storage.value));
					else
						return dispatch_invoke(std::forward<F>(func));
				}
				else if (has_error())
					return {in_place_error_t{}, std::move(_storage.error)};
				else
					return {};
			}
			/** @copydoc transform_value */
			template<typename F> requires is_value_invocable<const result &, F>::value
			constexpr auto transform_value(F &&func) const & noexcept(is_nothrow_value_invocable<const result &, F>::value) -> transform_value_result<const result &, F>
			{
				if (has_value()) [[likely]]
				{
					if constexpr (!std::is_void_v<Val>)
						return dispatch_invoke(std::forward<F>(func), _storage.value);
					else
						return dispatch_invoke(std::forward<F>(func));
				}
				else if (has_error())
					return {in_place_error_t{}, _storage.error};
				else
					return {};
			}
			/** @copydoc transform_value */
			template<typename F> requires is_value_invocable<const result &&, F>::value
			constexpr auto transform_value(F &&func) const && noexcept(is_nothrow_value_invocable<const result &&, F>::value) -> transform_value_result<const result &&, F>
			{
				if (has_value()) [[likely]]
				{
					if constexpr (!std::is_void_v<Val>)
						return dispatch_invoke(std::forward<F>(func), std::move(_storage.value));
					else
						return dispatch_invoke(std::forward<F>(func));
				}
				else if (has_error())
					return {in_place_error_t{}, std::move(_storage.error)};
				else
					return {};
			}

			/** Forwards the contained value or invokes either \a func with the contained error, and returns result of the expression as `rod::result`. */
			template<typename F> requires is_error_invocable<result &, F>::value
			constexpr auto transform_error(F &func) & noexcept(is_nothrow_error_invocable<result &, F>::value) -> transform_error_result<result &, F>
			{
				if (has_value()) [[likely]]
					return {in_place_value_t{}, _storage.value};
				else if (has_error())
					return {in_place_error_t{}, std::invoke(std::forward<F>(func), _storage.error)};
				else
					return {};
			}
			/** @copydoc transform_error */
			template<typename F> requires is_error_invocable<result &&, F>::value
			constexpr auto transform_error(F &&func) && noexcept(is_nothrow_error_invocable<result &&, F>::value) -> transform_error_result<result &&, F>
			{
				if (has_value()) [[likely]]
					return {in_place_value_t{}, std::move(_storage.value)};
				else if (has_error())
					return {in_place_error_t{}, std::invoke(std::forward<F>(func), std::move(_storage.error))};
				else
					return {};
			}
			/** @copydoc transform_error */
			template<typename F> requires is_error_invocable<const result &, F>::value
			constexpr auto transform_error(F &&func) const & noexcept(is_nothrow_error_invocable<const result &, F>::value) -> transform_error_result<const result &, F>
			{
				if (has_value()) [[likely]]
					return {in_place_value_t{}, _storage.value};
				else if (has_error())
					return {in_place_error_t{}, std::invoke(std::forward<F>(func), _storage.error)};
				else
					return {};
			}
			/** @copydoc transform_error */
			template<typename F> requires is_error_invocable<const result &&, F>::value
			constexpr auto transform_error(F &&func) const && noexcept(is_nothrow_error_invocable<const result &&, F>::value) -> transform_error_result<const result &&, F>
			{
				if (has_value()) [[likely]]
					return {in_place_value_t{}, std::move(_storage.value)};
				else if (has_error())
					return {in_place_error_t{}, std::invoke(std::forward<F>(func), std::move(_storage.error))};
				else
					return {};
			}

			/** Invokes either \a func_val with the contained value or \a func_err with the contained error, and returns the result as the common type. */
			template<typename FVal, typename FErr> requires is_value_invocable<result &, FVal>::value && is_error_invocable<result &, FErr>::value
			constexpr auto into(FVal &&func_val, FErr &&func_err) & noexcept(is_nothrow_value_invocable<result &, FVal>::value && is_nothrow_error_invocable<result &, FErr>::value) -> into_result<result &, FVal, FErr>
			{
				if (has_value()) [[likely]]
				{
					if constexpr (!std::is_void_v<Val>)
						return into_result<result &, FVal, FErr>(dispatch_invoke(std::forward<FVal>(func_val), _storage.value));
					else
						return into_result<result &, FVal, FErr>(dispatch_invoke(std::forward<FVal>(func_val)));
				}
				else if (has_error())
					return into_result<result &, FVal, FErr>(std::invoke(std::forward<FErr>(func_err), _storage.error));
				else
					return into_result<result &, FVal, FErr>();
			}
			/** @copydoc into */
			template<typename FVal, typename FErr> requires is_value_invocable<result &&, FVal>::value && is_error_invocable<result &&, FErr>::value
			constexpr auto into(FVal &&func_val, FErr &&func_err) && noexcept(is_nothrow_value_invocable<result &&, FVal>::value && is_nothrow_error_invocable<result &&, FErr>::value) -> into_result<result &&, FVal, FErr>
			{
				if (has_value()) [[likely]]
				{
					if constexpr (!std::is_void_v<Val>)
						return into_result<result &&, FVal, FErr>(dispatch_invoke(std::forward<FVal>(func_val), std::move(_storage.value)));
					else
						return into_result<result &&, FVal, FErr>(dispatch_invoke(std::forward<FVal>(func_val)));
				}
				else if (has_error())
					return into_result<result &&, FVal, FErr>(std::invoke(std::forward<FErr>(func_err), std::move(_storage.error)));
				else
					return into_result<result &&, FVal, FErr>();
			}
			/** @copydoc into */
			template<typename FVal, typename FErr> requires is_value_invocable<const result &, FVal>::value && is_error_invocable<const result &, FErr>::value
			constexpr auto into(FVal &&func_val, FErr &&func_err) const & noexcept(is_nothrow_value_invocable<const result &, FVal>::value && is_nothrow_error_invocable<const result &, FErr>::value) -> into_result<const result &, FVal, FErr>
			{
				if (has_value()) [[likely]]
				{
					if constexpr (!std::is_void_v<Val>)
						return into_result<const result &, FVal, FErr>(dispatch_invoke(std::forward<FVal>(func_val), _storage.value));
					else
						return into_result<const result &, FVal, FErr>(dispatch_invoke(std::forward<FVal>(func_val)));
				}
				else if (has_error())
					return into_result<const result &, FVal, FErr>(std::invoke(std::forward<FErr>(func_err), _storage.error));
				else
					return into_result<const result &, FVal, FErr>();
			}
			/** @copydoc into */
			template<typename FVal, typename FErr> requires is_value_invocable<const result &&, FVal>::value && is_error_invocable<const result &&, FErr>::value
			constexpr auto into(FVal &&func_val, FErr &&func_err) const && noexcept(is_nothrow_value_invocable<const result &&, FVal>::value && is_nothrow_error_invocable<const result &&, FErr>::value) -> into_result<const result &&, FVal, FErr>
			{
				if (has_value()) [[likely]]
				{
					if constexpr (!std::is_void_v<Val>)
						return into_result<const result &&, FVal, FErr>(dispatch_invoke(std::forward<FVal>(func_val), std::move(_storage.value)));
					else
						return into_result<const result &&, FVal, FErr>(dispatch_invoke(std::forward<FVal>(func_val)));
				}
				else if (has_error())
					return into_result<const result &&, FVal, FErr>(std::invoke(std::forward<FErr>(func_err), std::move(_storage.error)));
				else
					return into_result<const result &&, FVal, FErr>();
			}

			/** Invokes either \a func with the contained value or forwards `this`, returning the common type of the two. */
			template<typename F> requires is_value_invocable<result &, F>::value
			constexpr decltype(auto) into_value(F &&func) & noexcept(is_nothrow_value_invocable<result &, F>::value) { return into(std::forward<F>(func), [this](auto &&) noexcept -> result & { return *this; }); }
			/** @copydoc into_value */
			template<typename F> requires is_value_invocable<result &&, F>::value
			constexpr decltype(auto) into_value(F &&func) && noexcept(is_nothrow_value_invocable<result &&, F>::value) { return into(std::forward<F>(func), [this](auto &&) noexcept -> result && { return std::move(*this); }); }
			/** @copydoc into_value */
			template<typename F> requires is_value_invocable<const result &, F>::value
			constexpr decltype(auto) into_value(F &&func) const & noexcept(is_nothrow_value_invocable<const result &, F>::value) { return into(std::forward<F>(func), [this](auto &&) noexcept -> const result & { return *this; }); }
			/** @copydoc into_value */
			template<typename F> requires is_value_invocable<const result &&, F>::value
			constexpr decltype(auto) into_value(F &&func) const && noexcept(is_nothrow_value_invocable<const result &&, F>::value) { return into(std::forward<F>(func), [this](auto &&) noexcept -> const result && { return std::move(*this); }); }

			/** Invokes either \a func with the contained error or forwards `this`, returning the common type of the two. */
			template<typename F> requires is_error_invocable<result &, F>::value
			constexpr decltype(auto) into_error(F &&func) & noexcept(is_nothrow_error_invocable<result &, F>::value) { return into([this](auto &&) noexcept -> result & { return *this; }, std::forward<F>(func)); }
			/** @copydoc into_value */
			template<typename F> requires is_error_invocable<result &&, F>::value
			constexpr decltype(auto) into_error(F &&func) && noexcept(is_nothrow_error_invocable<result &&, F>::value) { return into([this](auto &&) noexcept -> result && { return std::move(*this); }, std::forward<F>(func)); }
			/** @copydoc into_value */
			template<typename F> requires is_error_invocable<const result &, F>::value
			constexpr decltype(auto) into_error(F &&func) const & noexcept(is_nothrow_error_invocable<const result &, F>::value) { return into([this](auto &&) noexcept -> const result & { return *this; }, std::forward<F>(func)); }
			/** @copydoc into_value */
			template<typename F> requires is_error_invocable<const result &&, F>::value
			constexpr decltype(auto) into_error(F &&func) const && noexcept(is_nothrow_error_invocable<const result &&, F>::value) { return into([this](auto &&) noexcept -> const result && { return std::move(*this); }, std::forward<F>(func)); }

			/** Returns the contained value or the result of invocation of \a func.
			 * @note `std::invoke_result_t&ltF;&gt;` must decay to either `Val` or `rod::result&ltVal, Err&gt;` */
			template<typename F> requires decays_to_same<std::invoke_result_t<F>, result> || decays_to_same<std::invoke_result_t<F>, Val>
			constexpr result or_else(F &&func) & noexcept(std::is_nothrow_invocable_v<F> && std::is_nothrow_constructible_v<result, result &>)
			{
				if constexpr (!std::is_void_v<Val>)
				{
					if (!has_value())
						return std::invoke(std::forward<F>(func));
					else
						return _storage.value;
				}
				else if (!has_value())
					std::invoke(std::forward<F>(func));
			}
			/** @copydoc or_else */
			template<typename F> requires decays_to_same<std::invoke_result_t<F>, result> || decays_to_same<std::invoke_result_t<F>, Val>
			constexpr result or_else(F &&func) && noexcept(std::is_nothrow_invocable_v<F> && std::is_nothrow_constructible_v<result, result &&>)
			{
				if constexpr (!std::is_void_v<Val>)
				{
					if (!has_value())
						return std::invoke(std::forward<F>(func));
					else
						return std::move(_storage.value);
				}
				else if (!has_value())
					std::invoke(std::forward<F>(func));
			}
			/** @copydoc or_else */
			template<typename F> requires decays_to_same<std::invoke_result_t<F>, result> || decays_to_same<std::invoke_result_t<F>, Val>
			constexpr result or_else(F &&func) const & noexcept(std::is_nothrow_invocable_v<F> && std::is_nothrow_constructible_v<result, const result &>)
			{
				if constexpr (!std::is_void_v<Val>)
				{
					if (!has_value())
						return std::invoke(std::forward<F>(func));
					else
						return _storage.value;
				}
				else if (!has_value())
					std::invoke(std::forward<F>(func));
			}
			/** @copydoc or_else */
			template<typename F> requires decays_to_same<std::invoke_result_t<F>, result> || decays_to_same<std::invoke_result_t<F>, Val>
			constexpr result or_else(F &&func) const && noexcept(std::is_nothrow_invocable_v<F> && std::is_nothrow_constructible_v<result, const result &&>)
			{
				if constexpr (!std::is_void_v<Val>)
				{
					if (!has_value())
						return std::invoke(std::forward<F>(func));
					else
						return std::move(_storage.value);
				}
				else if (!has_value())
					std::invoke(std::forward<F>(func));
			}

			template<typename Val2 = Val, typename Err2 = Err> requires (std::same_as<Val, Val2> || std::equality_comparable_with<Val, Val2>) && std::equality_comparable_with<Err, Err2>
			[[nodiscard]] friend constexpr bool operator==(const result &a, const result<Val2, Err2> &b) noexcept(noexcept(std::declval<Err>() == std::declval<Val2>()) && noexcept(std::declval<Err>() == std::declval<Err2>()))
			{
				if (a.has_value() && b.has_value())
				{
					if constexpr (!std::is_void_v<Val>)
						return a._value == b._value;
					else
						return true;
				}
				if (a.has_error() && b.has_error())
					return a._error == b._error;
				return false;
			}

			constexpr void swap(result &other) noexcept(is_nothrow_swappable::value) requires is_swappable::value
			{
				if (has_value() && other.has_value())
				{
					if constexpr (!std::is_void_v<Val>)
						adl_swap(_storage.value, other._storage._value);
				}
				else if (has_error() && other.has_error())
					adl_swap(_storage._error, _storage.other._error);
				else
				{
					auto tmp = std::move(other);
					other = std::move(*this);
					*this = std::move(tmp);
				}
			}
			friend constexpr void swap(result &a, result &b) noexcept(is_nothrow_swappable::value) requires is_swappable::value { a.swap(b); }

		private:
			[[noreturn]] constexpr void throw_missing_value() const
			{
				if constexpr (!requires { throw_exception(_storage.error); })
					ROD_THROW(bad_result_access("bad result access (value)"));
				else
					throw_exception(_storage.error);
			}
			[[noreturn]] constexpr void throw_missing_error() const
			{
				ROD_THROW(bad_result_access("bad result access (error)"));
			}

			template<typename Val2> requires(!std::is_void_v<Val>)
			constexpr void assign_value(Val2 &&val) noexcept(std::is_nothrow_assignable_v<Val, Val2>)
			{
				if (!has_value())
					new (&_storage.value) value_type(std::forward<Val2>(val));
				else
					_storage.value = std::forward<Val2>(val);
			}
			template<typename Err2>
			constexpr void assign_error(Err2 &&err) noexcept(std::is_nothrow_assignable_v<Err, Err2>)
			{
				if (!has_value())
					new (&_storage.error) error_type(std::forward<Err2>(err));
				else
					_storage.error = std::forward<Err2>(err);
			}

			constexpr void destroy() noexcept(is_nothrow_destructible::value)
			{
				if (has_value())
				{
					if constexpr (!std::is_void_v<Val> && !std::is_trivially_destructible_v<value_type>)
						_storage.value.~value_type();
				}
				else if (has_error())
				{
					if constexpr (!std::is_trivially_destructible_v<error_type>)
						_storage.error.~error_type();
				}
			}

			storage_t _storage;
			std::int8_t _state;
		};

		static_assert(std::is_standard_layout_v<result<void>>);
		static_assert(std::is_standard_layout_v<result<long>>);

#ifndef NDEBUG /* Extra check to return nullptr in debug mode to make debugging easier */
		template<typename Val, typename Err>
		template<typename U>
		constexpr auto result<Val, Err>::operator->() noexcept -> std::enable_if_t<std::negation_v<std::is_void<Val>>, U *> { return has_value() ? &_storage.value : nullptr; }
		template<typename Val, typename Err>
		template<typename U>
		constexpr auto result<Val, Err>::operator->() const noexcept -> std::enable_if_t<std::negation_v<std::is_void<Val>>, U *> { return has_value() ? &_storage.value : nullptr; }
#else
		template<typename Val, typename Err>
		template<typename U>
		constexpr auto result<Val, Err>::operator->() noexcept -> std::enable_if_t<std::negation_v<std::is_void<Val>>, U *> { return &_storage.value; }
		template<typename Val, typename Err>
		template<typename U>
		constexpr auto result<Val, Err>::operator->() const noexcept -> std::enable_if_t<std::negation_v<std::is_void<Val>>, U *> { return &_storage.value; }
#endif

		template<typename Val, typename Err>
		template<typename U>
		constexpr auto result<Val, Err>::operator*() & noexcept -> std::enable_if_t<std::negation_v<std::is_void<Val>>, std::add_lvalue_reference_t<U>> { return *operator->(); }
		template<typename Val, typename Err>
		template<typename U>
		constexpr auto result<Val, Err>::operator*() && noexcept -> std::enable_if_t<std::negation_v<std::is_void<Val>>, std::add_rvalue_reference_t<U>> { return std::move(*operator->()); }
		template<typename Val, typename Err>
		template<typename U>
		constexpr auto result<Val, Err>::operator*() const & noexcept -> std::enable_if_t<std::negation_v<std::is_void<Val>>, std::add_lvalue_reference_t<U>> { return *operator->(); }
		template<typename Val, typename Err>
		template<typename U>
		constexpr auto result<Val, Err>::operator*() const && noexcept -> std::enable_if_t<std::negation_v<std::is_void<Val>>, std::add_rvalue_reference_t<U>> { return std::move(*operator->()); }
	}

	/** Utility used to check if \a T is an instance of `rod::result`. */
	template<typename T>
	using is_result = _result::is_result<T>;
	/** Alias for `is_result&lt;T&gt;::value` */
	template<typename T>
	inline constexpr bool is_result_v = is_result<T>::value;

	/** Utility used to check if \a T is an instance of `rod::result` with value type \a Val. */
	template<typename T, typename Val>
	using is_result_with_value = _result::is_result_with_value<T, Val>;
	/** Alias for `is_result_with_value&lt;T, Val&gt;::value` */
	template<typename T, typename Val>
	inline constexpr bool is_result_with_value_v = is_result_with_value<T, Val>::value;

	/** Utility used to check if \a T is an instance of `rod::result` with error type \a Err. */
	template<typename T, typename Err>
	using is_result_with_error = _result::is_result_with_value<T, Err>;
	/** Alias for `is_result_with_error&lt;T, Err&gt;::value` */
	template<typename T, typename Err>
	inline constexpr bool is_result_with_error_v = is_result_with_error<T, Err>::value;

	using _result::bad_result_access;
	using _result::result;

	using _result::in_place_value_t;
	using _result::in_place_error_t;

	/** Instance of `rod::in_place_value_t`. */
	inline constexpr auto in_place_value = in_place_value_t{};
	/** Instance of `rod::in_place_error_t`. */
	inline constexpr auto in_place_error = in_place_error_t{};
}

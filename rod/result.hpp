/*
 * Created by switchblade on 2023-06-26.
 */

#pragma once

#include <system_error>
#include <optional>
#include <memory>

#include "utility.hpp"

namespace rod
{
	namespace _result
	{
		template<typename Val, typename Err>
		class result;

		/** Exception thrown when an invalid state was requested from `rod::result`. */
		class bad_result_access : public std::runtime_error
		{
			template<typename, typename>
			friend
			class result;

			explicit bad_result_access(const char *msg) : std::runtime_error(msg) {}
			explicit bad_result_access(const std::string &msg) : std::runtime_error(msg) {}

		public:
			ROD_API_PUBLIC ~bad_result_access() override;
		};

		/** Tag used to select value-state constructor for `rod::result`. */
		struct in_place_value_t {};
		/** Tag used to select error-state constructor for `rod::result`. */
		struct in_place_error_t {};

		struct throw_exception_t
		{
			template<decays_to<std::error_code> Err>
			[[noreturn]] constexpr void operator()(Err &&err) { throw std::system_error(std::forward<Err>(err)); }

			template<typename Err> requires tag_invocable<throw_exception_t, Err>
			[[noreturn]] constexpr void operator()(Err &&err) { tag_invoke(*this, std::forward<Err>(err)); }
			template<typename Err> requires(!tag_invocable<throw_exception_t, Err> && requires (Err &&err) { err.throw_exception(); })
			[[noreturn]] constexpr void operator()(Err &&err) { err.throw_exception(); }
		};

		template<typename Val, typename Err>
		union storage
		{
			constexpr storage() {}
			constexpr storage(const storage &) {}
			constexpr storage &operator=(const storage &) {}
			constexpr storage(storage &&) {}
			constexpr storage &operator=(storage &&) {}
			constexpr ~storage() {}

			Val value;
			Err error;
		};
		template<typename Err>
		union storage<void, Err>
		{
			constexpr storage() {}
			constexpr storage(const storage &) {}
			constexpr storage &operator=(const storage &) {}
			constexpr storage(storage &&) {}
			constexpr storage &operator=(storage &&) {}
			constexpr ~storage() {}

			Err error;
		};

		/** Utility type used to return a value or error result.
		 * @tparam Val Value type stored by the result.
		 * @tparam Err Error type stored by the result. */
		template<typename Val = void, typename Err = std::error_code>
		class result
		{
			template<typename, typename>
			friend class result;

			static_assert(!std::is_void_v<Err>, "`rod::result` does not support `void` errors, use `std::optional&lt;Val&gt;` instead");

		public:
			using value_type = Val;
			using error_type = Err;

		private:
			constexpr static std::int8_t is_empty = 0;
			constexpr static std::int8_t is_value = 1;
			constexpr static std::int8_t is_error = 2;

			using storage_t = storage<value_type, error_type>;

			template<typename Val2, typename Err2>
			using is_constructible = std::conjunction<std::disjunction<std::is_void<value_type>, std::is_constructible<value_type, Val2>>, std::is_constructible<error_type, Err2>>;
			template<typename Val2, typename Err2>
			using is_assignable = std::conjunction<is_constructible<Val2, Err2>, std::disjunction<std::is_void<value_type>, std::is_assignable<value_type, Val2>>, std::is_assignable<error_type, Err2>>;

			template<typename Val2, typename Err2>
			using is_nothrow_constructible = std::conjunction<std::disjunction<std::is_void<value_type>, std::is_nothrow_constructible<value_type, Val2>>, std::is_nothrow_constructible<error_type, Err2>>;
			template<typename Val2, typename Err2>
			using is_nothrow_assignable = std::conjunction<is_nothrow_constructible<Val2, Err2>, std::disjunction<std::is_void<value_type>, std::is_nothrow_assignable<value_type, Val2>>, std::is_nothrow_assignable<error_type, Err2>>;

			using is_copy_constructible = std::conjunction<std::disjunction<std::is_void<value_type>, std::is_copy_constructible<value_type>>, std::is_copy_constructible<error_type>>;
			using is_move_constructible = std::conjunction<std::disjunction<std::is_void<value_type>, std::is_move_constructible<value_type>>, std::is_move_constructible<error_type>>;
			using is_copy_assignable = std::conjunction<is_copy_constructible, std::disjunction<std::is_void<value_type>, std::is_copy_assignable<value_type>>, std::is_copy_assignable<error_type>>;
			using is_move_assignable = std::conjunction<is_move_constructible, std::disjunction<std::is_void<value_type>, std::is_move_assignable<value_type>>, std::is_move_assignable<error_type>>;

			using is_nothrow_copy_constructible = std::conjunction<std::disjunction<std::is_void<value_type>, std::is_nothrow_copy_constructible<value_type>>, std::is_nothrow_copy_constructible<error_type>>;
			using is_nothrow_move_constructible = std::conjunction<std::disjunction<std::is_void<value_type>, std::is_nothrow_move_constructible<value_type>>, std::is_nothrow_move_constructible<error_type>>;
			using is_nothrow_copy_assignable = std::conjunction<is_nothrow_copy_constructible, std::disjunction<std::is_void<value_type>, std::is_nothrow_copy_assignable<value_type>>, std::is_nothrow_copy_assignable<error_type>>;
			using is_nothrow_move_assignable = std::conjunction<is_nothrow_move_constructible, std::disjunction<std::is_void<value_type>, std::is_nothrow_move_assignable<value_type>>, std::is_nothrow_move_assignable<error_type>>;

			using is_nothrow_destructible = std::conjunction<std::disjunction<std::is_void<value_type>, std::is_nothrow_destructible<value_type>>, std::is_nothrow_destructible<error_type>>;

			using is_swappable = std::conjunction<is_move_constructible, std::disjunction<std::is_void<value_type>, std::is_swappable<value_type>>, std::is_swappable<error_type>>;
			using is_nothrow_swappable = std::conjunction<is_nothrow_move_constructible, std::disjunction<std::is_void<value_type>, std::is_nothrow_swappable<value_type>>, std::is_nothrow_swappable<error_type>>;

			template<typename T, typename F>
			using is_value_invocable = std::conditional_t<std::is_void_v<value_type>, std::is_invocable<F>, std::is_invocable<F, copy_cvref_t<T, value_type>>>;
			template<typename T, typename F>
			using is_nothrow_value_invocable = std::conjunction<is_value_invocable<T, F>, std::conditional_t<std::is_void_v<value_type>, std::is_nothrow_invocable<F>, std::is_nothrow_invocable<F, copy_cvref_t<T, value_type>>>>;

			template<typename T, typename F>
			using transform_return_type = std::conditional_t<std::is_void_v<value_type>, std::invoke_result_t<F>, std::invoke_result_t<F, copy_cvref_t<T, value_type>>>;
			template<typename T, typename F>
			using transform_result = result<transform_return_type<T, F>, error_type>;

		public:
			/** Initializes result with empty (neither value nor error) state. */
			constexpr result() noexcept : _state(std::is_void_v<value_type> ? is_value : is_empty) {}

			/** Copy-initializes result's value. */
			constexpr result(const value_type &val) noexcept(std::is_nothrow_copy_constructible_v<value_type>) requires(!std::is_void_v<value_type> && !std::same_as<value_type, error_type> && std::copy_constructible<value_type>)
			{
				std::construct_at(&_storage.value, val);
				_state = is_value;
			}
			/** Move-initializes result's value. */
			constexpr result(value_type &&val) noexcept(std::is_nothrow_move_constructible_v<value_type>) requires(!std::is_void_v<value_type> && !std::same_as<value_type, error_type> && std::move_constructible<value_type>)
			{
				std::construct_at(&_storage.value, std::forward<value_type>(val));
				_state = is_value;
			}

			/** Copy-initializes result's error. */
			constexpr result(const error_type &err) noexcept(std::is_nothrow_copy_constructible_v<error_type>) requires(!std::same_as<value_type, error_type> && std::copy_constructible<error_type>)
			{
				std::construct_at(&_storage.error, err);
				_state = is_error;
			}
			/** Move-initializes result's error. */
			constexpr result(error_type &&err) noexcept(std::is_nothrow_move_constructible_v<error_type>) requires(!std::same_as<value_type, error_type> && std::move_constructible<error_type>)
			{
				std::construct_at(&_storage.error, std::forward<error_type>(err));
				_state = is_error;
			}

			/** Direct-initializes result's value from passed arguments.
			 * @param args Arguments passed to the value type's constructor. */
			template<typename... Args> requires(!std::is_void_v<value_type> && std::constructible_from<value_type, Args...>)
			constexpr result(in_place_value_t, Args &&...args) noexcept(std::is_nothrow_constructible_v<value_type, Args...>)
			{
				std::construct_at(&_storage.value, std::forward<Args>(args)...);
				_state = is_value;
			}
			/** Direct-initializes result's value from passed initializer list and arguments.
			 * @param init Initializer list passed to the value type's constructor.
			 * @param args Arguments passed to the value type's constructor. */
			template<typename T, typename... Args> requires(!std::is_void_v<value_type> && std::constructible_from<value_type, const std::initializer_list<T> &, Args...>)
			constexpr result(in_place_value_t, std::initializer_list<T> init, Args &&...args) noexcept(std::is_nothrow_constructible_v<value_type, std::initializer_list<T>, Args...>)
			{
				std::construct_at(&_storage.value, init, std::forward<Args>(args)...);
				_state = is_value;
			}

			/** Direct-initializes result's error from passed arguments.
			 * @param args Arguments passed to the error type's constructor. */
			template<typename... Args> requires std::constructible_from<error_type, Args...>
			constexpr result(in_place_error_t, Args &&...args) noexcept(std::is_nothrow_constructible_v<error_type, Args...>)
			{
				std::construct_at(&_storage.error, std::forward<Args>(args)...);
				_state = is_error;
			}
			/** Direct-initializes result's error from passed initializer list and arguments.
			 * @param init Initializer list passed to the error type's constructor.
			 * @param args Arguments passed to the error type's constructor. */
			template<typename T, typename... Args> requires std::constructible_from<error_type, const std::initializer_list<T> &, Args...>
			constexpr result(in_place_error_t, std::initializer_list<T> init, Args &&...args) noexcept(std::is_nothrow_constructible_v<error_type, std::initializer_list<T>, Args...>)
			{
				std::construct_at(&_storage.error, init, std::forward<Args>(args)...);
				_state = is_error;
			}

			template<typename Val2, typename Err2> requires(!std::is_void_v<Val2> && !std::same_as<result, result<Val2, Err2>> && is_constructible<const Val2 &, const Err2 &>::value)
			constexpr result(const result<Val2, Err2> &other) noexcept(is_nothrow_constructible<const Val2 &, const Err2 &>::value) : _state(other._state)
			{
				if (has_value())
				{
					if constexpr (std::is_void_v<value_type>)
						std::construct_at(&_storage.value, other._storage.value);
				}
				else if (has_error())
					std::construct_at(&_storage.error, other._storage.error);
			}
			template<typename Val2, typename Err2> requires(!std::is_void_v<Val2> && !std::same_as<result, result<Val2, Err2>> && is_constructible<Val2, Err2>::value)
			constexpr result(result<Val2, Err2> &&other) noexcept(is_nothrow_constructible<Val2 &&, Err2 &&>::value) : _state(other._state)
			{
				if (has_value())
				{
					if constexpr (std::is_void_v<value_type>)
						std::construct_at(&_storage.value, std::move(other._storage.value));
				}
				else if (has_error())
					std::construct_at(&_storage.error, std::move(other._storage.error));
			}

			template<typename Val2, typename Err2> requires(!std::is_void_v<Val2> && !std::same_as<result, result<Val2, Err2>> && is_assignable<const Val2 &, const Err2 &>::value)
			constexpr result &operator=(const result<Val2, Err2> &other) noexcept(is_nothrow_assignable<const Val2 &, const Err2 &>::value)
			{
				if (_state != other._state)
					destroy();

				if (other.has_value())
				{
					if constexpr (std::is_void_v<value_type>)
						assign_value(other._storage.value);
				}
				else if (other.has_error())
					assign_error(other._storage.error);

				_state = other._state;
				return *this;
			}
			template<typename Val2, typename Err2> requires(!std::is_void_v<Val2> && !std::same_as<result, result<Val2, Err2>> && is_assignable<Val2, Err2>::value)
			constexpr result &operator=(result<Val2, Err2> &&other) noexcept(is_nothrow_move_assignable::value) requires is_move_assignable::value
			{
				if (_state != other._state)
					destroy();

				if (other.has_value())
				{
					if constexpr (std::is_void_v<value_type>)
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
					if constexpr (std::is_void_v<value_type>)
						std::construct_at(&_storage.value, other._storage.value);
				}
				else if (has_error())
					std::construct_at(&_storage.error, other._storage.error);
			}
			constexpr result(result &&other) noexcept(is_nothrow_move_constructible::value) requires is_move_constructible::value : _state(other._state)
			{
				if (has_value())
				{
					if constexpr (std::is_void_v<value_type>)
						std::construct_at(&_storage.value, std::move(other._storage.value));
				}
				else if (has_error())
					std::construct_at(&_storage.error, std::move(other._storage.error));
			}

			constexpr result &operator=(const result &other) noexcept(is_nothrow_copy_assignable::value) requires is_copy_assignable::value
			{
				if (_state != other._state)
					destroy();

				if (other.has_value())
				{
					if constexpr (std::is_void_v<value_type>)
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
					if constexpr (std::is_void_v<value_type>)
						assign_value(std::move(other._storage.value));
				}
				else if (other.has_error())
					assign_error(std::move(other._storage.error));

				_state = other._state;
				return *this;
			}

			constexpr ~result() noexcept(is_nothrow_destructible::value) { destroy(); }

			/** Destroys the contained value or error. */
			constexpr void reset() noexcept(is_nothrow_destructible::value)
			{
				destroy();
				_state = is_empty;
			}

			/** Emplaces a value to the result.
			 * @param args Arguments passed to the value.
			 * @return Reference to the contained value object. */
			template<typename... Args> requires(!std::is_void_v<value_type> && std::constructible_from<value_type, Args...>)
			constexpr value_type &emplace_value(Args &&...args) noexcept(is_nothrow_destructible::value && std::is_nothrow_constructible_v<value_type, Args...>)
			{
				reset();
				std::construct_at(&_storage.value, std::forward<Args>(args)...);
				_state = is_value;
				return _storage.value;
			}
			/** Emplaces an error to the result.
			 * @param args Arguments passed to the error.
			 * @return Reference to the contained error object. */
			template<typename... Args> requires std::constructible_from<error_type, Args...>
			constexpr error_type &emplace_error(Args &&...args) noexcept(is_nothrow_destructible::value && std::is_nothrow_constructible_v<error_type, Args...>)
			{
				reset();
				std::construct_at(&_storage.error, std::forward<Args>(args)...);
				_state = is_error;
				return _storage.error;
			}

			/** Checks if the result has neither value nor error state. */
			[[nodiscard]] constexpr bool empty() const noexcept { return _state == is_empty; }
			/** Checks if the result is not empty. */
			[[nodiscard]] constexpr explicit operator bool() const noexcept { return !empty(); }

			/** Checks if the result contains a value. */
			[[nodiscard]] constexpr bool has_value() const noexcept { return _state == is_value; }
			/** Checks if the result contains an error. */
			[[nodiscard]] constexpr bool has_error() const noexcept { return _state == is_error; }

			/** Returns pointer to the value of the result.
			 * @note If the result does not contain a value results in undefined behavior. */
			template<typename U = Val>
			[[nodiscard]] constexpr std::enable_if_t<std::negation_v<std::is_void<Val>>, U *> operator->() noexcept;
			/** @copydoc value */
			template<typename U = std::add_const_t<Val>>
			[[nodiscard]] constexpr std::enable_if_t<std::negation_v<std::is_void<Val>>, U *> operator->() const noexcept;

			/** Returns reference to the value of the result.
			 * @note If the result does not contain a value results in undefined behavior. */
			template<typename U = Val>
			[[nodiscard]] constexpr std::enable_if_t<std::negation_v<std::is_void<Val>>, U &> operator*() & noexcept;
			/** @copydoc value */
			template<typename U = Val>
			[[nodiscard]] constexpr std::enable_if_t<std::negation_v<std::is_void<Val>>, U &&> operator*() && noexcept;
			/** @copydoc value */
			template<typename U = std::add_const_t<Val>>
			[[nodiscard]] constexpr std::enable_if_t<std::negation_v<std::is_void<Val>>, U &> operator*() const & noexcept;
			/** @copydoc value */
			template<typename U = std::add_const_t<Val>>
			[[nodiscard]] constexpr std::enable_if_t<std::negation_v<std::is_void<Val>>, U &&> operator*() const && noexcept;

			/** Returns reference to the contained value of the result.
			 * @throw `rod::throw_exception(error())` if the result does not contain a value and  is ill-formed.
			 * @throw bad_result_access If the result does not contain a value and `rod::throw_exception(error())` is ill-formed. */
			constexpr decltype(auto) value() &
			{
				if (!has_value())
					throw_value_exception();
				else if constexpr (!std::is_void_v<value_type>)
					return _storage.value;
			}
			/** @copydoc value */
			constexpr decltype(auto) value() &&
			{
				if (!has_value())
					throw_value_exception();
				else if constexpr (!std::is_void_v<value_type>)
					return std::move(_storage.value);
			}
			/** @copydoc value */
			constexpr decltype(auto) value() const &
			{
				if (!has_value())
					throw_value_exception();
				else if constexpr (!std::is_void_v<value_type>)
					return _storage.value;
			}
			/** @copydoc value */
			constexpr decltype(auto) value() const &&
			{
				if (!has_value())
					throw_value_exception();
				else if constexpr (!std::is_void_v<value_type>)
					return std::move(_storage.value);
			}

			/** Returns the contained value or \a val if result does not contain a value. */
			template<typename Val2 = value_type> requires(!std::is_void_v<value_type> && std::copy_constructible<value_type> && std::constructible_from<value_type, Val2>)
			[[nodiscard]] constexpr value_type value_or(Val2 &&val) const & noexcept(std::is_nothrow_copy_constructible_v<value_type> && std::is_nothrow_constructible_v<value_type, Val2>)
			{
				if (!has_value()) [[unlikely]]
					return Val2(std::forward<Val2>(val));
				else
					return _storage.value;
			}
			/** @copydoc value_or */
			template<typename Val2 = value_type> requires(!std::is_void_v<value_type> && std::move_constructible<value_type> && std::constructible_from<value_type, Val2>)
			[[nodiscard]] constexpr value_type value_or(Val2 &&val) && noexcept(std::is_nothrow_move_constructible_v<value_type> && std::is_nothrow_constructible_v<value_type, Val2>)
			{
				if (!has_value()) [[unlikely]]
					return Val2(std::forward<Val2>(val));
				else
					return std::move(_storage.value);
			}

			/** Returns reference to the error of the result.
			 * @throw bad_result_access if the result does not contain an error. */
			constexpr decltype(auto) error() &
			{
				if (!has_error())
					throw_error_exception();
				else
					return _storage.error;
			}
			/** @copydoc error */
			constexpr decltype(auto) error() &&
			{
				if (!has_error())
					throw_error_exception();
				else
					return std::move(_storage.error);
			}
			/** @copydoc error */
			constexpr decltype(auto) error() const &
			{
				if (!has_error())
					throw_error_exception();
				else
					return _storage.error;
			}
			/** @copydoc error */
			constexpr decltype(auto) error() const &&
			{
				if (!has_error())
					throw_error_exception();
				else
					return std::move(_storage.error);
			}

			/** Returns the error state or \a err if result does not contain a value. */
			template<typename Err2 = error_type> requires std::copy_constructible<error_type> && std::constructible_from<error_type, Err2>
			[[nodiscard]] constexpr error_type error_or(Err2 &&err) const & noexcept(std::is_nothrow_copy_constructible_v<error_type> && std::is_nothrow_constructible_v<error_type, Err2>)
			{
				if (!has_error()) [[unlikely]]
					return Err(std::forward<Err2>(err));
				else
					return _storage.error;
			}
			/** @copydoc error_or */
			template<typename Err2 = error_type> requires std::move_constructible<error_type> && std::constructible_from<error_type, Err2>
			[[nodiscard]] constexpr error_type error_or(Err2 &&err) && noexcept(std::is_nothrow_move_constructible_v<error_type> && std::is_nothrow_constructible_v<error_type, Err2>)
			{
				if (!has_error()) [[unlikely]]
					return Err(std::forward<Err2>(err));
				else
					return std::move(_storage.error);
			}

			/** If result contains a value, invokes \a func with the value, then returns result of the expression as `rod::result`. Otherwise, returns the contained error. */
			template<typename F> requires is_value_invocable<const result &, F>::value
			constexpr transform_result<const result &, F> transform(F &&func) const & noexcept(is_nothrow_value_invocable<const result &, F>::value)
			{
				if (has_value()) [[likely]]
				{
					if constexpr (!std::is_void_v<value_type>)
						return std::invoke(std::forward<F>(func), _storage.value);
					else
						return std::invoke(std::forward<F>(func));
				}
				else if (has_error())
					return _storage.error;
				else
					return {};
			}
			/** @copydoc transform */
			template<typename F> requires is_value_invocable<result &&, F>::value
			constexpr transform_result<result &&, F> transform(F &&func) && noexcept(is_nothrow_value_invocable<result &&, F>::value)
			{
				if (has_value()) [[likely]]
				{
					if constexpr (!std::is_void_v<value_type>)
						return std::invoke(std::forward<F>(func), std::move(_storage.value));
					else
						return std::invoke(std::forward<F>(func));
				}
				else if (has_error())
					return std::move(_storage.error);
				else
					return {};
			}

			/** Returns the contained value or the result of invocation of \a func.
			 * @note `std::invoke_result_t&ltF;&gt;` must decay to either `value_type` or `rod::result&ltvalue_type, error_type&gt;` */
			template<typename F> requires decays_to<std::invoke_result_t<F>, result> || decays_to<std::invoke_result_t<F>, value_type>
			constexpr result or_else(F &&func) const & noexcept(std::is_nothrow_invocable_v<F>)
			{
				if constexpr (!std::is_void_v<value_type>)
				{
					if (!has_value())
						return std::invoke(std::forward<F>(func));
					else
						return _storage.value;
				}
				else if (!has_value())
					std::invoke(std::forward<F>(func));
			}
			/** @copydoc transform */
			template<typename F> requires decays_to<std::invoke_result_t<F>, result> || decays_to<std::invoke_result_t<F>, value_type>
			constexpr result or_else(F &&func) && noexcept(std::is_nothrow_invocable_v<F>)
			{
				if constexpr (!std::is_void_v<value_type>)
				{
					if (!has_value())
						return std::invoke(std::forward<F>(func));
					else
						return std::move(_storage.value);
				}
				else if (!has_value())
					std::invoke(std::forward<F>(func));
			}

			template<typename Val2 = value_type, typename Err2 = error_type> requires (std::same_as<value_type, Val2> || std::equality_comparable_with<value_type, Val2>) && std::equality_comparable_with<error_type, Err2>
			[[nodiscard]] friend constexpr bool operator==(const result &a, const result<Val2, Err2> &b) noexcept(noexcept(std::declval<error_type>() == std::declval<Val2>()) && noexcept(std::declval<error_type>() == std::declval<Err2>()))
			{
				if (a.has_value() && b.has_value())
				{
					if constexpr (!std::is_void_v<value_type>)
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
					if constexpr (!std::is_void_v<value_type>)
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
			[[noreturn]] constexpr void throw_value_exception() const
			{
				if constexpr (!_detail::callable<throw_exception_t, const Err &>)
					throw bad_result_access{"bad result access (value)"};
				else
					throw_exception_t{}(_storage.error);
			}
			[[noreturn]] constexpr void throw_error_exception() const
			{
				throw bad_result_access{"bad result access (error)"};
			}

			template<typename Val2> requires(!std::is_void_v<value_type>)
			constexpr void assign_value(Val2 &&val) noexcept(std::is_nothrow_assignable_v<value_type, Val2>)
			{
				if (!has_value())
					std::construct_at(&_storage.value, std::forward<Val2>(val));
				else
					_storage.value = std::forward<Val2>(val);
			}
			template<typename Err2>
			constexpr void assign_error(Err2 &&err) noexcept(std::is_nothrow_assignable_v<error_type, Err2>)
			{
				if (!has_value())
					std::construct_at(&_storage.error, std::forward<Err2>(err));
				else
					_storage.error = std::forward<Err2>(err);
			}

			constexpr void destroy() noexcept(is_nothrow_destructible::value)
			{
				if (has_value())
				{
					if constexpr (!std::is_void_v<value_type>)
						std::destroy_at(&_storage.value);
				}
				else if (has_error())
					std::destroy_at(&_storage.error);
			}

			storage_t _storage;
			std::int8_t _state;
		};

#ifndef NDEBUG /* Extra check to return nullptr in debug mode to make debugging easier */
		template<typename Val, typename Err>
		template<typename U>
		constexpr std::enable_if_t<std::negation_v<std::is_void<Val>>, U *> result<Val, Err>::operator->() noexcept { return has_value() ? &_storage.value : nullptr; }
		template<typename Val, typename Err>
		template<typename U>
		constexpr std::enable_if_t<std::negation_v<std::is_void<Val>>, U *> result<Val, Err>::operator->() const noexcept { return has_value() ? &_storage.value : nullptr; }
#else
		template<typename Val, typename Err>
		template<typename U>
		constexpr std::enable_if_t<std::negation_v<std::is_void<Val>>, U *> result<Val, Err>::operator->() noexcept { return &_storage.value; }
		template<typename Val, typename Err>
		template<typename U>
		constexpr std::enable_if_t<std::negation_v<std::is_void<Val>>, U *> result<Val, Err>::operator->() const noexcept { return &_storage.value; }
#endif

		template<typename Val, typename Err>
		template<typename U>
		constexpr std::enable_if_t<std::negation_v<std::is_void<Val>>, U &> result<Val, Err>::operator*() & noexcept { return *operator->(); }
		template<typename Val, typename Err>
		template<typename U>
		constexpr std::enable_if_t<std::negation_v<std::is_void<Val>>, U &> result<Val, Err>::operator*() const & noexcept { return *operator->(); }
		template<typename Val, typename Err>
		template<typename U>
		constexpr std::enable_if_t<std::negation_v<std::is_void<Val>>, U &&> result<Val, Err>::operator*() && noexcept { return std::move(*operator->()); }
		template<typename Val, typename Err>
		template<typename U>
		constexpr std::enable_if_t<std::negation_v<std::is_void<Val>>, U &&> result<Val, Err>::operator*() const && noexcept { return std::move(*operator->()); }
	}

	using _result::bad_result_access;
	using _result::result;

	using _result::in_place_value_t;
	using _result::in_place_error_t;

	/** Instance of `rod::in_place_value_t`. */
	inline constexpr auto in_place_value = in_place_value_t{};
	/** Instance of `rod::in_place_error_t`. */
	inline constexpr auto in_place_error = in_place_error_t{};

	using _result::throw_exception_t;

	/** Customization point object used to produce a C++ exception from an error object. Default overload exists for `std::error_code`. */
	inline constexpr auto throw_exception = throw_exception_t{};
}

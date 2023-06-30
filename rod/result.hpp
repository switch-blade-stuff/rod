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
	template<typename Val, typename Err>
	class result;

	/** Exception thrown when an invalid state was requested from `rod::result`. */
	class ROD_API_PUBLIC bad_result_access : public std::runtime_error
	{
		template<typename, typename>
		friend class result;

		explicit bad_result_access(const char *msg) : std::runtime_error(msg) {}
		explicit bad_result_access(const std::string &msg) : std::runtime_error(msg) {}

	public:
		~bad_result_access() override = default;
	};

	/** Tag used to select value-state constructor for `rod::result`. */
	struct in_place_value_t {};
	/** Tag used to select error-state constructor for `rod::result`. */
	struct in_place_error_t {};

	/** Instance of `rod::in_place_value_t`. */
	inline constexpr auto in_place_value = in_place_value_t{};
	/** Instance of `rod::in_place_error_t`. */
	inline constexpr auto in_place_error = in_place_error_t{};

	/** Utility type used to return a value or error result.
	 * @tparam Val Value type stored by the result.
	 * @tparam Err Error type stored by the result. */
	template<typename Val, typename Err>
	class result
	{
		template<typename, typename>
		friend class result;

		static_assert(!std::is_void_v<Val>, "`rod::result` does not support `void` values, use `std::optional<Err>` instead");
		static_assert(!std::is_void_v<Err>, "`rod::result` does not support `void` errors, use `std::optional<Val>` instead");

	public:
		using value_type = Val;
		using error_type = Err;

	private:
		constexpr static std::uint8_t is_empty = 0;
		constexpr static std::uint8_t is_value = 1;
		constexpr static std::uint8_t is_error = 2;

		template<typename Val2, typename Err2>
		using is_constructible = std::conjunction<std::is_constructible<value_type, Val2>, std::is_constructible<error_type, Err2>>;
		template<typename Val2, typename Err2>
		using is_assignable = std::conjunction<is_constructible<Val2, Err2>, std::is_assignable<value_type, Val2>, std::is_assignable<error_type, Err2>>;

		template<typename Val2, typename Err2>
		using is_nothrow_constructible = std::conjunction<std::is_nothrow_constructible<value_type, Val2>, std::is_nothrow_constructible<error_type, Err2>>;
		template<typename Val2, typename Err2>
		using is_nothrow_assignable = std::conjunction<is_nothrow_constructible<Val2, Err2>, std::is_nothrow_assignable<value_type, Val2>, std::is_nothrow_assignable<error_type, Err2>>;

		using is_copy_constructible = std::conjunction<std::is_copy_constructible<value_type>, std::is_copy_constructible<error_type>>;
		using is_move_constructible = std::conjunction<std::is_move_constructible<value_type>, std::is_move_constructible<error_type>>;
		using is_copy_assignable = std::conjunction<is_copy_constructible, std::is_copy_assignable<value_type>, std::is_copy_assignable<error_type>>;
		using is_move_assignable = std::conjunction<is_move_constructible, std::is_move_assignable<value_type>, std::is_move_assignable<error_type>>;

		using is_nothrow_copy_constructible = std::conjunction<std::is_nothrow_copy_constructible<value_type>, std::is_nothrow_copy_constructible<error_type>>;
		using is_nothrow_move_constructible = std::conjunction<std::is_nothrow_move_constructible<value_type>, std::is_nothrow_move_constructible<error_type>>;
		using is_nothrow_copy_assignable = std::conjunction<is_nothrow_copy_constructible, std::is_nothrow_copy_assignable<value_type>, std::is_nothrow_copy_assignable<error_type>>;
		using is_nothrow_move_assignable = std::conjunction<is_nothrow_move_constructible, std::is_nothrow_move_assignable<value_type>, std::is_nothrow_move_assignable<error_type>>;

		using is_nothrow_destructible = std::conjunction<std::is_nothrow_destructible<value_type>, std::is_nothrow_destructible<error_type>>;

		using is_swappable = std::conjunction<is_move_constructible, std::is_swappable<value_type>, std::is_swappable<error_type>>;
		using is_nothrow_swappable = std::conjunction<is_nothrow_move_constructible, std::is_nothrow_swappable<value_type>, std::is_nothrow_swappable<error_type>>;

	public:
		/** Initializes result with empty (neither value nor error) state. */
		constexpr result() noexcept : _state(is_empty) {}

		/** Copy-initializes result's value. */
		constexpr result(const value_type &val) noexcept(std::is_nothrow_copy_constructible_v<value_type>) requires(!std::same_as<value_type, error_type> && std::copy_constructible<value_type>) : result(in_place_value, val) {}
		/** Move-initializes result's value. */
		constexpr result(value_type &&val) noexcept(std::is_nothrow_move_constructible_v<value_type>) requires(!std::same_as<value_type, error_type> && std::move_constructible<value_type>) : result(in_place_value, std::forward<value_type>(val)) {}

		/** Copy-initializes result's error. */
		constexpr result(const error_type &err) noexcept(std::is_nothrow_copy_constructible_v<error_type>) requires(!std::same_as<value_type, error_type> && std::copy_constructible<error_type>) : result(in_place_error, err) {}
		/** Move-initializes result's error. */
		constexpr result(error_type &&err) noexcept(std::is_nothrow_move_constructible_v<error_type>) requires(!std::same_as<value_type, error_type> && std::move_constructible<error_type>) : result(in_place_error, std::forward<error_type>(err)) {}

		/** Direct-initializes result's value from passed arguments.
		 * @param args Arguments passed to the value type's constructor. */
		template<typename... Args> requires std::constructible_from<value_type, Args...>
		constexpr result(in_place_value_t, Args &&...args) noexcept(std::is_nothrow_constructible_v<value_type, Args...>) : _value(std::forward<Args>(args)...), _state(is_value) {}
		/** Direct-initializes result's value from passed initializer list and arguments.
		 * @param init Initializer list passed to the value type's constructor.
		 * @param args Arguments passed to the value type's constructor. */
		template<typename T, typename... Args> requires std::constructible_from<value_type, const std::initializer_list<T> &, Args...>
		constexpr result(in_place_value_t, std::initializer_list<T> init, Args &&...args) noexcept(std::is_nothrow_constructible_v<value_type, std::initializer_list<T>, Args...>) : _value(init, std::forward<Args>(args)...), _state(is_value) {}

		/** Direct-initializes result's error from passed arguments.
		 * @param args Arguments passed to the error type's constructor. */
		template<typename... Args> requires std::constructible_from<error_type, Args...>
		constexpr result(in_place_error_t, Args &&...args) noexcept(std::is_nothrow_constructible_v<error_type, Args...>) : _error(std::forward<Args>(args)...), _state(is_error) {}
		/** Direct-initializes result's error from passed initializer list and arguments.
		 * @param init Initializer list passed to the error type's constructor.
		 * @param args Arguments passed to the error type's constructor. */
		template<typename T, typename... Args> requires std::constructible_from<error_type, const std::initializer_list<T> &, Args...>
		constexpr result(in_place_error_t, std::initializer_list<T> init, Args &&...args) noexcept(std::is_nothrow_constructible_v<error_type, std::initializer_list<T>, Args...>) : _error(init, std::forward<Args>(args)...), _state(is_error) {}

		template<typename Val2, typename Err2> requires(!std::same_as<result, result<Val2, Err2>> && is_constructible<const Val2 &, const Err2 &>::value)
		constexpr result(const result<Val2, Err2> &other) noexcept(is_nothrow_constructible<const Val2 &, const Err2 &>::value) : _state(other._state)
		{
			if (has_value())
				std::construct_at(&_value, other._value);
			if (has_error())
				std::construct_at(&_error, other._error);
		}
		template<typename Val2, typename Err2> requires(!std::same_as<result, result<Val2, Err2>> && is_constructible<Val2, Err2>::value)
		constexpr result(result<Val2, Err2> &&other) noexcept(is_nothrow_constructible<Val2 &&, Err2 &&>::value) : _state(other._state)
		{
			if (has_value())
				std::construct_at(&_value, std::move(other._value));
			if (has_error())
				std::construct_at(&_error, std::move(other._error));
		}

		template<typename Val2, typename Err2> requires(!std::same_as<result, result<Val2, Err2>> && is_assignable<const Val2 &, const Err2 &>::value)
		constexpr result &operator=(const result<Val2, Err2> &other) noexcept(is_nothrow_assignable<const Val2 &, const Err2 &>::value)
		{
			if (_state != other._state)
				destroy();

			if (other.has_value())
			{
				if (!has_value())
					std::construct_at(&_value, other._value);
				else
					_value = other._value;
			}
			if (other.has_error())
			{
				if (!has_error())
					std::construct_at(&_error, other._error);
				else
					_error = other._error;
			}

			_state = other._state;
			return *this;
		}
		template<typename Val2, typename Err2> requires(!std::same_as<result, result<Val2, Err2>> && is_assignable<Val2, Err2>::value)
		constexpr result &operator=(result<Val2, Err2> &&other) noexcept(is_nothrow_move_assignable::value) requires is_move_assignable::value
		{
			if (_state != other._state)
				destroy();

			if (other.has_value())
			{
				if (!has_value())
					std::construct_at(&_value, std::move(other._value));
				else
					_value = std::move(other._value);
			}
			if (other.has_error())
			{
				if (!has_error())
					std::construct_at(&_error, std::move(other._error));
				else
					_error = std::move(other._error);
			}

			_state = other._state;
			return *this;
		}

		constexpr result(const result &other) noexcept(is_nothrow_copy_constructible::value) requires is_copy_constructible::value : _state(other._state)
		{
			if (has_value())
				std::construct_at(&_value, other._value);
			if (has_error())
				std::construct_at(&_error, other._error);
		}
		constexpr result(result &&other) noexcept(is_nothrow_move_constructible::value) requires is_move_constructible::value : _state(other._state)
		{
			if (has_value())
				std::construct_at(&_value, std::move(other._value));
			if (has_error())
				std::construct_at(&_error, std::move(other._error));
		}

		constexpr result &operator=(const result &other) noexcept(is_nothrow_copy_assignable::value) requires is_copy_assignable::value
		{
			if (_state != other._state)
				destroy();

			if (other.has_value())
			{
				if (!has_value())
					std::construct_at(&_value, other._value);
				else
					_value = other._value;
			}
			if (other.has_error())
			{
				if (!has_error())
					std::construct_at(&_error, other._error);
				else
					_error = other._error;
			}

			_state = other._state;
			return *this;
		}
		constexpr result &operator=(result &&other) noexcept(is_nothrow_move_assignable::value) requires is_move_assignable::value
		{
			if (_state != other._state)
				destroy();

			if (other.has_value())
			{
				if (!has_value())
					std::construct_at(&_value, std::move(other._value));
				else
					_value = std::move(other._value);
			}
			if (other.has_error())
			{
				if (!has_error())
					std::construct_at(&_error, std::move(other._error));
				else
					_error = std::move(other._error);
			}

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
		template<typename... Args> requires std::constructible_from<value_type, Args...>
		constexpr value_type &emplace_value(Args &&...args) noexcept(is_nothrow_destructible::value && std::is_nothrow_constructible_v<value_type, Args...>)
		{
			reset();
			std::construct_at(&_value, std::forward<Args>(args)...);
			_state = is_value;
			return _value;
		}
		/** Emplaces an error to the result.
		 * @param args Arguments passed to the error.
		 * @return Reference to the contained error object. */
		template<typename... Args> requires std::constructible_from<error_type, Args...>
		constexpr error_type &emplace_error(Args &&...args) noexcept(is_nothrow_destructible::value && std::is_nothrow_constructible_v<error_type, Args...>)
		{
			reset();
			std::construct_at(&_error, std::forward<Args>(args)...);
			_state = is_error;
			return _error;
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
		[[nodiscard]] constexpr auto *operator->() noexcept;
		/** @copydoc value */
		[[nodiscard]] constexpr auto *operator->() const noexcept;

		/** Returns reference to the value of the result.
		 * @note If the result does not contain a value results in undefined behavior. */
		[[nodiscard]] constexpr auto &operator*() & noexcept { return *operator->(); }
		/** @copydoc value */
		[[nodiscard]] constexpr auto &&operator*() && noexcept { return std::move(*operator->()); }
		/** @copydoc value */
		[[nodiscard]] constexpr auto &operator*() const & noexcept { return *operator->(); }
		/** @copydoc value */
		[[nodiscard]] constexpr auto &&operator*() const && noexcept { return std::move(*operator->()); }

		/** Returns reference to the contained value of the result.
		 * @throw std::system_error If the result does not contain a value and the error is same as `std::error_code`.
		 * @throw bad_result_access If the result does not contain a value and the error is not same as `std::error_code`. */
		[[nodiscard]] auto &value() & { return assert_value_state(_value); }
		/** @copydoc value */
		[[nodiscard]] auto &&value() && { return assert_value_state(std::move(_value)); }
		/** @copydoc value */
		[[nodiscard]] auto &value() const & { return assert_value_state(_value); }
		/** @copydoc value */
		[[nodiscard]] auto &&value() const && { return assert_value_state(std::move(_value)); }

		/** Returns the contained value or \a val if result does not contain a value. */
		template<typename Val2 = value_type> requires std::copy_constructible<Val> && std::constructible_from<value_type, Val2>
		[[nodiscard]] constexpr value_type value_or(Val2 &&val) const & noexcept(std::is_nothrow_copy_constructible_v<value_type> && std::is_nothrow_constructible_v<value_type, Val2>) { return has_value() ? _value : Val{std::forward<Val2>(val)}; }
		/** @copydoc value_or */
		template<typename Val2 = value_type> requires  std::move_constructible<Val> && std::constructible_from<value_type, Val2>
		[[nodiscard]] constexpr value_type value_or(Val2 &&val) && noexcept(std::is_nothrow_move_constructible_v<value_type> && std::is_nothrow_constructible_v<value_type, Val2>) { return has_value() ? std::move(_value) : Val{std::forward<Val2>(val)}; }

		/** Returns reference to the error of the result.
		 * @throw bad_result_access if the result does not contain an error. */
		[[nodiscard]] auto &error() & { return assert_error_state(_error); }
		/** @copydoc error */
		[[nodiscard]] auto &&error() && { return assert_error_state(std::move(_error)); }
		/** @copydoc error */
		[[nodiscard]] auto &error() const & { return assert_error_state(_error); }
		/** @copydoc error */
		[[nodiscard]] auto &&error() const && { return assert_error_state(std::move(_error)); }

		/** Returns the error state or \a err if result does not contain a value. */
		template<typename Err2 = error_type> requires std::copy_constructible<error_type> && std::constructible_from<error_type, Err2>
		[[nodiscard]] constexpr error_type error_or(Err2 &&err) const & noexcept(std::is_nothrow_copy_constructible_v<error_type> && std::is_nothrow_constructible_v<error_type, Err2>) { return has_error() ? _error : Err{std::forward<Err2>(err)}; }
		/** @copydoc error_or */
		template<typename Err2 = error_type> requires std::move_constructible<error_type> && std::constructible_from<error_type, Err2>
		[[nodiscard]] constexpr error_type error_or(Err2 &&err) && noexcept(std::is_nothrow_move_constructible_v<error_type> && std::is_nothrow_constructible_v<error_type, Err2>) { return has_error() ? std::move(_error) : Err{std::forward<Err2>(err)}; }

		template<typename Val2 = value_type, typename Err2 = error_type> requires std::equality_comparable_with<value_type, Val2> && std::equality_comparable_with<error_type, Err2>
		[[nodiscard]] friend constexpr bool operator==(const result &a, const result<Val2, Err2> &b) noexcept(noexcept(std::declval<error_type>() == std::declval<Val2>()) && noexcept(std::declval<error_type>() == std::declval<Err2>()))
		{
			if (a.has_value() && b.has_value())
				return a._value == b._value;
			else if (a.has_error() && b.has_error())
				return a._error == b._error;
			else
				return false;
		}

		constexpr void swap(result &other) noexcept(is_nothrow_swappable::value) requires is_swappable::value
		{
			using std::swap;
			if (has_value() && other.has_value())
				swap(_value, other._value);
			else if (has_error() && other.has_error())
				swap(_error, other._error);
			else
			{
				auto tmp = std::move(other);
				other = std::move(*this);
				*this = std::move(tmp);
			}
		}
		friend constexpr void swap(result &a, result &b) noexcept(is_nothrow_swappable::value) requires is_swappable::value { a.swap(b); }

	private:
		[[noreturn]] void throw_bad_access(auto msg, std::true_type) const
		{
			if (!has_error())
				throw_bad_access(msg, std::false_type{});
			else
				throw std::system_error(_error);
		}
		[[noreturn]] void throw_bad_access(auto msg, std::false_type) const
		{
			throw bad_result_access{msg};
		}
		template<typename T>
		[[nodiscard]] T &&assert_value_state(T &&fwd) const
		{
			if (!has_value()) [[unlikely]]
				throw_bad_access("bad result access (value)", std::is_same<error_type, std::error_code>{});
			else
				return std::forward<T>(fwd);
		}
		template<typename T>
		[[nodiscard]] T &&assert_error_state(T &&fwd) const
		{
			if (!has_error()) [[unlikely]]
				throw_bad_access("bad result access (error)", std::false_type{});
			else
				return std::forward<T>(fwd);
		}

		constexpr void destroy() noexcept(is_nothrow_destructible::value)
		{
			if (has_value())
				std::destroy_at(&_value);
			if (has_error())
				std::destroy_at(&_error);
		}

		union
		{
			value_type _value;
			error_type _error;
		};
		std::uint8_t _state;
	};

#ifndef NDEBUG /* Extra check to return nullptr in debug mode to make debugging easier */
	template<typename Val, typename Err>
	constexpr auto *result<Val, Err>::operator->() noexcept { return has_value() ? &_value : nullptr; }
	template<typename Val, typename Err>
	constexpr auto *result<Val, Err>::operator->() const noexcept { return has_value() ? &_value : nullptr; }
#else
	template<typename Val, typename Err>
	constexpr auto *result<Val, Err>::operator->() noexcept { return &_value; }
	template<typename Val, typename Err>
	constexpr auto *result<Val, Err>::operator->() const noexcept { return &_value; }
#endif
}

/*
 * Created by switchblade on 2023-06-26.
 */

#pragma once

#include <optional>
#include <new>

#include "utility.hpp"

namespace rod
{
	/** Exception thrown when an invalid state was requested from `rod::result`. */
	class ROD_API_PUBLIC result_state_error : public std::runtime_error
	{
	public:
		using std::runtime_error::runtime_error;
		using std::runtime_error::operator=;

		~result_state_error() override = default;
	};

	/** Trait used to specify the exception type thrown when a value is requested from `rod::result`, but only an error state is available. */
	template<typename Err>
	struct error_state_exception { using type = result_state_error; };
	/** Alias for `rod::error_state_exception<Err>::type`. */
	template<typename Err>
	using error_state_exception_t = typename error_state_exception<Err>::type;

	/** Specialization of `rod::error_state_exception` for `std::error_code` */
	template<>
	struct error_state_exception<std::error_code> { using type = std::system_error; };
	/** Specialization of `rod::error_state_exception` for `std::error_condition` */
	template<>
	struct error_state_exception<std::error_condition> { using type = std::system_error; };

	/** Tag used to select value-state constructor for `rod::result`. */
	struct in_place_value_t {};
	/** Tag used to select error-state constructor for `rod::result`. */
	struct in_place_error_t {};

	/** Instance of `rod::in_place_value_t`. */
	inline constexpr auto in_place_value = in_place_value_t{};
	/** Instance of `rod::in_place_error_t`. */
	inline constexpr auto in_place_error = in_place_error_t{};

	namespace detail
	{
		template<typename T>
		struct result_data;
		template<>
		struct result_data<void>
		{
			constexpr result_data() noexcept = default;
			constexpr explicit result_data(std::in_place_t) noexcept {}
		};
		template<typename T>
		struct result_data
		{
			constexpr result_data() noexcept {}
			constexpr ~result_data() noexcept {}

			template<typename... Args> requires std::constructible_from<T, Args...>
			constexpr explicit result_data(std::in_place_t, Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>) : data{std::forward<Args>(args)...} {}

			union { T data; };
		};

		enum result_flags : std::uint8_t
		{
			has_error = 1,
			has_value = 2,
		};
	}

	/** @brief Utility type used to return an optional value and/or error result.
	 *
	 * Result may be in one of the following states:
	 * <ul>
	 * <li>Error state - `result::has_value` evaluates to `false` and `result::has_error` evaluates to `true`; `result::error()` is well-formed if the corresponding type is not `void`.</li>
	 * <li>Value state - `result::has_value` evaluates to `true` and `result::has_error` evaluates to `false`; `result::value()` and `result::operator*()` are well-formed if the corresponding type is not `void`.</li>
	 * <li>Value + error state - `result::has_value` and `result::has_error` evaluate to `true`; `result::error()`, `result::value()`, and `result::operator*()` are well-formed if the corresponding type is not `void`.</li>
	 * </ul>
	 *
	 * @tparam Val Value type stored by the result.
	 * @tparam Err Error type stored by the result. */
	template<typename Val, typename Err>
	class result
	{
		template<typename, typename>
		friend
		class result;

		using value_data_t = detail::result_data<Val>;
		using error_data_t = detail::result_data<Err>;
		using flags_t = detail::result_flags;

		template<template<typename...> typename Trait, typename DefChoice, typename T, typename... Ts>
		using apply_trait = std::conditional_t<std::is_void_v<T>, DefChoice, detail::apply_tuple_t<Trait, T, Ts...>>;

		using is_swappable = std::conjunction<apply_trait<std::is_swappable, std::true_type, Val>, apply_trait<std::is_swappable, std::true_type, Err>>;
		using is_copy_constructible = std::conjunction<apply_trait<std::is_copy_constructible, std::true_type, Val>, apply_trait<std::is_copy_constructible, std::true_type, Err>>;
		using is_move_constructible = std::conjunction<apply_trait<std::is_move_constructible, std::true_type, Val>, apply_trait<std::is_move_constructible, std::true_type, Err>>;
		using is_copy_assignable = std::conjunction<is_copy_constructible, apply_trait<std::is_copy_assignable, std::true_type, Val>, apply_trait<std::is_copy_assignable, std::true_type, Err>>;
		using is_move_assignable = std::conjunction<is_copy_constructible, apply_trait<std::is_move_assignable, std::true_type, Val>, apply_trait<std::is_move_assignable, std::true_type, Err>>;

		using is_nothrow_swappable = std::conjunction<apply_trait<std::is_nothrow_swappable, std::true_type, Val>, apply_trait<std::is_nothrow_swappable, std::true_type, Err>>;
		using is_nothrow_copy_constructible = std::conjunction<apply_trait<std::is_nothrow_copy_constructible, std::true_type, Val>, apply_trait<std::is_nothrow_copy_constructible, std::true_type, Err>>;
		using is_nothrow_move_constructible = std::conjunction<apply_trait<std::is_nothrow_move_constructible, std::true_type, Val>, apply_trait<std::is_nothrow_move_constructible, std::true_type, Err>>;
		using is_nothrow_copy_assignable = std::conjunction<is_nothrow_copy_constructible, apply_trait<std::is_nothrow_copy_assignable, std::true_type, Val>, apply_trait<std::is_nothrow_copy_assignable, std::true_type, Err>>;
		using is_nothrow_move_assignable = std::conjunction<is_nothrow_copy_constructible, apply_trait<std::is_nothrow_move_assignable, std::true_type, Val>, apply_trait<std::is_nothrow_move_assignable, std::true_type, Err>>;

		using is_nothrow_destructible = std::conjunction<apply_trait<std::is_nothrow_destructible, std::true_type, Val>, apply_trait<std::is_nothrow_destructible, std::true_type, Err>>;

	public:
		/** Initializes result with empty (neither value nor error) state. */
		constexpr result() noexcept : _flags() {}

		/** Copy-constructs value state. */
		template<typename Val2> requires(!std::is_void_v<Val> && std::same_as<Val, Val2> && !std::same_as<Val, Err> && std::copy_constructible<Val>)
		constexpr result(const Val2 &val) noexcept(std::is_nothrow_copy_constructible_v<Val>) : result(in_place_value, val) {}
		/** Move-constructs value state. */
		template<typename Val2> requires(!std::is_void_v<Val> && std::same_as<Val, Val2> && !std::same_as<Val, Err> && std::move_constructible<Val>)
		constexpr result(Val2 &&val) noexcept(std::is_nothrow_move_constructible_v<Val>) : result(in_place_value, std::forward<Val>(val)) {}

		/** Copy-constructs error state. */
		template<typename Err2> requires(!std::is_void_v<Err> && std::same_as<Err, Err2> && !std::same_as<Val, Err> && std::copy_constructible<Err>)
		constexpr result(const Err2 &err) noexcept(std::is_nothrow_copy_constructible_v<Err>) : result(in_place_error, err) {}
		/** Move-constructs error state. */
		template<typename Err2> requires(!std::is_void_v<Err> && std::same_as<Err, Err2> && !std::same_as<Val, Err> && std::move_constructible<Err>)
		constexpr result(Err2 &&err) noexcept(std::is_nothrow_move_constructible_v<Err>) : result(in_place_error, std::forward<Err>(err)) {}

		/** Copy-constructs error & value states. */
		template<typename Val2, typename Err2> requires(!std::is_void_v<Val> && !std::is_void_v<Err> && std::same_as<Val, Val2> && std::same_as<Err, Err2> && !std::same_as<Val, Err> && std::copy_constructible<Val> && std::copy_constructible<Err>)
		constexpr result(const Val2 &val, const Err2 &err) noexcept(std::is_nothrow_copy_constructible_v<Val> && std::is_nothrow_copy_constructible_v<Err>)
		{
			emplace_value(val);
			emplace_error(err);
		}
		/** Move-constructs error & value states. */
		template<typename Val2, typename Err2> requires(!std::is_void_v<Val> && !std::is_void_v<Err> && std::same_as<Val, Val2> && std::same_as<Err, Err2> && !std::same_as<Val, Err> && std::move_constructible<Val> && std::move_constructible<Err>)
		constexpr result(Val2 &&val, Err2 &&err) noexcept(std::is_nothrow_move_constructible_v<Val> && std::is_nothrow_move_constructible_v<Err>)
		{
			emplace_value(std::forward<Val>(val));
			emplace_error(std::forward<Err>(err));
		}

		/** Initializes result with value state.
		 * @param args Arguments passed to the value. */
		template<typename... Args> requires std::constructible_from<value_data_t, std::in_place_t, Args...>
		constexpr result(in_place_value_t, Args &&...args) noexcept(std::is_nothrow_constructible_v<value_data_t, std::in_place_t, Args...>) : _value(std::in_place, std::forward<Args>(args)...), _flags{flags_t::has_value} {}
		/** Initializes result with error state.
		 * @param args Arguments passed to the error. */
		template<typename... Args> requires std::constructible_from<error_data_t, std::in_place_t, Args...>
		constexpr result(in_place_error_t, Args &&...args) noexcept(std::is_nothrow_constructible_v<error_data_t, std::in_place_t, Args...>) : _error(std::in_place, std::forward<Args>(args)...), _flags{flags_t::has_error} {}

		constexpr result(const result &other) noexcept(is_nothrow_copy_constructible::value) requires is_copy_constructible::value : _flags(other._flags)
		{
			/* Copy value. */
			if constexpr (!std::is_void_v<Val>)
				if (has_value()) new(&_value.data) value_data_t(std::in_place, other._value.data);
			/* Copy error. */
			if constexpr (!std::is_void_v<Err>)
				if (has_error()) new(&_error.data) error_data_t(std::in_place, other._error.data);
		}
		constexpr result(result &&other) noexcept(is_nothrow_move_constructible::value) requires is_move_constructible::value : _flags(std::exchange(other._flags, flags_t{}))
		{
			/* Move value. */
			if constexpr (!std::is_void_v<Val>)
				if (has_value()) new(&_value.data) value_data_t(std::in_place, std::move(other._value.data));
			/* Move error. */
			if constexpr (!std::is_void_v<Err>)
				if (has_error()) new(&_error.data) error_data_t(std::in_place, std::move(other._error.data));
		}

		constexpr result &operator=(const result &other) noexcept(is_nothrow_copy_assignable::value) requires is_copy_assignable::value
		{
			/* Copy value. */
			if constexpr (!std::is_void_v<Val>)
			{
				if (has_value() && other.has_value())
					_value.data = other._value.data;
				else if (other.has_value())
					new(&_value.data) value_data_t(std::in_place, other._value.data);
				else if (has_value())
					_value.data.~Val();
			}

			/* Copy error. */
			if constexpr (!std::is_void_v<Err>)
			{
				if (has_error() && other.has_error())
					_error.data = other._error.data;
				else if (other.has_error())
					new(&_error.data) value_data_t(std::in_place, other._error.data);
				else if (has_error())
					_error.data.~Err();
			}

			_flags = other._flags;
			return *this;
		}
		constexpr result &operator=(result &&other) noexcept(is_nothrow_move_assignable::value) requires is_move_assignable::value
		{
			/* Move value. */
			if constexpr (!std::is_void_v<Val>)
			{
				if (has_value() && other.has_value())
					_value.data = std::move(other._value.data);
				else if (other.has_value())
					new(&_value.data) value_data_t(std::in_place, std::move(other._value.data));
				else if (has_value())
					_value.data.~Val();
			}

			/* Move error. */
			if constexpr (!std::is_void_v<Err>)
			{
				if (has_error() && other.has_error())
					_error.data = std::move(other._error.data);
				else if (other.has_error())
					new(&_error.data) value_data_t(std::in_place, std::move(other._error.data));
				else if (has_error())
					_error.data.~Err();
			}

			_flags = std::exchange(other._flags, flags_t{});
			return *this;
		}

		constexpr ~result() noexcept(is_nothrow_destructible::value)
		{
			if constexpr (!std::is_void_v<Val>)
				if (has_value()) _value.data.~Val();
			if constexpr (!std::is_void_v<Err>)
				if (has_error()) _error.data.~Err();
		}

		/** Emplaces value state to the result.
		 * @param args Arguments passed to the value. */
		template<typename... Args> requires(apply_trait<std::is_constructible, std::bool_constant<sizeof...(Args) == 0>, Val, Args...>::value)
		constexpr void emplace_value(Args &&...args) noexcept(apply_trait<std::is_nothrow_constructible, std::true_type, Val, Args...>::value)
		{
			if constexpr (!std::is_void_v<Val>)
			{
				if (has_value()) _value.data.~Val();
				new (&_value.data) Val{std::forward<Args>(args)...};
			}
			_flags = static_cast<flags_t>(_flags | flags_t::has_value);
		}
		/** Emplaces error state to the result.
		 * @param args Arguments passed to the error. */
		template<typename... Args> requires(apply_trait<std::is_constructible, std::bool_constant<sizeof...(Args) == 0>, Err, Args...>::value)
		constexpr void emplace_error(Args &&...args) noexcept(apply_trait<std::is_nothrow_constructible, std::true_type, Err, Args...>::value)
		{
			if constexpr (!std::is_void_v<Err>)
			{
				if (_error.data) _error.data.~Err();
				new (&_error.data) Err{std::forward<Args>(args)...};
			}
			_flags = static_cast<flags_t>(_flags | flags_t::has_error);
		}

		/** Checks if the result has neither value nor error state. */
		[[nodiscard]] constexpr bool empty() const noexcept { return _flags == flags_t{}; }
		/** Checks if the result is not empty. */
		[[nodiscard]] constexpr explicit operator bool() const noexcept { return !empty(); }

		/** Checks if the result has value state. */
		[[nodiscard]] constexpr bool has_value() const noexcept { return _flags & flags_t::has_value; }
		/** Checks if the result has error state. */
		[[nodiscard]] constexpr bool has_error() const noexcept { return _flags & flags_t::has_error; }

		/** Returns reference to the value of the result.
		 * @note If the result is not in value state results in undefined behavior. */
		[[nodiscard]] constexpr auto &operator*() & noexcept requires(!std::same_as<Val, void>) { return _value.data; }
		/** @copydoc value */
		[[nodiscard]] constexpr auto &&operator*() && noexcept requires(!std::same_as<Val, void>) { return std::move(_value.data); }
		/** @copydoc value */
		[[nodiscard]] constexpr auto &operator*() const & noexcept requires(!std::same_as<Val, void>) { return _value.data; }
		/** @copydoc value */
		[[nodiscard]] constexpr auto &&operator*() const && noexcept requires(!std::same_as<Val, void>) { return std::move(_value.data); }

		/** Returns reference to the value of the result.
		 * @throw result_state_error if the result does not have either value or error state.
		 * @throw error_state_exception_t<Err> if the result does not have either value but has an error state. */
		[[nodiscard]] auto &value() & requires(!std::same_as<Val, void>) { return assert_value_state(_value.data); }
		/** @copydoc value */
		[[nodiscard]] auto &&value() && requires(!std::same_as<Val, void>) { return assert_value_state(std::move(_value.data)); }
		/** @copydoc value */
		[[nodiscard]] auto &value() const & requires(!std::same_as<Val, void>) { return assert_value_state(_value.data); }
		/** @copydoc value */
		[[nodiscard]] auto &&value() const && requires(!std::same_as<Val, void>) { return assert_value_state(std::move(_value.data)); }

		/** Returns the value state or \a val if result does not contain a value. */
		template<typename Val2> requires(!std::same_as<Val, void> && std::copy_constructible<Val> && std::constructible_from<Val, Val2>)
		[[nodiscard]] constexpr Val value_or(Val2 &&val) const & noexcept(std::is_nothrow_copy_constructible_v<Val> && std::is_nothrow_constructible_v<Val, Val2>) { return has_value() ? _value.data : Val{std::forward<Val2>(val)}; }
		/** @copydoc value_or */
		template<typename Val2> requires(!std::same_as<Val, void> && std::move_constructible<Val> && std::constructible_from<Val, Val2>)
		[[nodiscard]] constexpr Val value_or(Val2 &&val) && noexcept(std::is_nothrow_move_constructible_v<Val> && std::is_nothrow_constructible_v<Val, Val2>) { return has_value() ? std::move(_value.data) : Val{std::forward<Val2>(val)}; }

		/** Returns reference to the error of the result.
		 * @throw result_state_error if the result does not have error state. */
		[[nodiscard]] auto &error() & requires(!std::same_as<Err, void>) { return assert_error_state(_error.data); }
		/** @copydoc error */
		[[nodiscard]] auto &&error() && requires(!std::same_as<Err, void>) { return assert_error_state(std::move(_error.data)); }
		/** @copydoc error */
		[[nodiscard]] auto &error() const & requires(!std::same_as<Err, void>) { return assert_error_state(_error.data); }
		/** @copydoc error */
		[[nodiscard]] auto &&error() const && requires(!std::same_as<Err, void>) { return assert_error_state(std::move(_error.data)); }

		/** Returns the error state or \a err if result does not contain a value. */
		template<typename Err2> requires(!std::same_as<Err, void> && std::copy_constructible<Err> && std::constructible_from<Err, Err2>)
		[[nodiscard]] constexpr Err error_or(Err2 &&val) const & noexcept(std::is_nothrow_copy_constructible_v<Err> && std::is_nothrow_constructible_v<Err, Err2>) { return has_error() ? _error.data : Err{std::forward<Err2>(val)}; }
		/** @copydoc error_or */
		template<typename Err2> requires(!std::same_as<Err, void> && std::move_constructible<Err> && std::constructible_from<Err, Err2>)
		[[nodiscard]] constexpr Err error_or(Err2 &&val) && noexcept(std::is_nothrow_move_constructible_v<Err> && std::is_nothrow_constructible_v<Err, Err2>) { return has_error() ? std::move(_error.data) : Err{std::forward<Err2>(val)}; }

		constexpr void swap(result &other) noexcept(is_nothrow_swappable::value) requires is_swappable::value && is_move_constructible::value
		{
			using std::swap;
			if constexpr (!std::is_void_v<Val>)
			{
				if (has_value() && other.has_value())
					swap(_value.data, other._value.data);
				else if (other.has_value())
				{
					new(&_value.data) Val(std::move(other._value.data));
					other._value.data.~Val();
				}
				else if (has_value())
				{
					new(&other._value.data) Val(std::move(_value.data));
					_value.data.~Val();
				}
			}
			if constexpr (!std::is_void_v<Err>)
			{
				if (has_error() && other.has_error())
					swap(_error.data, other._error.data);
				else if (other.has_error())
				{
					new(&_error.data) Val(std::move(other._error.data));
					other._error.data.~Err();
				}
				else if (has_error())
				{
					new(&other._error.data) Val(std::move(_error.data));
					_error.data.~Err();
				}
			}
			swap(_flags, other._flags);
		}
		friend constexpr void swap(result &a, result &b) noexcept(is_nothrow_swappable::value) requires is_swappable::value && is_move_constructible::value { a.swap(b); }

	private:
		template<typename T>
		T &&assert_error_state(T &&fwd) const
		{
			if (!has_error()) [[unlikely]]
				throw result_state_error{"rod::result: requested invalid state (error)"};
			else
				return std::forward<T>(fwd);
		}
		template<typename T>
		T &&assert_value_state(T &&fwd) const
		{
			if (has_value()) [[likely]]
				return std::forward<T>(fwd);
			if (has_error())
			{
				if constexpr (std::same_as<error_state_exception_t<Err>, result_state_error>)
					throw error_state_exception_t<Err>{};
			}
			throw result_state_error{"rod::result: requested invalid state (value)"};
		}

		ROD_NO_UNIQUE_ADDRESS value_data_t _value;
		ROD_NO_UNIQUE_ADDRESS error_data_t _error;
		flags_t _flags = {};
	};
}

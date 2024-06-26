/*
 * Created by switchblade on 2023-08-31.
 */

#pragma once

#include <variant>
#include <chrono>

#include "utility.hpp"

namespace rod
{
	/** Structure used to describe either an absolute or relative timeout point.
	 * @tparam Clock Clock used for the timeout's time measurments. */
	template<typename Clock>
	class basic_timeout
	{
	public:
		using absolute_type = typename Clock::time_point;
		using relative_type = typename Clock::duration;

	private:
		using value_type = std::variant<std::monostate, relative_type, absolute_type>;

		constexpr explicit basic_timeout(value_type &&v) noexcept(std::is_nothrow_move_constructible_v<value_type>) : _value(std::forward<value_type>(v)) {}

	public:
		inline static const basic_timeout infinite = basic_timeout(value_type(std::monostate()));
		inline static const basic_timeout fallback = basic_timeout(std::chrono::seconds(60));
		inline static const basic_timeout max = basic_timeout(relative_type::max());
		inline static const basic_timeout min = basic_timeout(relative_type(0));

	public:
		/** Initializes a maximum relative timeout. */
		constexpr basic_timeout() noexcept : basic_timeout(infinite) {}
		/** Initializes an absolute timeout. */
		constexpr basic_timeout(const absolute_type &tp) noexcept(std::is_nothrow_constructible_v<value_type, const absolute_type &>) : basic_timeout(value_type(tp)) {}
		/** Initializes a relative timeout. */
		constexpr basic_timeout(const relative_type &dur) noexcept(std::is_nothrow_constructible_v<value_type, const relative_type &>) : basic_timeout(value_type(dur)) {}

		/** Initializes an absolute timeout from a system clock time point. */
		template<typename D = typename std::chrono::system_clock::duration> requires(!std::same_as<absolute_type, std::chrono::sys_time<D>> && std::constructible_from<absolute_type, std::chrono::sys_time<D>>)
		constexpr basic_timeout(std::chrono::sys_time<D> tp) noexcept(std::is_nothrow_constructible_v<absolute_type, std::chrono::sys_time<D>>) : _value(tp) {}
		/** Initializes a relative timeout from a generic duration. */
		template<typename R = typename std::chrono::system_clock::rep, typename P = typename std::chrono::system_clock::period> requires(!std::same_as<absolute_type, std::chrono::duration<R, P>> && std::constructible_from<relative_type, std::chrono::duration<R, P>>)
		constexpr basic_timeout(std::chrono::duration<R, P> dur) noexcept(std::is_nothrow_constructible_v<relative_type, std::chrono::duration<R, P>>) : _value(dur) {}

		constexpr basic_timeout(const basic_timeout &other) noexcept(std::is_nothrow_copy_constructible_v<absolute_type> && std::is_nothrow_copy_constructible_v<relative_type>) = default;
		constexpr basic_timeout(basic_timeout &&other) noexcept(std::is_nothrow_move_constructible_v<absolute_type> && std::is_nothrow_move_constructible_v<relative_type>) = default;

		constexpr basic_timeout &operator=(const basic_timeout &other) noexcept(std::is_nothrow_copy_assignable_v<absolute_type> && std::is_nothrow_copy_assignable_v<relative_type>) = default;
		constexpr basic_timeout &operator=(basic_timeout &&other) noexcept(std::is_nothrow_move_assignable_v<absolute_type> && std::is_nothrow_move_assignable_v<relative_type>) = default;

		constexpr ~basic_timeout() noexcept(std::is_nothrow_destructible_v<absolute_type> && std::is_nothrow_destructible_v<relative_type>) = default;

		/** Checks if the timeout is infinite. */
		[[nodiscard]] constexpr bool is_infinite() const noexcept { return _value.index() == 0; }
		/** Checks if the timeout is relative. */
		[[nodiscard]] constexpr bool is_relative() const noexcept { return _value.index() == 1; }
		/** Checks if the timeout is absolute. */
		[[nodiscard]] constexpr bool is_absolute() const noexcept { return _value.index() == 2; }

		/** If the timeout is absolute, converts it to relative difference using base time point \a base, otherwise returns the relative difference value. */
		[[nodiscard]] constexpr relative_type relative(absolute_type base = Clock::now()) const noexcept(noexcept(std::get<2>(_value) - base))
		{
			if (is_infinite())
				return relative_type::max();
			if (is_relative())
				return std::get<1>(_value);
			else
				return relative_type(std::get<2>(_value) - base);
		}
		/** If the timeout is relative, converts it to absolute time point using base time point \a base, otherwise returns the absolute time point value. */
		[[nodiscard]] constexpr absolute_type absolute(absolute_type base = Clock::now()) const noexcept(noexcept(base + std::get<1>(_value)))
		{
			if (is_infinite())
				return absolute_type::max();
			if (is_absolute())
				return std::get<2>(_value);
			else
				return absolute_type(base + std::get<1>(_value));
		}

		constexpr void swap(basic_timeout &other) noexcept(std::is_nothrow_swappable_v<absolute_type> && std::is_nothrow_swappable_v<relative_type>) { std::swap(_value, other._value); }
		friend constexpr void swap(basic_timeout &a, basic_timeout &b) noexcept(std::is_nothrow_swappable_v<absolute_type> && std::is_nothrow_swappable_v<relative_type>) { a.swap(b); }

		friend constexpr bool operator==(const basic_timeout &a, const basic_timeout &b) noexcept(noexcept(a._value == b._value)) = default;
		friend constexpr bool operator!=(const basic_timeout &a, const basic_timeout &b) noexcept(noexcept(a._value != b._value)) = default;

	private:
		std::variant<std::monostate, relative_type, absolute_type> _value = {};
	};

	/** Timeout type using `std::chrono::system_clock`. */
	using system_timeout = basic_timeout<std::chrono::system_clock>;
}

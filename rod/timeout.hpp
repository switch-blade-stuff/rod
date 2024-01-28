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

	public:
		/** Initializes a maximum relative timeout. */
		constexpr basic_timeout() noexcept(std::is_nothrow_default_constructible_v<relative_type>) : _value(relative_type::max()) {}
		/** Initializes an absolute timeout. */
		constexpr basic_timeout(const absolute_type &tp) noexcept(std::is_nothrow_copy_constructible_v<absolute_type>) : _value(tp) {}
		/** Initializes a relative timeout. */
		constexpr basic_timeout(const relative_type &dur) noexcept(std::is_nothrow_copy_constructible_v<relative_type>) : _value(dur) {}

		/** Initializes an absolute timeout from a system clock time point. */
		template<typename D = typename std::chrono::system_clock::duration> requires(!std::same_as<absolute_type, std::chrono::sys_time<D>> && std::constructible_from<absolute_type, std::chrono::sys_time<D> >)
		constexpr basic_timeout(std::chrono::sys_time<D> tp) noexcept(std::is_nothrow_constructible_v<absolute_type, std::chrono::sys_time<D>>) : _value(tp) {}
		/** Initializes a relative timeout from a generic duration. */
		template<typename R = typename std::chrono::system_clock::rep, typename P = typename std::chrono::system_clock::period> requires(!std::same_as<absolute_type, std::chrono::duration<R, P>> && std::constructible_from<relative_type, std::chrono::duration<R, P>>)
		constexpr basic_timeout(std::chrono::duration<R, P> dur) noexcept(std::is_nothrow_constructible_v<relative_type, std::chrono::duration<R, P>>) : _value(dur) {}

		constexpr basic_timeout(const basic_timeout &other) noexcept(std::is_nothrow_copy_constructible_v<absolute_type> && std::is_nothrow_copy_constructible_v<relative_type>) = default;
		constexpr basic_timeout(basic_timeout &&other) noexcept(std::is_nothrow_move_constructible_v<absolute_type> && std::is_nothrow_move_constructible_v<relative_type>) = default;

		constexpr basic_timeout &operator=(const basic_timeout &other) noexcept(std::is_nothrow_copy_assignable_v<absolute_type> && std::is_nothrow_copy_assignable_v<relative_type>) = default;
		constexpr basic_timeout &operator=(basic_timeout &&other) noexcept(std::is_nothrow_move_assignable_v<absolute_type> && std::is_nothrow_move_assignable_v<relative_type>) = default;

		constexpr ~basic_timeout() noexcept(std::is_nothrow_destructible_v<absolute_type> && std::is_nothrow_destructible_v<relative_type>) = default;

		/** Checks if the timeout is relative. */
		[[nodiscard]] constexpr bool is_relative() const noexcept { return _value.index() == 0; }
		/** If the timeout is relative, converts it to absolute time point using base time point \a tp, otherwise returns the absolute time point value. */
		[[nodiscard]] constexpr absolute_type absolute(absolute_type base = Clock::now()) const noexcept(noexcept(base + std::get<0>(_value))) { return absolute_type(_value.index() ? std::get<1>(_value) : base + std::get<0>(_value)); }
		/** If the timeout is absolute, converts it to relative difference using base time point \a tp, otherwise returns the relative difference value. */
		[[nodiscard]] constexpr relative_type relative(absolute_type base = Clock::now()) const noexcept(noexcept(std::get<1>(_value) - base)) { return relative_type(_value.index() ? std::get<1>(_value) - base : std::get<0>(_value)); }

		constexpr void swap(basic_timeout &other) noexcept(std::is_nothrow_swappable_v<absolute_type> && std::is_nothrow_swappable_v<relative_type>) { std::swap(_value, other._value); }
		friend constexpr void swap(basic_timeout &a, basic_timeout &b) noexcept(std::is_nothrow_swappable_v<absolute_type> && std::is_nothrow_swappable_v<relative_type>) { a.swap(b); }

		friend constexpr bool operator==(const basic_timeout &a, const basic_timeout &b) noexcept(noexcept(a._value == b._value)) = default;
		friend constexpr bool operator!=(const basic_timeout &a, const basic_timeout &b) noexcept(noexcept(a._value != b._value)) = default;

	private:
		std::variant<relative_type, absolute_type> _value;
	};

	/** Timeout type using `std::chrono::system_clock`. */
	using system_timeout = basic_timeout<std::chrono::system_clock>;
}

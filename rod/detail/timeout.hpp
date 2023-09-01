/*
 * Created by switch_blade on 2023-08-31.
 */

#pragma once

#include "file_clock.hpp"
#include "../utility.hpp"

namespace rod
{
	/** Structure used to describe either an absolute or relative timeout point.
	 * @tparam Clock Clock used for the timeout's time measurments. */
	template<typename Clock>
	struct basic_timeout
	{
		using absolute_type = typename Clock::time_point;
		using relative_type = typename Clock::duration;

		/** Initializes a maximum relative timeout. */
		constexpr basic_timeout() noexcept(std::is_nothrow_default_constructible_v<relative_type>) : relative(relative_type::max()), is_relative(true) {}
		/** Initializes an absolute timeout. */
		constexpr basic_timeout(const absolute_type &tp) noexcept(std::is_nothrow_copy_constructible_v<absolute_type>) : absolute(tp), is_relative(false) {}
		/** Initializes a relative timeout. */
		constexpr basic_timeout(const relative_type &dur) noexcept(std::is_nothrow_copy_constructible_v<relative_type>) : absolute(dur), is_relative(true) {}

		/** Initializes an absolute timeout from a system clock time point. */
		template<typename D = typename std::chrono::system_clock::duration> requires(!std::same_as<absolute_type, std::chrono::sys_time<D>> && std::constructible_from<absolute_type, std::chrono::sys_time<D> >)
		constexpr basic_timeout(std::chrono::sys_time<D> tp) noexcept(std::is_nothrow_constructible_v<absolute_type, std::chrono::sys_time<D>>) : absolute(tp), is_relative(false) {}
		/** Initializes a relative timeout from a generic duration. */
		template<typename R = typename std::chrono::system_clock::rep, typename P = typename std::chrono::system_clock::period> requires(!std::same_as<absolute_type, std::chrono::duration<R, P>> && std::constructible_from<relative_type, std::chrono::duration<R, P>>)
		constexpr basic_timeout(std::chrono::duration<R, P> dur) noexcept(std::is_nothrow_constructible_v<relative_type, std::chrono::duration<R, P>>) : absolute(dur), is_relative(true) {}

		constexpr basic_timeout(const basic_timeout &other) noexcept(std::is_nothrow_copy_constructible_v<absolute_type> && std::is_nothrow_copy_constructible_v<relative_type>)
		{
			if (other.is_relative)
				new (&relative) relative_type(other.relative);
			else
				new (&absolute) absolute_type(other.absolute);
		}
		constexpr basic_timeout(basic_timeout &&other) noexcept(std::is_nothrow_move_constructible_v<absolute_type> && std::is_nothrow_move_constructible_v<relative_type>)
		{
			if (other.is_relative)
				new (&relative) relative_type(std::move(other.relative));
			else
				new (&absolute) absolute_type(std::move(other.absolute));
		}
		constexpr basic_timeout &operator=(const basic_timeout &other) noexcept(std::is_nothrow_copy_assignable_v<absolute_type> && std::is_nothrow_copy_assignable_v<relative_type>)
		{
			if (is_relative != other.is_relative)
			{
				this->~basic_timeout();
				new (this) basic_timeout(other);
			}
			else if (other.is_relative)
				relative = other.relative;
			else
				absolute = other.absolute;
			return *this;
		}
		constexpr basic_timeout &operator=(basic_timeout &&other) noexcept(std::is_nothrow_move_assignable_v<absolute_type> && std::is_nothrow_move_assignable_v<relative_type>)
		{
			if (is_relative != other.is_relative)
			{
				this->~basic_timeout();
				new (this) basic_timeout(std::move(other));
			}
			else if (other.is_relative)
				relative = std::move(other.relative);
			else
				absolute = std::move(other.absolute);
			return *this;
		}
		constexpr ~basic_timeout() noexcept(std::is_nothrow_destructible_v<absolute_type> && std::is_nothrow_destructible_v<relative_type>)
		{
			if (is_relative)
				relative.~relative_type();
			else
				absolute.~absolute_type();
		}

		constexpr void swap(basic_timeout &other) noexcept(std::is_nothrow_swappable_v<absolute_type> && std::is_nothrow_swappable_v<relative_type>) { swap(*this, other); }
		friend constexpr void swap(basic_timeout &a, basic_timeout &b) noexcept(std::is_nothrow_swappable_v<absolute_type> && std::is_nothrow_swappable_v<relative_type>)
		{
			if (a.is_relative != b.is_relative)
			{
				auto tmp_a = std::move(a);
				auto tmp_b = std::move(b);
				a.~basic_tiomeout();
				new (&a) basic_timeout(std::move(tmp_b));
				b.~basic_tiomeout();
				new (&b) basic_timeout(std::move(tmp_a));
			}
			else if (a.is_relative)
				adl_swap(a.relative, b.relative);
			else
				adl_swap(a.absolute, b.absolute);
		}

		union
		{
			absolute_type absolute;
			relative_type relative;
		};
		bool is_relative;
	};

	/** Timeout type using `std::chrono::system_clock`. */
	using system_timeout = basic_timeout<std::chrono::system_clock>;
	/** Timeout type using `file_clock`. */
	using file_timeout = basic_timeout<file_clock>;
}

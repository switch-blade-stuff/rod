/*
 * Created by switchblade on 2023-08-31.
 */

#pragma once

#include <chrono>

#include "../utility.hpp"

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
		constexpr basic_timeout() noexcept(std::is_nothrow_default_constructible_v<relative_type>) : _relative(relative_type::max()), _is_relative(true) {}
		/** Initializes an absolute timeout. */
		constexpr basic_timeout(const absolute_type &tp) noexcept(std::is_nothrow_copy_constructible_v<absolute_type>) : _absolute(tp), _is_relative(false) {}
		/** Initializes a relative timeout. */
		constexpr basic_timeout(const relative_type &dur) noexcept(std::is_nothrow_copy_constructible_v<relative_type>) : _absolute(dur), _is_relative(true) {}

		/** Initializes an absolute timeout from a system clock time point. */
		template<typename D = typename std::chrono::system_clock::duration> requires(!std::same_as<absolute_type, std::chrono::sys_time<D>> && std::constructible_from<absolute_type, std::chrono::sys_time<D> >)
		constexpr basic_timeout(std::chrono::sys_time<D> tp) noexcept(std::is_nothrow_constructible_v<absolute_type, std::chrono::sys_time<D>>) : _absolute(tp), _is_relative(false) {}
		/** Initializes a relative timeout from a generic duration. */
		template<typename R = typename std::chrono::system_clock::rep, typename P = typename std::chrono::system_clock::period> requires(!std::same_as<absolute_type, std::chrono::duration<R, P>> && std::constructible_from<relative_type, std::chrono::duration<R, P>>)
		constexpr basic_timeout(std::chrono::duration<R, P> dur) noexcept(std::is_nothrow_constructible_v<relative_type, std::chrono::duration<R, P>>) : _absolute(dur), _is_relative(true) {}

		constexpr basic_timeout(const basic_timeout &other) noexcept(std::is_nothrow_copy_constructible_v<absolute_type> && std::is_nothrow_copy_constructible_v<relative_type>)
		{
			if (other._is_relative)
				new (&_relative) relative_type(other._relative);
			else
				new (&_absolute) absolute_type(other._absolute);
		}
		constexpr basic_timeout(basic_timeout &&other) noexcept(std::is_nothrow_move_constructible_v<absolute_type> && std::is_nothrow_move_constructible_v<relative_type>)
		{
			if (other._is_relative)
				new (&_relative) relative_type(std::move(other._relative));
			else
				new (&_absolute) absolute_type(std::move(other._absolute));
		}
		constexpr basic_timeout &operator=(const basic_timeout &other) noexcept(std::is_nothrow_copy_assignable_v<absolute_type> && std::is_nothrow_copy_assignable_v<relative_type>)
		{
			if (_is_relative != other._is_relative)
			{
				this->~basic_timeout();
				new (this) basic_timeout(other);
			}
			else if (other._is_relative)
				_relative = other._relative;
			else
				_absolute = other._absolute;
			return *this;
		}
		constexpr basic_timeout &operator=(basic_timeout &&other) noexcept(std::is_nothrow_move_assignable_v<absolute_type> && std::is_nothrow_move_assignable_v<relative_type>)
		{
			if (_is_relative != other._is_relative)
			{
				this->~basic_timeout();
				new (this) basic_timeout(std::move(other));
			}
			else if (other._is_relative)
				_relative = std::move(other._relative);
			else
				_absolute = std::move(other._absolute);
			return *this;
		}
		constexpr ~basic_timeout() noexcept(std::is_nothrow_destructible_v<absolute_type> && std::is_nothrow_destructible_v<relative_type>)
		{
			if (_is_relative)
			{
				if constexpr (!std::is_trivially_destructible_v<relative_type>)
					_relative.~relative_type();
			}
			else
			{
				if constexpr (!std::is_trivially_destructible_v<absolute_type>)
					_absolute.~absolute_type();
			}
		}

		/** Checks if the timeout is relative. */
		[[nodiscard]] constexpr bool is_relative() const noexcept { return _is_relative; }
		/** If the timeout is relative, converts it to absolute time point using base time point \a tp, otherwise returns the absolute time point value. */
		[[nodiscard]] constexpr absolute_type absolute(absolute_type base = Clock::now()) const noexcept(noexcept(base + _relative)) { return absolute_type(_is_relative ? base + _relative : _absolute); }
		/** If the timeout is absolute, converts it to relative difference using base time point \a tp, otherwise returns the relative difference value. */
		[[nodiscard]] constexpr relative_type relative(absolute_type base = Clock::now()) const noexcept(noexcept(_absolute - base)) { return relative_type(_is_relative ? _relative : _absolute - base); }

		constexpr void swap(basic_timeout &other) noexcept(std::is_nothrow_swappable_v<absolute_type> && std::is_nothrow_swappable_v<relative_type>)
		{
			if (_is_relative != other._is_relative)
			{
				auto tmp_this = std::move(*this), tmp_other = std::move(other);
				this->~basic_tiomeout();
				new (this) basic_timeout(std::move(tmp_other));
				other.~basic_tiomeout();
				new (&other) basic_timeout(std::move(tmp_this));
			}
			else if (_is_relative)
				adl_swap(_relative, other._relative);
			else
				adl_swap(_absolute, other._absolute);
		}
		friend constexpr void swap(basic_timeout &a, basic_timeout &b) noexcept(std::is_nothrow_swappable_v<absolute_type> && std::is_nothrow_swappable_v<relative_type>) { a.swap(b); }

		friend constexpr bool operator==(const basic_timeout &a, const basic_timeout &b) noexcept(noexcept(a._absolute == b._absolute) && noexcept(a._relative == b._relative))
		{
			if (a._is_relative == b._is_relative)
			{
				if (a._is_relative)
					return a._relative == b._relative;
				else
					return a._absolute == b._absolute;
			}
			return false;
		}
		friend constexpr bool operator!=(const basic_timeout &a, const basic_timeout &b) noexcept(noexcept(a._absolute != b._absolute) && noexcept(a._relative != b._relative))
		{
			if (a._is_relative == b._is_relative)
			{
				if (a._is_relative)
					return a._relative != b._relative;
				else
					return a._absolute != b._absolute;
			}
			return true;
		}

	private:
		union
		{
			absolute_type _absolute;
			relative_type _relative;
		};
		bool _is_relative;
	};

	/** Timeout type using `std::chrono::system_clock`. */
	using system_timeout = basic_timeout<std::chrono::system_clock>;
}

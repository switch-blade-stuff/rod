/*
 * Created by switchblade on 2023-05-21.
 */

#pragma once

#ifdef __unix__

#include <chrono>
#include <ctime>

#include "../detail/config.hpp"

namespace rod
{
	/** Clock type representing the `CLOCK_MONOTONIC` clock of the `clock_gettime` syscall. */
	struct monotonic_clock
	{
		using rep = std::int64_t;
		using period = std::ratio<1, 10'000'000>;
		using duration = std::chrono::duration<rep, period>;

		class time_point
		{
		public:
			using duration = typename monotonic_clock::duration;

			static constexpr time_point max() noexcept { return {std::numeric_limits<std::int64_t>::max(), 999'999'999}; }
			static constexpr time_point min() noexcept { return {std::numeric_limits<std::int64_t>::min(), -999'999'999}; }

		public:
			constexpr time_point() noexcept = default;
			constexpr time_point(std::int64_t sec, long long nsec) noexcept : _sec(sec), _nsec(nsec) { normalize(); }

			template<typename Clock, typename Dur>
			constexpr time_point(const std::chrono::time_point<Clock, Dur> &other) noexcept
			{
				const auto dur = other.time_since_epoch();
				const auto sec = std::chrono::duration_cast<std::chrono::seconds>(dur);
				const auto rem = std::chrono::duration_cast<std::chrono::nanoseconds>(dur - sec);

				_sec = sec.count();
				_nsec = rem.count();
				normalize();
			}
			template<typename Clock, typename Dur>
			[[nodiscard]] constexpr operator std::chrono::time_point<Clock, Dur>() const noexcept
			{
				return Clock::now() + time_since_epoch();
			}

			[[nodiscard]] constexpr std::int64_t seconds() const noexcept { return _sec; }
			[[nodiscard]] constexpr long long nanoseconds() const noexcept { return _nsec; }

			[[nodiscard]] constexpr duration time_since_epoch() const noexcept
			{
				const auto sec = std::duration_cast<duration>(std::chrono::seconds{_sec});
				const auto nsec = std::duration_cast<duration>(std::chrono::nanoseconds{_nsec});
				return sec + nsec;
			}

			template<typename Rep, typename Ratio>
			constexpr time_point &operator+=(const std::chrono::duration<Rep, Ratio> &d) noexcept
			{
				const auto sec = std::chrono::duration_cast<std::chrono::seconds>(d);
				const auto rem = std::chrono::duration_cast<std::chrono::nanoseconds>(d - sec);

				_sec += sec.count();
				_nsec += rem.count();
				return normalize();
			}
			template<typename Rep, typename Ratio>
			constexpr time_point &operator-=(const std::chrono::duration<Rep, Ratio> &d) noexcept
			{
				const auto sec = std::chrono::duration_cast<std::chrono::seconds>(d);
				const auto rem = std::chrono::duration_cast<std::chrono::nanoseconds>(d - sec);

				_sec -= sec.count();
				_nsec -= rem.count();
				return normalize();
			}

			friend constexpr duration operator-(const time_point &a, const time_point &b) noexcept { return duration{(a._sec - b._sec) * 10'000'000 + (a._nsec - b._nsec) / 100}; }
			template<typename Rep, typename Period>
			friend constexpr time_point operator+(const time_point &a, std::chrono::duration<Rep, Period> b) noexcept
			{
				auto result = a;
				result += b;
				return result;
			}
			template<typename Rep, typename Period>
			friend constexpr time_point operator-(const time_point &a, std::chrono::duration<Rep, Period> b) noexcept
			{
				auto result = a;
				result -= b;
				return result;
			}

			friend constexpr bool operator==(const time_point &a, const time_point &b) noexcept
			{
				return a._sec == b._sec && a._nsec == b._nsec;
			}
			friend constexpr auto operator<=>(const time_point &a, const time_point &b) noexcept
			{
				if (a._sec < b._sec || (a._sec == b._sec && a._nsec < b._nsec))
					return std::weak_ordering::less;
				else if (a._sec > b._sec || (a._sec == b._sec && a._nsec > b._nsec))
					return std::weak_ordering::greater;
				else
					return std::weak_ordering::equivalent;
			}

		private:
			constexpr time_point &normalize() noexcept
			{
				constexpr std::int64_t scale = 1'000'000'000;
				const auto overflow = _nsec / scale;

				_sec += overflow;
				_nsec -= overflow * scale;
				if (_sec < 0 && _nsec > 0)
				{
					_sec += 1;
					_nsec -= scale;
				}
				else if (_sec > 0 && _nsec < 0)
				{
					_sec -= 1;
					_nsec += scale;
				}
				return *this;
			}

			std::int64_t _sec = {};
			long long _nsec = {};
		};

		static constexpr bool is_steady = true;

		static ROD_API_PUBLIC time_point now() noexcept;
	};
}
#endif

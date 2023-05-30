/*
 * Created by switchblade on 2023-05-21.
 */

#pragma once

#ifdef __unix__

#include <chrono>

#include "../detail/config.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
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
			constexpr time_point(std::int64_t sec, long long nsec) noexcept : m_sec(sec), m_nsec(nsec) { normalize(); }

			[[nodiscard]] constexpr std::int64_t seconds() const noexcept { return m_sec; }
			[[nodiscard]] constexpr long long nanoseconds() const noexcept { return m_nsec; }

			template<typename Rep, typename Ratio>
			constexpr time_point &operator+=(const std::chrono::duration<Rep, Ratio> &d) noexcept
			{
				const auto sec = std::chrono::duration_cast<std::chrono::seconds>(d);
				const auto rem = std::chrono::duration_cast<std::chrono::nanoseconds>(d - sec);

				m_sec += sec.count();
				m_nsec += rem.count();
				return normalize();
			}
			template<typename Rep, typename Ratio>
			constexpr time_point &operator-=(const std::chrono::duration<Rep, Ratio> &d) noexcept
			{
				const auto sec = std::chrono::duration_cast<std::chrono::seconds>(d);
				const auto rem = std::chrono::duration_cast<std::chrono::nanoseconds>(d - sec);

				m_sec -= sec.count();
				m_nsec -= rem.count();
				return normalize();
			}

			friend constexpr duration operator-(const time_point &a, const time_point &b) noexcept { return duration{(a.m_sec - b.m_sec) * 10'000'000 + (a.m_nsec - b.m_nsec) / 100}; }
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
				return a.m_sec == b.m_sec && a.m_nsec == b.m_nsec;
			}
			friend constexpr auto operator<=>(const time_point &a, const time_point &b) noexcept
			{
				if (a.m_sec < b.m_sec || (a.m_sec == b.m_sec && a.m_nsec < b.m_nsec))
					return std::weak_ordering::less;
				else if (a.m_sec > b.m_sec || (a.m_sec == b.m_sec && a.m_nsec > b.m_nsec))
					return std::weak_ordering::greater;
				else
					return std::weak_ordering::equivalent;
			}

		private:
			constexpr time_point &normalize() noexcept
			{
				constexpr std::int64_t scale = 1'000'000'000;
				const auto overflow = m_nsec / scale;

				m_sec += overflow;
				m_nsec -= overflow * scale;
				if (m_sec < 0 && m_nsec > 0)
				{
					m_sec += 1;
					m_nsec -= scale;
				}
				else if (m_sec > 0 && m_nsec < 0)
				{
					m_sec -= 1;
					m_nsec += scale;
				}
				return *this;
			}

			std::int64_t m_sec = {};
			long long m_nsec = {};
		};

		static constexpr bool is_steady = true;

		static ROD_PUBLIC time_point now() noexcept;
	};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

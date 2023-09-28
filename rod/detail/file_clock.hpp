/*
 * Created by switch_blade on 2023-07-15.
 */

#pragma once

#include "timeout.hpp"

namespace rod::fs
{
#ifdef ROD_WIN32
	/** System clock used for filesystem IO operations, such as Win32 `FILETIME` or POSIX `timespec`.
	 * @note `time_point` is guaranteed to be convertible to and from `system_clock::time_point`. */
	struct file_clock
	{
		using duration = std::chrono::duration<std::int64_t , std::ratio<1, 10'000'000>>;
		using period = typename duration::period;
		using rep = typename duration::rep;

		static constexpr bool is_steady = false;

		class time_point
		{
		public:
			using clock = file_clock;
			using duration = typename clock::duration;
			using period = typename duration::period;
			using rep = typename duration::rep;

			static constexpr time_point max() noexcept { return {std::numeric_limits<rep>::max()}; }
			static constexpr time_point min() noexcept { return {std::numeric_limits<rep>::min()}; }

		private:
			static constexpr auto win32_unix_diff = duration(period::den * 11'644'473'600);

			[[nodiscard]] static constexpr duration unix_to_win32(duration dur) noexcept { return dur + win32_unix_diff; }
			[[nodiscard]] static constexpr duration win32_to_unix(duration dur) noexcept { return dur - win32_unix_diff; }

		public:
			constexpr time_point() noexcept : _ticks() {}
			constexpr time_point(rep ticks) noexcept : _ticks(ticks) {}

			template<typename Dur = typename std::chrono::system_clock::duration>
			constexpr time_point(const std::chrono::sys_time<Dur> &other) noexcept : time_point(std::chrono::duration_cast<duration>(unix_to_win32(other.time_since_epoch())).count()) {}
			template<typename Dur = typename std::chrono::system_clock::duration>
			[[nodiscard]] constexpr operator std::chrono::sys_time<Dur>() const noexcept { return std::chrono::duration_cast<Dur>(win32_to_unix(time_since_epoch())); }

			[[nodiscard]] constexpr duration time_since_epoch() const noexcept { return duration(_ticks); }

			template<typename Rep, typename Ratio>
			constexpr time_point &operator+=(const std::chrono::duration<Rep, Ratio> &d) noexcept { return (_ticks += std::chrono::duration_cast<duration>(d).count(), *this); }
			template<typename Rep, typename Ratio>
			constexpr time_point &operator-=(const std::chrono::duration<Rep, Ratio> &d) noexcept { return (_ticks -= std::chrono::duration_cast<duration>(d).count(), *this); }

			friend constexpr duration operator-(const time_point &a, const time_point &b) noexcept { return duration{a._ticks - b._ticks}; }
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

			friend constexpr bool operator==(const time_point &a, const time_point &b) noexcept { return a._ticks == b._ticks; }
			friend constexpr auto operator<=>(const time_point &a, const time_point &b) noexcept { return a._ticks <=> b._ticks; }

		private:
			std::int64_t _ticks;
		};

		static ROD_API_PUBLIC time_point now() noexcept;
	};
#else
	/** System clock used for filesystem IO operations, such as Win32 `FILETIME` or POSIX `timespec`.
	 * @note `time_point` is guaranteed to be convertible to and from `system_clock::time_point`. */
	using file_clock = std::chrono::system_clock;
#endif

	/** Timeout type using `file_clock`. */
	using file_timeout = basic_timeout<file_clock>;
}

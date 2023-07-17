/*
 * Created by switch_blade on 2023-07-15.
 */

#pragma once

#include "../detail/config.hpp"

#ifdef ROD_WIN32

#include <chrono>

#include "../file.hpp"

namespace rod
{
	/** Clock type representing the system filetime clock. */
	struct filetime_clock
	{
		using rep = std::int64_t;
		using period = std::ratio<1, 10'000'000>;
		using duration = std::chrono::duration<rep, period>;

		class time_point
		{
		public:
			using duration = typename filetime_clock::duration;

			static constexpr time_point max() noexcept { return {std::numeric_limits<rep>::max()}; }
			static constexpr time_point min() noexcept { return {std::numeric_limits<rep>::min()}; }

		public:
			constexpr time_point() noexcept : _ticks() {}
			constexpr time_point(rep ticks) noexcept : _ticks(ticks) {}
			constexpr time_point(std::uint32_t lw, std::uint32_t hw) noexcept : _lw(lw), _hw(hw) {}

			template<typename Clock, typename Dur>
			constexpr time_point(const std::chrono::time_point<Clock, Dur> &other) noexcept : time_point(std::chrono::duration_cast<duration>(other.time_since_epoch()).count()) {}
			template<typename Clock, typename Dur>
			[[nodiscard]] constexpr operator std::chrono::time_point<Clock, Dur>() const noexcept { return Clock::now() + time_since_epoch(); }

			[[nodiscard]] constexpr duration time_since_epoch() const noexcept { return duration{_ticks}; }

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
			union
			{
				struct
				{
					std::uint32_t _hw;
					std::uint32_t _lw;
				};

				std::int64_t _ticks;
			};
		};

		static constexpr bool is_steady = true;

		static ROD_API_PUBLIC time_point now() noexcept;

		/** Returns the file creation time for the specified file handle. */
		static time_point create_time(const basic_file &file) noexcept { return create_time(file.native_handle()); }
		/** @copydoc create_time */
		static ROD_API_PUBLIC time_point create_time(typename basic_file::native_handle_type handle) noexcept;

		/** Returns the file access time for the specified file handle. */
		static time_point access_time(const basic_file &file) noexcept { return create_time(file.native_handle()); }
		/** @copydoc create_time */
		static ROD_API_PUBLIC time_point access_time(typename basic_file::native_handle_type handle) noexcept;

		/** Returns the file modification time for the specified file handle. */
		static time_point modify_time(const basic_file &file) noexcept { return create_time(file.native_handle()); }
		/** @copydoc create_time */
		static ROD_API_PUBLIC time_point modify_time(typename basic_file::native_handle_type handle) noexcept;
	};
}
#endif

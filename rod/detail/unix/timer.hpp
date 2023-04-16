/*
 * Created by switchblade on 2023-04-10.
 */

#pragma once

#ifdef __unix__

#include <optional>
#include <memory>
#include <chrono>

#include <csignal>
#include <ctime>

#include "../utility.hpp"

namespace rod::detail
{
	template<typename, typename>
	class platform_timer;
	template<typename Clock>
	class platform_timer<Clock, void>
	{
		constexpr static clockid_t native_clock_id() noexcept
		{
			/* Use boottime clock for linux steady_clock, as CLOCK_MONOTONIC does not count time spent in suspension. */
			if constexpr (std::same_as<Clock, std::chrono::steady_clock>)
			{
#ifdef __linux__
				return CLOCK_BOOTTIME;
#else
				return CLOCK_MONOTONIC;
#endif
			}

			/* Use realtime for all other clocks. */
			return CLOCK_REALTIME;
		}

	public:
		using native_handle_type = timer_t;

	public:
		constexpr platform_timer() noexcept = default;
		~platform_timer() { if (m_handle) timer_delete(*m_handle); }

		template<typename D>
		void start(std::chrono::time_point<Clock, D> tp, void (*invoke)(void *))
		{
		}
		template<typename R, typename P>
		void start(std::chrono::duration<R, P> dur, void (*invoke)(void *))
		{
		}

		[[nodiscard]] constexpr native_handle_type native_handle() const noexcept { return m_handle.value_or(native_handle_type{}); }

	private:
		bool init(std::error_code &err) noexcept
		{
			native_handle_type handle;
			sigevent evt = {
					.
			};

			if (timer_create(native_clock_id(), &evt, &handle) < 0) [[unlikely]]
			{
				err = std::make_error_code(static_cast<std::errc>(errno));
				return false;
			}

			m_handle = handle;
			return true;
		}

		std::optional<native_handle_type> m_handle;
	};
}

#endif

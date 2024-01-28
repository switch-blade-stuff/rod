/*
 * Created by switch_blade on 2023-12-01.
 */

#pragma once

#include <thread>
#include <atomic>

namespace rod::_detail
{
	struct tid_lock
	{
		tid_lock(tid_lock &&) = delete;
		tid_lock(const tid_lock &) = delete;

		constexpr tid_lock() noexcept = default;

		void lock() noexcept
		{
			for (;;)
			{
				auto null_tid = std::thread::id();
				if (!tid.compare_exchange_strong(null_tid, std::this_thread::get_id(), std::memory_order_acq_rel))
					tid.wait(null_tid, std::memory_order_acquire);
				else
					break;
			}
		}
		void unlock() noexcept
		{
			tid.store(std::thread::id(), std::memory_order_release);
			tid.notify_one();
		}
		bool try_lock() noexcept
		{
			auto null_tid = std::thread::id();
			return tid.compare_exchange_strong(null_tid, std::this_thread::get_id(), std::memory_order_acq_rel);
		}

		std::atomic<std::thread::id> tid;
	};
}
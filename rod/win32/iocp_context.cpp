/*
 * Created by switch_blade on 2023-07-04.
 */

#include "iocp_context.hpp"

#ifdef ROD_WIN32

#include <cassert>
#include <ranges>
#include <array>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

/* The following (undocumented) NTDLL functions are used:
 *  NtSetIoCompletion(iocp, key, apc, status, info)
 *  NtRemoveIoCompletionEx(iocp, buff, size, removed, timeout, alert)
 *
 * PostQueuedCompletionStatus and GetQueuedCompletionStatusEx are not used
 * because they do not allow for passing extra data alongside the completion
 * event. NTDLL functions have an APC context argument which we use in order
 * to differentiate between IO completion events and "dummy" wakeup events. */

namespace rod::_iocp
{
	[[noreturn]] inline void throw_status(auto status, const char *msg) { throw std::system_error{static_cast<int>(ntapi::instance.RtlNtStatusToDosError(status)), std::system_category(), msg}; }
	[[noreturn]] inline void throw_last_error(const char *msg) { throw std::system_error{static_cast<int>(::GetLastError()), std::system_category(), msg}; }

	int io_entry_t::get_state() const noexcept
	{
		if (err_pos > started)
			return -1;
		else if (!waiting)
			return 1;

		for (std::size_t i = 0; i < started; ++i)
		{
			/* Use volatile here to prevent compiler from messing up the order. */
			volatile auto &iosb = batch[i];
			if (iosb.status == STATUS_PENDING)
				return 0;
		}

		/* Use atomic fence to synchronize the volatile iosb above.
		 * Cannot directly use atomic iosb since it is not guaranteed
		 * to be lock-free and win32 kernel will for sure not respect
		 * any user-space locks. */
		std::atomic_thread_fence(std::memory_order_acquire);
		return 1;
	}
	int io_entry_t::get_error() const noexcept
	{
		if (err_pos > started)
			return 0;

		volatile const auto &status = batch[err_pos].status;
		auto err = ntapi::instance.RtlNtStatusToDosError(status);
		std::atomic_thread_fence(std::memory_order_acquire);
		return static_cast<int>(err);
	}

	void io_operation_base::cancel_io()
	{
		for (std::uint16_t i = entry->started; i-- != 0;)
		{
			ntapi::io_status_block iosb, *iosb_ptr = &entry->batch[i];
			if (!ntapi::instance.NtCancelIoFileEx(handle, iosb_ptr, &iosb))
				throw_last_error("NtCancelIoFileEx");
		}
	}
	result<std::size_t, std::error_code> io_operation_base::io_result() const noexcept
	{
		std::size_t result = 0;
		for (std::size_t i = 0; i < entry->started; ++i)
		{
			if (const auto &iosb = entry->batch[i]; iosb.status < 0) [[unlikely]]
				return std::error_code{static_cast<int>(ntapi::instance.RtlNtStatusToDosError(iosb.status)), std::system_category()};
			else
				result += iosb.info;
		}
		return result;
	}
	result<std::size_t, std::error_code> io_operation_base::batch_read(std::size_t pos, std::span<std::byte> buff) noexcept
	{
		ntapi::large_integer offset;
		std::size_t n_scheduled = 0;
		ntapi::ulong chunk;

		for (; entry->started < io_entry_t::max_size && n_scheduled < buff.size(); n_scheduled += chunk)
		{
			auto &iosb = entry->batch[entry->started++];
			iosb.status = STATUS_PENDING;
			iosb.info = 0;

			chunk = static_cast<ntapi::ulong>(buff.size() - n_scheduled);
			offset.quad = static_cast<ntapi::longlong>(pos);

			if (const auto status = ntapi::instance.NtReadFile(handle, nullptr, nullptr, reinterpret_cast<ntapi::ulong_ptr>(&iosb), &iosb, buff.data() + n_scheduled, chunk, &offset, nullptr); status < 0) [[unlikely]]
				return std::error_code{static_cast<int>(ntapi::instance.RtlNtStatusToDosError(status)), std::system_category()};
			else
				entry->waiting += (status == STATUS_PENDING || notify_func != nullptr);
		}
		return n_scheduled;
	}
	result<std::size_t, std::error_code> io_operation_base::batch_write(std::size_t pos, std::span<std::byte> buff) noexcept
	{
		ntapi::large_integer offset;
		std::size_t n_scheduled = 0;
		ntapi::ulong chunk;

		for (; entry->started < io_entry_t::max_size && n_scheduled < buff.size(); n_scheduled += chunk)
		{
			auto &iosb = entry->batch[entry->started++];
			iosb.status = STATUS_PENDING;
			iosb.info = 0;

			chunk = static_cast<ntapi::ulong>(buff.size() - n_scheduled);
			offset.quad = static_cast<ntapi::longlong>(pos);

			if (const auto status = ntapi::instance.NtWriteFile(handle, nullptr, nullptr, reinterpret_cast<ntapi::ulong_ptr>(&iosb), &iosb, buff.data() + n_scheduled, chunk, &offset, nullptr); status < 0) [[unlikely]]
				return std::error_code{static_cast<int>(ntapi::instance.RtlNtStatusToDosError(status)), std::system_category()};
			else
				entry->waiting += (status == STATUS_PENDING || notify_func != nullptr);
		}
		return n_scheduled;
	}

	std::error_code file_handle::open(const char *path, int mode, int prot) noexcept
	{
		if (auto err = _file::system_handle::open(path, mode, prot | _file::system_handle::overlapped); err) [[unlikely]]
			return err;
		if (auto err = _ctx->port_bind(_file::system_handle::native_handle()); err) [[unlikely]]
			return err;
		return {};
	}
	std::error_code file_handle::open(const wchar_t *path, int mode, int prot) noexcept
	{
		if (auto err = _file::system_handle::open(path, mode, prot | _file::system_handle::overlapped); err) [[unlikely]]
			return err;
		if (auto err = _ctx->port_bind(_file::system_handle::native_handle()); err) [[unlikely]]
			return err;
		return {};
	}

	context::context(std::size_t entries)
	{
		if (auto hnd = ::CreateIoCompletionPort(nullptr, INVALID_HANDLE_VALUE, 0, 1); !hnd)
			throw_last_error("CreateIoCompletionPort");
		else
			_iocp.release(hnd);

		if (auto hnd = ::CreateWaitableTimer(nullptr, false, nullptr); !hnd)
			throw_last_error("CreateWaitableTimer");
		else
			_thread_timer.release(hnd);

		_io_entry_buff.resize(entries);
		for (auto &entry : _io_entry_buff)
			_io_entry_pool.push_back(&entry);
	}
	context::~context()
	{
		std::size_t waiting_ops = _io_entry_buff.size();
		for (; !_io_entry_pool.empty(); --waiting_ops)
			_io_entry_pool.pop_front();

#ifdef _WIN64
		constexpr std::size_t buff_size = 8192 / sizeof(ntapi::io_completion_info);
#else
		constexpr std::size_t buff_size = 4096 / sizeof(ntapi::io_completion_info);
#endif

		while (waiting_ops != 0)
		{
			std::array<ntapi::io_completion_info, buff_size> buff;
			ntapi::ulong received;

			if (const auto status = ntapi::instance.NtRemoveIoCompletionEx(_iocp.native_handle(), buff.data(), buff_size, &received, nullptr, false); status < 0)
				throw_status(status, "NtRemoveIoCompletionEx");

			for (auto &event: std::ranges::take_view(buff, received))
				if (event.apc_context != 0)
				{
					auto entry = find_entry(reinterpret_cast<ntapi::io_status_block *>(event.apc_context));
					if (--entry.first->waiting == 0)
						--waiting_ops;
				}
		}
	}

	void context::schedule_producer(operation_base *node)
	{
		assert(!node->next);
		if (!_producer_queue.push(node))
			return;

		/* Notify a sleeping consumer thread via a dummy completion event. */
		if (const auto status = ntapi::instance.NtSetIoCompletion(_iocp.native_handle(), 0, 0, 0, 0); status < 0)
			throw_status(status, "NtSetIoCompletion");
	}
	void context::schedule_consumer(operation_base *node)
	{
		assert(!node->next);
		_consumer_queue.push_back(node);
	}

	void context::add_timer(timer_operation_base *node) noexcept
	{
		assert(!node->timer_next);
		assert(!node->timer_prev);
		/* Process pending timers if the inserted timer is the new front. */
		_timer_pending |= _timer_queue.insert(node) == node;
	}
	void context::del_timer(timer_operation_base *node) noexcept
	{
		/* Process pending timers if we are erasing the front. */
		_timer_pending |= _timer_queue.front() == node;
		_timer_queue.erase(node);
	}

	/* Win32 timers do not work with IOCP, so use a waitable timer instead. */
	inline void context::timeout_handler() noexcept
	{
		_timer_started = false;
		_timer_pending = true;
	}
	inline void context::set_timer(time_point tp)
	{
		const auto callback = [](void *ptr, auto...) { static_cast<context *>(ptr)->timeout_handler(); };
		LARGE_INTEGER timeout = {.QuadPart = tp.time_since_epoch().count()};

		if (!::SetWaitableTimer(_thread_timer.native_handle(), &timeout, 0, callback, this, false))
			throw_last_error("SetWaitableTimer");
		else
			_timer_started = true;
	}

	void context::request_stop() { _stop_src.request_stop(); }
	bool context::is_consumer_thread() const noexcept { return _consumer_tid.load(std::memory_order_acquire) == std::this_thread::get_id(); }

	void context::release_io_entry(io_entry_t *entry) noexcept
	{
		if (entry->waiting)
			entry->parent = {};
		else if (_waiting_queue.empty())
			_io_entry_pool.push_front(entry);
		else
		{
			/* Reuse the entry for other operations. This avoids needless shuffling of entry nodes. */
			const auto node = static_cast<io_operation_base *>(_waiting_queue.pop_front());
			schedule_consumer(std::construct_at(entry, node)->parent);
		}
	}
	bool context::acquire_io_entry(io_operation_base *node) noexcept
	{
		if (_io_entry_pool.empty()) [[unlikely]]
			return false;

		std::construct_at(_io_entry_pool.pop_front(), node);
		return true;
	}
	void context::schedule_waiting(io_operation_base *node) noexcept
	{
		assert(!node->entry);
		assert(!node->next);
		_waiting_queue.push_back(node);
	}
	void context::schedule_pending(io_operation_base *node) noexcept
	{
		assert(node->entry);
		assert(!node->next);
		if (!node->entry->waiting)
			_consumer_queue.push_front(node);
		else
			_pending_queue.push_back(node);
	}
	inline void context::schedule_complete(std::pair<io_entry_t *, std::size_t> entry)
	{
		if (entry.first->batch[entry.second].status < 0 && entry.first->err_pos > entry.second)
			entry.first->err_pos = entry.second;
		if (--entry.first->waiting > 0)
			return;

		if (!entry.first->parent)
			release_io_entry(entry.first);
		else if (!entry.first->parent_notified)
		{
			entry.first->parent_notified = true;
			schedule_consumer(entry.first->parent);
		}
	}

	inline void context::acquire_consumer_queue() noexcept
	{
		while (!_pending_queue.empty())
		{
			const auto node = static_cast<io_operation_base *>(_pending_queue.pop_front());
			const auto entry = node->entry;

			if (entry->get_state())
			{
				entry->parent_notified = true;
				schedule_consumer(entry->parent);
			}
		}
	}
	inline bool context::acquire_producer_queue() noexcept
	{
		if (!_producer_queue.try_terminate())
		{
			_consumer_queue.merge_back(std::move(_producer_queue));
			return false;
		}
		return true;
	}
	inline void context::acquire_elapsed_timers()
	{
		if (!_timer_pending)
			return;

		if (!_timer_queue.empty())
			for (const auto now = clock::now(); !_timer_queue.empty() && _timer_queue.front()->timeout <= now;)
			{
				const auto node = _timer_queue.pop_front();

				/* Handle timer cancellation. */
				if (node->stop_possible)
				{
					node->dispatched.test_and_set(std::memory_order_release);
					if (node->stop_requested.test(std::memory_order_acquire))
						continue;
				}
				schedule_consumer(node);
			}

		/* Disarm or start the timer. */
		if (_timer_queue.empty())
		{
			if (_timer_started && ::CancelWaitableTimer(_thread_timer.native_handle()))
			{
				_timer_started = false;
				_timer_pending = false;
			}
			return;
		}

		const auto next_timeout = _timer_queue.front()->timeout;
		if (_next_timeout < next_timeout)
		{
			_next_timeout = next_timeout;
			set_timer(next_timeout);
		}
		_timer_pending = false;
	}

	inline std::error_code context::port_bind(void *hnd)
	{
		if (::CreateIoCompletionPort(hnd, _iocp.native_handle(), 0, 0) == nullptr) [[unlikely]]
			return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
		if (!::SetFileCompletionNotificationModes(hnd, FILE_SKIP_COMPLETION_PORT_ON_SUCCESS | FILE_SKIP_SET_EVENT_ON_HANDLE)) [[unlikely]]
			return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
		return {};
	}
	inline void context::port_wait()
	{
		static constinit ntapi::large_integer zero_timeout = {};
#ifdef _WIN64
		constexpr std::size_t buff_size = 8192 / sizeof(ntapi::io_completion_info);
#else
		constexpr std::size_t buff_size = 4096 / sizeof(ntapi::io_completion_info);
#endif

		auto timeout = !_consumer_queue.empty() ? &zero_timeout : nullptr;
		std::array<ntapi::io_completion_info, buff_size> buff;
		ntapi::ulong received;

		if (const auto status = ntapi::instance.NtRemoveIoCompletionEx(_iocp.native_handle(), buff.data(), buff_size, &received, timeout, false); status < 0)
			throw_status(status, "NtRemoveIoCompletionEx");

		for (auto &event : std::ranges::take_view(buff, received))
		{
			if (event.apc_context != 0)
				schedule_complete(find_entry(reinterpret_cast<ntapi::io_status_block *>(event.apc_context)));
			else
				_wait_pending = false;
		}
	}

	void context::run()
	{
		while (!_stop_pending)
		{
			for (auto queue = std::move(_consumer_queue); !queue.empty();)
				queue.pop_front()->notify();
			if (std::exchange(_stop_pending, false)) [[unlikely]]
				return;

			acquire_consumer_queue();
			acquire_elapsed_timers();

			/* Acquire pending operations from the producer queue & wait for IOCP events. */
			if (_wait_pending || (_wait_pending = acquire_producer_queue()))
				port_wait();
		}
	}
}
#endif
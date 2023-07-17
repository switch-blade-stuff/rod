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
#if SIZE_MAX >= UINT64_MAX
	constexpr std::size_t default_entries = 256;
#else
	constexpr std::size_t default_entries = 128;
#endif

	[[noreturn]] inline void throw_status(auto status, const char *msg) { throw std::system_error{static_cast<int>(ntapi::instance.RtlNtStatusToDosError(status)), std::system_category(), msg}; }
	[[noreturn]] inline void throw_last_error(const char *msg) { throw std::system_error{static_cast<int>(::GetLastError()), std::system_category(), msg}; }

	bool io_entry::get_state() const noexcept
	{
		if (pending != 0)
		{
			for (std::size_t i = 0; i < started; ++i)
			{
				/* Use volatile here to prevent compiler from messing up the order. */
				const volatile auto &iosb = batch[i];
				if (iosb.status == STATUS_PENDING)
					return false;
			}

			/* Use atomic fence to synchronize the volatile iosb above.
			 * Cannot directly use atomic iosb since it is not guaranteed
			 * to be lock-free and win32 kernel will for sure not respect
			 * any user-space locks. */
			std::atomic_thread_fence(std::memory_order_acquire);
		}
		return true;
	}
	void io_operation_base::cancel_io(void *hnd)
	{
		for (std::uint16_t i = entry->started; i-- != 0;)
		{
			ntapi::io_status_block iosb, *iosb_ptr = &entry->batch[i];
			if (!ntapi::instance.NtCancelIoFileEx(hnd, iosb_ptr, &iosb))
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
	std::size_t io_operation_base::start_io(io_cmd<async_read_some_at_t, std::span<std::byte>> cmd)
	{
		ntapi::large_integer offset;
		std::size_t batch_bytes = 0;
		ntapi::ulong chunk;

		entry->parent_notified = false;
		entry->started = 0;
		for (; entry->started < io_entry::max_size && batch_bytes < cmd.buff.size(); batch_bytes += chunk, ++entry->started)
		{
			auto &iosb = entry->batch[entry->started];
			iosb.status = STATUS_PENDING;
			iosb.info = 0;

			chunk = static_cast<ntapi::ulong>(cmd.buff.size() - batch_bytes);
			offset.quad = static_cast<ntapi::longlong>(cmd.pos);

			if (const auto status = ntapi::instance.NtReadFile(cmd.hnd, nullptr, nullptr, reinterpret_cast<ntapi::ulong_ptr>(&iosb), &iosb, cmd.buff.data() + batch_bytes, chunk, &offset, nullptr); status >= 0)
				entry->pending += (status == STATUS_PENDING || notify_func != nullptr);
			else
			{
				cancel_io(cmd.hnd);
				throw_status(status, "NtReadFile");
			}
		}
		return batch_bytes;
	}
	std::size_t io_operation_base::start_io(io_cmd<async_write_some_at_t, std::span<std::byte>> cmd)
	{
		ntapi::large_integer offset;
		std::size_t batch_bytes = 0;
		ntapi::ulong chunk;

		entry->parent_notified = false;
		entry->started = 0;
		for (; entry->started < io_entry::max_size && batch_bytes < cmd.buff.size(); batch_bytes += chunk, ++entry->started)
		{
			auto &iosb = entry->batch[entry->started];
			iosb.status = STATUS_PENDING;
			iosb.info = 0;

			chunk = static_cast<ntapi::ulong>(cmd.buff.size() - batch_bytes);
			offset.quad = static_cast<ntapi::longlong>(cmd.pos);

			if (const auto status = ntapi::instance.NtWriteFile(cmd.hnd, nullptr, nullptr, reinterpret_cast<ntapi::ulong_ptr>(&iosb), &iosb, cmd.buff.data() + batch_bytes, chunk, &offset, nullptr); status >= 0)
				entry->pending += (status == STATUS_PENDING || notify_func != nullptr);
			else
			{
				cancel_io(cmd.hnd);
				throw_status(status, "NtWriteFile");
			}
		}
		return batch_bytes;
	}

	std::error_code file_handle::open(const char *path, int mode, int prot) noexcept
	{
		if (auto err = _file::system_handle::open(path, mode | _file::system_handle::overlapped, prot); err) [[unlikely]]
			return err;
		if (auto err = _ctx->port_bind(_file::system_handle::native_handle()); err) [[unlikely]]
			return err;
		return {};
	}
	std::error_code file_handle::open(const wchar_t *path, int mode, int prot) noexcept
	{
		if (auto err = _file::system_handle::open(path, mode | _file::system_handle::overlapped, prot); err) [[unlikely]]
			return err;
		if (auto err = _ctx->port_bind(_file::system_handle::native_handle()); err) [[unlikely]]
			return err;
		return {};
	}

	context::context() : context(default_entries) {}
	context::context(std::size_t max_entries)
	{
		if (auto hnd = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1); !hnd)
			throw_last_error("CreateIoCompletionPort");
		else
			_iocp.release(hnd);

		if (auto hnd = ::CreateWaitableTimer(nullptr, false, nullptr); !hnd)
			throw_last_error("CreateWaitableTimer");
		else
			_thread_timer.release(hnd);

		_io_entry_buff.resize(max_entries);
		for (auto &entry : _io_entry_buff)
			_io_entry_pool.push_back(&entry);
	}
	context::~context()
	{
		std::size_t pending_ops = _io_entry_buff.size();
		for (; !_io_entry_pool.empty(); --pending_ops)
			_io_entry_pool.pop_front();

#ifdef _WIN64
		constexpr std::size_t buff_size = 8192 / sizeof(ntapi::io_completion_info);
#else
		constexpr std::size_t buff_size = 4096 / sizeof(ntapi::io_completion_info);
#endif

		while (pending_ops != 0)
		{
			std::array<ntapi::io_completion_info, buff_size> buff;
			ntapi::large_integer zero_timeout = {.quad = 0};
			ntapi::ulong received;

			if (const auto status = ntapi::instance.NtRemoveIoCompletionEx(_iocp.native_handle(), buff.data(), static_cast<ntapi::ulong>(std::min(buff_size, pending_ops)), &received, &zero_timeout, false); status < 0)
				throw_status(status, "NtRemoveIoCompletionEx");

			for (auto &event: std::ranges::take_view(buff, received))
				if (event.apc_context != 0)
				{
					auto entry = find_entry(event.apc_context);
					if (--entry.first->pending == 0)
						--pending_ops;
				}
		}
	}

	void context::schedule_producer(operation_base *node)
	{
		assert(!node->next);
		if (!_producer_queue.push(node))
			return;

		/* Notify a sleeping consumer thread via a dummy completion event. */
		if (const auto status = ntapi::instance.NtSetIoCompletion(_iocp.native_handle(), 0, event_id::wakeup, 0, 0); status < 0)
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
		/* Notify a sleeping consumer thread via a dummy completion event. */
		if (const auto status = ntapi::instance.NtSetIoCompletion(_iocp.native_handle(), 0, event_id::timeout, 0, 0); status < 0)
			throw_status(status, "NtSetIoCompletion");
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

	void context::release_io_entry(io_entry *entry) noexcept
	{
		if (entry->pending)
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
		if (!node->entry->pending)
			_consumer_queue.push_front(node);
		else
			_pending_queue.push_back(node);
	}

	inline void context::schedule_complete(std::pair<io_entry *, std::uint8_t> entry) noexcept
	{
		if (entry.first->batch[entry.second].status < 0 && entry.first->err_pos > entry.second)
			entry.first->err_pos = entry.second;
		if (--entry.first->pending > 0)
			return;

		if (!entry.first->parent)
			release_io_entry(entry.first);
		else if (!std::exchange(entry.first->parent_notified, true))
			schedule_consumer(entry.first->parent);
	}
	inline std::pair<io_entry *, std::uint8_t> context::find_entry(ntapi::ulong_ptr apc) noexcept
	{
		const auto diff = apc - reinterpret_cast<ntapi::ulong_ptr>(_io_entry_buff.data());
		const auto iosb = reinterpret_cast<ntapi::io_status_block *>(apc);
		const auto ptr = _io_entry_buff.data() + diff / sizeof(io_entry);
		const auto idx = static_cast<std::uint8_t>(iosb - ptr->batch);
		return {ptr, idx};
	}

	inline void context::acquire_pending_events() noexcept
	{
		while (!_pending_queue.empty())
		{
			const auto node = static_cast<io_operation_base *>(_pending_queue.pop_front());
			const auto entry = node->entry;

			if (entry->get_state())
			{
				schedule_consumer(entry->parent);
				entry->parent_notified = true;
			}
		}
	}
	inline bool context::acquire_producer_queue() noexcept
	{
		if (_producer_queue.empty())
			return false;

		_consumer_queue.merge_back(std::move(_producer_queue));
		return true;
	}
	inline void context::acquire_elapsed_timers()
	{
		if (!_timer_pending)
			return;

		if (!_timer_queue.empty())
			for (const auto now = clock::now(); !_timer_queue.empty() && _timer_queue.front()->tp <= now;)
			{
				const auto node = _timer_queue.pop_front();

				/* Handle timer cancellation. */
				if (!(node->flags.fetch_or(flags_t::dispatched, std::memory_order_acq_rel) & flags_t::stop_requested))
					schedule_consumer(node);
			}

		/* Disarm or start the timer. */
		if (_timer_queue.empty())
		{
			if (_timer_started)
			{
				::CancelWaitableTimer(_thread_timer.native_handle());
				_timer_started = false;
				_timer_pending = false;
			}
			return;
		}

		if (const auto next_timeout = _timer_queue.front()->tp; !_timer_started || _next_timeout >= next_timeout)
		{
			set_timer(next_timeout);
			_next_timeout = next_timeout;
		}
		_timer_pending = false;
	}

	inline std::error_code context::port_bind(void *hnd)
	{
		if (::CreateIoCompletionPort(hnd, _iocp.native_handle(), 0, 0) != _iocp.native_handle()) [[unlikely]]
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

		auto timeout = _producer_queue.active() ? &zero_timeout : nullptr;
		std::array<ntapi::io_completion_info, buff_size> buff;
		ntapi::ulong received;

		if (const auto status = ntapi::instance.NtRemoveIoCompletionEx(_iocp.native_handle(), buff.data(), buff_size, &received, timeout, _timer_started); status < 0)
			throw_status(status, "NtRemoveIoCompletionEx");
		else if (status == STATUS_TIMEOUT)
			_producer_queue.try_terminate();
		else if (status != STATUS_USER_APC && status != 0x101 /*STATUS_ALERTED*/)
		{
			for (auto &event: std::ranges::take_view(buff, received))
				switch (event.apc_context)
				{
				default:
					schedule_complete(find_entry(event.apc_context));
					break;
				case event_id::timeout:
					_timer_started = false;
					_timer_pending = true;
					[[fallthrough]];
				case event_id::wakeup:
					_producer_queue.try_activate();
					break;
				}
		}
	}

	void context::run()
	{
		/* Make sure only one thread is allowed to run at a given time. */
		if (std::thread::id id = {}; !_consumer_tid.compare_exchange_strong(id, std::this_thread::get_id(), std::memory_order_acq_rel))
			throw std::system_error(std::make_error_code(std::errc::device_or_resource_busy), "Only one thread may invoke `iocp_context::run` at a given time");

		const auto g = defer_invoke([&]()
		{
			_producer_queue.try_activate();
			_consumer_tid.store({}, std::memory_order_release);
		});

		while (!_stop_pending)
		{
			for (auto queue = std::move(_consumer_queue); !queue.empty();)
				queue.pop_front()->notify();
			if (std::exchange(_stop_pending, false)) [[unlikely]]
				return;

			acquire_elapsed_timers();
			acquire_pending_events();

			/* Acquire pending operations from the producer queue & wait for IOCP events. */
			if (!_producer_queue.active() || !acquire_producer_queue())
				port_wait();
		}
	}
	void context::finish()
	{
		/* Notification function will be reset on dispatch, so set it here instead of the constructor. */
		notify_func = [](operation_base *ptr) noexcept { static_cast<context *>(ptr)->_stop_pending = true; };
		schedule(this);
	}
}
#endif
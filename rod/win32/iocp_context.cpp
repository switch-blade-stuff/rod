/*
 * Created by switchblade on 2023-07-04.
 */

#include "../detail/win32/ntapi.hpp"
#include "iocp_context.hpp"

#include <cassert>
#include <ranges>

/* The following (undocumented) NTDLL functions are used:
 *  NtSetIoCompletion(iocp, key, apc, ntstatus, info)
 *  NtRemoveIoCompletionEx(iocp, buff, size, removed, timeout, alert)
 *
 * PostQueuedCompletionStatus and GetQueuedCompletionStatusEx are not used
 * because they do not allow for passing extra data alongside the completion
 * event. NTDLL functions have an APC context argument which we use in order
 * to differentiate between IO completion events and "dummy" wakeup events. */

namespace rod::_iocp
{
	using namespace _win32;

	result<> io_operation_base::cancel_events() noexcept
	{
		const auto &ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		std::error_code error;
		for (auto event = event_queue.front(); event; event = event->next)
		{
			const auto status = ntapi->cancel_io(handle, reinterpret_cast<io_status_block *>(event->iosb));
			if (is_status_failure(status) && status != 0xc0000120 /*STATUS_CANCELLED*/ && !error) [[unlikely]]
				error = status_error_code(status);
		}

		if (error) [[unlikely]]
			return error;
		else
			return {};
	}
	result<> io_operation_base::release_events() noexcept
	{
		while (!event_queue.empty())
		{
			const auto event = event_queue.pop_front();
			ctx->release_io_event(event);
		}
		return {};
	}

	context::context() : context(event_buffer_size) {}
	context::context(std::size_t buff_size)
	{
		if (auto hnd = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1); !hnd) [[unlikely]]
			_detail::throw_error_code(_win32::dos_error_code(::GetLastError()), "CreateIoCompletionPort");
		else
			_iocp.release(hnd);

		if (auto hnd = ::CreateWaitableTimer(nullptr, false, nullptr); !hnd) [[unlikely]]
			_detail::throw_error_code(_win32::dos_error_code(::GetLastError()), "CreateWaitableTimer");
		else
			_thread_timer.release(hnd);

		_io_event_buff = make_malloc_ptr_for_overwrite<io_event[]>(buff_size);
		if (_io_event_buff.get() == nullptr) [[unlikely]]
			_detail::throw_error_code(std::errc::not_enough_memory);
		else
			_active.test_and_set(std::memory_order_relaxed);
	}

	void context::schedule_producer(operation_base *node) noexcept
	{
		if (!_producer_queue.push(node))
			return;

		/* Notify a sleeping consumer thread via a dummy completion event. ntapi should already be initialized if context constructor was successful. */
		const auto status = ntapi::instance().value().NtSetIoCompletion(_iocp.native_handle(), 0, event_id::wakeup, 0, 0);
		if (is_status_failure(status)) [[unlikely]]
			_detail::throw_error_code(status_error_code(status), "NtSetIoCompletion");
	}
	void context::schedule_consumer(operation_base *node) noexcept
	{
		assert(_consumer_lock.tid.load() == std::this_thread::get_id());
		_consumer_queue.push_back(node);
	}
	void context::schedule_waitlist(operation_base *node) noexcept
	{
		assert(_consumer_lock.tid.load() == std::this_thread::get_id());
		_waitlist_queue.push_back(node);
	}

	void context::add_timer(timer_operation_base *node) noexcept
	{
		if (node->to == fs::file_timeout()) [[unlikely]]
			return;

		/* Process pending timers if the inserted timer is the new front. */
		_timer_pending |= _timer_queue.insert(node) == node;
	}
	void context::del_timer(timer_operation_base *node) noexcept
	{
		if (node->to == fs::file_timeout()) [[unlikely]]
			return;

		/* Process pending timers if we are erasing the front. */
		_timer_pending |= _timer_queue.front() == node;
		_timer_queue.erase(node);
	}

	io_event *context::request_io_event() noexcept
	{
		if (_io_requested < event_buffer_size)
			return _io_event_buff.get() + _io_requested++;
		else if (!_io_event_pool.empty())
			return _io_event_pool.pop_front();
		else
			return nullptr;
	}
	void context::release_io_event(io_event *event) noexcept
	{
		if (event == nullptr) [[unlikely]]
			return;

		_io_event_pool.push_front(event);
	}

	result<> context::port_bind(void *hnd) noexcept
	{
		if (::CreateIoCompletionPort(hnd, _iocp.native_handle(), 0, 0) != _iocp.native_handle().value) [[unlikely]]
			return dos_error_code(::GetLastError());
		if (!::SetFileCompletionNotificationModes(hnd, FILE_SKIP_COMPLETION_PORT_ON_SUCCESS | FILE_SKIP_SET_EVENT_ON_HANDLE)) [[unlikely]]
			return dos_error_code(::GetLastError());
		return {};
	}
	void context::set_timer(time_point tp) noexcept
	{
		LARGE_INTEGER timeout = {.QuadPart = tp.time_since_epoch().count()};
		const auto callback = [](void *ptr, auto...)
		{
			const auto ctx = static_cast<context *>(ptr);
			const auto status = ntapi::instance().value().NtSetIoCompletion(ctx->_iocp.native_handle(), 0, event_id::timeout, 0, 0);
			if (is_status_failure(status)) [[unlikely]]
						_detail::throw_error_code(status_error_code(status), "NtSetIoCompletion");
		};

		/* Win32 timers do not work with IOCP, so use a waitable timer instead. */
		if (!::SetWaitableTimer(_thread_timer.native_handle(), &timeout, 0, callback, this, false))
			_detail::throw_error_code(dos_error_code(::GetLastError()), "SetWaitableTimer");
		else
			_timer_started = true;
	}

	void context::acquire_waiting_events() noexcept
	{
		/* NOTE: The following loop makes the assumption that all waiting IO will be scheduled as pending.
		 * If this is not the case (ex. premature completion), operations will be starved until the next call. */
		for (auto io_avail = (event_buffer_size - _io_requested) + _io_event_pool.size; !_waitlist_queue.empty();)
		{
			const auto node = _waitlist_queue.pop_front();
			schedule_consumer(node);

			if (io_avail > static_cast<io_operation_base *>(node)->waiting)
				io_avail -= static_cast<io_operation_base *>(node)->waiting;
			else
				break;
		}
	}
	void context::acquire_elapsed_timers() noexcept
	{
		if (!_timer_pending)
			return;

		auto new_time = _next_timeout;
		for (const auto now = clock::now(); !_timer_queue.empty();)
		{
			if (const auto abs = _timer_queue.front()->to.absolute(); abs > now)
			{
				new_time = abs;
				break;
			}

			/* Ignore notify request if we have already requested a stop or a notify for this node. */
			const auto timer = _timer_queue.pop_front();
			if (int expected = 0; timer->flags.compare_exchange_strong(expected, flags_t::timeout_requested, std::memory_order_acq_rel))
				schedule_consumer(timer);
		}

		/* Disarm or start the timer. */
		if (!_timer_queue.empty())
		{
			if (_next_timeout >= new_time || !_timer_started)
				set_timer(_next_timeout = new_time);
			_timer_pending = false;
		}
		else if (_timer_started)
		{
			::CancelWaitableTimer(_thread_timer.native_handle());
			_timer_started = false;
			_timer_pending = false;
		}
	}
	void context::acquire_producer_queue() noexcept
	{
		if (_producer_queue.empty())
			return;
		_consumer_queue.merge_back(std::move(_producer_queue));
	}

	std::size_t context::run_once()
	{
		const auto g = lock_consumer_queue();
		std::size_t notified = 0;

		for (auto queue = std::move(_consumer_queue); !queue.empty(); ++notified)
			queue.pop_front()->notify();

		return notified;
	}
	std::size_t context::poll(bool block)
	{
		static constinit LARGE_INTEGER zero_timeout = {};
#ifdef _WIN64
		constexpr std::size_t buff_size = 8192 / sizeof(io_completion_info);
#else
		constexpr std::size_t buff_size = 4096 / sizeof(io_completion_info);
#endif

		const auto g = lock_consumer_queue();
		if (!has_pending() &&  block) [[unlikely]]
		{
			auto timeout = _producer_queue.active() ? &zero_timeout : nullptr;
			auto io_buff = std::array<io_completion_info, buff_size>();
			auto received = ULONG(0);

			const auto ntapi = ntapi::instance();
			if (ntapi.has_error()) [[unlikely]]
				_detail::throw_error_code(ntapi.error());

			_consumer_lock.unlock();
			{
				const auto status = ntapi->NtRemoveIoCompletionEx(_iocp.native_handle(), io_buff.data(), buff_size, &received, timeout, _timer_started);
				if (is_status_failure(status)) [[unlikely]]
					_detail::throw_error_code(status_error_code(status));
				if (status == STATUS_TIMEOUT) [[unlikely]]
					_producer_queue.try_terminate();
			}
			_consumer_lock.lock();

			/* Dispatch received completion events. Consumer must be re-locked for IO callbacks. */
			for (auto &info : std::ranges::take_view(io_buff, received))
				switch (info.apc_context)
				{
				default:
				{
					const auto event = std::bit_cast<io_event *>(info.apc_context);
					auto &iosb = reinterpret_cast<io_status_block &>(event->iosb);
					auto &base = *static_cast<io_operation_base *>(event->parent);
					base.event_cb(status_error_code(iosb.status), iosb.info, event->data);
					break;
				}
				case event_id::timeout:
					_timer_started = false;
					_timer_pending = true;
					[[fallthrough]];
				case event_id::wakeup:
					_producer_queue.try_activate();
					break;
				}
		}

		acquire_elapsed_timers();
		acquire_producer_queue();
		acquire_waiting_events();
		return _consumer_queue.size;
	}

	template<auto IoFunc, typename Op>
	result<std::size_t> file_handle::invoke_io_func(io_event &event, void *hnd, io_buffer<Op> &buff, extent_type &off) noexcept
	{
		auto &iosb = reinterpret_cast<io_status_block &>(event.iosb);
		const auto req_ptr = buff.data();
		const auto req_len = buff.size();

		if (req_len > std::size_t(std::numeric_limits<ULONG>::max())) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);
		if (off > extent_type(std::numeric_limits<LONGLONG>::max())) [[unlikely]]
			return std::make_error_code(std::errc::value_too_large);

		const auto ntapi = ntapi::instance();
		if (ntapi.has_error()) [[unlikely]]
			return ntapi.error();

		auto offset = LARGE_INTEGER{.QuadPart = LONGLONG(off)};
		iosb.status = STATUS_PENDING;
		buff = {req_ptr, 0};
		off += req_len;

		const auto status = ((*ntapi).*IoFunc)(hnd, nullptr, nullptr, std::bit_cast<std::uintptr_t>(&event), &iosb, req_ptr, ULONG(req_len), &offset, nullptr);
		if (is_status_failure(status)) [[unlikely]]
			return status_error_code(status);
		if (status != STATUS_PENDING)
			return req_len;
		else
			return 0;
	}

	result<std::size_t> file_handle::start_io(io_event &event, void *hnd, io_buffer<read_some_at_t> &buff, extent_type &off) noexcept
	{
		return invoke_io_func<&ntapi::NtReadFile, read_some_at_t>(event, hnd, buff, off);
	}
	result<std::size_t> file_handle::start_io(io_event &event, void *hnd, io_buffer<write_some_at_t> &buff, extent_type &off) noexcept
	{
		return invoke_io_func<&ntapi::NtWriteFile, write_some_at_t>(event, hnd, buff, off);
	}
}
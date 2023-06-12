/*
 * Created by switchblade on 2023-05-21.
 */

#include "epoll_context.hpp"

#ifdef ROD_HAS_EPOLL

#include <cassert>

#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>

namespace rod::_epoll
{
#if SIZE_MAX >= UINT64_MAX
	constexpr std::size_t default_max_events = 256;
#else
	constexpr std::size_t default_max_events = 128;
#endif

	enum event_id : std::uint64_t { timer_timeout = 1, queue_dispatch };

	[[noreturn]] inline void throw_errno(const char *msg) { throw std::system_error{errno, std::system_category(), msg}; }

	inline detail::unique_descriptor init_epoll_fd()
	{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
		const auto fd = epoll_create1(EPOLL_CLOEXEC);
		if (fd < 0) throw_errno("epoll_create1(EPOLL_CLOEXEC)");
#else
		const auto fd = epoll_create(1);
		if (fd < 0) throw_errno("epoll_create(1)");
#endif
		return detail::unique_descriptor{fd};
	}
	inline detail::unique_descriptor init_timer_fd(const detail::unique_descriptor &epoll_fd)
	{
		const auto fd = timerfd_create(CLOCK_MONOTONIC, 0);
		if (fd < 0) throw_errno("timerfd_create(CLOCK_MONOTONIC, 0)");

		/* Register timer descriptor with EPOLL. */
		epoll_event event = {};
		event.events = EPOLLIN;
		event.data.u64 = event_id::timer_timeout;
		if (epoll_ctl(epoll_fd.native_handle(), EPOLL_CTL_ADD, fd, &event))
			throw_errno("epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd)");
		else
			return detail::unique_descriptor{fd};
	}
	inline detail::unique_descriptor init_event_fd(const detail::unique_descriptor &epoll_fd)
	{
		const auto fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
		if (fd < 0) throw_errno("eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)");

		/* Register timer descriptor with EPOLL. */
		epoll_event event = {};
		event.events = EPOLLIN;
		event.data.u64 = event_id::queue_dispatch;
		if (epoll_ctl(epoll_fd.native_handle(), EPOLL_CTL_ADD, fd, &event))
			throw_errno("epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event_fd)");
		else
			return detail::unique_descriptor{fd};
	}

	context::context() : context(default_max_events) {}
	context::context(std::size_t max) : _epoll_fd(init_epoll_fd()), _timer_fd(init_timer_fd(_epoll_fd)), _event_fd(init_event_fd(_epoll_fd))
	{
		/* Events are heap-allocated in order to avoid excessive stack usage and allow for user-specified buffer sizes. */
		_buff_size = std::min<std::size_t>(max ? max : default_max_events, std::numeric_limits<int>::max());
		_event_buff = new epoll_event[_buff_size];
	}
	context::~context()
	{
		assert(!is_consumer_thread());

		epoll_event event = {};
		epoll_ctl(_epoll_fd.native_handle(), EPOLL_CTL_DEL, _timer_fd.native_handle(), &event);
		epoll_ctl(_epoll_fd.native_handle(), EPOLL_CTL_DEL, _event_fd.native_handle(), &event);
		delete[] static_cast<epoll_event *>(_event_buff);
	}

	void context::request_stop() { _stop_source.request_stop(); }
	bool context::is_consumer_thread() const noexcept { return _consumer_tid.load(std::memory_order_acquire) == std::this_thread::get_id(); }

	void context::schedule_producer(operation_base *node, std::error_code &err) noexcept
	{
		assert(!node->_next);
		if (_producer_queue.push(node))
		{
			/* Notify the event file descriptor to wake up the consumer thread. */
			const std::uint64_t token = 1;
			_event_fd.write(&token, sizeof(token), err);
		}
	}
	void context::schedule_consumer(operation_base *node) noexcept
	{
		assert(!node->_next);
		_consumer_queue.push_back(node);
	}
	void context::schedule_producer(operation_base *node)
	{
		std::error_code err;
		schedule_producer(node, err);
		if (err) throw std::system_error(err, "write(event_fd)");
	}

	inline void context::add_event(auto *data, int flags, int fd) noexcept
	{
		epoll_event event = {};
		event.data.ptr = data;
		event.events = EPOLLRDHUP | EPOLLHUP | flags;
		epoll_ctl(_epoll_fd.native_handle(), EPOLL_CTL_ADD, fd, &event);
	}
	void context::add_io(operation_base *node, io_cmd_t<schedule_read_some_t> cmd) noexcept { add_event(node, EPOLLIN, cmd.fd.native_handle()); }
	void context::add_io(operation_base *node, io_cmd_t<schedule_write_some_t> cmd) noexcept { add_event(node, EPOLLOUT, cmd.fd.native_handle()); }
	void context::add_io(operation_base *node, io_cmd_t<schedule_read_some_at_t> cmd) noexcept { add_event(node, EPOLLIN, cmd.fd.native_handle()); }
	void context::add_io(operation_base *node, io_cmd_t<schedule_write_some_at_t> cmd) noexcept { add_event(node, EPOLLOUT, cmd.fd.native_handle()); }
	void context::del_io(int fd) noexcept
	{
		epoll_event event = {};
		epoll_ctl(_epoll_fd.native_handle(), EPOLL_CTL_DEL, fd, &event);
	}

	void context::add_timer(timer_base *node) noexcept
	{
		assert(!node->_timer_next);
		assert(!node->_timer_prev);
		/* Process pending timers if the inserted timer is the new front. */
		_timer_pending |= _timers.insert(node) == node;
	}
	void context::del_timer(timer_base *node) noexcept
	{
		/* Process pending timers if we are erasing the front. */
		_timer_pending |= _timers.front() == node;
		_timers.erase(node);
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

		if (!_timers.empty())
			for (const auto now = monotonic_clock::now(); !_timers.empty() && _timers.front()->_tp <= now;)
			{
				const auto node = _timers.pop_front();
				/* Handle timer cancellation. */
				if (node->_stop_possible())
				{
					const auto flags = node->_flags.fetch_or(flags_t::dispatched, std::memory_order_acq_rel);
					if (flags & flags_t::stop_requested) continue;
				}
				schedule_consumer(node);
			}

		/* Disarm or start a timeout on the timer descriptor. */
		if (_timers.empty())
		{
			if (_timer_started)
			{
				set_timer(time_point{});
				_timer_started = false;
				_timer_pending = false;
			}
		}
		else if (const auto next_timeout = _timers.front()->_tp; _timer_started && _next_timeout < next_timeout)
			_timer_pending = false;
		else if ((_timer_started = !set_timer(next_timeout)))
		{
			_next_timeout = next_timeout;
			_timer_pending = false;
		}
	}
	inline bool context::set_timer(time_point tp)
	{
		::itimerspec timeout = {};
		timeout.it_value.tv_sec = tp.seconds();
		timeout.it_value.tv_nsec = tp.nanoseconds();
		if (const auto res =  timerfd_settime(_timer_fd.native_handle(), TFD_TIMER_ABSTIME, &timeout, nullptr); res == EINVAL)
			throw std::system_error{EINVAL, std::system_category(), "timerfd_settime"};
		else
			return res;
	}
	inline void context::epoll_wait()
	{
		const auto blocking = _consumer_queue.empty();
		const auto events = static_cast<epoll_event *>(_event_buff);

		int n_events;
		for (;;)
		{
			if ((n_events = ::epoll_wait(_epoll_fd.native_handle(), events, _buff_size, blocking ? -1 : 0)) >= 0)
				break;
			else if (const auto err = std::error_code{errno, std::system_category()}; err.value() != EINTR)
				throw std::system_error(err, "epoll_wait");
		}
		for (auto pos = static_cast<std::size_t>(n_events); pos-- != 0;)
		{
			auto &event = events[pos];
			switch (std::error_code err; event.data.u64)
			{
			case event_id::timer_timeout: /* Timer elapsed notification event. */
			{
				_timer_started = false;
				_timer_pending = true;

				// Read the eventfd to clear the signal.
				std::uint64_t token;
				_timer_fd.read(&token, sizeof(token), err);
				if (err) throw std::system_error(err, "read(timer_fd)");
				break;
			}
			case event_id::queue_dispatch: /* Producer queue notification event. */
			{
				std::uint64_t token;
				_event_fd.read(&token, sizeof(token), err);
				if (err)
					throw std::system_error(err, "read(event_fd)");
				else
					_wait_pending = false;
				break;
			}
			default: /* IO operation event. */
				schedule_consumer(static_cast<operation_base *>(event.data.ptr));
				break;
			}
		}
	}

	void context::run()
	{
		/* Make sure only one thread is allowed to run at a given time. */
		if (std::thread::id id = {}; !_consumer_tid.compare_exchange_strong(id, std::this_thread::get_id(), std::memory_order_acq_rel))
			throw std::system_error(std::make_error_code(std::errc::device_or_resource_busy), "Only one thread may invoke `epoll_context::run` at a given time");

		struct thread_guard
		{
			~thread_guard() { tid.store(std::thread::id{}, std::memory_order_release); }
			std::atomic<std::thread::id> &tid;
		} g = {_consumer_tid};

		for (;;)
		{
			for (auto queue = std::move(_consumer_queue); !queue.empty();)
				queue.pop_front()->_notify();
			if (std::exchange(_stop_pending, false)) [[unlikely]]
				return;

			/* Schedule execution of elapsed timers. */
			acquire_elapsed_timers();

			/* Acquire pending operations from the producer queue & wait for EPOLL events. */
			if (_wait_pending || (_wait_pending = acquire_producer_queue()))
				epoll_wait();
		}
	}
	void context::finish()
	{
		/* Notification function will be reset on dispatch, so set it here instead of the constructor. */
		_notify_func = [](operation_base *ptr) noexcept { static_cast<context *>(ptr)->_stop_pending = true; };
		schedule(this);
	}
}
#endif

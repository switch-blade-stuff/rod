/*
 * Created by switchblade on 2023-05-21.
 */

#include "epoll_context.hpp"

#ifdef ROD_HAS_EPOLL

#include <cassert>

#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <cstdio>

namespace rod::_epoll
{
#if SIZE_MAX >= UINT64_MAX
	/* 10k buffer. */
	constexpr std::size_t default_max_events = 256;
#else
	/* 5k buffer. */
	constexpr std::size_t default_max_events = 128;
#endif

	enum event_id : std::uint64_t { timer_event, queue_event, };

	[[noreturn]] inline void throw_errno(const char *msg) { throw std::system_error{errno, std::system_category(), msg}; }

	inline detail::unique_descriptor init_epoll_fd()
	{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 27)
		const auto fd = epoll_create(1);
			if (fd < 0) [[unlikely]] throw std::system_error{errno, std::system_category(), "epoll_create(1)"};
#else
		const auto fd = epoll_create1(EPOLL_CLOEXEC);
		if (fd < 0) [[unlikely]] throw_errno("epoll_create1(EPOLL_CLOEXEC)");
#endif
		return detail::unique_descriptor{fd};
	}
	inline detail::unique_descriptor init_timer_fd(const detail::unique_descriptor &epoll_fd)
	{
		const auto fd = timerfd_create(CLOCK_MONOTONIC, 0);
		if (fd < 0) [[unlikely]] throw_errno("timerfd_create(CLOCK_MONOTONIC, 0)");

		/* Register timer descriptor with EPOLL. */
		epoll_event event = {};
		event.events = EPOLLIN;
		event.data.u64 = event_id::timer_event;
		if (epoll_ctl(epoll_fd.native_handle(), EPOLL_CTL_ADD, fd, &event))
			[[unlikely]] throw_errno("epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &event)");
		else
			return detail::unique_descriptor{fd};
	}
	inline detail::unique_descriptor init_event_fd(const detail::unique_descriptor &epoll_fd)
	{
		const auto fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
		if (fd < 0) [[unlikely]] throw_errno("eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)");

		/* Register timer descriptor with EPOLL. */
		epoll_event event = {};
		event.events = EPOLLIN;
		event.data.u64 = event_id::queue_event;
		if (epoll_ctl(epoll_fd.native_handle(), EPOLL_CTL_ADD, fd, &event))
			[[unlikely]] throw_errno("epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event_fd, &event)");
		else
			return detail::unique_descriptor{fd};
	}

	context::context() : context(default_max_events) {}
	context::context(std::size_t max) : m_epoll_fd(init_epoll_fd()), m_timer_fd(init_timer_fd(m_epoll_fd)), m_event_fd(init_event_fd(m_epoll_fd))
	{
		/* Events are heap-allocated in order to avoid excessive stack usage and allow for user-specified buffer sizes. */
		m_buff_size = std::min<std::size_t>(max ? max : default_max_events, std::numeric_limits<int>::max());
		m_event_buff = new epoll_event[m_buff_size];
	}
	context::~context()
	{
		assert(m_consumer_tid == std::thread::id{});
		epoll_event event = {};
		epoll_ctl(m_epoll_fd.native_handle(), EPOLL_CTL_DEL, m_timer_fd.native_handle(), &event);
		epoll_ctl(m_epoll_fd.native_handle(), EPOLL_CTL_DEL, m_event_fd.native_handle(), &event);
		delete[] static_cast<epoll_event *>(m_event_buff);
	}

	void context::request_stop() { m_stop_source.request_stop(); }

	void context::schedule_producer(operation_base *node, std::error_code &err) noexcept
	{
		assert(!node->_next);
		if (m_producer_queue.push(node))
		{
			/* Notify the event file descriptor to wake up the consumer thread. */
			const std::uint64_t token = 1;
			m_event_fd.write(&token, sizeof(token), err);
		}
	}
	void context::schedule_consumer(operation_base *node) noexcept
	{
		assert(!node->_next);
		m_consumer_queue.push_back(node);
	}

	void context::add_io(detail::basic_descriptor fd, operation_base *node) noexcept
	{
		epoll_event event = {};
		event.data.ptr = node;
		event.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
		epoll_ctl(m_epoll_fd.native_handle(), EPOLL_CTL_ADD, fd.native_handle(), &event);
	}
	void context::del_io(detail::basic_descriptor fd) noexcept
	{
		epoll_event event = {};
		epoll_ctl(m_epoll_fd.native_handle(), EPOLL_CTL_DEL, fd.native_handle(), &event);
	}

	void context::add_timer(timer_node *node) noexcept
	{
		assert(!node->_timer_next);
		assert(!node->_timer_prev);
		/* Process pending timers if the inserted timer is the new front. */
		m_timer_pending |= m_timers.insert(node) == node;
	}
	void context::del_timer(timer_node *node) noexcept
	{
		/* Process pending timers if we are erasing the front. */
		m_timer_pending |= m_timers.front() == node;
		m_timers.erase(node);
	}

	inline bool context::acquire_producer_queue() noexcept
	{
		if (!m_producer_queue.try_terminate())
		{
			m_consumer_queue.merge_back(static_cast<consumer_queue_t>(std::move(m_producer_queue)));
			return false;
		}
		return true;
	}
	inline void context::epoll_wait()
	{
		const auto blocking = m_consumer_queue.empty();
		const auto events = static_cast<epoll_event *>(m_event_buff);
		const auto res = ::epoll_wait(m_epoll_fd.native_handle(), events, m_buff_size, blocking ? -1 : 0);
		if (res < 0) [[unlikely]] throw_errno("epoll_wait");

		for (auto pos = static_cast<std::size_t>(res); pos-- != 0;)
		{
			auto &event = events[pos];
			switch (std::error_code err; event.data.u64)
			{
			case event_id::timer_event: /* Timer elapsed notification event. */
			{
				m_timer_fd_started = false;
				m_timer_pending = true;

				// Read the eventfd to clear the signal.
				std::uint64_t token;
				m_timer_fd.read(&token, sizeof(token), err);
				if (err) [[unlikely]] throw std::system_error(err, "read(timer_fd)");
				break;
			}
			case event_id::queue_event: /* Producer queue notification event. */
			{
				std::uint64_t token;
				m_event_fd.read(&token, sizeof(token), err);
				if (err)
					[[unlikely]] throw std::system_error(err, "read(event_fd)");
				else
					m_epoll_pending = false;
				break;
			}
			default: /* IO operation event. */
				schedule_consumer(static_cast<operation_base *>(event.data.ptr));
				break;
			}
		}
	}

	inline auto set_timer_fd(int fd, time_point tp = {})
	{
		::itimerspec timeout = {};
		timeout.it_value.tv_sec = tp.seconds();
		timeout.it_value.tv_nsec = tp.nanoseconds();
		if (const auto res =  timerfd_settime(fd, TFD_TIMER_ABSTIME, &timeout, nullptr); res == EINVAL)
			throw std::system_error{EINVAL, std::system_category(), "timerfd_settime"};
		else
			return res;
	}

	void context::run()
	{
		/* Make sure only one thread is allowed to run at a given time. */
		if (std::thread::id id = {}; !m_consumer_tid.compare_exchange_strong(id, std::this_thread::get_id(), std::memory_order_acq_rel))
			throw std::system_error(std::make_error_code(std::errc::device_or_resource_busy), "Only one thread may invoke `epoll_context::run`");

		struct tid_guard
		{
			~tid_guard() { tid.store(std::thread::id{}, std::memory_order_release); }
			std::atomic<std::thread::id> &tid;
		} g = {m_consumer_tid};

		for (;;)
		{
			/* Always run local thread queue to completion before checking for stop requests.
			 * Timers & pending producer queue will be handled after the stop request check. */
			for (auto queue = std::move(m_consumer_queue); !queue.empty();)
				queue.pop_front()->_notify();
			if (std::exchange(m_stop_pending, false)) [[unlikely]]
				return;

			/* Schedule execution of elapsed timers. */
			if (m_timer_pending)
			{
				for (const auto now = monotonic_clock::now(); !m_timers.empty() && m_timers.front()->_timeout <= now;)
				{
					auto node = m_timers.pop_front();

					/* Handle timer cancellation. */
					if (node->_flags.load(std::memory_order_relaxed) & timer_node::stop_possible)
					{
						const auto flags = node->_flags.fetch_or(timer_node::dispatched, std::memory_order_acq_rel);
						if (flags & timer_node::stop_requested) continue;
					}
					schedule_consumer(node);
				}

				/* Disarm timer descriptor (set timeout to 0) if there is no more pending timers. */
				if (m_timers.empty())
				{
					if (std::exchange(m_timer_fd_started, {}))
						set_timer_fd(m_timer_fd.native_handle());
					m_timer_pending = false;
				}
				else if (!m_timers.empty())
				{
					/* Start timer_fd timer for the earliest pending time point. */
					const auto next_timeout = m_timers.front()->_timeout;
					if (m_timer_fd_started && m_next_timeout <= next_timeout)
						m_timer_pending = false;
					else if ((m_timer_fd_started = !set_timer_fd(m_timer_fd.native_handle(), next_timeout)))
					{
						m_next_timeout = next_timeout;
						m_timer_pending = false;
					}
				}
			}

			/* Acquire pending operations from the producer queue & wait for EPOLL events. */
			if (m_epoll_pending || (m_epoll_pending = acquire_producer_queue()))
				epoll_wait();
		}
	}

	void context::finish()
	{
		/* Notification function will be reset on dispatch, so set it here instead of the constructor. */
		_notify_func = [](operation_base *ptr) noexcept { static_cast<context *>(ptr)->m_stop_pending = true; };

		std::error_code err;
		schedule(this, err);
		if (err) [[unlikely]] throw std::system_error{err, "write(event_fd)"};
	}
}
#endif

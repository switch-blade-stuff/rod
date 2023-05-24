/*
 * Created by switchblade on 2023-05-21.
 */

#include "epoll_context.hpp"

#ifdef ROD_HAS_EPOLL

#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <cassert>

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

	inline detail::descriptor_handle init_epoll_fd()
	{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 27)
		const auto fd = epoll_create(1);
			if (fd < 0) [[unlikely]] throw std::system_error{errno, std::system_category(), "epoll_create(1)"};
#else
		const auto fd = epoll_create1(EPOLL_CLOEXEC);
		if (fd < 0) [[unlikely]] throw_errno("epoll_create1(EPOLL_CLOEXEC)");
#endif
		return detail::descriptor_handle{fd};
	}
	inline detail::descriptor_handle init_timer_fd(const detail::descriptor_handle &epoll_fd)
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
			return detail::descriptor_handle{fd};
	}
	inline detail::descriptor_handle init_event_fd(const detail::descriptor_handle &epoll_fd)
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
			return detail::descriptor_handle{fd};
	}

	context::context() : context(default_max_events) {}
	context::context(std::size_t max) : m_epoll_fd(init_epoll_fd()), m_timer_fd(init_timer_fd(m_epoll_fd)), m_event_fd(init_event_fd(m_epoll_fd))
	{
		/* Events are heap-allocated in order to avoid excessive stack usage and allow for user-specified buffer sizes. */
		max = std::min<std::size_t>(max ? max : default_max_events, std::numeric_limits<int>::max());
		m_events = {new epoll_event[max], max};
	}
	context::~context()
	{
		assert(m_consumer_tid == std::thread::id{});
		epoll_event event = {};
		epoll_ctl(m_epoll_fd.native_handle(), EPOLL_CTL_DEL, m_timer_fd.native_handle(), &event);
		epoll_ctl(m_epoll_fd.native_handle(), EPOLL_CTL_DEL, m_event_fd.native_handle(), &event);
		delete[] m_events.data();
	}

	void context::schedule_producer(operation_base *node)
	{
		assert(!node->_next);
		if (m_producer_queue.push(node, false))
		{
			/* Notify the event file descriptor to wake up the consumer thread. */
			std::error_code err = {};
			const std::uint64_t token = 1;
			m_event_fd.write(&token, sizeof(token), err);
			if (err) [[unlikely]] throw std::system_error(err, "write(event_fd)");
		}
	}
	void context::schedule_consumer(operation_base *node) noexcept
	{
		assert(!node->_next);
		m_consumer_queue.push_back(node);
	}

	void context::insert_timer(timer_node *node) noexcept
	{
		/* Process pending timers if the inserted timer is the new front. */
		m_timer_pending |= m_timers.insert(node) == node;
	}
	void context::erase_timer(timer_node *node) noexcept
	{
		/* Process pending timers if we are erasing the front. */
		m_timer_pending |= m_timers.front() == node;
		m_timers.erase(node);
	}

	inline bool context::acquire_producer_queue() noexcept
	{
		if (!m_producer_queue.try_terminate(false))
		{
			m_consumer_queue.merge_back(static_cast<consumer_queue_t>(std::move(m_producer_queue)));
			return false;
		}
		return true;
	}
	inline void context::epoll_wait()
	{
		const auto blocking = !m_consumer_queue.empty();
		const auto res = ::epoll_wait(m_epoll_fd.native_handle(), m_events.data(), m_events.size(), blocking ? -1 : 0);
		if (res < 0) [[unlikely]] throw_errno("epoll_wait");

		for (auto pos = static_cast<std::size_t>(res); pos-- != 0;)
		{
			auto &event = m_events[pos];
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

	inline auto set_timer_fd(int fd, time_point tp = {}) noexcept
	{
		::itimerspec timeout = {};
		timeout.it_value.tv_sec = tp.seconds();
		timeout.it_value.tv_nsec = tp.nanoseconds();
		return timerfd_settime(fd, TFD_TIMER_ABSTIME, &timeout, nullptr);
	}

	void context::run_impl()
	{
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
			for (const auto now = monotonic_clock::now(); !m_timers.empty() && m_timers.front()->_timeout <= now;)
			{
				auto node = m_timers.pop_front();

				/* Handle timer cancellation. */
				if (node->_flags.load(std::memory_order_relaxed) & timer_node::stop_possible)
				{
					const auto flags = node->_flags.fetch_add(timer_node::dispatched, std::memory_order_acq_rel);
					if (flags & timer_node::stop_requested) continue;
				}
				schedule_consumer(node);
			}

			/* Disarm timer descriptor (set timeout to 0) if there is no more pending timers. */
			if (m_timers.empty() && std::exchange(m_timer_fd_started, {}))
			{
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

			/* Acquire pending operations from the producer queue & wait for EPOLL events. */
			if (m_epoll_pending || (m_epoll_pending = acquire_producer_queue()))
				epoll_wait();
		}
	}
}
#endif

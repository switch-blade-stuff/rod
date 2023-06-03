/*
 * Created by switchblade on 2023-05-29.
 */

#include "io_uring_context.hpp"

#ifdef ROD_HAS_LIBURING

#include <cassert>

#include <sys/eventfd.h>
#include <sys/mman.h>
#include <liburing.h>

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::_io_uring
{
#if SIZE_MAX >= UINT64_MAX
	constexpr std::size_t default_entries = 256;
#else
	constexpr std::size_t default_entries = 128;
#endif

	enum event_id : std::uint64_t { timer_timeout = 1, queue_dispatch, timer_cancel };

	[[noreturn]] inline void throw_errno(const char *msg) { throw std::system_error{errno, std::system_category(), msg}; }

	context::context() : context(default_entries) {}
	context::context(std::size_t entries)
	{
		constexpr auto mmap_mode = MAP_SHARED | MAP_POPULATE;
		constexpr auto mmap_prot = PROT_READ | PROT_WRITE;
		io_uring_params params = {};
		{
			entries = std::min<std::size_t>(entries, std::numeric_limits<unsigned int>::max());
			if (auto fd = io_uring_setup(entries, &params); fd < 0)
				[[unlikely]] throw_errno("io_uring_setup");
			else
				m_uring_fd.release(fd);
		}
		{
			if (const auto fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC); fd < 0)
				[[unlikely]] throw_errno("eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)");
			else
				m_event_fd.release(fd);
		}
		{
			const auto size = params.cq_entries * sizeof(io_uring_cqe) + params.cq_off.cqes;
			const auto data = ::mmap(nullptr, size, mmap_prot, mmap_mode, m_uring_fd.native_handle(), IORING_OFF_CQ_RING);
			if (!data) [[unlikely]] throw_errno("mmap");

			const auto bytes = static_cast<std::byte *>(data);
			m_cq.entries = reinterpret_cast<io_uring_cqe *>(bytes + params.cq_off.cqes);
			m_cq.overflow = reinterpret_cast<unsigned *>(bytes + params.cq_off.overflow);
			m_cq.mask = *reinterpret_cast<unsigned *>(bytes + params.cq_off.ring_mask);
			m_cq.tail = reinterpret_cast<unsigned *>(bytes + params.cq_off.tail);
			m_cq.head = reinterpret_cast<unsigned *>(bytes + params.cq_off.head);
			m_cq.size = params.cq_entries;
			m_cq_mmap.release(data, size);
		}
		{
			const auto size = params.sq_entries * sizeof(std::uint32_t) + params.sq_off.array;
			const auto data = ::mmap(nullptr, size, mmap_prot, mmap_mode, m_uring_fd.native_handle(), IORING_OFF_SQ_RING);
			if (!data) [[unlikely]] throw_errno("mmap");

			const auto bytes = static_cast<std::byte *>(data);
			m_sq.idx_data = reinterpret_cast<unsigned *>(bytes + params.sq_off.array);
			m_sq.dropped = reinterpret_cast<unsigned *>(bytes + params.sq_off.dropped);
			m_sq.mask = *reinterpret_cast<unsigned *>(bytes + params.sq_off.ring_mask);
			m_sq.tail = reinterpret_cast<unsigned *>(bytes + params.sq_off.tail);
			m_sq.head = reinterpret_cast<unsigned *>(bytes + params.sq_off.head);
			m_sq.size = params.sq_entries;
			m_sq_mmap.release(data, size);
		}
		{
			const auto size = params.sq_entries * sizeof(io_uring_sqe);
			const auto data = ::mmap(nullptr, size, mmap_prot, mmap_mode, m_uring_fd.native_handle(), IORING_OFF_SQES);
			if (!data) [[unlikely]] throw_errno("mmap");

			m_sq.entries = static_cast<io_uring_sqe *>(data);
			m_sqe_mmap.release(data, size);
		}
	}
	context::~context() = default;

	void context::request_stop() { m_stop_source.request_stop(); }
	bool context::is_consumer_thread() const noexcept { return m_consumer_tid.load(std::memory_order_acquire) == std::this_thread::get_id(); }

	void context::schedule_producer(operation_base_t *node, std::error_code &err) noexcept
	{
		assert(!node->_next);
		if (m_producer_queue.push(node))
		{
			const std::uint64_t token = 1;
			m_event_fd.write(&token, sizeof(token), err);
		}
	}
	void context::schedule_consumer(operation_base_t *node) noexcept
	{
		assert(!node->_next);
		m_waitlist_queue.push_back(node);
	}
	void context::schedule_waitlist(operation_base_t *node) noexcept
	{
		assert(!node->_next);
		m_waitlist_queue.push_back(node);
	}

	bool context::submit_io_event(operation_base_t *node, _system_ctx::io_id id, const _system_ctx::io_cmd<> &cmd) noexcept
	{
		return submit_sqe([&](io_uring_sqe *sq, std::uint32_t idx) noexcept
		{
			sq[idx] = {};
			switch (id)
			{
				case _system_ctx::io_id::read: [[fallthrough]];
				case _system_ctx::io_id::read_at:
					sq[idx].opcode = IORING_OP_READV;
					break;
				case _system_ctx::io_id::write: [[fallthrough]];
				case _system_ctx::io_id::write_at:
					sq[idx].opcode = IORING_OP_WRITEV;
					break;
				default: [[unlikely]] std::terminate();
			}
			sq[idx].user_data = std::bit_cast<std::uintptr_t>(node);
			sq[idx].addr = std::bit_cast<std::uintptr_t>(&cmd.buff);
			sq[idx].off = static_cast<std::size_t>(cmd.off);
			sq[idx].fd = cmd.fd.native_handle();
			sq[idx].len = 1;
		});
	}
	bool context::cancel_io_event(operation_base_t *node) noexcept
	{
		return submit_sqe([&](io_uring_sqe *sq, std::uint32_t idx) noexcept
		{
			sq[idx] = {};
			sq[idx].addr = std::bit_cast<std::uintptr_t>(node);
#ifdef IORING_ASYNC_CANCEL_ALL
			sq[idx].cancel_flags = IORING_ASYNC_CANCEL_ALL;
#else
			sq[idx].rw_flags = 1;
#endif
			sq[idx].opcode = IORING_OP_ASYNC_CANCEL;
		});
	}

	bool context::submit_timer_event(time_point timeout) noexcept
	{
		const auto res = submit_sqe([&](io_uring_sqe *sq, std::uint32_t idx) noexcept
	    {
			sq[idx] = {};
		    sq[idx].opcode = IORING_OP_TIMEOUT;
			sq[idx].addr = std::bit_cast<std::uintptr_t>(&m_ktime);
		    sq[idx].user_data = event_id::timer_timeout;
#ifdef IORING_TIMEOUT_ABS
			sq[idx].timeout_flags = IORING_TIMEOUT_ABS;
#else
			sq[idx].rw_flags = 1;
#endif
		    m_ktime.tv_sec = timeout.seconds();
		    m_ktime.tv_nsec = timeout.nanoseconds();
	    });
		return (m_active_timers += res, res);
	}
	bool context::cancel_timer_event() noexcept
	{
		return submit_sqe([&](io_uring_sqe *sq, std::uint32_t idx) noexcept
		{
			sq[idx] = {};
			sq[idx].opcode = IORING_OP_TIMEOUT_REMOVE;
			sq[idx].addr = std::bit_cast<std::uintptr_t>(&m_timers);
			sq[idx].user_data = event_id::timer_cancel;
		});
	}

	void context::add_timer(timer_node_t *node) noexcept
	{
		assert(!node->_timer_next);
		assert(!node->_timer_prev);
		/* Process pending timers if the inserted timer is the new front. */
		m_timer_pending |= m_timers.insert(node) == node;
	}
	void context::del_timer(timer_node_t *node) noexcept
	{
		/* Process pending timers if we are erasing the front. */
		m_timer_pending |= m_timers.front() == node;
		m_timers.erase(node);
	}
}
ROD_TOPLEVEL_NAMESPACE_CLOSE
#endif

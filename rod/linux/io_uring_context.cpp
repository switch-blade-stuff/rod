/*
 * Created by switchblade on 2023-05-29.
 */

#include "io_uring_context.hpp"

#ifdef ROD_HAS_LIBURING

#include <cassert>

#include <sys/eventfd.h>
#include <sys/mman.h>
#include <liburing.h>

/* TODO: Figure out why Valgrind causes a deadlock.
 * Valdrind invoked with callgrind also deadlocks, even though it should only track function calls.
 * Simulating slowdown via sleep(1) does not deadlock, nor does deadlock happen under normal operation.
 *
 * Neither CLang's -fsanitize=address nor -fsanitize=memory cause this deadlock, and neither does it
 * happen under normal operation. I have no clue what the hell is happening when running through Valgrind. */

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
				throw_errno("io_uring_setup");
			else
				_uring_fd.release(fd);
		}
		{
			if (const auto fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC); fd < 0)
				throw_errno("eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)");
			else
				_event_fd.release(fd);
		}
		{
			const auto size = params.cq_entries * sizeof(io_uring_cqe) + params.cq_off.cqes;
			const auto data = ::mmap(nullptr, size, mmap_prot, mmap_mode, _uring_fd.native_handle(), IORING_OFF_CQ_RING);
			if (!data) throw_errno("mmap");

			const auto bytes = static_cast<std::byte *>(data);
			_cq.entries = reinterpret_cast<io_uring_cqe *>(bytes + params.cq_off.cqes);
			_cq.overflow = reinterpret_cast<unsigned *>(bytes + params.cq_off.overflow);
			_cq.mask = *reinterpret_cast<unsigned *>(bytes + params.cq_off.ring_mask);
			_cq.tail = reinterpret_cast<unsigned *>(bytes + params.cq_off.tail);
			_cq.head = reinterpret_cast<unsigned *>(bytes + params.cq_off.head);
			_cq.size = params.cq_entries;
			_cq_mmap.release(data, size);
		}
		{
			const auto size = params.sq_entries * sizeof(std::uint32_t) + params.sq_off.array;
			const auto data = ::mmap(nullptr, size, mmap_prot, mmap_mode, _uring_fd.native_handle(), IORING_OFF_SQ_RING);
			if (!data) throw_errno("mmap");

			const auto bytes = static_cast<std::byte *>(data);
			_sq.idx_data = reinterpret_cast<unsigned *>(bytes + params.sq_off.array);
			_sq.dropped = reinterpret_cast<unsigned *>(bytes + params.sq_off.dropped);
			_sq.mask = *reinterpret_cast<unsigned *>(bytes + params.sq_off.ring_mask);
			_sq.tail = reinterpret_cast<unsigned *>(bytes + params.sq_off.tail);
			_sq.head = reinterpret_cast<unsigned *>(bytes + params.sq_off.head);
			_sq.size = params.sq_entries;
			_sq_mmap.release(data, size);
		}
		{
			const auto size = params.sq_entries * sizeof(io_uring_sqe);
			const auto data = ::mmap(nullptr, size, mmap_prot, mmap_mode, _uring_fd.native_handle(), IORING_OFF_SQES);
			if (!data) throw_errno("mmap");

			_sq.entries = static_cast<io_uring_sqe *>(data);
			_sqe_mmap.release(data, size);
		}
	}
	context::~context() { assert(!is_consumer_thread()); }

	void context::request_stop() { _stop_source.request_stop(); }
	bool context::is_consumer_thread() const noexcept { return _consumer_tid.load(std::memory_order_acquire) == std::this_thread::get_id(); }

	void context::schedule_producer(operation_base *node, std::error_code &err) noexcept
	{
		assert(!node->next);
		if (_producer_queue.push(node))
		{
			const std::uint64_t token = 1;
			_event_fd.write(&token, sizeof(token), err);
		}
	}
	void context::schedule_consumer(operation_base *node) noexcept
	{
		assert(!node->next);
		_consumer_queue.push_back(node);
	}
	void context::schedule_waitlist(operation_base *node) noexcept
	{
		assert(!node->next);
		_waitlist_queue.push_back(node);
	}
	void context::schedule_producer(operation_base *node)
	{
		std::error_code err;
		schedule_producer(node, err);
		if (err) throw std::system_error(err, "write(event_fd)");
	}

	inline bool context::submit_io_event(int op, auto *data, int fd, auto *addr, std::size_t n, std::size_t off) noexcept
	{
		return submit_sqe([&](io_uring_sqe *sq, std::uint32_t idx) noexcept
		{
			sq[idx] = {};
			sq[idx].opcode = op;
			sq[idx].user_data = std::bit_cast<std::uintptr_t>(data);
			sq[idx].addr = std::bit_cast<std::uintptr_t>(addr);
			sq[idx].off = static_cast<std::uint64_t>(off);
			sq[idx].len = n;
			sq[idx].fd = fd;
		});
	}
	inline bool context::submit_timer_event(time_point timeout) noexcept
	{
		const auto res = submit_sqe([&](io_uring_sqe *sq, std::uint32_t idx) noexcept
		{
			sq[idx] = {};
			sq[idx].opcode = IORING_OP_TIMEOUT;
			sq[idx].addr = std::bit_cast<std::uintptr_t>(&_ktime);
			sq[idx].user_data = event_id::timer_timeout;
#ifdef IORING_TIMEOUT_ABS
		    sq[idx].timeout_flags = IORING_TIMEOUT_ABS;
#else
			sq[idx].rw_flags = 1;
#endif
			_ktime.tv_sec = timeout.seconds();
			_ktime.tv_nsec = timeout.nanoseconds();
		});
		return (_active_timers += res, res);
	}
	inline bool context::cancel_timer_event() noexcept
	{
		return submit_sqe([&](io_uring_sqe *sq, std::uint32_t idx) noexcept
		{
			sq[idx] = {};
			sq[idx].opcode = IORING_OP_TIMEOUT_REMOVE;
			sq[idx].user_data = event_id::timer_cancel;
			sq[idx].addr = event_id::timer_timeout;
		});
	}
	inline bool context::submit_queue_event() noexcept
	{
		return submit_sqe([&](io_uring_sqe *sq, std::uint32_t idx) noexcept
		{
			if (!_producer_queue.try_terminate())
			{
				_consumer_queue.merge_back(std::move(_producer_queue));
				return false;
			}

			sq[idx] = {};
			sq[idx].opcode = IORING_OP_POLL_ADD;
			sq[idx].fd = _event_fd.native_handle();
			sq[idx].user_data = event_id::queue_dispatch;
			sq[idx].poll_events = POLL_IN;
			return true;
		});
	}

	bool context::submit_io_event(operation_base *node, io_cmd_t<schedule_read_some_t> cmd) noexcept
	{
		return submit_io_event(IORING_OP_READ, node, cmd.fd.native_handle(), cmd.buff.data(), cmd.buff.size(), -1);
	}
	bool context::submit_io_event(operation_base *node, io_cmd_t<schedule_write_some_t> cmd) noexcept
	{
		return submit_io_event(IORING_OP_WRITE, node, cmd.fd.native_handle(), cmd.buff.data(), cmd.buff.size(), -1);
	}
	bool context::submit_io_event(operation_base *node, io_cmd_t<schedule_read_some_at_t> cmd) noexcept
	{
		return submit_io_event(IORING_OP_READ, node, cmd.fd.native_handle(), cmd.buff.data(), cmd.buff.size(), cmd.off);
	}
	bool context::submit_io_event(operation_base *node, io_cmd_t<schedule_write_some_at_t> cmd) noexcept
	{
		return submit_io_event(IORING_OP_WRITE, node, cmd.fd.native_handle(), cmd.buff.data(), cmd.buff.size(), cmd.off);
	}
	bool context::cancel_io_event(operation_base *node) noexcept
	{
		return submit_sqe([&](io_uring_sqe *sq, std::uint32_t idx) noexcept
		{
			sq[idx] = {};
			sq[idx].user_data = sq[idx].addr = std::bit_cast<std::uintptr_t>(node);
#ifdef IORING_ASYNC_CANCEL_ALL
			sq[idx].cancel_flags = IORING_ASYNC_CANCEL_ALL;
#else
			sq[idx].rw_flags = 1;
#endif
			sq[idx].opcode = IORING_OP_ASYNC_CANCEL;
		});
	}

	void context::add_timer(timer_base *node) noexcept
	{
		assert(!node->timer_next);
		assert(!node->timer_prev);
		/* Process pending timers if the inserted timer is the new front. */
		_timer_pending |= _timers.insert(node) == node;
	}
	void context::del_timer(timer_base *node) noexcept
	{
		/* Process pending timers if we are erasing the front. */
		_timer_pending |= _timers.front() == node;
		_timers.erase(node);
	}

	inline void context::acquire_consumer_queue()
	{
		const auto tail = std::atomic_ref{*_cq.tail}.load(std::memory_order_acquire);
		const auto head = std::atomic_ref{*_cq.head}.load(std::memory_order_acquire);
		const auto num = tail - head;
		if (!num) return;

		/* Temporary queue is used to allow for errors during dispatch. */
		consumer_queue_t tmp_queue;

		for (std::uint32_t i = 0; i < num; ++i)
			switch (auto &event = _cq.entries[(head + i) & _cq.mask]; event.user_data)
			{
			case event_id::timer_timeout: /* Timer elapsed notification event. */
				_timer_pending |= (event.res != ECANCELED);
				_timer_started = --_active_timers;
				[[fallthrough]];
			case event_id::timer_cancel:
				break;
			case event_id::queue_dispatch: /* Producer queue notification event. */
			{
				std::error_code err;
				std::uint64_t token;
				if (event.res < 0)
					throw std::system_error(std::error_code{-event.res, std::system_category()}, "read(event_fd)");
				else if (_event_fd.read(&token, sizeof(token), err); err)
					throw std::system_error(err, "read(event_fd)");
				else
					_wait_pending = false;
				break;
			}
			default: /* IO operation event. */
				auto *node = std::bit_cast<operation_base *>(static_cast<std::uintptr_t>(event.user_data));
				node->result = static_cast<std::ptrdiff_t>(event.res);
				tmp_queue.push_back(node);
			}

		_consumer_queue.merge_back(std::move(tmp_queue));
		std::atomic_ref{*_cq.head}.store(tail, std::memory_order_release);
		_cq.pending -= num;
	}
	inline void context::acquire_elapsed_timers()
	{
		if (!_timer_pending)
			return;

		if (!_timers.empty())
			for (const auto now = clock::now(); !_timers.empty() && _timers.front()->timeout <= now;)
			{
				const auto node = _timers.pop_front();

				/* Handle timer cancellation. */
				if (node->stop_possible())
				{
					const auto flags = node->flags.fetch_or(flags_t::dispatched, std::memory_order_acq_rel);
					if (flags & flags_t::stop_requested) continue;
				}
				schedule_consumer(node);
			}

		/* Disarm or start a timeout event. */
		if (_timers.empty())
		{
			if (_timer_started && cancel_timer_event())
			{
				_timer_started = false;
				_timer_pending = false;
			}
			return;
		}

		if (const auto next_timeout = _timers.front()->timeout; _timer_started)
		{
			if (_next_timeout >= next_timeout)
				_timer_pending = false;
			else if (cancel_timer_event() && (_timer_started = submit_timer_event(next_timeout)))
			{
				_next_timeout = next_timeout;
				_timer_pending = false;
			}
		}
		else if ((_timer_started = submit_timer_event(next_timeout)))
		{
			_next_timeout = next_timeout;
			_timer_pending = false;
		}
	}
	inline void context::uring_enter()
	{
		const auto has_pending = _sq.pending || !_consumer_queue.empty();
		if (!has_pending && !_wait_pending) _wait_pending = submit_queue_event();

		std::uint32_t count = 0, flags = 0;
		if (!has_pending && (_wait_pending || _cq.pending + _sq.pending == _cq.size))
		{
			flags = IORING_ENTER_GETEVENTS;
			count = 1;
		}

		for (;;)
		{
			if (const auto res = ::io_uring_enter(_uring_fd.native_handle(), _sq.pending, count, flags, nullptr); res >= 0)
			{
				_sq.pending -= res;
				_cq.pending += res;
				break;
			}
			else if (const auto err = std::error_code{-res, std::system_category()}; err.value() != EINTR)
				throw std::system_error(err, "io_uring_enter");
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
				queue.pop_front()->notify();
			if (std::exchange(_stop_pending, false)) [[unlikely]]
				return;

			acquire_consumer_queue();
			acquire_elapsed_timers();

			/* Handle producer & waitlist queue items. */
			if (!_wait_pending && !_producer_queue.empty())
				_consumer_queue.merge_back(std::move(_producer_queue));
			while (!_waitlist_queue.empty() && _sq.pending < _sq.size && _cq.pending + _sq.pending < _cq.size)
				_waitlist_queue.pop_front()->notify();

			if (_consumer_queue.empty() || _sq.pending)
				uring_enter();
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

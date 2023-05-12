/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include "detail/config.hpp"

#ifdef ROD_HAS_COROUTINES

#include <atomic>

#include "scheduling.hpp"

namespace rod
{
	namespace _generator
	{
		template<typename T>
		class generator_iterator;
		template<typename T>
		class generator;

		template<typename T>
		class generator_promise
		{
		public:
			using value_type = std::remove_reference_t<T>;
			using reference = std::conditional_t<std::is_reference_v<T>, T, T &>;

		public:
			std::suspend_never await_transform(auto &&) = delete;

			[[nodiscard]] inline generator<T> get_return_object() noexcept;

			constexpr std::suspend_always initial_suspend() const noexcept { return {}; }
			constexpr std::suspend_always final_suspend() const noexcept { return {}; }

			void unhandled_exception() { m_err = std::current_exception(); }
			void rethrow_exception() { if (m_err) std::rethrow_exception(std::move(m_err)); }
			void return_void() {}

			std::suspend_always yield_value(value_type &value) noexcept requires (!std::is_rvalue_reference_v<T>)
			{
				m_result = std::addressof(value);
				return {};
			}
			std::suspend_always yield_value(value_type &&value) noexcept
			{
				m_result = std::addressof(value);
				return {};
			}

			reference value() const noexcept { return *static_cast<value_type *>(m_result); }

		private:
			std::exception_ptr m_err;
			void *m_result = nullptr;
		};

		struct sentinel {};
		template<typename T>
		class generator_iterator
		{
			friend class generator<T>;

		public:
			using value_type = typename generator_promise<T>::value_type;
			using reference = typename generator_promise<T>::reference;
			using pointer = std::add_pointer_t<value_type>;

			using iterator_category = std::input_iterator_tag;
			using difference_type = std::ptrdiff_t;

		private:
			generator_iterator(std::coroutine_handle<generator_promise<T>> h) noexcept : m_handle(h) { next(); }

		public:
			generator_iterator() noexcept = default;

			void operator++(int) { operator++(); }
			generator_iterator &operator++() { return (next(), *this); }

			[[nodiscard]] reference operator*() const noexcept { return m_handle.promise().value(); }
			[[nodiscard]] pointer operator->() const noexcept { return std::addressof(operator*()); }

			[[nodiscard]] friend bool operator==(const generator_iterator &i, sentinel) noexcept { return i.done(); }
			[[nodiscard]] friend bool operator==(sentinel, const generator_iterator &i) noexcept { return i.done(); }

		private:
			[[nodiscard]] bool done() const noexcept { return !m_handle || m_handle.done(); }

			void next()
			{
				if (m_handle)
				{
					m_handle.resume();
					if (m_handle.done()) m_handle.promise().rethrow_exception();
				}
			}

			std::coroutine_handle<generator_promise<T>> m_handle = {};
		};

		template<typename T>
		class generator
		{
			static_assert(!std::same_as<T, void>, "Cannot generate `void`");

		public:
			using promise_type = generator_promise<T>;
			using iterator = generator_iterator<T>;

		public:
			generator() = delete;
			generator(const generator &) = delete;
			generator &operator=(const generator &) = delete;

			explicit generator(std::coroutine_handle<promise_type> h) noexcept : m_handle(h) {}

			generator(generator &&other) noexcept : m_handle(std::exchange(other.m_handle, {})) {}
			generator &operator=(generator &&other) noexcept { return (swap(other), *this); }
			~generator() { if (m_handle) m_handle.destroy(); }

			/** Returns input iterator used to obtain values yielded by the generator. */
			[[nodiscard]] iterator begin() { return {m_handle}; }
			/** Returns sentinel for the `begin` iterator. */
			[[nodiscard]] sentinel end() noexcept { return {}; }

			void swap(generator &other) noexcept { std::swap(m_handle, other.m_handle); }
			friend void swap(generator &a, generator &b) noexcept { a.swap(b); }

		private:
			std::coroutine_handle<promise_type> m_handle;
		};

		template<typename T>
		generator<T> generator_promise<T>::get_return_object() noexcept { return generator<T>{std::coroutine_handle<generator_promise>::from_promise(*this)}; }
	}

	/** Generator range, whose iterator resumes the associated coroutine on increment and returns the yielded result on dereference.
	 * @note Generators must yield a value repeatedly, and as such cannot `co_return` anything other than `void`.
	 * @note Generators must yield synchronously, and as such cannot be suspended via `co_await`. */
	template<typename T>
	using generator = _generator::generator<T>;

	namespace _generator_task
	{
		class next_awaiter;
		class yield_awaiter;

		template<typename T>
		class generator_iterator;
		template<typename T>
		class generator_promise;
		template<typename T>
		class generator_task;

		class promise_base
		{
			friend class yield_awaiter;
			friend class next_awaiter;

			using stop_func = std::coroutine_handle<> (*)(void *) noexcept;

#ifndef ROD_HAS_INLINE_RESUME
			enum state_t : std::size_t
			{
				value_ready = 0b1'0000,
				consumer_running = 0b0'0001,
				producer_running = 0b0'0010,
				consumer_suspended = 0b0'0100,
				producer_suspended = 0b0'1000,
				cancelled = 31,
			};
#endif

		public:
			promise_base(const promise_base &) = delete;
			promise_base &operator=(const promise_base &) = delete;

			constexpr promise_base() noexcept = default;

			constexpr std::suspend_always initial_suspend() const noexcept { return {}; }
			inline yield_awaiter final_suspend() noexcept;
			constexpr void return_void() noexcept {}

			constexpr bool done() const noexcept { return m_result == nullptr; }
			void rethrow_exception() { if (m_err) std::rethrow_exception(std::move(m_err)); }
			void unhandled_exception() noexcept
			{
				m_err = std::current_exception();
#ifndef ROD_HAS_INLINE_RESUME
				/* Rethrow & terminate if an exception is received during a cancelled state,
				 * since the caller does not exist anymore and as such cannot handle the error. */
				if (m_state.load(std::memory_order_acquire) == cancelled)
					std::rethrow_exception(std::move(m_err));
#endif
			}

			[[nodiscard]] std::coroutine_handle<> unhandled_stopped() noexcept
			{
#ifndef ROD_HAS_INLINE_RESUME
				/* Since we are stopping, there is no ready value. */
				m_state.store(consumer_running, std::memory_order_release);
#endif
				return m_stop_func(m_consumer.address());
			}

#ifndef ROD_HAS_INLINE_RESUME
			bool cancel() noexcept { return m_state.exchange(cancelled, std::memory_order_acq_rel) == (value_ready | producer_suspended); }
#endif

		protected:
			inline yield_awaiter await_yield() noexcept;

			template<typename Other>
			void bind(std::coroutine_handle<Other> h) noexcept
			{
				if constexpr (requires { h.unhandled_stopped(); })
					m_stop_func = [](void *p) noexcept -> std::coroutine_handle<> { return std::coroutine_handle<Other>::from_address(p).promise().unhandled_stopped(); };
				else
					m_stop_func = [](void *) noexcept -> std::coroutine_handle<> { std::terminate(); };
			}

#ifndef ROD_HAS_INLINE_RESUME
			std::size_t resume_consumer(std::size_t state = (value_ready | consumer_running)) noexcept
			{
				m_state.store(state, std::memory_order_relaxed);
				m_consumer.resume();
				return m_state.load(std::memory_order_acquire);
			}

			std::atomic<std::size_t> m_state = value_ready | producer_suspended;
#endif

			std::coroutine_handle<> m_consumer = {};
			std::exception_ptr m_err = {};
			stop_func m_stop_func = {};
			void *m_result = {};
		};

		class next_awaiter
		{
#ifndef ROD_HAS_INLINE_RESUME
			using state_t = promise_base::state_t;
#endif

		protected:
			next_awaiter(promise_base &promise, std::coroutine_handle<> producer) noexcept : m_producer(producer), m_promise(&promise)
			{
#ifndef ROD_HAS_INLINE_RESUME
				auto state = promise.m_state.load(std::memory_order_acquire);
				if (state == (state_t::value_ready | state_t::producer_suspended))
					state = resume_producer();
				m_state = state;
#endif
			}

		public:
			constexpr next_awaiter() noexcept = default;

#ifdef ROD_HAS_INLINE_RESUME
			constexpr bool await_ready() const noexcept { return false; }
			template<typename P>
			std::coroutine_handle<> await_suspend(std::coroutine_handle<P> consumer) noexcept
			{
				m_promise->m_consumer = consumer;
				m_promise->bind(consumer);
				return m_producer;
			}
#else
			constexpr bool await_ready() const noexcept { return m_state == (state_t::value_ready | state_t::producer_suspended); }
			template<typename P>
			__declspec(noinline) bool await_suspend(std::coroutine_handle<P> consumer) noexcept
			{
				auto state = m_state;
				m_promise->m_consumer = consumer;
				m_promise->bind(consumer);

				if (state == (state_t::value_ready | state_t::producer_running))
				{
					if (m_promise->m_state.compare_exchange_strong(
							state, state_t::consumer_suspended,
							std::memory_order_release,
							std::memory_order_acquire))
						return true;

					state = resume_producer();
					if (state == (state_t::value_ready | state_t::producer_suspended))
						return false;
				}
				return m_promise->m_state.compare_exchange_strong(
						state, state_t::consumer_suspended,
						std::memory_order_release,
						std::memory_order_relaxed);
			}
#endif

		protected:
#ifndef ROD_HAS_INLINE_RESUME
			std::size_t resume_producer(std::size_t state = state_t::consumer_running) noexcept
			{
				m_promise->m_state.store(state, std::memory_order_relaxed);
				m_producer.resume();
				return m_promise->m_state.load(std::memory_order_acquire);
			}

			std::size_t m_state = {};
#endif

			std::coroutine_handle<> m_producer = {};
			promise_base *m_promise = nullptr;
		};

		class yield_awaiter
		{
#ifndef ROD_HAS_INLINE_RESUME
			using state_t = promise_base::state_t;
#endif

		public:
#ifdef ROD_HAS_INLINE_RESUME
			yield_awaiter(std::coroutine_handle<> consumer) noexcept : m_consumer(consumer) {}

			constexpr bool await_ready() const noexcept { return false; }
			std::coroutine_handle<> await_suspend(std::coroutine_handle<>) noexcept { return m_consumer; }
#else
			yield_awaiter(promise_base &promise, std::size_t state) noexcept : m_promise(&promise), m_state(state) {}

			constexpr bool await_ready() const noexcept { return m_state == state_t{}; }
			__declspec(noinline) bool await_suspend(std::coroutine_handle<> producer) noexcept
			{
				auto state = m_state;
				if (state == state_t::consumer_running)
				{
					if (m_promise->m_state.compare_exchange_strong(
							state, state_t::value_ready | state_t::producer_suspended,
							std::memory_order_release, std::memory_order_acquire))
						return true;

					if (state == state_t::consumer_suspended)
					{
						state = m_promise->resume_consumer();
						if (state == state_t::consumer_suspended)
							return false;
					}
				}

				if (state == (state_t::value_ready | state_t::producer_running))
				{
					/* Consumer is running and a value has been yielded, try to suspend the producer. */
					if (m_promise->m_state.compare_exchange_strong(
							state, state_t::value_ready | state_t::producer_suspended,
							std::memory_order_release, std::memory_order_acquire))
						return true;

					if (state == state_t::consumer_suspended)
						return false;
				}

				/* Generator object is destroyed, cleanup the coroutine. */
				producer.destroy();
				return true;
			}
#endif

			constexpr void await_resume() noexcept {}

		private:
#ifdef ROD_HAS_INLINE_RESUME
			std::coroutine_handle<> m_consumer;
#else
			promise_base *m_promise;
			std::size_t m_state;
#endif
		};

		template<typename T>
		class generator_promise : public promise_base
		{
		public:
			using value_type = std::remove_reference_t<T>;
			using reference = std::conditional_t<std::is_reference_v<T>, T, T &>;

		public:
			constexpr generator_promise() noexcept = default;

			inline generator_task<T> get_return_object() noexcept;

			template<typename A>
			[[nodiscard]] decltype(auto) await_transform(A &&value)
			{
				return as_awaitable(std::forward<A>(value), *this);
			}

			yield_awaiter yield_value(value_type &value) noexcept requires (!std::is_rvalue_reference_v<T>)
			{
				m_result = std::addressof(value);
				return await_yield();
			}
			yield_awaiter yield_value(value_type &&value) noexcept
			{
				m_result = std::addressof(value);
				return await_yield();
			}

			constexpr reference value() const noexcept { return static_cast<reference>(*static_cast<T *>(m_result)); }
		};

		template<typename T>
		class begin_awaiter : public next_awaiter
		{
			friend class generator_task<T>;

			begin_awaiter(std::coroutine_handle<generator_promise<T>> h) noexcept : next_awaiter(h.promise(), h) {}

		public:
			constexpr begin_awaiter() noexcept = default;

			bool await_ready() const noexcept { return !m_producer || next_awaiter::await_ready(); }

			inline generator_iterator<T> await_resume();
		};
		template<typename T>
		class iterator_awaiter : public next_awaiter
		{
			friend class generator_iterator<T>;

			iterator_awaiter(generator_iterator<T> &iter) noexcept : next_awaiter(iter.m_handle.promise(), iter.m_handle), m_iter(iter) {}

		public:
			inline generator_iterator<T> &await_resume();

		private:
			generator_iterator<T> &m_iter;
		};

		template<typename T>
		class generator_iterator
		{
			friend class begin_awaiter<T>;
			friend class iterator_awaiter<T>;

		public:
			using value_type = std::remove_reference_t<T>;
			using reference = std::add_lvalue_reference_t<T>;
			using pointer = std::add_pointer_t<value_type>;

			using iterator_category = std::input_iterator_tag;
			using difference_type = std::ptrdiff_t;

		private:
			generator_iterator(std::coroutine_handle<generator_promise<T>> h) noexcept : m_handle(h) {}

		public:
			constexpr generator_iterator() noexcept = default;

			iterator_awaiter<T> operator++() noexcept { return {*this}; }

			[[nodiscard]] reference operator*() const noexcept { return m_handle.promise().value(); }
			[[nodiscard]] pointer operator->() const noexcept { return std::addressof(operator*()); }

			[[nodiscard]] bool operator==(const generator_iterator &other) const noexcept { return m_handle == other.m_handle; }

		private:
			std::coroutine_handle<generator_promise<T>> m_handle = {};
		};

		template<typename T>
		class generator_task
		{
		public:
			using promise_type = generator_promise<T>;
			using iterator = generator_iterator<T>;

		public:
			generator_task() = delete;
			generator_task(const generator_task &) = delete;
			generator_task &operator=(const generator_task &) = delete;

			explicit generator_task(std::coroutine_handle<promise_type> h) noexcept : m_handle(h) {}

			generator_task(generator_task &&other) noexcept : m_handle(std::exchange(other.m_handle, {})) {}
			generator_task &operator=(generator_task &&other) noexcept { return (swap(other), *this); }


#ifdef ROD_HAS_INLINE_RESUME
			~generator_task() { if (m_handle) m_handle.destroy(); }
#else
			~generator_task() { if (m_handle && m_handle.promise().cancel()) m_handle.destroy(); }
#endif

			/** Returns awaiter object that suspends until a result has been yielded by the generator, then returns an input iterator. */
			[[nodiscard]] begin_awaiter<T> begin() noexcept { return m_handle ? begin_awaiter<T>{m_handle} : begin_awaiter<T>{}; }
			/** Returns sentinel for the iterator produced by the `begin` awaiter. */
			[[nodiscard]] iterator end() noexcept { return iterator{}; }

			void swap(generator_task &other) noexcept { std::swap(m_handle, other.m_handle); }
			friend void swap(generator_task &a, generator_task &b) noexcept { a.swap(b); }

		private:
			std::coroutine_handle<promise_type> m_handle;
		};

		yield_awaiter promise_base::await_yield() noexcept
		{
#ifdef ROD_HAS_INLINE_RESUME
			return yield_awaiter{m_consumer};
#else
			auto state = m_state.load(std::memory_order_acquire);
			if (state == state_t{}) state = resume_consumer();
			return yield_awaiter{*this, state};
#endif
		}
		yield_awaiter promise_base::final_suspend() noexcept
		{
			m_result = nullptr;
			return await_yield();
		}

		template<typename T>
		generator_task<T> generator_promise<T>::get_return_object() noexcept { return generator_task<T>{std::coroutine_handle<generator_promise>::from_promise(*this)}; }

		template<typename T>
		generator_iterator<T> begin_awaiter<T>::await_resume()
		{
			if (!m_promise)
				return generator_iterator<T>{};
			else if (m_promise->done())
			{
				m_promise->rethrow_exception();
				return generator_iterator<T>{};
			}
			return {std::coroutine_handle<generator_promise<T>>::from_promise(*static_cast<generator_promise<T> *>(m_promise))};
		}
		template<typename T>
		generator_iterator<T> &iterator_awaiter<T>::await_resume()
		{
			if (m_promise->done())
			{
				m_iter = generator_iterator<T>{};
				m_promise->rethrow_exception();
			}
			return m_iter;
		}
	}

	/** Generator task coroutine, whose iterator returns an awaitable on increment and returns the yielded result on dereference.
	 * @note Generator tasks must yield a value repeatedly, and as such cannot `co_return` anything other than `void`.
	 * @note Generator tasks return awaitable iterators, and as such can be suspended via `co_await`. */
	template<typename T>
	using generator_task = _generator_task::generator_task<T>;
}

#endif

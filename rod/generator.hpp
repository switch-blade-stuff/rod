/*
 * Created by switchblade on 2023-04-16.
 */

#pragma once

#include "detail/config.hpp"

#ifdef ROD_HAS_COROUTINES

#include "scheduling.hpp"

namespace rod
{
	namespace _generator
	{
		struct sentinel {};
		template<typename T>
		class iterator;
		template<typename T>
		class generator;

		template<typename T>
		class promise_base
		{
		public:
			using value_type = std::remove_reference_t<T>;
			using reference = std::conditional_t<std::is_reference_v<T>, T, T &>;

		public:
			std::suspend_never await_transform(auto &&) = delete;

			void unhandled_exception() { _err = std::current_exception(); }
			void rethrow_exception() { if (_err) std::rethrow_exception(_err); }

			constexpr std::suspend_always initial_suspend() const noexcept { return {}; }
			constexpr std::suspend_always final_suspend() const noexcept { return {}; }
			void return_void() {}

			std::suspend_always yield_value(typename promise_base<T>::value_type &&value) noexcept { return (_result = std::addressof(value), std::suspend_always{}); }
			std::suspend_always yield_value(typename promise_base<T>::value_type &value) noexcept requires(!std::is_rvalue_reference_v<T>) { return (_result = std::addressof(value), std::suspend_always{}); }

			reference value() const noexcept { return *static_cast<value_type *>(_result); }

		protected:
			void *_result = nullptr;
			std::exception_ptr _err;
		};

		template<typename T>
		class iterator
		{
			friend class generator<T>;

		public:
			using value_type = typename promise_base<T>::value_type;
			using reference = typename promise_base<T>::reference;
			using pointer = std::add_pointer_t<value_type>;

			using iterator_category = std::input_iterator_tag;
			using difference_type = std::ptrdiff_t;

		private:
			iterator(std::coroutine_handle<promise_base<T>> h) noexcept : _handle(h) { next(); }

		public:
			iterator() noexcept = default;

			void operator++(int) { operator++(); }
			iterator &operator++() { return (next(), *this); }

			[[nodiscard]] reference operator*() const noexcept { return _handle.promise().value(); }
			[[nodiscard]] pointer operator->() const noexcept { return std::addressof(operator*()); }

			[[nodiscard]] friend bool operator==(const iterator &i, sentinel) noexcept { return !i._handle || i._handle.done(); }
			[[nodiscard]] friend bool operator==(sentinel, const iterator &i) noexcept { return i == sentinel{}; }

		private:
			void next()
			{
				if (!_handle)
					return;

				_handle.resume();
				if (!_handle.done())
					return;

				_handle.promise().rethrow_exception();
			}

			std::coroutine_handle<promise_base<T>> _handle = {};
		};

		template<typename T, typename Alloc>
		struct promise : public promise_base<T>, public with_allocator_promise<promise<T, Alloc>, Alloc>
		{
			using with_allocator_promise<promise<T, Alloc>, Alloc>::operator new;
			using with_allocator_promise<promise<T, Alloc>, Alloc>::operator delete;

			[[nodiscard]] inline generator<T> get_return_object() noexcept;
		};

		template<typename T>
		class generator
		{
			static_assert(!std::is_void_v<T>, "Cannot generate `void`");

		public:
			using promise_type = promise<T, std::allocator<void>>;
			template<typename Alloc>
			using allocator_promise_type = promise<T, Alloc>;

			using iterator = iterator<T>;

		public:
			generator() = delete;
			generator(const generator &) = delete;
			generator &operator=(const generator &) = delete;

			generator(generator &&other) noexcept : generator(other.release()) {}
			generator &operator=(generator &&other) noexcept { return (swap(other), *this); }

			template<std::derived_from<promise_base<T>> P>
			constexpr explicit generator(std::coroutine_handle<P> h) noexcept : _handle(std::coroutine_handle<promise_base<T>>::from_address(h.address())) {}
			constexpr explicit generator(std::coroutine_handle<void> h) noexcept : _handle(std::coroutine_handle<promise_base<T>>::from_address(h.address())) {}

			~generator() { if (_handle) _handle.destroy(); }

			/** Returns input iterator used to obtain values yielded by the generator. */
			[[nodiscard]] iterator begin() { return {_handle}; }
			/** Returns sentinel for the `begin` iterator. */
			[[nodiscard]] sentinel end() noexcept { return {}; }

			/** Releases the underlying coroutine handle. */
			std::coroutine_handle<> release() { return std::exchange(_handle, std::coroutine_handle<promise_base<T>>{}); }

			constexpr void swap(generator &other) noexcept { std::swap(_handle, other._handle); }
			friend constexpr void swap(generator &a, generator &b) noexcept { a.swap(b); }

		private:
			std::coroutine_handle<promise_base<T>> _handle;
		};

		template<typename T, typename Alloc>
		generator<T> promise<T, Alloc>::get_return_object() noexcept { return generator<T>{std::coroutine_handle<promise>::from_promise(*this)}; }
	}

	/** Generator range, whose iterator resumes the associated coroutine on increment and returns the yielded result on dereference.
	 * @note Generators must yield a value repeatedly, and as such cannot `co_return` anything other than `void`.
	 * @note Generators must yield synchronously, and as such cannot be suspended via `co_await`. */
	template<typename T>
	using generator = _generator::generator<T>;

	namespace _generator_task
	{
		template<typename T>
		class next_awaiter;
		template<typename T>
		class yield_awaiter;
		template<typename T>
		class iterator;

		template<typename T>
		class generator_task;

		template<typename T>
		class promise_base
		{
			template<typename>
			friend class yield_awaiter;
			template<typename>
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
			using value_type = std::remove_reference_t<T>;
			using reference = std::conditional_t<std::is_reference_v<T>, T, T &>;

		public:
			promise_base(const promise_base &) = delete;
			promise_base &operator=(const promise_base &) = delete;

			constexpr promise_base() noexcept = default;

			template<typename A>
			[[nodiscard]] decltype(auto) await_transform(A &&value) { return as_awaitable(std::forward<A>(value), *this); }

			constexpr std::suspend_always initial_suspend() const noexcept { return {}; }
			inline yield_awaiter<T> final_suspend() noexcept;
			constexpr void return_void() noexcept {}

			constexpr bool done() const noexcept { return _result == nullptr; }
			void rethrow_exception() { if (_err) std::rethrow_exception(std::move(_err)); }
			void unhandled_exception() noexcept
			{
				_err = std::current_exception();
#ifndef ROD_HAS_INLINE_RESUME
				/* Rethrow & terminate if an exception is received during a cancelled state,
				 * since the caller does not exist anymore and as such cannot handle the error. */
				if (_state.load(std::memory_order_acquire) == cancelled)
					std::rethrow_exception(std::move(_err));
#endif
			}

			[[nodiscard]] std::coroutine_handle<> unhandled_stopped() noexcept
			{
#ifndef ROD_HAS_INLINE_RESUME
				/* Since we are stopping, there is no ready value. */
				_state.store(consumer_running, std::memory_order_release);
#endif
				return _stop_func(_consumer.address());
			}

			yield_awaiter<T> yield_value(value_type &&value) noexcept { return (_result = std::addressof(value), await_yield()); }
			yield_awaiter<T> yield_value(value_type &value) noexcept requires(!std::is_rvalue_reference_v<T>) { return (_result = std::addressof(value), await_yield()); }

			reference value() const noexcept { return static_cast<reference>(*static_cast<T *>(_result)); }

#ifndef ROD_HAS_INLINE_RESUME
			bool cancel() noexcept { return _state.exchange(cancelled, std::memory_order_acq_rel) == (value_ready | producer_suspended); }
#endif

		protected:
			inline yield_awaiter<T> await_yield() noexcept;

			template<typename Other>
			void bind(std::coroutine_handle<Other> h) noexcept
			{
				if constexpr (requires { h.unhandled_stopped(); })
					_stop_func = [](void *p) noexcept -> std::coroutine_handle<> { return std::coroutine_handle<Other>::from_address(p).promise().unhandled_stopped(); };
				else
					_stop_func = [](void *) noexcept -> std::coroutine_handle<> { std::terminate(); };
			}

#ifndef ROD_HAS_INLINE_RESUME
			std::size_t resume_consumer(std::size_t state = (value_ready | consumer_running)) noexcept
			{
				_state.store(state, std::memory_order_relaxed);
				_consumer.resume();
				return _state.load(std::memory_order_acquire);
			}

			std::atomic<std::size_t> _state = value_ready | producer_suspended;
#endif

			std::coroutine_handle<> _consumer = {};
			std::exception_ptr _err = {};
			stop_func _stop_func = {};
			void *_result = {};
		};

		template<typename T>
		class next_awaiter
		{
#ifndef ROD_HAS_INLINE_RESUME
			using state_t = typename promise_base<T>::state_t;
#endif

		protected:
			next_awaiter(promise_base<T> &promise, std::coroutine_handle<> producer) noexcept : _producer(producer), _promise(&promise)
			{
#ifndef ROD_HAS_INLINE_RESUME
				auto state = promise._state.load(std::memory_order_acquire);
				if (state == (state_t::value_ready | state_t::producer_suspended))
					state = resume_producer();
				_state = state;
#endif
			}

		public:
			constexpr next_awaiter() noexcept = default;

#ifdef ROD_HAS_INLINE_RESUME
			constexpr bool await_ready() const noexcept { return false; }
			template<typename P>
			std::coroutine_handle<> await_suspend(std::coroutine_handle<P> consumer) noexcept
			{
				_promise->_consumer = consumer;
				_promise->bind(consumer);
				return _producer;
			}
#else
			constexpr bool await_ready() const noexcept { return _state == (state_t::value_ready | state_t::producer_suspended); }
			template<typename P>
			__declspec(noinline) bool await_suspend(std::coroutine_handle<P> consumer) noexcept
			{
				auto state = _state;
				_promise->_consumer = consumer;
				_promise->bind(consumer);

				if (state == (state_t::value_ready | state_t::producer_running))
				{
					if (_promise->_state.compare_exchange_strong(
							state, state_t::consumer_suspended,
							std::memory_order_release,
							std::memory_order_acquire))
						return true;

					state = resume_producer();
					if (state == (state_t::value_ready | state_t::producer_suspended))
						return false;
				}
				return _promise->_state.compare_exchange_strong(
						state, state_t::consumer_suspended,
						std::memory_order_release,
						std::memory_order_relaxed);
			}
#endif

		protected:
#ifndef ROD_HAS_INLINE_RESUME
			std::size_t resume_producer(std::size_t state = state_t::consumer_running) noexcept
			{
				_promise->_state.store(state, std::memory_order_relaxed);
				_producer.resume();
				return _promise->_state.load(std::memory_order_acquire);
			}

			std::size_t _state = {};
#endif

			std::coroutine_handle<> _producer = {};
			promise_base<T> *_promise = nullptr;
		};

		template<typename T>
		class yield_awaiter
		{
#ifndef ROD_HAS_INLINE_RESUME
			using state_t = typename promise_base<T>::state_t;
#endif

		public:
#ifdef ROD_HAS_INLINE_RESUME
			yield_awaiter(std::coroutine_handle<> consumer) noexcept : _consumer(consumer) {}

			constexpr bool await_ready() const noexcept { return false; }
			std::coroutine_handle<> await_suspend(std::coroutine_handle<>) noexcept { return _consumer; }
#else
			yield_awaiter(promise_base<T> &promise, std::size_t state) noexcept : _promise(&promise), _state(state) {}

			constexpr bool await_ready() const noexcept { return _state == state_t{}; }
			__declspec(noinline) bool await_suspend(std::coroutine_handle<> producer) noexcept
			{
				auto state = _state;
				if (state == state_t::consumer_running)
				{
					if (_promise->_state.compare_exchange_strong(
							state, state_t::value_ready | state_t::producer_suspended,
							std::memory_order_release, std::memory_order_acquire))
						return true;

					if (state == state_t::consumer_suspended)
					{
						state = _promise->resume_consumer();
						if (state == state_t::consumer_suspended)
							return false;
					}
				}

				if (state == (state_t::value_ready | state_t::producer_running))
				{
					/* Consumer is running and a value has been yielded, try to suspend the producer. */
					if (_promise->_state.compare_exchange_strong(
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
			std::coroutine_handle<> _consumer;
#else
			promise_base<T> *_promise;
			std::size_t _state;
#endif
		};

		template<typename T>
		class begin_awaiter : public next_awaiter<T>
		{
		public:
			constexpr begin_awaiter() noexcept = default;
			explicit begin_awaiter(std::coroutine_handle<promise_base<T>> h) noexcept : next_awaiter<T>(h.promise(), h) {}

			bool await_ready() const noexcept { return !next_awaiter<T>::_producer || next_awaiter<T>::await_ready(); }

			inline iterator<T> await_resume();
		};
		template<typename T>
		class iterator_awaiter : public next_awaiter<T>
		{
		public:
			explicit iterator_awaiter(iterator<T> &iter) noexcept : next_awaiter<T>(iter._handle.promise(), iter._handle), _iter(iter) {}

			inline iterator<T> &await_resume();

		private:
			iterator<T> &_iter;
		};

		template<typename T>
		class iterator
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
			iterator(std::coroutine_handle<promise_base<T>> h) noexcept : _handle(h) {}

		public:
			constexpr iterator() noexcept = default;

			iterator_awaiter<T> operator++() noexcept { return iterator_awaiter<T>{*this}; }

			[[nodiscard]] reference operator*() const noexcept { return _handle.promise().value(); }
			[[nodiscard]] pointer operator->() const noexcept { return std::addressof(operator*()); }

			[[nodiscard]] friend constexpr bool operator==(const iterator &, const iterator &) noexcept = default;

		private:
			std::coroutine_handle<promise_base<T>> _handle = {};
		};

		template<typename T, typename Alloc>
		struct promise : public promise_base<T>, public with_allocator_promise<promise<T, Alloc>, Alloc>
		{
			using with_allocator_promise<promise<T, Alloc>, Alloc>::operator new;
			using with_allocator_promise<promise<T, Alloc>, Alloc>::operator delete;

			inline generator_task<T> get_return_object() noexcept;
		};

		template<typename T>
		class generator_task
		{
			static_assert(!std::is_void_v<T>, "Cannot generate `void`");

		public:
			using promise_type = promise<T, std::allocator<void>>;
			template<typename Alloc>
			using allocator_promise_type = promise<T, Alloc>;

			using iterator = iterator<T>;

		public:
			generator_task() = delete;
			generator_task(const generator_task &) = delete;
			generator_task &operator=(const generator_task &) = delete;

			generator_task(generator_task &&other) noexcept : generator_task(other.release()) {}
			generator_task &operator=(generator_task &&other) noexcept { return (swap(other), *this); }

			template<std::derived_from<promise_base<T>> P>
			constexpr explicit generator_task(std::coroutine_handle<P> h) noexcept : _handle(std::coroutine_handle<promise_base<T>>::from_address(h.address())) {}
			constexpr explicit generator_task(std::coroutine_handle<void> h) noexcept : _handle(std::coroutine_handle<promise_base<T>>::from_address(h.address())) {}

#ifdef ROD_HAS_INLINE_RESUME
			~generator_task() { if (_handle) _handle.destroy(); }
#else
			~generator_task() { if (_handle && _handle.promise().cancel()) _handle.destroy(); }
#endif

			/** Returns awaiter object that suspends until a result has been yielded by the generator, then returns an input iterator. */
			[[nodiscard]] begin_awaiter<T> begin() noexcept { return _handle ? begin_awaiter<T>{_handle} : begin_awaiter<T>{}; }
			/** Returns sentinel for the iterator produced by the `begin` awaiter. */
			[[nodiscard]] iterator end() noexcept { return iterator{}; }

			/** Releases the underlying coroutine handle. */
			std::coroutine_handle<> release() { return std::exchange(_handle, std::coroutine_handle<promise_base<T>>{}); }

			constexpr void swap(generator_task &other) noexcept { std::swap(_handle, other._handle); }
			friend constexpr void swap(generator_task &a, generator_task &b) noexcept { a.swap(b); }

		private:
			std::coroutine_handle<promise_base<T>> _handle;
		};

		template<typename T>
		yield_awaiter<T> promise_base<T>::await_yield() noexcept
		{
#ifdef ROD_HAS_INLINE_RESUME
			return yield_awaiter<T>{_consumer};
#else
			auto state = _state.load(std::memory_order_acquire);
			if (state == state_t{}) state = resume_consumer();
			return yield_awaiter<T>{*this, state};
#endif
		}
		template<typename T>
		yield_awaiter<T> promise_base<T>::final_suspend() noexcept
		{
			_result = nullptr;
			return await_yield();
		}

		template<typename T>
		iterator<T> begin_awaiter<T>::await_resume()
		{
			if (!next_awaiter<T>::_promise)
				return iterator<T>{};
			else if (next_awaiter<T>::_promise->done())
			{
				next_awaiter<T>::_promise->rethrow_exception();
				return iterator<T>{};
			}
			return iterator<T>{std::coroutine_handle<promise_base<T>>::from_promise(*next_awaiter<T>::_promise)};
		}
		template<typename T>
		iterator<T> &iterator_awaiter<T>::await_resume()
		{
			if (next_awaiter<T>::_promise->done())
			{
				_iter = iterator<T>{};
				next_awaiter<T>::_promise->rethrow_exception();
			}
			return _iter;
		}

		template<typename T, typename Alloc>
		generator_task<T> promise<T, Alloc>::get_return_object() noexcept { return generator_task<T>{std::coroutine_handle<promise>::from_promise(*this)}; }
	}

	/** Generator task coroutine, whose iterator returns an awaitable on increment and returns the yielded result on dereference.
	 * @note Generator tasks must yield a value repeatedly, and as such cannot `co_return` anything other than `void`.
	 * @note Generator tasks return awaitable iterators, and as such can be suspended via `co_await`. */
	template<typename T>
	using generator_task = _generator_task::generator_task<T>;
}
#endif

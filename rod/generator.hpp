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

			void unhandled_exception() { _err = std::current_exception(); }
			void rethrow_exception() { if (_err) std::rethrow_exception(std::move(_err)); }
			void return_void() {}

			std::suspend_always yield_value(value_type &value) noexcept requires(!std::is_rvalue_reference_v<T>)
			{
				_result = std::addressof(value);
				return {};
			}
			std::suspend_always yield_value(value_type &&value) noexcept
			{
				_result = std::addressof(value);
				return {};
			}

			reference value() const noexcept { return *static_cast<value_type *>(_result); }

		private:
			std::exception_ptr _err;
			void *_result = nullptr;
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
			generator_iterator(std::coroutine_handle<generator_promise<T>> h) noexcept : _handle(h) { next(); }

		public:
			generator_iterator() noexcept = default;

			void operator++(int) { operator++(); }
			generator_iterator &operator++() { return (next(), *this); }

			[[nodiscard]] reference operator*() const noexcept { return _handle.promise().value(); }
			[[nodiscard]] pointer operator->() const noexcept { return std::addressof(operator*()); }

			[[nodiscard]] friend bool operator==(const generator_iterator &i, sentinel) noexcept { return i.done(); }
			[[nodiscard]] friend bool operator==(sentinel, const generator_iterator &i) noexcept { return i.done(); }

		private:
			[[nodiscard]] bool done() const noexcept { return !_handle || _handle.done(); }

			void next()
			{
				if (_handle)
				{
					_handle.resume();
					if (_handle.done()) _handle.promise().rethrow_exception();
				}
			}

			std::coroutine_handle<generator_promise<T>> _handle = {};
		};

		template<typename T>
		class generator
		{
			template<typename, typename...>
			friend struct std::coroutine_traits;

			static_assert(!std::same_as<T, void>, "Cannot generate `void`");

		public:
			using promise_type = generator_promise<T>;
			using iterator = generator_iterator<T>;

		public:
			generator() = delete;
			generator(const generator &) = delete;
			generator &operator=(const generator &) = delete;

			generator(generator &&other) noexcept : _handle(std::exchange(other._handle, {})) {}
			generator &operator=(generator &&other) noexcept { return (swap(other), *this); }

			template<std::derived_from<promise_type> P>
			constexpr explicit generator(std::coroutine_handle<P> h) noexcept : _handle(std::coroutine_handle<promise_type>::from_address(h.address())) {}

			~generator() { if (_handle) _handle.destroy(); }

			/** Returns input iterator used to obtain values yielded by the generator. */
			[[nodiscard]] iterator begin() { return {_handle}; }
			/** Returns sentinel for the `begin` iterator. */
			[[nodiscard]] sentinel end() noexcept { return {}; }

			constexpr void swap(generator &other) noexcept { std::swap(_handle, other._handle); }
			friend constexpr void swap(generator &a, generator &b) noexcept { a.swap(b); }

		private:
			std::coroutine_handle<promise_type> _handle;
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
			friend yield_awaiter;
			friend next_awaiter;

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

#ifndef ROD_HAS_INLINE_RESUME
			bool cancel() noexcept { return _state.exchange(cancelled, std::memory_order_acq_rel) == (value_ready | producer_suspended); }
#endif

		protected:
			inline yield_awaiter await_yield() noexcept;

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

		class next_awaiter
		{
#ifndef ROD_HAS_INLINE_RESUME
			using state_t = promise_base::state_t;
#endif

		protected:
			next_awaiter(promise_base &promise, std::coroutine_handle<> producer) noexcept : _producer(producer), _promise(&promise)
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
			promise_base *_promise = nullptr;
		};

		class yield_awaiter
		{
#ifndef ROD_HAS_INLINE_RESUME
			using state_t = promise_base::state_t;
#endif

		public:
#ifdef ROD_HAS_INLINE_RESUME
			yield_awaiter(std::coroutine_handle<> consumer) noexcept : _consumer(consumer) {}

			constexpr bool await_ready() const noexcept { return false; }
			std::coroutine_handle<> await_suspend(std::coroutine_handle<>) noexcept { return _consumer; }
#else
			yield_awaiter(promise_base &promise, std::size_t state) noexcept : _promise(&promise), _state(state) {}

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
			promise_base *_promise;
			std::size_t _state;
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

			yield_awaiter yield_value(value_type &value) noexcept requires(!std::is_rvalue_reference_v<T>)
			{
				_result = std::addressof(value);
				return await_yield();
			}
			yield_awaiter yield_value(value_type &&value) noexcept
			{
				_result = std::addressof(value);
				return await_yield();
			}

			constexpr reference value() const noexcept { return static_cast<reference>(*static_cast<T *>(_result)); }
		};

		template<typename T>
		class begin_awaiter : public next_awaiter
		{
			friend class generator_task<T>;

			begin_awaiter(std::coroutine_handle<generator_promise<T>> h) noexcept : next_awaiter(h.promise(), h) {}

		public:
			constexpr begin_awaiter() noexcept = default;

			bool await_ready() const noexcept { return !_producer || next_awaiter::await_ready(); }

			inline generator_iterator<T> await_resume();
		};
		template<typename T>
		class iterator_awaiter : public next_awaiter
		{
			friend class generator_iterator<T>;

			iterator_awaiter(generator_iterator<T> &iter) noexcept : next_awaiter(iter._handle.promise(), iter._handle), _iter(iter) {}

		public:
			inline generator_iterator<T> &await_resume();

		private:
			generator_iterator<T> &_iter;
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
			generator_iterator(std::coroutine_handle<generator_promise<T>> h) noexcept : _handle(h) {}

		public:
			constexpr generator_iterator() noexcept = default;

			iterator_awaiter<T> operator++() noexcept { return {*this}; }

			[[nodiscard]] reference operator*() const noexcept { return _handle.promise().value(); }
			[[nodiscard]] pointer operator->() const noexcept { return std::addressof(operator*()); }

			[[nodiscard]] bool operator==(const generator_iterator &other) const noexcept { return _handle == other._handle; }

		private:
			std::coroutine_handle<generator_promise<T>> _handle = {};
		};

		template<typename T>
		class generator_task
		{
			template<typename, typename...>
			friend struct std::coroutine_traits;

		public:
			using promise_type = generator_promise<T>;
			using iterator = generator_iterator<T>;

		public:
			generator_task() = delete;
			generator_task(const generator_task &) = delete;
			generator_task &operator=(const generator_task &) = delete;

			generator_task(generator_task &&other) noexcept : _handle(std::exchange(other._handle, {})) {}
			generator_task &operator=(generator_task &&other) noexcept { return (swap(other), *this); }

			template<std::derived_from<promise_type> P>
			constexpr explicit generator_task(std::coroutine_handle<P> h) noexcept : _handle(std::coroutine_handle<promise_type>::from_address(h.address())) {}

#ifdef ROD_HAS_INLINE_RESUME
			~generator_task() { if (_handle) _handle.destroy(); }
#else
			~generator_task() { if (_handle && _handle.promise().cancel()) _handle.destroy(); }
#endif

			/** Returns awaiter object that suspends until a result has been yielded by the generator, then returns an input iterator. */
			[[nodiscard]] begin_awaiter<T> begin() noexcept { return _handle ? begin_awaiter<T>{_handle} : begin_awaiter<T>{}; }
			/** Returns sentinel for the iterator produced by the `begin` awaiter. */
			[[nodiscard]] iterator end() noexcept { return iterator{}; }

			constexpr void swap(generator_task &other) noexcept { std::swap(_handle, other._handle); }
			friend constexpr void swap(generator_task &a, generator_task &b) noexcept { a.swap(b); }

		private:
			std::coroutine_handle<promise_type> _handle;
		};

		yield_awaiter promise_base::await_yield() noexcept
		{
#ifdef ROD_HAS_INLINE_RESUME
			return yield_awaiter{_consumer};
#else
			auto state = _state.load(std::memory_order_acquire);
			if (state == state_t{}) state = resume_consumer();
			return yield_awaiter{*this, state};
#endif
		}
		yield_awaiter promise_base::final_suspend() noexcept
		{
			_result = nullptr;
			return await_yield();
		}

		template<typename T>
		generator_task<T> generator_promise<T>::get_return_object() noexcept { return generator_task<T>{std::coroutine_handle<generator_promise>::from_promise(*this)}; }

		template<typename T>
		generator_iterator<T> begin_awaiter<T>::await_resume()
		{
			if (!_promise)
				return generator_iterator<T>{};
			else if (_promise->done())
			{
				_promise->rethrow_exception();
				return generator_iterator<T>{};
			}
			return {std::coroutine_handle<generator_promise<T>>::from_promise(*static_cast<generator_promise<T> *>(_promise))};
		}
		template<typename T>
		generator_iterator<T> &iterator_awaiter<T>::await_resume()
		{
			if (_promise->done())
			{
				_iter = generator_iterator<T>{};
				_promise->rethrow_exception();
			}
			return _iter;
		}
	}

	/** Generator task coroutine, whose iterator returns an awaitable on increment and returns the yielded result on dereference.
	 * @note Generator tasks must yield a value repeatedly, and as such cannot `co_return` anything other than `void`.
	 * @note Generator tasks return awaitable iterators, and as such can be suspended via `co_await`. */
	template<typename T>
	using generator_task = _generator_task::generator_task<T>;
}

template<typename T, typename... Args>
struct std::coroutine_traits<rod::generator<T>, Args...> { using promise_type = typename rod::generator<T>::promise_type; };
template<typename T, typename Alloc, typename... Args>
struct std::coroutine_traits<rod::generator<T>, std::allocator_arg_t, Alloc, Args...> { using promise_type = rod::detail::promise_with_allocator<typename rod::generator<T>::promise_type, std::decay_t<Alloc>>; };
template<typename T, typename U, typename Alloc, typename... Args>
struct std::coroutine_traits<rod::generator<T>, U, std::allocator_arg_t, Alloc, Args...> { using promise_type = rod::detail::promise_with_allocator<typename rod::generator<T>::promise_type, std::decay_t<Alloc>>; };

template<typename T, typename... Args>
struct std::coroutine_traits<rod::generator_task<T>, Args...> { using promise_type = typename rod::generator_task<T>::promise_type; };
template<typename T, typename Alloc, typename... Args>
struct std::coroutine_traits<rod::generator_task<T>, std::allocator_arg_t, Alloc, Args...> { using promise_type = rod::detail::promise_with_allocator<typename rod::generator_task<T>::promise_type, std::decay_t<Alloc>>; };
template<typename T, typename U, typename Alloc, typename... Args>
struct std::coroutine_traits<rod::generator_task<T>, U, std::allocator_arg_t, Alloc, Args...> { using promise_type = rod::detail::promise_with_allocator<typename rod::generator_task<T>::promise_type, std::decay_t<Alloc>>; };

#endif

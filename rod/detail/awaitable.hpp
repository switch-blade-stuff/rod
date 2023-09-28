/*
 * Created by switchblade on 2023-04-09.
 */

#pragma once

#include "config.hpp"

#ifdef ROD_HAS_COROUTINES

#include <coroutine>
#include <new>

#include "queries/completion.hpp"
#include "async_base.hpp"

namespace rod
{
	namespace _detail
	{
		template<typename P, typename A>
		concept has_await_transform = requires(P p, A a) { p.await_transform(a); };

		template<typename T>
		concept has_member_await = requires(T &&value) { static_cast<T &&>(value).operator co_await(); };
		template<typename T>
		concept has_static_await = requires(T &&value) { operator co_await(static_cast<T &&>(value)); };

		template<has_member_await T>
		constexpr decltype(auto) get_awaiter(T &&value, auto *) noexcept(noexcept(static_cast<T &&>(value).operator co_await()))
		{
			return static_cast<T &&>(value).operator co_await();
		}
		template<has_static_await T> requires(!has_member_await<T>)
		constexpr decltype(auto) get_awaiter(T &&value, auto *) noexcept(noexcept(operator co_await(static_cast<T &&>(value))))
		{
			return operator co_await(static_cast<T &&>(value));
		}
		template<typename T> requires(!has_member_await<T> && !has_static_await<T>)
		constexpr T &&get_awaiter(T &&value, auto *) noexcept { return static_cast<T &&>(value); }

		template<typename T>
		concept await_suspend_result = std::same_as<T, void> || std::same_as<T, bool> || instance_of<T, std::coroutine_handle>;

		template<typename A, typename P>
		concept with_await_suspend = std::same_as<P, void> || requires(A &a, std::coroutine_handle<P> h)
		{
			{ a.await_suspend(h) } -> await_suspend_result;
		};
		template<typename A, typename P>
		concept is_awaiter = requires(A &a)
		{
			/* Use a ternary to allow for non-constexpr evaluation. */
			a.await_ready() ? true : false;
			a.await_resume();
		} && with_await_suspend<A, P>;

		template<typename A, typename P = void>
		concept is_awaitable = requires (A &&a, P *p) { { get_awaiter(std::forward<A>(a), p) } -> is_awaiter<P>; };
		template<typename A, typename P = void>
		using await_result_type = decltype(get_awaiter(std::declval<A>(), std::declval<P *>()).await_resume());

		struct undefined_promise
		{
			inline std::coroutine_handle<> unhandled_stopped() noexcept;

			inline undefined_promise get_return_object() noexcept;
			inline undefined_promise initial_suspend() noexcept;
			inline undefined_promise final_suspend() noexcept;
			inline void unhandled_exception() noexcept;
			inline void return_void() noexcept;
		};

		template<typename P = undefined_promise>
		struct undefined_coroutine : std::coroutine_handle<P> { using promise_type = P; };
	}

	namespace _as_awaitable
	{
		struct unit {};
		template<typename S, typename P>
		using value_t = _detail::single_sender_value_type<S, env_of_t<P>>;
		template<typename S, typename P>
		using result_or_unit = std::conditional_t<std::is_void_v<value_t<S, P>>, unit, value_t<S, P>>;

		template<typename, typename>
		struct awaitable { class type; };
		template<typename, typename>
		struct receiver { class type; };

		template<typename S, typename P>
		class receiver<S, P>::type
		{
			friend class awaitable<S, P>::type;

			using result_t = std::variant<std::monostate, result_or_unit<S, P>, std::exception_ptr>;
			using handle_t = std::coroutine_handle<P>;

			constexpr type(result_t *result, handle_t handle) : _result_ptr(result), _cont_handle(handle) {}

		public:
			using is_receiver = std::true_type;

			friend env_of_t<P> tag_invoke(get_env_t, const type &r) noexcept(_detail::nothrow_callable<get_env_t, const P &>) { return get_env(std::as_const(r._cont_handle.promise())); }

			template<typename... Vs> requires std::constructible_from<result_or_unit<S, P>, Vs...>
			friend void tag_invoke(set_value_t, type &&r, Vs &&...vs) noexcept
			{
				try { r._result_ptr->template emplace<1>(std::forward<Vs>(vs)...); }
				catch (...) { r._result_ptr->template emplace<2>(std::current_exception()); }
				r._cont_handle.resume();
			}
			template<typename Err>
			friend void tag_invoke(set_error_t, type &&r, Err &&err) noexcept
			{
				r._result_ptr->template emplace<2>(_detail::to_except_ptr(std::forward<Err>(err)));
				r._cont_handle.resume();
			}
			friend void tag_invoke(set_stopped_t, type &&r) noexcept
			{
				static_cast<std::coroutine_handle<>>(r._cont_handle.promise().unhandled_stopped()).resume();
			}

		private:
			result_t *_result_ptr;
			handle_t _cont_handle;
		};

		template<typename S, typename P>
		class awaitable<S, P>::type
		{
			using result_t = std::variant<std::monostate, result_or_unit<S, P>, std::exception_ptr>;
			using receiver_t = typename receiver<S, P>::type;

		public:
			constexpr type(S &&s, std::coroutine_handle<P> h) : _state(connect(std::forward<S>(s), receiver_t{&_result, h})) {}

			[[nodiscard]] constexpr bool await_ready() const noexcept { return false; }
			void await_suspend(std::coroutine_handle<P>) noexcept { start(_state); }

			constexpr value_t<S, P> await_resume()
			{
				switch (_result.index())
				{
				case 1:
					if constexpr (!std::is_void_v<value_t<S, P>>)
						return std::forward<value_t<S, P>>(std::get<1>(_result));
					else
						return;
				case 2: std::rethrow_exception(std::get<2>(_result));
				default: [[unlikely]] std::terminate();
				}
			}

		private:
			connect_result_t<S, receiver_t> _state;
			result_t _result = {};
		};

		template<typename S, typename P>
		concept awaitable_sender = _detail::single_sender<S, env_of_t<P>> && sender_to<S, typename receiver<S, P>::type> && requires(P &p)
		{
			{ p.unhandled_stopped() } -> std::convertible_to<std::coroutine_handle<>>;
		};

		class as_awaitable_t
		{
			template<typename T, typename P>
			using awaitable_t = typename awaitable<T, P>::type;

		public:
			template<typename T, typename P> requires tag_invocable<as_awaitable_t, T, P &>
			[[nodiscard]] constexpr _detail::is_awaitable decltype(auto) operator()(T &&t, P &p) const noexcept(nothrow_tag_invocable<as_awaitable_t, T, P &>)
			{
				return tag_invoke(*this, std::forward<T>(t), p);
			}
			template<typename T, typename P> requires(!tag_invocable<as_awaitable_t, T, P &> && !_detail::is_awaitable<T, _detail::undefined_promise> && awaitable_sender<T, P>)
			[[nodiscard]] constexpr awaitable_t<T, P> operator()(T &&t, P &p) const noexcept(std::is_nothrow_constructible_v<awaitable_t<T, P>, T, std::coroutine_handle<P>>)
			{
				return awaitable_t<T, P>{std::forward<T>(t), std::coroutine_handle<P>::from_promise(p)};
			}
			template<typename T, typename P>
			[[nodiscard]] constexpr _detail::is_awaitable decltype(auto) operator()(T &&t, P &) const noexcept { return std::forward<T>(t); }
		};
	}

	using _as_awaitable::as_awaitable_t;

	/** @brief Utility used to transform a sender into a form awaitable within a coroutine.
	 *
	 * Given awaitable `a` resulting from expression `as_awaitable(snd, prm)`, if \a snd completes via the value channel, `a.await_resume()`
	 * will return the stored result; otherwise, if \a snd completes via the error channel, it will re-throw the stored exception.
	 * `a.await_suspend()` invokes `start` CBO on the internal connection state, and `a.await_ready()` always returns `false`.
	 *
	 * @param snd Sender to transform into an awaitable.
	 * @param prm Coroutine promise to use for the sender awaitable.
	 * @return Awaitable object used to suspend execution until completion of \a snd. */
	inline constexpr auto as_awaitable = as_awaitable_t{};

	namespace _detail
	{
		template<typename Child>
		struct with_await_transform
		{
			template<typename T> requires tag_invocable<as_awaitable_t, T, Child &>
			constexpr decltype(auto) await_transform(T &&value) noexcept(nothrow_tag_invocable<as_awaitable_t, T, Child &>)
			{
				return tag_invoke(as_awaitable, std::forward<T>(value), static_cast<Child &>(*this));
			}
			template<typename T>
			constexpr T &&await_transform(T &&value) noexcept { return std::forward<T>(value); }
		};
		template<typename E>
		struct env_promise : with_await_transform<env_promise<E>>
		{
			friend inline const E &tag_invoke(get_env_t, const env_promise &e) noexcept { return e._env; }

			inline undefined_coroutine<> get_return_object() noexcept;
			inline std::suspend_always initial_suspend() noexcept;
			inline std::suspend_always final_suspend() noexcept;

			inline std::coroutine_handle<> unhandled_stopped() noexcept;
			inline void unhandled_exception() noexcept;
			inline void return_void() noexcept;

			E _env;
		};
	}

	template<typename S, typename E> requires(!tag_invocable<get_completion_signatures_t, S, E> && !requires { typename std::remove_cvref_t<S>::completion_signatures; })
	constexpr decltype(auto) get_completion_signatures_t::operator()(S &&, E &&) const
	{
		static_assert(_detail::is_awaitable<S, _detail::env_promise<E>>, "Sender must be an awaitable type");

		using result_t = _detail::await_result_type<S, _detail::env_promise<E>>;
		if constexpr (!std::is_void_v<std::remove_cv_t<result_t>>)
			return completion_signatures<set_value_t(result_t), set_error_t(std::exception_ptr), set_stopped_t()>{};
		else
			return completion_signatures<set_value_t(), set_error_t(std::exception_ptr), set_stopped_t()>{};
	}

	namespace _detail
	{
		template<typename = void>
		class awaitable_promise;
		template<typename R>
		class awaitable_operation
		{
			friend class awaitable_promise<R>;

		public:
			using promise_type = awaitable_promise<R>;

		public:
			awaitable_operation() = delete;
			awaitable_operation(const awaitable_operation &) = delete;
			awaitable_operation &operator=(const awaitable_operation &) = delete;

			awaitable_operation(awaitable_operation &&other) noexcept : _handle(std::exchange(other._handle, {})) {}
			awaitable_operation &operator=(awaitable_operation &&other) noexcept
			{
				if (&other != this) std::exchange(_handle, other._handle).destroy();
				return *this;
			}

			explicit awaitable_operation(std::coroutine_handle<> handle) noexcept : _handle(handle) {}

			~awaitable_operation() { if (_handle) _handle.destroy(); }

			friend void tag_invoke(start_t, awaitable_operation &op) noexcept { op._handle.resume(); }

		private:
			std::coroutine_handle<> _handle = {};
		};

		template<typename R>
		class awaitable_promise
		{
		public:
			friend constexpr env_of_t<R> tag_invoke(get_env_t, const awaitable_promise &p) noexcept(_detail::nothrow_callable<get_env_t, const R &>) { return get_env(p._rcv); }

		public:
			awaitable_promise(auto &, R &r) noexcept : _rcv(r) {}

			[[noreturn]] std::suspend_always final_suspend() noexcept { std::terminate(); }
			[[noreturn]] void unhandled_exception() noexcept { std::terminate(); }
			[[noreturn]] void return_void() noexcept { std::terminate(); }

			[[nodiscard]] auto get_return_object() noexcept { return awaitable_operation<R>{std::coroutine_handle<awaitable_promise>::from_promise(*this)}; }
			[[nodiscard]] std::coroutine_handle<> unhandled_stopped() noexcept { return (set_stopped(std::move(_rcv)), std::noop_coroutine()); }
			constexpr std::suspend_always initial_suspend() noexcept { return {}; }

			template<typename A>
			constexpr decltype(auto) await_transform(A &&a) noexcept(_detail::nothrow_callable<as_awaitable_t, A, awaitable_promise &>)
			{
				return as_awaitable(std::forward<A>(a), *this);
			}

		private:
			R &_rcv;
		};

		template<typename F>
		struct connect_awaiter : private empty_base<F>
		{
			using empty_base<F>::empty_base;
			using empty_base<F>::operator=;

			constexpr bool await_ready() const noexcept { return false; }
			[[noreturn]] void await_resume() noexcept { std::terminate(); }
			constexpr void await_suspend(std::coroutine_handle<>) noexcept { std::invoke(empty_base<F>::value()); }
		};
		template<typename F>
		connect_awaiter(F &&) -> connect_awaiter<std::decay_t<F>>;

		template<typename S, typename R>
		awaitable_operation<R> connect_awaitable(S s, R r)
		{
			using result_t = await_result_type<S, awaitable_promise<R>>;
			using signs_t = completion_signatures<make_signature_t<set_value_t, result_t>, set_error_t(std::exception_ptr), set_stopped_t()>;
			static_assert(receiver_of<R, signs_t>);

			constexpr auto complete = []<typename F, typename... Args>(F f, Args &&...args) noexcept
			{
				return connect_awaiter{[&, f]() { f(std::forward<Args>(args)...); }};
			};

			std::exception_ptr err;
			try
			{
				if constexpr (std::is_void_v<result_t>)
					co_await (co_await std::move(s), complete(set_value, std::move(r)));
				else
					co_await complete(set_value, std::move(r), co_await std::move(s));
			}
			catch(...) { err = std::current_exception(); }
			co_await complete(set_error, std::move(r), std::move(err));
		}
	}

	template<sender S, receiver R> requires(!tag_invocable<connect_t, S, R>)
	constexpr operation_state decltype(auto) connect_t::operator()(S &&snd, R &&rcv) const
	{
		static_assert(_detail::is_awaitable<S, _detail::awaitable_promise<R>>, "Sender must be an awaitable type");
		return _detail::connect_awaitable<std::decay_t<S>, std::decay_t<R>>(std::forward<S>(snd), std::forward<R>(rcv));
	}

	template<_detail::is_awaitable<_detail::env_promise<empty_env>> S>
	inline constexpr bool enable_sender<S> = true;

	/** Base type used to make a child coroutine promise type support awaiting on sender objects. */
	template<_detail::class_type P>
	class with_awaitable_senders
	{
		using stop_func = std::coroutine_handle<> (*)(void *) noexcept;

	public:
		template<typename Other> requires(!std::same_as<Other, void>)
		constexpr void set_continuation(std::coroutine_handle<Other> h) noexcept
		{
			_cont = h;
			bind(h);
		}
		template<typename T>
		[[nodiscard]] decltype(auto) await_transform(T &&value)
		{
			return as_awaitable(std::forward<T>(value), static_cast<P &>(*this));
		}

		[[nodiscard]] std::coroutine_handle<> unhandled_stopped() noexcept { return _stop_func(_cont.address()); }
		[[nodiscard]] std::coroutine_handle<> continuation() const noexcept { return _cont; }

	private:
		template<typename Other>
		void bind(std::coroutine_handle<Other> h) noexcept
		{
			if constexpr (requires { h.unhandled_stopped(); })
				_stop_func = [](void *p) noexcept -> std::coroutine_handle<> { return std::coroutine_handle<Other>::from_address(p).promise().unhandled_stopped(); };
			else
				_stop_func = [](void *) noexcept -> std::coroutine_handle<> { std::terminate(); };
		}

		std::coroutine_handle<> _cont = {};
		stop_func _stop_func = {};
	};

	/** Base type used to implement allocator-aware coroutine promises. */
	template<typename Promise, typename Alloc = std::allocator<Promise>>
	class with_allocator_promise;

	namespace _detail
	{
		template<typename T, typename Alloc>
		concept with_allocator_awaitable = requires { typename T::template allocator_promise_type<Alloc>; } && std::derived_from<typename T::template allocator_promise_type<Alloc>, with_allocator_promise<typename T::template allocator_promise_type<Alloc>, Alloc>>;

		template<typename Promise, typename Alloc>
		struct alignas(Alloc) with_allocator_base
		{
			using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<Promise>;
			using traits = std::allocator_traits<allocator_type>;

			[[nodiscard]] static constexpr std::size_t align_size(std::size_t n) noexcept
			{
				const auto rem = n % sizeof(Promise);
				return n + (rem ? sizeof(Promise) - rem : 0);
			}

			with_allocator_base(const with_allocator_base &) = delete;
			with_allocator_base &operator=(const with_allocator_base &) = delete;
			with_allocator_base(with_allocator_base &&) = delete;
			with_allocator_base &operator=(with_allocator_base &&) = delete;

			constexpr with_allocator_base() noexcept = default;
			constexpr ~with_allocator_base() noexcept = default;
		};
	}

	template<typename Promise, typename Alloc> requires instance_of<Alloc, std::allocator>
	class with_allocator_promise<Promise, Alloc> : _detail::with_allocator_base<Promise, Alloc>
	{
	public:
		template<typename... Args>
		constexpr void *operator new(std::size_t sz, Args &&...) { return ::operator new(sz, std::align_val_t{alignof(Promise)}); }
		template<typename... Args>
		constexpr void operator delete(void *, std::size_t, Args &&...) {}

		constexpr void operator delete(void *ptr, [[maybe_unused]] std::size_t sz)
		{
#ifndef __clang__
			::operator delete(ptr, sz, std::align_val_t{alignof(Promise)});
#else
			::operator delete(ptr, std::align_val_t{alignof(Promise)});
#endif
		}
	};
	template<typename Promise, typename Alloc> requires(!instance_of<Alloc, std::allocator> && std::is_empty_v<Alloc> && std::allocator_traits<Alloc>::is_always_equal::value)
	class with_allocator_promise<Promise, Alloc> : _detail::with_allocator_base<Promise, Alloc>
	{
		using base_t = _detail::with_allocator_base<Promise, Alloc>;
		using allocator_type = typename base_t::allocator_type;

		template<typename Alloc2>
		static constexpr void *allocate(std::size_t sz, Alloc2 &&alloc2)
		{
			if (std::is_constant_evaluated())
				return ::operator new(sz, std::align_val_t{alignof(Promise)});
			else
			{
				auto tmp_alloc = allocator_type{std::forward<Alloc2>(alloc2)};
				const auto n = base_t::align_size(sz) / sizeof(Promise);
				return base_t::traits::allocate(tmp_alloc, n);
			}
		}
		static constexpr void deallocate(void *ptr, [[maybe_unused]] std::size_t sz)
		{
			if (std::is_constant_evaluated())
			{
#ifndef __clang__
				::operator delete(ptr, sz, std::align_val_t{alignof(Promise)});
#else
				::operator delete(ptr, std::align_val_t{alignof(Promise)});
#endif
			}
			else
			{
				const auto n = base_t::align_size(sz) / sizeof(Promise);
				const auto obj = static_cast<Promise *>(ptr);

				auto tmp_alloc = allocator_type{};
				base_t::traits::deallocate(tmp_alloc, obj, n);
			}
		}

	public:
		template<typename Alloc2, typename... Args> requires std::constructible_from<allocator_type, Alloc2>
		constexpr void *operator new(std::size_t sz, std::allocator_arg_t, Alloc2 &&alloc, Args &&...) { return allocate(sz, std::forward<Alloc2>(alloc)); }
		template<typename Alloc2, typename... Args> requires std::constructible_from<allocator_type, Alloc2>
		constexpr void operator delete(void *, std::size_t, std::allocator_arg_t, Alloc2 &&, Args &&...) {}

		template<typename I, typename Alloc2, typename... Args> requires std::constructible_from<allocator_type, Alloc2>
		constexpr void *operator new(std::size_t sz, I &&, std::allocator_arg_t, Alloc2 &&alloc, Args &&...) { return allocate(sz, std::forward<Alloc2>(alloc)); }
		template<typename I, typename Alloc2, typename... Args> requires std::constructible_from<allocator_type, Alloc2>
		constexpr void operator delete(void *, std::size_t, I &&, std::allocator_arg_t, Alloc2 &&, Args &&...) {}

		constexpr void operator delete(void *ptr, std::size_t sz) { deallocate(ptr, sz); }
	};
	template<typename Promise, typename Alloc> requires(!instance_of<Alloc, std::allocator> && !std::is_empty_v<Alloc> || !std::allocator_traits<Alloc>::is_always_equal::value)
	class with_allocator_promise<Promise, Alloc> : _detail::with_allocator_base<Promise, Alloc>
	{
		using base_t = _detail::with_allocator_base<Promise, Alloc>;
		using allocator_type = typename base_t::allocator_type;

		template<typename Alloc2>
		static constexpr void *allocate(std::size_t sz, Alloc2 &&alloc2)
		{
			if (std::is_constant_evaluated())
				return ::operator new(sz, std::align_val_t{alignof(Promise)});
			else
			{
				const auto n = base_t::align_size(sz + sizeof(allocator_type)) / sizeof(Promise);
				auto tmp_alloc = allocator_type(std::forward<Alloc2>(alloc2));

				const auto mem = reinterpret_cast<std::byte *>(base_t::traits::allocate(tmp_alloc, n));
				const auto data = reinterpret_cast<Promise *>(mem + std::max(sizeof(Promise), sizeof(allocator_type)));
				const auto alloc = reinterpret_cast<allocator_type *>(mem);

				try
				{
					std::construct_at(alloc, std::move(tmp_alloc));
					return data;
				}
				catch (...)
				{
					base_t::traits::deallocate(tmp_alloc, data, n);
					throw;
				}
			}
		}
		static constexpr void deallocate(std::byte *mem, [[maybe_unused]] std::size_t sz)
		{
			if (std::is_constant_evaluated())
			{
#ifndef __clang__
				::operator delete(static_cast<void *>(mem), sz, std::align_val_t{alignof(Promise)});
#else
				::operator delete(static_cast<void *>(mem), std::align_val_t{alignof(Promise)});
#endif
			}
			else
			{
				const auto n = base_t::align_size(sz + sizeof(allocator_type)) / sizeof(Promise);
				const auto alloc = reinterpret_cast<allocator_type *>(mem - std::max(sizeof(Promise), sizeof(allocator_type)));
				const auto data = reinterpret_cast<Promise *>(mem);

				auto tmp_alloc = std::move(*alloc);
				std::destroy_at(alloc);
				base_t::traits::deallocate(tmp_alloc, data, n);
			}
		}

	public:
		template<typename Alloc2, typename... Args> requires std::constructible_from<allocator_type, Alloc2>
		constexpr void *operator new(std::size_t sz, std::allocator_arg_t, Alloc2 &&alloc, Args &&...) { return allocate(sz, std::forward<Alloc2>(alloc)); }
		template<typename Alloc2, typename... Args> requires std::constructible_from<allocator_type, Alloc2>
		constexpr void operator delete(void *, std::size_t, std::allocator_arg_t, Alloc2 &&, Args &&...) {}

		template<typename I, typename Alloc2, typename... Args> requires std::constructible_from<allocator_type, Alloc2>
		constexpr void *operator new(std::size_t sz, I &&, std::allocator_arg_t, Alloc2 &&alloc, Args &&...) { return allocate(sz, std::forward<Alloc2>(alloc)); }
		template<typename I, typename Alloc2, typename... Args> requires std::constructible_from<allocator_type, Alloc2>
		constexpr void operator delete(void *, std::size_t, I &&, std::allocator_arg_t, Alloc2 &&, Args &&...) {}

		constexpr void operator delete(void *ptr, std::size_t sz) { deallocate(static_cast<std::byte *>(ptr), sz); }
	};
}

template<typename T, typename Alloc, typename... Args> requires rod::_detail::with_allocator_awaitable<T, std::decay_t<Alloc>>
struct std::coroutine_traits<T, std::allocator_arg_t, Alloc, Args...> { using promise_type = typename T::template allocator_promise_type<std::decay_t<Alloc>>; };
template<typename T, typename I, typename Alloc, typename... Args> requires rod::_detail::with_allocator_awaitable<T, std::decay_t<Alloc>>
struct std::coroutine_traits<T, I &, std::allocator_arg_t, Alloc, Args...> { using promise_type = typename T::template allocator_promise_type<std::decay_t<Alloc>>; };
template<typename T, typename I, typename Alloc, typename... Args> requires rod::_detail::with_allocator_awaitable<T, std::decay_t<Alloc>>
struct std::coroutine_traits<T, I &&, std::allocator_arg_t, Alloc, Args...> { using promise_type = typename T::template allocator_promise_type<std::decay_t<Alloc>>; };
#endif

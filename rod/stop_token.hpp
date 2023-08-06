/*
 * Created by switchblade on 2023-04-14.
 */

#pragma once

#include <stop_token>
#include <thread>
#include <atomic>

#include "detail/factories/read.hpp"

namespace rod
{
	/** Concept used to define a stoppable token type. */
	template<typename T>
	concept stoppable_token = std::copyable<T> && std::equality_comparable<T> && requires(const T t)
	{
		{ T(t) } noexcept;
		{ t.stop_possible() } noexcept -> std::same_as<bool>;
		{ t.stop_requested() } noexcept -> std::same_as<bool>;
	};
	/** Alias for `T::callback_type&lt;CB&gt;`. */
	template<typename T, typename CB>
	using stop_callback_for_t = std::conditional_t<std::same_as<T, std::stop_token>, std::stop_callback<CB>, typename T::template callback_type<CB>>;

	/** Concept used to define a stoppable token type that can accept a callback type `CB` initialized from `Init`. */
	template<typename T, typename CB, typename Init = CB>
	concept stoppable_token_for = stoppable_token<T> && std::invocable<CB> && std::constructible_from<CB, Init> && requires { typename stop_callback_for_t<T, CB>; } &&
	                              std::constructible_from<stop_callback_for_t<T, CB>, const T &, Init>;

	/** Concept used to define a stoppable token type for which `T::stop_possible()` always returns `false`. */
	template<class T>
	concept unstoppable_token = stoppable_token<T> && requires(T t) { requires(!t.stop_possible()); };

	/** Structure used to define a never-stop stop token. */
	class never_stop_token
	{
		struct callback { explicit callback(never_stop_token, auto &&) noexcept {}};

	public:
		template<typename>
		using callback_type = callback;

		[[nodiscard]] static constexpr bool stop_requested() noexcept { return false; }
		[[nodiscard]] static constexpr bool stop_possible() noexcept { return false; }

		friend constexpr bool operator==(const never_stop_token &, const never_stop_token &) noexcept { return true; };
	};

	inline namespace _get_stop_token
	{
		struct get_stop_token_t
		{
			[[nodiscard]] constexpr friend bool tag_invoke(forwarding_query_t, get_stop_token_t) noexcept { return true; }

			template<typename E, typename U = std::remove_cvref_t<E>> requires nothrow_tag_invocable<get_stop_token_t, const U &>
			[[nodiscard]] constexpr decltype(auto) operator()(E &&e) const noexcept { return tag_invoke(*this, std::as_const(e)); }
			template<typename E, typename U = std::remove_cvref_t<E>> requires(!nothrow_tag_invocable<get_stop_token_t, const U &>)
			[[nodiscard]] constexpr never_stop_token operator()(E &&) const noexcept { return {}; }
			[[nodiscard]] constexpr auto operator()() const noexcept { return read(*this); }
		};
	}

	/** Customization point object used to obtain stop token associated with the passed object. */
	inline constexpr auto get_stop_token = get_stop_token_t{};
	/** Alias for `decltype(get_stop_token(std::declval&lt;T&gt;()))` */
	template<typename T>
	using stop_token_of_t = std::remove_cvref_t<decltype(get_stop_token(std::declval<T>()))>;

	namespace _detail
	{
		template<typename Env>
		concept stoppable_env = stoppable_token<stop_token_of_t<Env &>>;

		template<typename, auto>
		struct stop_cb_adaptor {};
		template<typename Env, typename Op, void (Op::*Stop)()> requires stoppable_env<Env>
		struct stop_cb_adaptor<Env, Stop>
		{
			struct callback
			{
				void operator()() const { (op->*Stop)(); }
				Op *op;
			};

			constexpr void init(auto &&env, Op *ptr) noexcept { data.emplace(get_stop_token(env), callback{ptr}); }
			constexpr void reset() noexcept { data.reset(); }

			std::optional<stop_callback_for_t<stop_token_of_t<Env &>, callback>> data;
		};
	}

	class in_place_stop_token;
	template<typename CB>
	class in_place_stop_callback;

	/** Stop source used to query for stop requests of the associated `in_place_stop_source`. */
	class in_place_stop_source
	{
		template<typename>
		friend class in_place_stop_callback;

		enum status_t { is_busy = 1, has_req = 2, };

		struct node_t
		{
			node_t() = delete;
			node_t(node_t &&) = delete;

			node_t(in_place_stop_source *src, void (*invoke)(node_t *) noexcept) noexcept : invoke_func(invoke), src(src) { if (src) src->insert(this); }
			~node_t() noexcept { if (src) src->erase(this); }

			void invoke() noexcept { invoke_func(this); }
			bool invoke(bool &removed_ref) noexcept
			{
				removed = &removed_ref;
				invoke_func(this);
				removed = {};
				return removed_ref;
			}

			void (*invoke_func)(node_t *) noexcept;
			in_place_stop_source *src;
			node_t *next_node = {};
			node_t **this_ptr = {};

			std::atomic_flag complete;
			bool *removed = nullptr;
		};

	public:
		[[nodiscard]] static constexpr bool stop_possible() noexcept { return true; }

	public:
		in_place_stop_source(in_place_stop_source &&) noexcept = delete;

		constexpr in_place_stop_source() noexcept = default;
		constexpr ~in_place_stop_source() noexcept = default;

		/** Returns a stop token associated with this stop source. */
		[[nodiscard]] constexpr in_place_stop_token get_token() const noexcept;
		/** Checks if stop has been requested. */
		[[nodiscard]] bool stop_requested() const noexcept { return _flags.load(std::memory_order_acquire) & status_t::has_req; }

		/** Sends a stop request via this stop source. */
		bool request_stop() noexcept
		{
			if (!try_lock(status_t::has_req)) return false;
			for (_tid = std::this_thread::get_id(); _nodes; lock())
			{
				const auto node = pop();
				unlock(status_t::has_req);
				if (bool removed = false; !node->invoke(removed))
				{
					node->complete.test_and_set(std::memory_order_release);
					node->complete.notify_one();
				}
			}
			return (unlock(status_t::has_req), true);
		}

	private:
		status_t lock() noexcept
		{
			for (auto flags = _flags.load(std::memory_order_relaxed);;)
			{
				while (flags & status_t::is_busy)
				{
					_flags.wait(flags, std::memory_order_acq_rel);
					flags = _flags.load(std::memory_order_relaxed);
				}

				if (_flags.compare_exchange_weak(
						flags, static_cast<status_t>(flags | status_t::is_busy),
						std::memory_order_acq_rel, std::memory_order_relaxed))
					return flags;
			}
		}
		void unlock(status_t val = {}) noexcept
		{
			_flags.store(val, std::memory_order_release);
			_flags.notify_one();
		}
		bool try_lock(status_t val = {}) noexcept
		{
			for (auto flags = _flags.load(std::memory_order_relaxed);;)
			{
				for (; flags; flags = _flags.load(std::memory_order_relaxed))
				{
					if (flags & status_t::has_req) return false;
					if (flags & status_t::is_busy) _flags.wait(flags, std::memory_order_acq_rel);
				}

				if (_flags.compare_exchange_weak(
						flags, static_cast<status_t>(val | status_t::is_busy),
						std::memory_order_acq_rel, std::memory_order_relaxed))
					return true;
			}
		}

		[[nodiscard]] node_t *pop() noexcept
		{
			const auto node = _nodes;
			_nodes = node->next_node;
			if (_nodes) _nodes->this_ptr = &_nodes;
			node->this_ptr = {};
			return node;
		}
		void insert(node_t *node) noexcept
		{
			if (!try_lock())
			{
				node->invoke();
				return;
			}

			node->next_node = _nodes;
			node->this_ptr = &_nodes;
			if (_nodes) _nodes->this_ptr = &node->next_node;
			_nodes = node;

			unlock();
		}
		void erase(node_t *node) noexcept
		{
			if (auto old = lock(); node->this_ptr)
			{
				if ((*node->this_ptr = node->next_node))
					node->next_node->this_ptr = node->this_ptr;
				unlock(old);
			}
			else
			{
				const auto tid = _tid;
				unlock(old);

				if (tid != std::this_thread::get_id())
					node->complete.wait(false, std::memory_order_acq_rel);
				else if (node->removed)
					*node->removed = true;
			}
		}

		std::atomic<status_t> _flags = {};
		std::thread::id _tid = {};
		node_t *_nodes = {};
	};

	/** Stop token used to query for stop requests of the associated `in_place_stop_source`. */
	class in_place_stop_token
	{
		template<typename>
		friend class in_place_stop_callback;
		friend class in_place_stop_source;

	public:
		template<typename CB>
		using callback_type = in_place_stop_callback<CB>;

	private:
		constexpr in_place_stop_token(in_place_stop_source *src) noexcept : _src(src) {}

	public:
		constexpr in_place_stop_token() noexcept = default;
		constexpr ~in_place_stop_token() noexcept = default;

		/** Checks if the associated stop source has been requested to stop. */
		[[nodiscard]] bool stop_requested() const noexcept { return _src && _src->stop_requested(); }
		/** Checks if the associated stop source can be requested to stop. */
		[[nodiscard]] constexpr bool stop_possible() const noexcept { return _src && _src->stop_possible(); }

		constexpr void swap(in_place_stop_token &other) noexcept { std::swap(_src, other._src); }
		friend constexpr void swap(in_place_stop_token &a, in_place_stop_token &b) noexcept { return a.swap(b); }

		friend constexpr bool operator==(const in_place_stop_token &, const in_place_stop_token &) noexcept = default;

	private:
		in_place_stop_source *_src = nullptr;
	};

	constexpr in_place_stop_token in_place_stop_source::get_token() const noexcept { return {const_cast<in_place_stop_source *>(this)}; }

	/** Structure used to associate callback `CB` with an `in_place_stop_source`. */
	template<typename CB>
	class in_place_stop_callback : in_place_stop_source::node_t
	{
		using node_base = in_place_stop_source::node_t;

	public:
		using callback_type = CB;

	private:
		static void invoke(node_base *node) noexcept { static_cast<in_place_stop_callback *>(node)->_cb(); }

	public:
		/** Adds a stop callback function \a fn to the stop source associated with stop token \a st. */
		template<typename F> requires std::constructible_from<CB, F>
		in_place_stop_callback(in_place_stop_token st, F &&fn) noexcept(std::is_nothrow_constructible_v<CB, F>) : node_base(st._src, invoke), _cb(std::forward<F>(fn)) {}

	private:
		ROD_NO_UNIQUE_ADDRESS CB _cb;
	};

	template<typename F>
	in_place_stop_callback(in_place_stop_token, F) -> in_place_stop_callback<F>;

	static_assert(stoppable_token<in_place_stop_token>);
}

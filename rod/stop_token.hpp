/*
 * Created by switchblade on 2023-04-14.
 */

#pragma once

#include <stop_token>
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
	/** Alias for `T::callback_type<CB>`. */
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
		class get_stop_token_t
		{
			template<typename E>
			static constexpr bool is_stoppable = stoppable_token<tag_invoke_result_t<get_stop_token_t, const E &>>;
			template<typename E>
			static constexpr bool has_tag_invoke = nothrow_tag_invocable<get_stop_token_t, const E &>;

		public:
			[[nodiscard]] constexpr friend bool tag_invoke(forwarding_query_t, get_stop_token_t) noexcept { return true; }

			template<typename E, typename U = std::remove_cvref_t<E>> requires has_tag_invoke<U> && is_stoppable<U>
			[[nodiscard]] constexpr decltype(auto) operator()(E &&e) const noexcept { return tag_invoke(*this, std::as_const(e)); }
			template<typename E>
			[[nodiscard]] constexpr never_stop_token operator()(E &&) const noexcept { return {}; }
			[[nodiscard]] constexpr auto operator()() const noexcept { return read(*this); }
		};
	}

	/** Customization point object used to obtain stop token associated with the passed object. */
	inline constexpr auto get_stop_token = get_stop_token_t{};
	/** Alias for `decltype(get_stop_token(std::declval<T>()))` */
	template<typename T>
	using stop_token_of_t = std::remove_cvref_t<decltype(get_stop_token(std::declval<T>()))>;

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

			node_t(in_place_stop_source *src, void (*invoke)(node_t *) noexcept) noexcept : src(src), invoke_func(invoke) { if (src) src->insert(this); }
			~node_t() noexcept { if (src) src->erase(this); }

			void invoke() noexcept { invoke_func(this); }

			node_t *unlink() noexcept
			{
				if (next_node) std::exchange(next_node, nullptr)->this_ptr = this_ptr;
				if (this_ptr) *std::exchange(this_ptr, nullptr) = next_node;
				return this;
			}
			void link(node_t *&list) noexcept
			{
				if (list) list->this_ptr = &next_node;
				next_node = std::exchange(list, this);
				this_ptr = &list;
			}

			node_t *next_node = {};
			node_t **this_ptr = {};
			in_place_stop_source *src;
			void (*invoke_func)(node_t *) noexcept;
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
		[[nodiscard]] bool stop_requested() const noexcept { return m_flags.load(std::memory_order_acquire) & status_t::has_req; }

		/** Sends a stop request via this stop source. */
		bool request_stop() noexcept
		{
			if (!try_lock(status_t::has_req))
				return false;

			while (m_nodes) m_nodes->unlink()->invoke();
			return (unlock(status_t::has_req), true);
		}

	private:
		auto lock() noexcept
		{
			for (auto flags = m_flags.load(std::memory_order_relaxed);;)
			{
				while (flags & status_t::is_busy)
				{
					m_flags.wait(flags);
					flags = m_flags.load(std::memory_order_relaxed);
				}

				if (m_flags.compare_exchange_weak(
						flags, static_cast<status_t>(flags | status_t::is_busy),
						std::memory_order_acq_rel, std::memory_order_relaxed))
					return flags;
			}
		}
		void unlock(status_t val = {}) noexcept
		{
			m_flags.store(val, std::memory_order_release);
			m_flags.notify_one();
		}
		bool try_lock(status_t val = {}) noexcept
		{
			for (auto flags = m_flags.load(std::memory_order_relaxed);;)
			{
				for (; flags; flags = m_flags.load(std::memory_order_relaxed))
				{
					if (flags & status_t::has_req) return false;
					if (flags & status_t::is_busy) m_flags.wait(flags);
				}

				if (m_flags.compare_exchange_weak(
						flags, static_cast<status_t>(val | status_t::is_busy),
						std::memory_order_acq_rel, std::memory_order_relaxed))
					return true;
			}
		}

		void insert(node_t *node) noexcept
		{
			if (!try_lock())
			{
				node->invoke();
				return;
			}

			node->link(m_nodes);
			unlock();
		}
		void erase(node_t *node) noexcept
		{
			auto old = lock();
			node->unlink();
			unlock(old);
		}

		node_t *m_nodes = {};
		std::atomic<status_t> m_flags = {};
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
		constexpr in_place_stop_token(in_place_stop_source *src) noexcept : m_src(src) {}

	public:
		constexpr in_place_stop_token() noexcept = default;
		constexpr ~in_place_stop_token() noexcept = default;

		/** Checks if the associated stop source has been requested to stop. */
		[[nodiscard]] bool stop_requested() const noexcept { return m_src && m_src->stop_requested(); }
		/** Checks if the associated stop source can be requested to stop. */
		[[nodiscard]] constexpr bool stop_possible() const noexcept { return m_src && m_src->stop_possible(); }

		constexpr void swap(in_place_stop_token &other) noexcept { std::swap(m_src, other.m_src); }
		friend consteval void swap(in_place_stop_token &a, in_place_stop_token &b) noexcept { return a.swap(b); }

		friend constexpr bool operator==(const in_place_stop_token &, const in_place_stop_token &) noexcept = default;

	private:
		in_place_stop_source *m_src = nullptr;
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
		static void invoke(node_base *node) noexcept { static_cast<in_place_stop_callback *>(node)->m_cb(); }

	public:
		/** Adds a stop callback function \a fn to the stop source associated with stop token \a st. */
		template<typename F> requires std::constructible_from<CB, F>
		in_place_stop_callback(in_place_stop_token st, F &&fn) noexcept(std::is_nothrow_constructible_v<CB, F>) : node_base(st.m_src, invoke), m_cb(std::forward<F>(fn)) {}

	private:
		[[ROD_NO_UNIQUE_ADDRESS]] CB m_cb;
	};

	template<typename F>
	in_place_stop_callback(in_place_stop_token, F) -> in_place_stop_callback<F>;

	static_assert(stoppable_token<in_place_stop_token>);
}

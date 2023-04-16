/*
 * Created by switchblade on 2023-04-14.
 */

#pragma once

#include <stop_token>
#include <atomic>

#include "sender.hpp"

namespace rod
{
	/** Alias for `T::callback_type<CB>`. */
	template<typename T, typename CB>
	using stop_callback_for_t = typename T::template callback_type<CB>;

	/** Concept used to define a stoppable token type. */
	template<typename T>
	concept stoppable_token = std::copyable<T> && std::equality_comparable<T> && requires(const T t)
	{
		{ T(t) } noexcept;
		{ t.stop_possible() } noexcept -> std::same_as<bool>;
		{ t.stop_requested() } noexcept -> std::same_as<bool>;
	};

	/** Concept used to define a stoppable token type that can accept a callback type `CB` initialized from `Init`. */
	template<typename T, typename CB, typename Init = CB>
	concept stoppable_token_for = stoppable_token<T> && std::invocable<CB> && std::constructible_from<CB, Init> && requires { typename stop_callback_for_t<T, CB>; } &&
	                              std::constructible_from<stop_callback_for_t<T, CB>, const T &, Init>;

	/** Concept used to define a stoppable token type for which `T::stop_possible()` always returns `false`. */
	template<class T>
	concept unstoppable_token = stoppable_token<T> && requires(T t) { requires(!t.stop_possible()); };

	namespace detail
	{
		class get_allocator_t
		{
			template<typename T, typename U = decltype(std::as_const(std::declval<T>()))>
			static constexpr bool is_invocable = tag_invocable<get_allocator_t, U>;

		public:
			[[nodiscard]] constexpr friend bool tag_invoke(forwarding_query_t, get_allocator_t) noexcept { return true; }

			[[nodiscard]] constexpr auto operator()() const noexcept { return read(*this); }
			template<typename R>
			[[nodiscard]] constexpr decltype(auto) operator()(R &&r) const noexcept requires is_invocable<R>
			{
				return tag_invoke(*this, std::as_const(r));
			}
		};
		class get_stop_token_t
		{
			template<typename T, typename U = decltype(std::as_const(std::declval<T>()))>
			static constexpr bool is_invocable = tag_invocable<get_stop_token_t, U>;
			template<typename T, typename U = decltype(std::as_const(std::declval<T>()))>
			using result_t = tag_invoke_result_t<get_stop_token_t, U>;

		public:
			[[nodiscard]] constexpr friend bool tag_invoke(forwarding_query_t, get_stop_token_t) noexcept { return true; }

			[[nodiscard]] constexpr auto operator()() const noexcept { return read(*this); }
			template<typename Env>
			[[nodiscard]] constexpr decltype(auto) operator()(Env &&env) const noexcept requires(is_invocable<Env> && stoppable_token<result_t<Env>>)
			{
				return tag_invoke(*this, std::as_const(env));
			}
		};
	}

	using detail::get_allocator_t;
	using detail::get_stop_token_t;

	/** Customization point object used to obtain allocator associated with the passed object. */
	inline constexpr auto get_allocator = get_allocator_t{};
	/** Alias for `decltype(get_allocator(std::declval<T>()))` */
	template<typename T>
	using allocator_of_t = std::remove_cvref_t<decltype(get_allocator(std::declval<T>()))>;

	/** Customization point object used to obtain stop token associated with the passed object. */
	inline constexpr auto get_stop_token = get_stop_token_t{};
	/** Alias for `decltype(get_stop_token(std::declval<T>()))` */
	template<typename T>
	using stop_token_of_t = std::remove_cvref_t<decltype(get_stop_token(std::declval<T>()))>;

	/** Structure used to define a never-stop stop token. */
	class never_stop_token
	{
		struct callback { explicit callback(never_stop_token, auto &&) noexcept {}};

	public:
		template<typename>
		using callback_type = callback;

		[[nodiscard]] static constexpr bool stop_requested() noexcept { return false; }
		[[nodiscard]] static constexpr bool stop_possible() noexcept { return false; }

		[[nodiscard]] friend constexpr bool operator==(const never_stop_token &, const never_stop_token &) noexcept { return true; };
	};

	class in_place_stop_token;
	template<typename CB>
	class in_place_stop_callback;

	/** Stop source used to query for stop requests of the associated `in_place_stop_source`. */
	class in_place_stop_source
	{
		template<typename>
		friend class in_place_stop_callback;

		struct node_t
		{
			node_t() = delete;
			node_t(node_t &&) = delete;

			constexpr node_t(void (*invoke)(node_t *) noexcept) noexcept : invoke_func(invoke) {}

			void invoke() noexcept { invoke_func(this); }

			void link(node_t *&ptr) noexcept
			{
				if (ptr) ptr->this_ptr = &next_node;
				next_node = std::exchange(ptr, this);
				this_ptr = &ptr;
			}
			node_t *unlink() noexcept
			{
				if (next_node) next_node->this_ptr = this_ptr;
				if (this_ptr) *this_ptr = next_node;
				return this;
			}

			node_t *next_node = nullptr;
			node_t **this_ptr = nullptr;
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
		[[nodiscard]] bool stop_requested() const noexcept { return m_stop.test(); }

		/** Sends a stop request via this stop source. */
		bool request_stop() noexcept
		{
			if (m_stop.test_and_set())
				return false;

			while (auto *node = pop_node())
				node->invoke();
			return true;
		}

	private:
		void lock() noexcept
		{
			while (m_busy.test_and_set())
				m_busy.wait(true);
		}
		void unlock() noexcept
		{
			m_busy.clear();
			m_busy.notify_one();
		}

		void push_node(node_t *node) noexcept
		{
			if (stop_requested())
			{
				node->invoke();
				return;
			}

			lock();
			node->link(m_nodes);
			unlock();
		}
		[[nodiscard]] node_t *pop_node() noexcept
		{
			auto result = (lock(), m_nodes ? m_nodes->unlink() : nullptr);
			return (unlock(), result);
		}

		node_t *m_nodes = {};
		std::atomic_flag m_busy = {};
		std::atomic_flag m_stop = {};
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

		[[nodiscard]] friend constexpr bool operator==(const in_place_stop_token &, const in_place_stop_token &) noexcept = default;

	private:
		in_place_stop_source *m_src = nullptr;
	};

	constexpr in_place_stop_token in_place_stop_source::get_token() const noexcept { return {const_cast<in_place_stop_source *>(this)}; }

	/** Structure used to associate callback `CB` with an `in_place_stop_source`. */
	template<typename CB>
	class in_place_stop_callback : in_place_stop_source::node_t, detail::ebo_helper<CB>
	{
		using node_base = in_place_stop_source::node_t;
		using func_base = detail::ebo_helper<CB>;

	public:
		using callback_type = CB;

	private:
		static void invoke(node_base *node) noexcept { static_cast<in_place_stop_callback *>(node)->func_base::value()(); }

	public:
		/** Adds a stop callback function \a fn to the stop source associated with stop token \a st. */
		template<typename F>
		in_place_stop_callback(in_place_stop_token st, F &&fn) noexcept(std::is_nothrow_constructible_v<CB, F>) requires std::constructible_from<CB, F>
				: node_base(invoke), func_base(std::forward<F>(fn)) { if (st.m_src) st.m_src->push_node(this); }
		/** Removes the callback from the associated stop token. */
		~in_place_stop_callback() { node_base::unlink(); }
	};

	template<typename F>
	in_place_stop_callback(in_place_stop_token, F) -> in_place_stop_callback<F>;
}

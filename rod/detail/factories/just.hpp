/*
 * Created by switchblade on 2023-04-17.
 */

#pragma once

#include "../queries/completion.hpp"
#include "../adaptors/transfer.hpp"
#include "../adaptors/then.hpp"
#include "../adaptors/bulk.hpp"
#include "../concepts.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	namespace _just
	{
		struct just_t;
		struct just_error_t;
		struct just_stopped_t;

		template<typename, typename, movable_value...>
		struct operation { struct type; };
		template<typename, movable_value...>
		struct sender { struct type; };

		template<typename R, typename C, movable_value... Ts>
		struct operation<R, C, Ts...>::type
		{
			friend constexpr void tag_invoke(start_t, type &op) noexcept { std::apply([&op](Ts &...vals) { C{}(std::move(op._rcv), std::move(vals)...); }, op._values); }

			[[ROD_NO_UNIQUE_ADDRESS]] std::tuple<Ts...> _values;
			[[ROD_NO_UNIQUE_ADDRESS]] R _rcv;
		};
		template<typename C, movable_value... Ts>
		struct sender<C, Ts...>::type
		{
			using is_sender = std::true_type;
			using _signs_t = completion_signatures<C(Ts...)>;

			friend constexpr empty_env tag_invoke(get_env_t, const type &) noexcept { return {}; }
			template<decays_to<type> T, typename E>
			friend constexpr _signs_t tag_invoke(get_completion_signatures_t, T &&, E) { return {}; }

			template<receiver_of<_signs_t> R> requires(std::copy_constructible<Ts> && ...)
			friend constexpr auto tag_invoke(connect_t, const type &s, R &&r) noexcept(std::is_nothrow_constructible_v<std::decay_t<R>, R> && (std::is_nothrow_copy_constructible_v<Ts> && ...))
			{
				using operation_t = typename operation<std::decay_t<R>, C, Ts...>::type;
				return operation_t{s._values, std::forward<R>(r)};
			}
			template<receiver_of<_signs_t> R>
			friend constexpr auto tag_invoke(connect_t, type &&s, R &&r) noexcept(std::is_nothrow_constructible_v<std::decay_t<R>, R> && (std::is_nothrow_move_constructible_v<Ts> && ...))
			{
				using operation_t = typename operation<std::decay_t<R>, C, Ts...>::type;
				return operation_t{std::move(s._values), std::forward<R>(r)};
			}

			[[ROD_NO_UNIQUE_ADDRESS]] std::tuple<Ts...> _values;
		};
	}

	using _just::just_t;
	using _just::just_error_t;
	using _just::just_stopped_t;

	struct _just::just_t
	{
		template<typename... Vs>
		[[nodiscard]] constexpr rod::sender auto operator()(Vs &&...vals) const noexcept((std::is_nothrow_constructible_v<std::decay_t<Vs>, Vs> && ...))
		{
			using sender_t = typename sender<set_value_t, std::decay_t<Vs>...>::type;
			return sender_t{{std::forward<Vs>(vals)...}};
		}
	};

	/** Returns a sender that passes a set of values through the value channel.
	 * @param vals Values to be sent through the value channel.
	 * @return Sender that completes via `set_value(vals...)`. */
	inline constexpr auto just = just_t{};

	struct _just::just_error_t
	{
		template<typename Err>
		[[nodiscard]] constexpr auto operator()(Err &&err) const noexcept(std::is_nothrow_constructible_v<std::decay_t<Err>, Err>)
		{
			using sender_t = typename sender<set_error_t, std::decay_t<Err>>::type;
			return sender_t{{std::forward<Err>(err)}};
		}
	};

	/** Returns a sender that passes an error through the error channel.
	 * @param err Error to be sent through the error channel.
	 * @return Sender that completes via `set_error(err)`. */
	inline constexpr auto just_error = just_error_t{};

	struct _just::just_stopped_t { [[nodiscard]] constexpr typename sender<set_stopped_t>::type operator()() const noexcept { return {}; }};

	/** Returns a sender that completes through the stop channel.
	 * @return Sender that completes via `set_stopped()`. */
	inline constexpr auto just_stopped = just_stopped_t{};

	namespace _just
	{
		class transfer_just_t
		{
			template<typename Sch>
			using back_adaptor = detail::back_adaptor<transfer_just_t, std::decay_t<Sch>>;

			template<typename S, typename... Vs>
			constexpr static bool default_nothrow = detail::nothrow_callable<just_t, Vs...> && detail::nothrow_callable<transfer_t, std::invoke_result_t<just_t, Vs...>>;

		public:
			template<scheduler S, typename... Vs> requires tag_invocable<transfer_just_t, S, Vs...>
			[[nodiscard]] constexpr rod::sender auto operator()(S &&sch, Vs &&...vals) const noexcept(nothrow_tag_invocable<transfer_just_t, S, Vs...>)
			{
				return tag_invoke(*this, std::forward<S>(sch), std::forward<Vs>(vals)...);
			}
			template<scheduler S, typename... Vs> requires(!tag_invocable<transfer_just_t, S, Vs...>)
			[[nodiscard]] constexpr rod::sender auto operator()(S &&sch, Vs &&...vals) const noexcept(default_nothrow<S, Vs...>)
			{
				return transfer(just(std::forward<Vs>(vals)...), std::forward<S>(sch));
			}

			template<scheduler Sch>
			[[nodiscard]] constexpr back_adaptor<Sch> operator()(Sch &&sch) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Sch>, transfer_just_t, Sch>)
			{
				return {*this, std::forward<Sch>(sch)};
			}
		};
	}

	using _just::transfer_just_t;

	/** Returns a sender that transfers a set of values to the specified scheduler through the value channel.
	 * @param sch Scheduler to schedule the value sender on. If omitted, creates a pipe-able sender adaptor.
	 * @param vals Values to send through the value channel.
	 * @return Sender that completes via `set_value(vals...)` on the \a sch scheduler. */
	inline constexpr auto transfer_just = transfer_just_t{};

	namespace _just
	{
		struct just_invoke_t
		{
			template<movable_value F, typename... Args> requires std::invocable<F, Args...>
			[[nodiscard]] constexpr rod::sender auto operator()(F &&fn, Args &&...args) const noexcept(detail::nothrow_callable<then_t, std::invoke_result_t<just_t, Args...>, F>)
			{
				return then(just(std::forward<Args>(args)...), std::forward<F>(fn));
			}
		};
	}

	using _just::just_invoke_t;

	/** Returns a sender that will execute \a fn with \a args and pass the result via the value channel.
	 * Equivalent to `just(args...) | then(fn)`.
	 * @param fn Functor who's result to pass into the value channel.
	 * @param args Arguments to use for invocation of \a fn.
	 * @return Sender that completes via `set_value(fn(args...))`. */
	inline constexpr auto just_invoke = just_invoke_t{};

	namespace _just
	{
		struct just_bulk_t
		{
			template<movable_value F, std::integral Shape, typename... Args> requires std::invocable<F, Shape, detail::decayed_ref<Args>...>
			[[nodiscard]] constexpr rod::sender auto operator()(Shape shape, F &&fn, Args &&...args) const noexcept(detail::nothrow_callable<bulk_t, std::invoke_result_t<just_t, Args...>, Shape, F>)
			{
				return bulk(just(std::forward<Args>(args)...), std::move(shape), std::forward<F>(fn));
			}
		};
	}

	using _just::just_bulk_t;

	/** Returns a sender that passes a set of values through the value channel and executes \a fn over an index space specified by \a shape.
	 * Equivalent to `just(args...) | bulk(shape, fn)`.
	 * @param shape Integral shape (maximum loop index) of the bulk task.
	 * @param fn Functor to be executed \a shape times with \a args.
	 * @param args Values to send through the value channel.
	 * @return Result of `just(args...) | bulk(shape, fn)`. */
	inline constexpr auto just_bulk = just_bulk_t{};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

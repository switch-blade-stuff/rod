/*
 * Created by switchblade on 2023-06-29.
 */

#pragma once

#include "../../result.hpp"
#include "../byte_buffer.hpp"

namespace rod::_sync_io
{
	template<typename T, typename Snd, typename Rcv, typename... Args>
	struct operation { class type; };
	template<typename T, typename Rcv, typename... Args>
	struct receiver { class type; };
	template<typename T, typename Snd, typename... Args>
	struct sender { class type; };

	template<typename T, typename Rcv, typename... Args>
	struct operation_base : empty_base<Rcv>, std::tuple<Args...>
	{
		template<typename Rcv2, typename... Args2>
		constexpr explicit operation_base(Rcv2 &&rcv, Args2 &&...args) noexcept(std::is_nothrow_constructible_v<Rcv, Rcv2> && (std::is_nothrow_constructible_v<Args, Args2> && ...))
				: empty_base<Rcv>(std::forward<Rcv2>(rcv)), std::tuple<Args...>(std::forward<Args2>(args)...) {}

		[[nodiscard]] constexpr decltype(auto) rcv() noexcept { return empty_base<Rcv>::value(); }
		[[nodiscard]] constexpr decltype(auto) rcv() const noexcept { return empty_base<Rcv>::value(); }
		[[nodiscard]] constexpr decltype(auto) args() noexcept { return static_cast<std::tuple<Args...> &>(*this); }
		[[nodiscard]] constexpr decltype(auto) args() const noexcept { return static_cast<const std::tuple<Args...> &>(*this); }
	};

	template<typename T, typename Snd, typename Rcv, typename... Args>
	class operation<T, Snd, Rcv, Args...>::type : operation_base<T, Rcv, Args...>
	{
		using receiver_t = typename receiver<T, Rcv, Args...>::type;
		using operation_base = operation_base<T, Rcv, Args...>;
		using state_t = connect_result_t<Snd, receiver_t>;

	public:
		type(type &&) = delete;
		type &operator=(type &&) = delete;

		constexpr explicit type(Snd &&snd, Rcv rcv, std::tuple<Args...> args) : operation_base(std::move(rcv), std::move(args)), _state(connect(std::forward<Snd>(snd), receiver_t(this))) {}

		friend constexpr void tag_invoke(start_t, type &op) noexcept { start(op._state); }

	private:
		state_t _state;
	};
	template<typename T, typename Rcv, typename... Args>
	class receiver<T, Rcv, Args...>::type
	{
	public:
		using is_receiver = std::true_type;

	private:
		using operation_base = operation_base<T, Rcv, Args...>;

	public:
		constexpr explicit type(operation_base *op) noexcept : _op(op) {}

		friend constexpr env_of_t<Rcv> tag_invoke(get_env_t, const type &r) noexcept(nothrow_tag_invocable<get_env_t, const Rcv &>) { return get_env(r._op->rcv()); }

		friend constexpr void tag_invoke(set_value_t, type &&r) noexcept
		{
			if constexpr ((std::is_nothrow_move_constructible_v<Args> && ...))
				try { r.complete(); } catch (...) { set_error(std::move(r._op->rcv()), std::current_exception()); }
			else
				r.complete();
		}
		template<_detail::completion_channel C, typename... Vs> requires(!std::same_as<C, set_value_t> && _detail::callable<C, Rcv, Vs...>)
		friend constexpr void tag_invoke(C, type &&r, Vs &&...args) noexcept { C{}(std::move(r._op->rcv()), std::forward<Vs>(args)...); }

	private:
		constexpr void complete() noexcept((std::is_nothrow_move_constructible_v<Args> && ...))
		{
			std::apply([&]<typename... Vs>(Vs &&...vs) noexcept((std::is_nothrow_move_constructible_v<Args> && ...))
			{
				if (const auto res = T{}(std::forward<Vs>(vs)...); res.has_value())
					set_value(std::move(_op->rcv()), res.value());
				else
					set_error(std::move(_op->rcv()), res.error());
			}, std::move(_op->args()));
		}

		operation_base *_op;
	};
	template<typename T, typename Snd, typename... Args>
	class sender<T, Snd, Args...>::type : empty_base<Snd>, std::tuple<Args...>
	{
	public:
		using is_sender = std::true_type;

	private:
		template<typename U, typename Rcv>
		using operation_t = typename operation<T, copy_cvref_t<U, Snd>, Rcv, Args...>::type;
		template<typename Rcv>
		using receiver_t = typename receiver<T, Rcv, Args...>::type;

		template<typename... Ts>
		using value_signs_t = completion_signatures<>;
		template<typename Err>
		using error_signs_t = _detail::concat_tuples_t<completion_signatures<_detail::make_signature_t<set_error_t, Err>>,
									std::conditional_t<std::conjunction_v<std::is_nothrow_move_constructible<Args>...>,
										completion_signatures<set_error_t(std::exception_ptr)>,
										completion_signatures<>>>;
		template<typename U, typename E>
		using signs_t = make_completion_signatures<copy_cvref_t<U, Snd>, E, completion_signatures<set_value_t(std::size_t), set_error_t(std::error_code)>, value_signs_t, error_signs_t>;

	public:
		template<typename Snd2, typename... Args2>
		constexpr explicit type(Snd2 &&snd, Args2 &&...args) noexcept(std::is_nothrow_constructible_v<Snd, Snd2> && std::is_nothrow_constructible_v<std::tuple<Args...>, Args2...>)
				: empty_base<Snd>(std::forward<Snd2>(snd)), std::tuple<Args...>(std::forward<Args2>(args)...) {}

		friend constexpr env_of_t<Snd> tag_invoke(get_env_t, const type &s) noexcept(nothrow_tag_invocable<get_env_t, const Snd &>) { return get_env(s.empty_base<Snd>::value()); }

		template<decays_to<type> U, typename E>
		friend constexpr signs_t<U, E> tag_invoke(get_completion_signatures_t, U &&, E) noexcept { return {}; }
		template<decays_to<type> U, rod::receiver Rcv>
		friend constexpr operation_t<U, Rcv> tag_invoke(connect_t, U &&s, Rcv r) noexcept(std::is_nothrow_constructible_v<operation_t<U, Rcv>, copy_cvref_t<U, Snd>, Rcv, copy_cvref_t<U, std::tuple<Args...>>>)
		{
			return operation_t<U, Rcv>(std::forward<U>(s).empty_base<Snd>::value(), std::move(r), static_cast<copy_cvref_t<U, std::tuple<Args...>>>(s));
		}
	};
}

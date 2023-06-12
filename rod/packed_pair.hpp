/*
 * Created by switchblade on 2023-04-27.
 */

#pragma once

#include <tuple>

#include "utility.hpp"

namespace rod
{
	/** Pair with support for Empty Base Optimization (EBO). */
	template<typename T0, typename T1>
	class packed_pair
	{
	public:
		using first_type = T0;
		using second_type =	T1;

	private:
		template<typename... Ts>
		using first_nothrow_constructible = typename detail::bind_front<std::is_nothrow_constructible, T0>::template type<Ts...>;
		template<typename... Ts>
		using second_nothrow_constructible = typename detail::bind_front<std::is_nothrow_constructible, T1>::template type<Ts...>;

		template<typename Args0, typename Args1>
		static constexpr bool nothrow_constructible = std::conjunction_v<detail::apply_tuple_t<second_nothrow_constructible, Args1>,
		                                                                 detail::apply_tuple_t<first_nothrow_constructible, Args0>>;

		template<std::size_t... Is0, typename... Args0, std::size_t... Is1, typename... Args1>
		constexpr packed_pair(std::index_sequence<Is0...>, std::tuple<Args0...> &args0, std::index_sequence<Is1...>, std::tuple<Args1...> &args1)
				: first(std::forward<Args0>(std::get<Is0>(args0))...), second(std::forward<Args1>(std::get<Is1>(args1))...) {}

	public:
		constexpr packed_pair() noexcept(nothrow_constructible<type_list_t<>, type_list_t<>>) = default;
		constexpr packed_pair(const packed_pair &) noexcept(nothrow_constructible<type_list_t<const T0 &>, type_list_t<const T1 &>>) = default;
		constexpr packed_pair(packed_pair &&) noexcept(nothrow_constructible<type_list_t<T0>, type_list_t<T1>>) = default;

		constexpr packed_pair(const T0 &v0, const T1 &v1) noexcept(nothrow_constructible<type_list_t<const T0 &>, type_list_t<const T1 &>>) : first(v0), second(v1) {}
		template<typename U0 = T0, typename U1 = T1>
		constexpr packed_pair(U0 &&v0, U1 &&v1) noexcept(nothrow_constructible<type_list_t<U0>, type_list_t<U1>>) : first(std::forward<U0>(v0)), second(std::forward<U1>(v1)) {}

		template<typename U0, typename U1>
		constexpr packed_pair(std::pair<U0, U1> &p) noexcept(nothrow_constructible<type_list_t<U0 &>, type_list_t<U1 &>>) : first(p.first), second(p.second) {}
		template<typename U0, typename U1>
		constexpr packed_pair(packed_pair<U0, U1> &p) noexcept(nothrow_constructible<type_list_t<U0 &>, type_list_t<U1 &>>) : first(p.first), second(p.second) {}

		template<typename U0 = T0, typename U1 = T1>
		constexpr packed_pair(const std::pair<U0, U1> &p) noexcept(nothrow_constructible<type_list_t<const U0 &>, type_list_t<const U1 &>>) : first(p.first), second(p.second) {}
		template<typename U0, typename U1>
		constexpr packed_pair(const packed_pair<U0, U1> &p) noexcept(nothrow_constructible<type_list_t<const U0 &>, type_list_t<const U1 &>>) : first(p.first), second(p.second) {}

		template<typename U0 = T0, typename U1 = T1>
		constexpr packed_pair(std::pair<U0, U1> &&p) noexcept(nothrow_constructible<type_list_t<U0>, type_list_t<U1>>) : first(std::move(p.first)), second(std::move(p.second)) {}
		template<typename U0, typename U1>
		constexpr packed_pair(packed_pair<U0, U1> &&p) noexcept(nothrow_constructible<type_list_t<U0>, type_list_t<U1>>) : first(std::move(p.first)), second(std::move(p.second)) {}

		template<typename U0, typename U1>
		constexpr packed_pair(const std::pair<U0, U1> &&p) noexcept(nothrow_constructible<type_list_t<U0>, type_list_t<U1>>) : first(std::move(p.first)), second(std::move(p.second)) {}
		template<typename U0, typename U1>
		constexpr packed_pair(const packed_pair<U0, U1> &&p) noexcept(nothrow_constructible<type_list_t<U0>, type_list_t<U1>>) : first(std::move(p.first)), second(std::move(p.second)) {}

		template<typename... Args0, typename... Args1>
		constexpr packed_pair(std::piecewise_construct_t, std::tuple<Args0...> args0, std::tuple<Args1...> args1) noexcept(nothrow_constructible<type_list_t<Args0...>, type_list_t<Args1...>>)
			: packed_pair(std::index_sequence_for<Args0...>{}, args0, std::index_sequence_for<Args1...>{}, args1) {}

		constexpr packed_pair &operator=(const packed_pair &) noexcept(std::is_nothrow_copy_assignable_v<T0> && std::is_nothrow_copy_assignable_v<T1>) = default;
		template<typename U0 = T0, typename U1 = T1>
		constexpr packed_pair &operator=(const std::pair<U0, U1> &p) const noexcept(std::is_nothrow_assignable_v<T0, const U0 &> && std::is_nothrow_assignable_v<T1, const U1 &>)
		{
			first = p.first;
			second = p.second;
			return *this;
		}
		template<typename U0, typename U1>
		constexpr packed_pair &operator=(const packed_pair<U0, U1> &p) const noexcept(std::is_nothrow_assignable_v<T0, const U0 &> && std::is_nothrow_assignable_v<T1, const U1 &>)
		{
			first = p.first;
			second = p.second;
			return *this;
		}

		constexpr packed_pair &operator=(packed_pair &&) noexcept(std::is_nothrow_move_assignable_v<T0> && std::is_nothrow_move_assignable_v<T1>) = default;
		template<typename U0 = T0, typename U1 = T1>
		constexpr packed_pair &operator=(std::pair<U0, U1> &&p) const noexcept(std::is_nothrow_assignable_v<T0, U0> && std::is_nothrow_assignable_v<T1, U1>)
		{
			first = std::move(p.first);
			second = std::move(p.second);
			return *this;
		}
		template<typename U0, typename U1>
		constexpr packed_pair &operator=(packed_pair<U0, U1> &&p) const noexcept(std::is_nothrow_assignable_v<T0, U0> && std::is_nothrow_assignable_v<T1, U1>)
		{
			first = std::move(p.first);
			second = std::move(p.second);
			return *this;
		}

		constexpr const packed_pair &operator=(const packed_pair &p) const noexcept(std::is_nothrow_assignable_v<const T0 &, const T0 &> && std::is_nothrow_assignable_v<const T1 &, const T1 &>)
		{
			first = p.first;
			second = p.second;
			return *this;
		}
		template<typename U0 = T0, typename U1 = T1>
		constexpr const packed_pair &operator=(const std::pair<U0, U1> &p) const noexcept(std::is_nothrow_assignable_v<const T0 &, const U0 &> && std::is_nothrow_assignable_v<const T1 &, const U1 &>)
		{
			first = p.first;
			second = p.second;
			return *this;
		}
		template<typename U0, typename U1>
		constexpr const packed_pair &operator=(const packed_pair<U0, U1> &p) const noexcept(std::is_nothrow_assignable_v<const T0 &, const U0 &> && std::is_nothrow_assignable_v<const T1 &, const U1 &>)
		{
			first = p.first;
			second = p.second;
			return *this;
		}

		constexpr const packed_pair &operator=(packed_pair &&p) const noexcept(std::is_nothrow_assignable_v<const T0 &, T0> && std::is_nothrow_assignable_v<const T1 &, T1>)
		{
			first = std::move(p.first);
			second = std::move(p.second);
			return *this;
		}
		template<typename U0 = T0, typename U1 = T1>
		constexpr const packed_pair &operator=(std::pair<U0, U1> &&p) const noexcept(std::is_nothrow_assignable_v<const T0 &, U0> && std::is_nothrow_assignable_v<const T1 &, U1>)
		{
			first = std::move(p.first);
			second = std::move(p.second);
			return *this;
		}
		template<typename U0, typename U1>
		constexpr const packed_pair &operator=(packed_pair<U0, U1> &&p) const noexcept(std::is_nothrow_assignable_v<const T0 &, U0> && std::is_nothrow_assignable_v<const T1 &, U1>)
		{
			first = std::move(p.first);
			second = std::move(p.second);
			return *this;
		}

		template<std::integral auto I> requires(I < 2)
		friend constexpr auto &get(packed_pair<T0, T1> &p) noexcept
		{
			if constexpr (I == 0)
				return p.first;
			else
				return p.second;
		}
		template<std::integral auto I> requires(I < 2)
		friend constexpr auto &&get(packed_pair<T0, T1> &&p) noexcept
		{
			if constexpr (I == 0)
				return std::move(p.first);
			else
				return std::move(p.second);
		}
		template<std::integral auto I> requires(I < 2)
		friend constexpr const auto &get(const packed_pair<T0, T1> &p) noexcept
		{
			if constexpr (I == 0)
				return p.first;
			else
				return p.second;
		}
		template<std::integral auto I> requires(I < 2)
		friend constexpr const auto &&get(const packed_pair<T0, T1> &&p) noexcept
		{
			if constexpr (I == 0)
				return std::move(p.first);
			else
				return std::move(p.second);
		}

		constexpr void swap(packed_pair &other) noexcept(std::is_nothrow_swappable_v<T0> && std::is_nothrow_swappable_v<T1>)
		{
			using std::swap;
			swap(first, other.first);
			swap(second, other.second);
		}
		friend constexpr void swap(packed_pair &a, packed_pair &b) noexcept(std::is_nothrow_swappable_v<packed_pair>) { return a.swap(b); }

		ROD_NO_UNIQUE_ADDRESS first_type first;
		ROD_NO_UNIQUE_ADDRESS second_type second;
	};

	template<instance_of<std::reference_wrapper> T0, instance_of<std::reference_wrapper> T1, typename P = packed_pair<T0 &, T1 &>>
	[[nodiscard]] constexpr P make_packed_pair(T0 &&v0, T1 &&v1) noexcept { return P{std::forward<T0>(v0), std::forward<T1>(v1)}; }
	template<typename T0, typename T1, typename P = packed_pair<std::decay_t<T0>, std::decay_t<T1>>>
	[[nodiscard]] constexpr P make_packed_pair(T0 &&v0, T1 &&v1) noexcept(std::is_nothrow_constructible_v<P, T0, T1>) { return P{std::forward<T0>(v0), std::forward<T1>(v1)}; }
}

template<typename T0,  typename T1>
struct std::tuple_size<rod::packed_pair<T0, T1>> : std::integral_constant<std::size_t, 2> {};
template<typename T0, typename T1>
struct std::tuple_element<0, rod::packed_pair<T0, T1>> { using type = T0; };
template<typename T0, typename T1>
struct std::tuple_element<1, rod::packed_pair<T0, T1>> { using type = T1; };

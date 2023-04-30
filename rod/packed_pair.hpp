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
	class packed_pair : detail::ebo_helper<T0>, detail::ebo_helper<T1>
	{
		using first_base = detail::ebo_helper<T0>;
		using second_base = detail::ebo_helper<T1>;

	public:
		using first_type = T0;
		using second_type =	T1;

	private:
		template<typename... Ts>
		using first_nothrow_constructible = typename detail::bind_front<std::is_nothrow_constructible, T0>::template type<Ts...>;
		template<typename... Ts>
		using second_nothrow_constructible = typename detail::bind_front<std::is_nothrow_constructible, T1>::template type<Ts...>;

		template<typename Args0, typename Args1>
		static constexpr bool nothrow_constructible = std::conjunction_v<detail::apply_types_t<second_nothrow_constructible, Args1>,
		                                                                 detail::apply_types_t<first_nothrow_constructible, Args0>>;

		template<std::size_t... Is0, typename... Args0, std::size_t... Is1, typename... Args1>
		constexpr packed_pair(std::index_sequence<Is0...>, std::tuple<Args0...> &args0,
		                      std::index_sequence<Is1...>, std::tuple<Args1...> &args1)
				: first_base(std::forward<Args0>(std::get<Is0>(args0))...),
				  second_base(std::forward<Args1>(std::get<Is1>(args1))...)
		{
		}

	public:
		constexpr packed_pair() noexcept(nothrow_constructible<type_list_t<>, type_list_t<>>) = default;
		constexpr packed_pair(const packed_pair &) noexcept(nothrow_constructible<type_list_t<const T0 &>, type_list_t<const T1 &>>) = default;
		constexpr packed_pair(packed_pair &&) noexcept(nothrow_constructible<type_list_t<T0>, type_list_t<T1>>) = default;

		constexpr packed_pair(const T0 &v0, const T1 &v1) noexcept(nothrow_constructible<type_list_t<const T0 &>, type_list_t<const T1 &>>) : first_base(v0), second_base(v1) {}
		template<typename U0 = T0, typename U1 = T1>
		constexpr packed_pair(U0 &&v0, U1 &&v1) noexcept(nothrow_constructible<type_list_t<U0>, type_list_t<U1>>) : first_base(std::forward<U0>(v0)), second_base(std::forward<U1>(v1)) {}

		template<typename U0, typename U1>
		constexpr packed_pair(std::pair<U0, U1> &p) noexcept(nothrow_constructible<type_list_t<U0 &>, type_list_t<U1 &>>) : first_base(p.first), second_base(p.second) {}
		template<typename U0, typename U1>
		constexpr packed_pair(packed_pair<U0, U1> &p) noexcept(nothrow_constructible<type_list_t<U0 &>, type_list_t<U1 &>>) : first_base(p.first()), second_base(p.second()) {}

		template<typename U0 = T0, typename U1 = T1>
		constexpr packed_pair(const std::pair<U0, U1> &p) noexcept(nothrow_constructible<type_list_t<const U0 &>, type_list_t<const U1 &>>) : first_base(p.first), second_base(p.second) {}
		template<typename U0, typename U1>
		constexpr packed_pair(const packed_pair<U0, U1> &p) noexcept(nothrow_constructible<type_list_t<const U0 &>, type_list_t<const U1 &>>) : first_base(p.first()), second_base(p.second()) {}

		template<typename U0 = T0, typename U1 = T1>
		constexpr packed_pair(std::pair<U0, U1> &&p) noexcept(nothrow_constructible<type_list_t<U0>, type_list_t<U1>>) : first_base(std::move(p.first)), second_base(std::move(p.second)) {}
		template<typename U0, typename U1>
		constexpr packed_pair(packed_pair<U0, U1> &&p) noexcept(nothrow_constructible<type_list_t<U0>, type_list_t<U1>>) : first_base(std::move(p.first())), second_base(std::move(p.second())) {}

		template<typename U0, typename U1>
		constexpr packed_pair(const std::pair<U0, U1> &&p) noexcept(nothrow_constructible<type_list_t<U0>, type_list_t<U1>>) : first_base(std::move(p.first)), second_base(std::move(p.second)) {}
		template<typename U0, typename U1>
		constexpr packed_pair(const packed_pair<U0, U1> &&p) noexcept(nothrow_constructible<type_list_t<U0>, type_list_t<U1>>) : first_base(std::move(p.first())), second_base(std::move(p.second())) {}

		template<typename... Args0, typename... Args1>
		constexpr packed_pair(std::piecewise_construct_t, std::tuple<Args0...> args0, std::tuple<Args1...> args1) noexcept(nothrow_constructible<type_list_t<Args0...>, type_list_t<Args1...>>)
			: packed_pair(std::make_index_sequence<sizeof...(Args0)>{}, args0, std::make_index_sequence<sizeof...(Args1)>{}, args1) {}

		constexpr packed_pair &operator=(const packed_pair &) noexcept(std::is_nothrow_copy_assignable_v<T0> && std::is_nothrow_copy_assignable_v<T1>) = default;
		template<typename U0 = T0, typename U1 = T1>
		constexpr packed_pair &operator=(const std::pair<U0, U1> &p) const noexcept(std::is_nothrow_assignable_v<T0, const U0 &> && std::is_nothrow_assignable_v<T1, const U1 &>)
		{
			first_base::operator=(p.first);
			second_base::operator=(p.second);
			return *this;
		}
		template<typename U0, typename U1>
		constexpr packed_pair &operator=(const packed_pair<U0, U1> &p) const noexcept(std::is_nothrow_assignable_v<T0, const U0 &> && std::is_nothrow_assignable_v<T1, const U1 &>)
		{
			first_base::operator=(p.first());
			second_base::operator=(p.second());
			return *this;
		}

		constexpr packed_pair &operator=(packed_pair &&) noexcept(std::is_nothrow_move_assignable_v<T0> && std::is_nothrow_move_assignable_v<T1>) = default;
		template<typename U0 = T0, typename U1 = T1>
		constexpr packed_pair &operator=(std::pair<U0, U1> &&p) const noexcept(std::is_nothrow_assignable_v<T0, U0> && std::is_nothrow_assignable_v<T1, U1>)
		{
			first_base::operator=(std::move(p.first));
			second_base::operator=(std::move(p.second));
			return *this;
		}
		template<typename U0, typename U1>
		constexpr packed_pair &operator=(packed_pair<U0, U1> &&p) const noexcept(std::is_nothrow_assignable_v<T0, U0> && std::is_nothrow_assignable_v<T1, U1>)
		{
			first_base::operator=(std::move(p.first()));
			second_base::operator=(std::move(p.second()));
			return *this;
		}

		constexpr const packed_pair &operator=(const packed_pair &) const noexcept(std::is_nothrow_assignable_v<const T0 &, const T0 &> && std::is_nothrow_assignable_v<const T1 &, const T1 &>) = default;
		template<typename U0 = T0, typename U1 = T1>
		constexpr const packed_pair &operator=(const std::pair<U0, U1> &p) const noexcept(std::is_nothrow_assignable_v<const T0 &, const U0 &> && std::is_nothrow_assignable_v<const T1 &, const U1 &>)
		{
			first_base::operator=(p.first);
			second_base::operator=(p.second);
			return *this;
		}
		template<typename U0, typename U1>
		constexpr const packed_pair &operator=(const packed_pair<U0, U1> &p) const noexcept(std::is_nothrow_assignable_v<const T0 &, const U0 &> && std::is_nothrow_assignable_v<const T1 &, const U1 &>)
		{
			first_base::operator=(p.first());
			second_base::operator=(p.second());
			return *this;
		}

		constexpr const packed_pair &operator=(packed_pair &&) const noexcept(std::is_nothrow_assignable_v<const T0 &, T0> && std::is_nothrow_assignable_v<const T1 &, T1>) = default;
		template<typename U0 = T0, typename U1 = T1>
		constexpr const packed_pair &operator=(std::pair<U0, U1> &&p) const noexcept(std::is_nothrow_assignable_v<const T0 &, U0> && std::is_nothrow_assignable_v<const T1 &, U1>)
		{
			first_base::operator=(std::move(p.first));
			second_base::operator=(std::move(p.second));
			return *this;
		}
		template<typename U0, typename U1>
		constexpr const packed_pair &operator=(packed_pair<U0, U1> &&p) const noexcept(std::is_nothrow_assignable_v<const T0 &, U0> && std::is_nothrow_assignable_v<const T1 &, U1>)
		{
			first_base::operator=(std::move(p.first()));
			second_base::operator=(std::move(p.second()));
			return *this;
		}

		[[nodiscard]] constexpr T0 &first() & noexcept { return detail::ebo_helper<T0>::value(); }
		[[nodiscard]] constexpr T0 &first() const & noexcept { return detail::ebo_helper<T0>::value(); }
		[[nodiscard]] constexpr T0 &&first() && noexcept { return std::move(detail::ebo_helper<T0>::value()); }
		[[nodiscard]] constexpr T0 &&first() const && noexcept { return std::move(detail::ebo_helper<T0>::value()); }

		[[nodiscard]] constexpr T1 &second() & noexcept { return detail::ebo_helper<T1>::value(); }
		[[nodiscard]] constexpr T1 &second() const & noexcept { return detail::ebo_helper<T1>::value(); }
		[[nodiscard]] constexpr T1 &&second() && noexcept { return std::move(detail::ebo_helper<T1>::value()); }
		[[nodiscard]] constexpr T1 &&second() const && noexcept { return std::move(detail::ebo_helper<T1>::value()); }

		constexpr void swap(packed_pair &other) noexcept(std::is_nothrow_swappable_v<T0> && std::is_nothrow_swappable_v<T1>)
		{
			using std::swap;
			swap(first(), other.first());
			swap(second(), other.second());
		}
		friend constexpr void swap(packed_pair &a, packed_pair &b) noexcept(std::is_nothrow_swappable_v<packed_pair>) { return a.swap(b); }
	};

	template<instance_of<std::reference_wrapper> T0, instance_of<std::reference_wrapper> T1, typename P = packed_pair<T0 &, T1 &>>
	[[nodiscard]] constexpr P make_packed_pair(T0 &&v0, T1 &&v1) noexcept { return {std::forward<T0>(v0), std::forward<T1>(v1)}; }
	template<typename T0, typename T1, typename P = packed_pair<std::decay_t<T0>, std::decay_t<T1>>>
	[[nodiscard]] constexpr P make_packed_pair(T0 &&v0, T1 &&v1) noexcept(std::is_nothrow_constructible_v<P, T0, T1>) { return {std::forward<T0>(v0), std::forward<T1>(v1)}; }
}

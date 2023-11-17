/*
 * Created by switchblade on 2023-06-07.
 */

#pragma once

#include "async_base.hpp"

namespace rod
{
	namespace _receiver_adaptor
	{
		struct default_base {};

		template<typename Child, typename Base>
		struct receiver_adaptor { class type; };
		template<typename Child, typename Base>
		class receiver_adaptor<Child, Base>::type : empty_base<Base>
		{
			friend Child;

			template<typename T, typename Res = copy_cvref_t<std::add_rvalue_reference_t<T>, type>> requires std::derived_from<std::decay_t<T>, type>
			static constexpr decltype(auto) base_cast(T &&value) noexcept { return (Res) std::forward<T>(value); }
			template<typename T>
			static constexpr decltype(auto) get_base(T &&value) noexcept
			{
				if constexpr (!decays_to_same<Base, default_base>)
					return base_cast(std::forward<T>(value)).base();
				else
					return std::forward<T>(value).base();
			}

			static constexpr int do_get_env = 1;

			template<typename T>
			static constexpr bool has_get_env() noexcept { return !requires { requires bool(int(std::decay_t<T>::do_get_env)); }; }
			template<typename T>
			constexpr static auto dispatch_get_env(T &&r) noexcept(noexcept(std::forward<T>(r).get_env())) -> decltype(std::forward<T>(r).do_get_env()) { return std::forward<T>(r).do_get_env(); }

			static constexpr int do_set_value = 1;
			static constexpr int do_set_error = 1;
			static constexpr int do_set_stopped = 1;

			template<typename T>
			static constexpr bool has_set_value() noexcept { return !requires { requires bool(int(std::decay_t<T>::do_set_value)); }; }
			template<typename T>
			static constexpr bool has_set_error() noexcept { return !requires { requires bool(int(std::decay_t<T>::do_set_error)); }; }
			template<typename T>
			static constexpr bool has_set_stopped() noexcept { return !requires { requires bool(int(std::decay_t<T>::do_set_stopped)); }; }

			template<typename... Args>
            constexpr static decltype(auto) dispatch_set_value(Child &&r, Args &&...args) noexcept
			{
				static_assert(noexcept(std::forward<Child>(r).do_set_value(std::forward<Args>(args)...)));
				return std::forward<Child>(r).do_set_value(std::forward<Args>(args)...);
            }
			template<typename Err>
			constexpr static decltype(auto) dispatch_set_error(Child &&r, Err &&err) noexcept
			{
				static_assert(noexcept(std::forward<Child>(r).do_set_error(std::forward<Err>(err))));
				return std::forward<Child>(r).do_set_error(std::forward<Err>(err));
			}
			constexpr static decltype(auto) dispatch_set_stopped(Child &&r) noexcept
			{
				static_assert(noexcept(std::forward<Child>(r).do_set_stopped()));
				return std::forward<Child>(r).do_set_stopped();
			}

		public:
			using is_receiver = std::true_type;

			using empty_base<Base>::empty_base;

			template<std::same_as<get_env_t> Q, decays_to_same<Child> T = Child> requires(has_get_env<T>())
			friend constexpr decltype(auto) tag_invoke(Q, T &&r) noexcept { return dispatch_get_env(std::forward<T>(r)); }
			template<std::same_as<get_env_t> Q, decays_to_same<Child> T = Child> requires(!has_get_env<T>())
			friend constexpr decltype(auto) tag_invoke(Q, T &&r) noexcept { return Q{}(get_base(std::forward<T>(r))); }

			template<std::same_as<set_value_t> C, decays_to_same_or_derived<Child> T = Child, typename... Args> requires(has_set_value<T>())
			friend constexpr void tag_invoke(C, T &&r, Args &&...args) noexcept { dispatch_set_value(std::forward<T>(r), std::forward<Args>(args)...); }
			template<std::same_as<set_value_t> C, decays_to_same_or_derived<Child> T = Child, typename... Args> requires(!has_set_value<T>())
			friend constexpr void tag_invoke(C, T &&r, Args &&...args) noexcept { C{}(get_base(std::forward<T>(r)), std::forward<Args>(args)...); }

			template<std::same_as<set_error_t> C, typename Err, decays_to_same_or_derived<Child> T = Child> requires(has_set_error<T>())
			friend constexpr void tag_invoke(C, T &&r, Err &&err) noexcept { dispatch_set_error(std::forward<T>(r), std::forward<Err>(err)); }
			template<std::same_as<set_error_t> C, typename Err, decays_to_same_or_derived<Child> T = Child> requires(!has_set_error<T>())
			friend constexpr void tag_invoke(C, T &&r, Err &&err) noexcept { C{}(get_base(std::forward<T>(r)), std::forward<Err>(err)); }

			template<std::same_as<set_stopped_t> C, decays_to_same_or_derived<Child> T = Child> requires(has_set_stopped<T>())
			friend constexpr void tag_invoke(C, T &&r) noexcept { dispatch_set_stopped(std::forward<T>(r)); }
			template<std::same_as<set_stopped_t> C, decays_to_same_or_derived<Child> T = Child> requires(!has_set_stopped<T>())
			friend constexpr void tag_invoke(C, T &&r) noexcept { C{}(get_base(std::forward<T>(r))); }

		protected:
			constexpr decltype(auto) base() & noexcept { return static_cast<Base &>(empty_base<Base>::value()); }
			constexpr decltype(auto) base() const & noexcept { return static_cast<std::add_const_t<Base> &>(empty_base<Base>::value()); }
			constexpr decltype(auto) base() && noexcept { return static_cast<Base &&>(empty_base<Base>::value()); }
			constexpr decltype(auto) base() const && noexcept { return static_cast<std::add_const_t<Base> &&>(empty_base<Base>::value()); }
		};
	}

	template<typename Child, typename Base = _receiver_adaptor::default_base>
	using receiver_adaptor = typename _receiver_adaptor::receiver_adaptor<Child, Base>::type;
}

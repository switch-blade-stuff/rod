/*
 * Created by switchblade on 2023-06-07.
 */

#pragma once

#include "concepts.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	namespace _receiver_adaptor
	{
		struct default_base {};

		template<typename To, typename From>
		[[nodiscard]] inline copy_cvref_t<From &&, To> base_cast(From &&from) noexcept
		{
			static_assert(std::is_reference_v<copy_cvref_t<From &&, To>>);
			static_assert(std::is_base_of_v<To, std::decay_t<From>>);
			return (copy_cvref_t<From &&, To>) std::forward<From>(from);
		}

		template<typename Base>
		struct adaptor_base { struct type; };
		template<typename Child, typename Base>
		struct receiver_adaptor { class type; };

		template<>
		struct adaptor_base<default_base>::type { };
		template<typename Base>
		struct adaptor_base<Base>::type
		{
		public:
			constexpr type() noexcept(std::is_nothrow_default_constructible_v<Base>) = default;
			template<typename T> requires std::constructible_from<Base, T>
			constexpr explicit type(T &&value) noexcept(std::is_nothrow_constructible_v<Base, T>) : _base(std::forward<T>(value)) {}

			constexpr Base &base() & noexcept { return _base; }
			constexpr const Base &base() const & noexcept { return _base; }
			constexpr Base &&base() && noexcept { return std::move(_base); }
			constexpr const Base &&base() const && noexcept { return std::move(_base); }

		private:
			[[ROD_NO_UNIQUE_ADDRESS]] Base _base = {};
		};

		template<typename Child, typename Base>
		class receiver_adaptor<Child, Base>::type : adaptor_base<Base>::type
		{
			friend Child;

			template<typename T>
			static decltype(auto) get_base(T &&value) noexcept
			{
				if constexpr (!decays_to<Base, default_base>)
					return base_cast<typename adaptor_base<Base>::type>(std::forward<T>(value)).base();
				else
					return std::forward<T>(value).base();
			}

			static constexpr int get_env = 1;

			template<typename T>
			static constexpr bool has_get_env() noexcept { return !requires { requires bool(int(T::get_env)); }; }
			template<typename T>
			constexpr static auto dispatch_get_env(T &&r) noexcept(noexcept(std::forward<T>(r).get_env())) -> decltype(std::forward<T>(r).get_env())
			{
				return std::forward<T>(r).get_env();
			}

			static constexpr int set_value = 1;
			static constexpr int set_error = 1;
			static constexpr int set_stopped = 1;

			template<typename T>
			static constexpr bool has_set_value() noexcept { return !requires { requires bool(int(T::set_value)); }; }
			template<typename T>
			static constexpr bool has_set_error() noexcept { return !requires { requires bool(int(T::set_error)); }; }
			template<typename T>
			static constexpr bool has_set_stopped() noexcept { return !requires { requires bool(int(T::set_stopped)); }; }

			template<typename T, typename... Args>
            constexpr static decltype(auto) dispatch_set_value(T &&r, Args &&...args) noexcept
			{
              static_assert(noexcept(std::forward<T>(r).set_value(std::forward<Args>(args)...)));
              return std::forward<T>(r).set_value(std::forward<Args>(args)...);
            }
			template<typename T, typename Err>
			constexpr static decltype(auto) dispatch_set_error(T &&r, Err &&err) noexcept
			{
				static_assert(noexcept(std::forward<T>(r).set_error(std::forward<Err>(err))));
				return std::forward<T>(r).set_error(std::forward<Err>(err));
			}
			template<typename T>
			constexpr static decltype(auto) dispatch_set_stopped(T &&r) noexcept
			{
				static_assert(noexcept(std::forward<T>(r).set_stopped()));
				return std::forward<T>(r).set_stopped();
			}

		public:
			using is_receiver = std::true_type;

			using adaptor_base<Base>::type::type;

			template<std::same_as<get_env_t> Q> requires(has_get_env<Child>())
			friend decltype(auto) tag_invoke(Q, Child &&r) noexcept { return dispatch_get_env(std::forward<Child>(r)); }
			template<std::same_as<get_env_t> Q, typename T = Child> requires(!has_get_env<T>())
			friend decltype(auto) tag_invoke(Q, T &&r) noexcept { return rod::get_env(get_base(std::forward<T>(r))); }

			template<std::same_as<set_value_t> C, typename... Args> requires(has_set_value<Child>())
			friend void tag_invoke(C, Child &&r, Args &&...args) noexcept { dispatch_set_value(std::forward<Child>(r), std::forward<Args>(args)...); }
			template<std::same_as<set_value_t> C, typename T = Child, typename... Args> requires(!has_set_value<T>())
			friend void tag_invoke(C, T &&r, Args &&...args) noexcept { rod::set_value(get_base(std::forward<T>(r)), std::forward<Args>(args)...); }

			template<std::same_as<set_error_t> C, typename Err> requires(has_set_error<Child>())
			friend void tag_invoke(C, Child &&r, Err &&err) noexcept { dispatch_set_error(std::forward<Child>(r), std::forward<Err>(err)); }
			template<std::same_as<set_error_t> C, typename Err, typename T = Child> requires(!has_set_error<T>())
			friend void tag_invoke(C, T &&r, Err &&err) noexcept { rod::set_error(get_base(std::forward<T>(r)), std::forward<Err>(err)); }

			template<std::same_as<set_stopped_t> C> requires(has_set_stopped<Child>())
			friend void tag_invoke(C, Child &&r) noexcept { dispatch_set_stopped(std::forward<Child>(r)); }
			template<std::same_as<set_stopped_t> C, typename T = Child> requires(!has_set_stopped<T>())
			friend void tag_invoke(C, T &&r) noexcept { rod::set_stopped(get_base(std::forward<T>(r))); }

		protected:
			using adaptor_base<Base>::type::base;
		};
	}

	template<typename Child, typename Base = _receiver_adaptor::default_base>
	using receiver_adaptor = typename _receiver_adaptor::receiver_adaptor<Child, Base>::type;
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

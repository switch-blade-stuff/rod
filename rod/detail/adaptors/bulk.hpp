/*
 * Created by switchblade on 2023-05-15.
 */

#pragma once

#include <functional>

#include "../queries/completion.hpp"
#include "../receiver_adaptor.hpp"
#include "closure.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod
{
	namespace _bulk
	{
		template<typename, typename, typename>
		struct receiver { class type; };
		template<typename, typename, typename>
		struct sender { struct type; };

		template<typename Rcv, typename Shape, typename F>
		class receiver<Rcv, Shape, F>::type : public receiver_adaptor<type, Rcv>
		{
			friend receiver_adaptor<type, Rcv>;

		public:
			constexpr type(Rcv rcv, Shape shape, F fn) : receiver_adaptor<type, Rcv>(std::move(rcv)), _shape(shape), _fn(std::move(fn)) {}

		private:
			template<typename... Args>
			void set_value(Args &&...args) && noexcept requires detail::callable<F, Shape, Args &...> try
			{
				for (auto i = Shape{}; i != _shape; ++i) _fn(i, args...);
				rod::set_value(std::move(receiver_adaptor<type, Rcv>::base()), std::forward<Args>(args)...);
			}
			catch (...)
			{
				rod::set_error(std::move(receiver_adaptor<type, Rcv>::base()), std::current_exception());
			}
			template<typename... Args>
			void set_value(Args &&...args) && noexcept requires detail::nothrow_callable<F, Shape, Args &...>
			{
				for (auto i = Shape{}; i != _shape; ++i) _fn(i, args...);
				rod::set_value(std::move(receiver_adaptor<type, Rcv>::base()), std::forward<Args>(args)...);
			}

			[[ROD_NO_UNIQUE_ADDRESS]] Shape _shape;
			[[ROD_NO_UNIQUE_ADDRESS]] F _fn;
		};

		template<typename Snd, typename Shape, typename F>
		struct sender<Snd, Shape, F>::type
		{
			using is_sender = std::true_type;

			template<typename Rcv>
			using _receiver_t = typename receiver<Rcv, Shape, F>::type;

			template<typename... Ts>
			using _test_nothrow = std::is_nothrow_invocable<F, Shape, detail::decayed_ref<Ts>...>;
			template<typename S, typename E>
			using _error_signs_t = std::conditional_t<value_types_of_t<S, E, _test_nothrow, std::conjunction>::value, completion_signatures<>, completion_signatures<set_error_t(std::exception_ptr)>>;
			template<typename T, typename E>
			using _signs_t = make_completion_signatures<copy_cvref_t<T, Snd>, E, _error_signs_t<copy_cvref_t<T, Snd>, E>>;

			friend constexpr env_of_t<Snd> tag_invoke(get_env_t, const type &s) noexcept(detail::nothrow_callable<get_env_t, const Snd &>) { return get_env(s._snd); }
			template<decays_to<type> T, typename E>
			friend constexpr _signs_t<T, E> tag_invoke(get_completion_signatures_t, T &&, E) noexcept { return {}; }

			template<decays_to<type> T, rod::receiver Rcv> requires sender_to<copy_cvref_t<T, Snd>, _receiver_t<Rcv>>
			friend constexpr decltype(auto) tag_invoke(connect_t, T &&s, Rcv rcv) noexcept(detail::nothrow_callable<connect_t, copy_cvref_t<T, Snd>, _receiver_t<Rcv>>)
			{
				return connect(std::forward<T>(s)._snd, _receiver_t<Rcv>{std::move(rcv), s._shape, std::forward<T>(s)._fn});
			}

			[[ROD_NO_UNIQUE_ADDRESS]] Shape _shape;
			[[ROD_NO_UNIQUE_ADDRESS]] Snd _snd;
			[[ROD_NO_UNIQUE_ADDRESS]] F _fn;
		};

		class bulk_t
		{
			template<typename Snd>
			using value_scheduler = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Snd, typename Shape, typename F>
			using sender_t = typename sender<std::decay_t<Snd>, Shape, std::decay_t<F>>::type;
			template<typename Shape, typename F>
			using back_adaptor = detail::back_adaptor<bulk_t, Shape, std::decay_t<F>>;
			
		public:
			template<rod::sender Snd, std::integral Shape, movable_value F> requires detail::tag_invocable_with_completion_scheduler<bulk_t, set_value_t, Snd, Snd, Shape, F>
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd, Shape shape, F &&fn) const noexcept(nothrow_tag_invocable<bulk_t, value_scheduler<Snd>, Snd, Shape, F>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::move(shape), std::forward<F>(fn));
			}
			template<rod::sender Snd, std::integral Shape, movable_value F> requires(!detail::tag_invocable_with_completion_scheduler<bulk_t, set_value_t, Snd, Snd, Shape, F> && tag_invocable<bulk_t, Snd, Shape, F>)
			[[nodiscard]] constexpr rod::sender auto operator()(Snd &&snd, Shape shape, F &&fn) const noexcept(nothrow_tag_invocable<bulk_t, Snd, Shape, F>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::move(shape), std::forward<F>(fn));
			}
			template<rod::sender Snd, std::integral Shape, movable_value F> requires(!detail::tag_invocable_with_completion_scheduler<bulk_t, set_value_t, Snd, Snd, Shape, F> && !tag_invocable<bulk_t, Snd, Shape, F>)
			[[nodiscard]] constexpr sender_t<Snd, Shape, F> operator()(Snd &&snd, Shape shape, F &&fn) const noexcept(std::is_nothrow_constructible_v<sender_t<Snd, Shape, F>, Shape, Snd, F>)
			{
				return sender_t<Snd, Shape, F>{std::move(shape), std::forward<Snd>(snd), std::forward<F>(fn)};
			}

			template<std::integral Shape, movable_value F>
			[[nodiscard]] constexpr back_adaptor<Shape, F> operator()(Shape shape, F &&fn) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Shape, F>, bulk_t, Shape, F>)
			{
				return back_adaptor<Shape, F>{*this, {std::move(shape), std::forward<F>(fn)}};
			}
		};
	}

	using _bulk::bulk_t;

	/** Customization point object used to execute a functor repeatedly over an index space.
	 * @param snd Sender who's value channel results will be passed as arguments to the functor. If omitted, creates a pipe-able sender adaptor.
	 * @param shape Integral shape (maximum loop index) of the bulk task.
	 * @param fn Functor to be executed \a shape times with value results of \a snd.
	 * @return Sender that invokes \a fn \a shape times with value results of \a snd, then forwards the value channel results to connected receiver. */
	inline constexpr auto bulk = bulk_t{};
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

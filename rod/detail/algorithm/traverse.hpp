/*
 * Created by switch_blade on 2023-09-25.
 */

#pragma once

#include "../directory_handle.hpp"
#include "../receiver_adaptor.hpp"

namespace rod
{
	namespace _traverse
	{
		using handle_error = typename decltype(fs::directory_handle::open({}, {}))::error_type;

		template<typename V>
		using accept_entry_result = decltype(std::declval<V>().accept_entry(std::declval<const fs::directory_handle &>(), std::declval<fs::path_view>(), std::declval<stat>(), std::declval<stat::query>(), std::declval<std::size_t>()));
		template<typename V>
		using ignore_error_result = decltype(std::declval<V>().ignore_error(std::declval<handle_error>(), std::declval<const fs::directory_handle &>(), std::declval<fs::path_view>(), std::declval<std::size_t>()));
		template<typename V>
		using accept_handle_result = decltype(std::declval<V>().accept_handle(std::declval<const fs::directory_handle &>(), std::declval<std::size_t>()));
		template<typename V>
		using visitor_result = decltype(std::declval<V>().result());

		/* Metafunctions to deduce an optional result error type. */
		template<typename T>
		struct result_errors_impl { using type = std::tuple<>; };
		template<typename T> requires instance_of<T, result>
		struct result_errors_impl<T> { using type = std::tuple<typename T::error_type>; };
		template<typename T>
		using result_errors = typename result_errors_impl<std::decay_t<T>>::type;

		/* Metafunctions to deduce a tuple of result value types, which may itself be a tuple or `void`. */
		template<typename T>
		struct result_values_impl { using type = std::tuple<T>; };
		template<typename T> requires tuple_like<T>
		struct result_values_impl<T> { using type = T; };
		template<typename T> requires std::is_void_v<T>
		struct result_values_impl<T> { using type = std::tuple<>; };
		template<typename T> requires instance_of<T, result>
		struct result_values_impl<T> : result_values_impl<typename T::value_type> {};
		template<typename T>
		using result_values = typename result_values_impl<std::decay_t<T>>::type;

		/* Metafunctions to deduce visitor's error types from `visitor::result`, `visitor::accept_entry`, `visitor::ignore_error`, and `visitor::accept_handle`. */
		template<typename V>
		using visitor_errors = _detail::concat_tuples_t<result_errors<visitor_result<V>>, result_errors<accept_entry_result<V>>, result_errors<ignore_error_result<V>>, result_errors<accept_handle_result<V>>>;
		/* Metafunctions to deduce visitor's value types from `visitor::result`. */
		template<typename V>
		using visitor_values = _detail::concat_tuples_t<result_values<visitor_result<V>>>;

		/* Metafunctions to deduce exception status of receiver's value completion channel and visitor's functions. */
		template<typename V>
		using nothrow_visitor_status = std::conjunction<_detail::apply_tuple_t<_detail::all_nothrow_decay_copyable, visitor_errors<V>>, std::bool_constant<noexcept(std::declval<V>().result())>>;
		template<typename Rcv, typename V>
		using nothrow_value_channel = _detail::apply_tuple_t<_detail::bind_front<std::is_nothrow_invocable, set_value_t, Rcv &&>::template type, visitor_values<V>>;

		/* Metafunctions to deduce a tuple of error types as a combination of visitor's errors, handle error and exception status. */
		template<typename Rcv, typename V>
		using exception_error = std::conditional_t<nothrow_visitor_status<V>::value && nothrow_value_channel<Rcv, V>::value, std::tuple<>, std::tuple<std::exception_ptr>>;
		template<typename Rcv, typename V>
		using errors_tuple = _detail::concat_tuples_t<std::tuple<handle_error>, exception_error<Rcv, V>, visitor_errors<V>>;

		/* Visitor ignore & accept functions may either return a boolean or a boolean result to indicate an explicit error. */
		template<typename T>
		concept boolean_result = (instance_of<T, result> && std::convertible_to<typename T::value_type, bool>) || std::convertible_to<T, bool>;
	}

	namespace fs
	{
		template<typename V>
		concept traverse_visitor = requires(V &v, _traverse::handle_error e, const directory_handle &h, path_view p, stat s, stat::query q, std::size_t l)
		{
			{ v.accept_entry(h, p, s, q, l) } -> _traverse::boolean_result;
			{ v.ignore_error(e, h, p, l) } -> _traverse::boolean_result;
			{ v.accept_handle(h, l) } -> _traverse::boolean_result;
			{ v.result() };
		};
	}

	namespace _traverse
	{
		template<typename Snd, typename V>
		struct sender { class type; };
		template<typename Sch, typename Rcv, typename V>
		struct receiver { class type; };
		template<typename Sch, typename Rcv, typename V>
		struct operation { class type; };

		template<typename C, typename...>
		struct channel_result
		{
			constexpr channel_result() noexcept = default;

			constexpr std::tuple<> operator()() const noexcept { return {}; }
		};
		template<typename V>
		struct channel_result<set_value_t, V> : empty_base<V>
		{
			channel_result() = delete;
			template<typename V2>
			constexpr explicit channel_result(V2 &&v) noexcept(std::is_nothrow_constructible_v<V, V2>) : empty_base<V>(std::forward<V2>(v)) {}

			constexpr decltype(auto) operator()() noexcept(noexcept(std::move(empty_base<V>::value()).result())) { return empty_base<V>::value().result(); }
		};
		template<typename E>
		struct channel_result<set_error_t, E> : empty_base<E>
		{
			channel_result() = delete;
			template<typename E2>
			constexpr explicit channel_result(E2 &&e) noexcept(std::is_nothrow_constructible_v<E, E2>) : empty_base<E>(std::forward<E2>(e)) {}

			constexpr decltype(auto) operator()() && noexcept { return std::forward_as_tuple(std::move(empty_base<E>::value())); }
		};

		template<typename Sch, typename Rcv, typename V>
		class receiver<Sch, Rcv, V>::type
		{
			using next_state_t = connect_result_t<schedule_result_t<Sch &>, type>;
			using operation_t = typename operation<Sch, Rcv, V>::type;
			using receiver_t = typename receiver<Sch, Rcv, V>::type;

		public:
			constexpr type(operation_t *op, fs::directory_handle &&base) : type(nullptr, op, std::forward<fs::directory_handle>(base), 0) {}
			constexpr type(next_state_t *next, operation_t *op, fs::directory_handle &&base, std::size_t level) : _next(next), _op(op), _base(std::forward<fs::directory_handle>(base)), _level(level) {}

			~type() { delete _next; }

		private:
			/* Algorithm (depth-first recursive async traversal):
			 * 1. Enumerate the top-level directory until EOF.
			 * 2. For each directory entry:
			 *  2.1. Report (and optionally skip) enumerated entry via `visitor.accept_entry`.
			 *  2.2. If the entry is a subdirectory (and not skipped in 2.1):
			 *      2.1.1. Try to open subdirectory handle for reading.
			 *      2.1.2. Report and/or skip errors via `visitor.ignore_error`.
			 *      2.1.3. Filter subdirectory handle via `visitor.accept_handle`.
			 *      2.1.4. Increment worker counter and schedule subdirectory traversal.
			 * 3. Decrement worker counter.
			 * 4. If the worker counter is 0, complete the downstream receiver with result of `visitor.complete`.
			 *
			 * In case an error is reported at any stage of the above algorithm, it is propagated to the downstream receiver.
			 * Additional checks are preformed to ensure no new operations are scheduled after an error is reported. */
			inline void run() noexcept;

			next_state_t *_next;
			operation_t *_op;

			fs::directory_handle _base;
			std::size_t _level;
		};

		template<typename Sch, typename Rcv, typename V>
		class operation<Sch, Rcv, V>::type : empty_base<Sch>, empty_base<Rcv>
		{
			friend class receiver<Sch, Rcv, V>::type;

			using receiver_t = typename receiver<Sch, Rcv, V>::type;
			using state_t = connect_result_t<schedule_result_t<Sch &>, receiver_t>;

			using stopped_variant = std::variant<channel_result<set_stopped_t>>;
			using value_variant = std::variant<channel_result<set_value_t, V>>;
			template<typename... Errs>
			using error_variant = std::variant<channel_result<set_error_t, std::decay_t<Errs>>...>;
			using data_t = unique_tuple_t<_detail::concat_tuples_t<value_variant, _detail::apply_tuple_t<error_variant, errors_tuple<Rcv, V>>, stopped_variant>>;

		public:
			type(type &&) = delete;
			type &operator=(type &&) = delete;

			template<typename Sch2, typename Rcv2, typename V2>
			constexpr type(Sch2 &&sch, Rcv2 &&rcv, V2 &&v, fs::directory_handle &&base) : empty_base<Sch>(std::forward<Sch2>(sch)), empty_base<Rcv>(std::forward<Rcv2>(rcv)), _workers(1), _data(std::forward<V2>(v))
			{
				_worker_state = new state_t(connect(schedule(empty_base<Sch>::value()), receiver_t(this, std::forward<fs::directory_handle>(base))));
			}

			~type() { delete _worker_state; }

			friend constexpr void tag_invoke(start_t, type &op) noexcept { start(*op._worker_state); }

		private:
			[[nodiscard]] auto &visitor() noexcept { return std::get<channel_result<set_value_t, V>>(_data).empty_base<V>::value(); }
			[[nodiscard]] auto &visitor() const noexcept { return std::get<channel_result<set_value_t, V>>(_data).empty_base<V>::value(); }

			template<typename C>
			constexpr void apply_result(auto &res) noexcept
			{
				const auto do_apply = [&]() noexcept(noexcept(std::move(res)()))
				{
					if constexpr (std::is_void_v<decltype(std::move(res)())>)
						(std::move(res)(), invoke_channel<C>());
					else
						apply_channel<C>(std::move(res)());
				};
				if constexpr (!noexcept(do_apply()))
					try { do_apply(); } catch (...) { invoke_channel<set_error_t>(std::current_exception()); }
				else
					do_apply();
			}
			template<typename C, typename Res>
			constexpr void apply_channel(Res &&res) noexcept
			{
				invoke_channel<C>(std::forward<Res>(res));
			}
			template<typename C, typename Res> requires tuple_like<std::decay_t<Res>>
			constexpr void apply_channel(Res &&res) noexcept
			{
				std::apply([&]<typename... Args>(Args &&...args) { invoke_channel<C>(std::forward<Args>(args)...); }, std::forward<Res>(res));
			}
			template<typename C, typename Res> requires decays_to_instance_of<Res, result>
			constexpr void apply_channel(Res &&res) noexcept
			{
				if (res.has_error())
					set_error(std::move(empty_base<Rcv>::value()), res.error());
				else
					apply_channel<C>(std::move(*res));
			}
			template<typename C, typename... Args>
			constexpr void invoke_channel(Args &&...args) noexcept
			{
				if constexpr (!_detail::nothrow_callable<C, Rcv &&, Args...> && !std::same_as<C, set_error_t>)
					try { C{}(std::move(empty_base<Rcv>::value()), std::forward<Args>(args)...); } catch(...) { invoke_channel<set_error_t>(std::current_exception()); }
				else
					C{}(std::move(empty_base<Rcv>::value()), std::forward<Args>(args)...);
			}

			std::size_t lock() noexcept
			{
				std::size_t workers;
				while ((workers = _workers.exchange(0, std::memory_order_acq_rel)) == 0)
					_workers.wait(workers);
				return workers;
			}
			void unlock(std::size_t workers) noexcept
			{
				_workers.store(workers, std::memory_order_release);
				_workers.notify_one();
			}

			auto decref_guard() noexcept
			{
				const auto workers = lock();
				return std::make_pair(workers, defer_invoke([this, workers]() noexcept { unlock(workers - 1); }));
			}
			auto incref_guard() noexcept
			{
				const auto workers = lock();
				return std::make_pair(workers, defer_invoke([this, workers]() noexcept { unlock(workers + 1); }));
			}

			void complete_next() noexcept
			{
				/* Complete the selected channel using the result provided by channel_result::operator() */
				std::visit([&]<typename C, typename... Ts>(channel_result<C, Ts...> &res) noexcept { apply_result<C>(res); }, _data);
				/* Delete the worker chain once the last one completes. This will bypass the destructor's `delete`. */
				delete std::exchange(_worker_state, nullptr);
			}
			void complete_stop() noexcept
			{
				const auto [workers, _] = decref_guard();
				/* Switch to the stop state on first stop request. */
				if (_data.index() == 0) [[likely]]
					_data.template emplace<channel_result<set_stopped_t>>();
				/* Last worker completes the operation. */
				if (workers == 1) [[unlikely]]
					complete_next();
			}
			void complete_value() noexcept
			{
				const auto [workers, _] = decref_guard();
				/* Last worker completes the operation. */
				if (workers == 1) [[unlikely]]
					complete_next();
			}
			template<typename Err> requires std::constructible_from<data_t, channel_result<set_error_t, std::decay_t<Err>>>
			void complete_error(Err &&err) noexcept
			{
				const auto [workers, _] = decref_guard();
				/* Switch to the error state on first error. */
				if (_data.index() == 0) [[likely]]
					_data.template emplace<channel_result<set_error_t, std::decay_t<Err>>>(std::forward<Err>(err));
				/* Last worker completes the operation. */
				if (workers == 1) [[unlikely]]
					complete_next();
			}

			std::atomic<std::size_t> _workers;
			state_t *_worker_state = nullptr;
			data_t _data;
		};

		template<typename Sch, typename Rcv, typename V>
		void receiver<Sch, Rcv, V>::type::run() noexcept
		{
			auto buffs = std::vector<read_some_buffer_t<fs::directory_handle>>(64);
			auto req = read_some_request_t<fs::directory_handle>{.buffs = buffs};
			for (;;)
			{
				auto read_res = read_some(_base, std::move(req));
				if (read_res.has_error()) [[unlikely]]
				{
					_op->complete_error(read_res.error());
					return;
				}

				/* Expand the buffer if full and attempt another read. */
				if (read_res->size() == buffs.size()) [[unlikely]]
				{
					buffs.resize(buffs.size() * 2);
					req.buffs = {std::move(*read_res), buffs};
					continue;
				}

				/* Enumerate directory entries. */
				for (auto &entry : *read_res)
				{
					auto [st, st_mask] = entry.st();

					/* Make sure we have the required stats. Type is needed for further subdirectory traversal. */
					if (!bool(st_mask & stat::query::type))
					{
						auto st_res = get_stat(st, _base, entry.path(), stat::query::type);
						if (st_res.has_error()) [[unlikely]]
						{
							_op->complete_error(st_res.error());
							return;
						}
						st_mask |= *st_res;
					}

					{ /* Report entry to the visitor. */
						auto accept_res = _op->visitor().accept_entry(_base, entry.path(), st, st_mask, _level);
						bool accept = bool(accept_res);
						if constexpr (decays_to_instance_of<decltype(accept_res), result>)
						{
							if (accept_res.has_error()) [[unlikely]]
							{
								_op->complete_error(accept_res.error());
								return;
							}
							accept = bool(*accept_res);
						}
						if (!accept || st.type != fs::file_type::directory)
							continue;
					}

					/* Attempt to open the subdirectory and report errors. */
					auto subdir = fs::directory_handle::open(_base, entry.path());
					if (subdir.has_error()) [[unlikely]]
					{
						/* Ignore the subdirectory as requested by the user. */
						auto ignore_res = _op->visitor().ignore_error(subdir.error(), _base, entry.path(), _level);
						bool ignore = bool(ignore_res);
						if constexpr (decays_to_instance_of<decltype(ignore_res), result>)
						{
							if (ignore_res.has_error()) [[unlikely]]
							{
								_op->complete_error(ignore_res.error());
								return;
							}
							ignore = bool(*ignore_res);
						}
						if (!ignore)
						{
							_op->complete_error(subdir.error());
							return;
						}
						continue;
					}

					{ /* Report subdirectory handle to the visitor. */
						auto accept_res = _op->visitor().accept_handle(*subdir, _level + 1);
						auto accept = bool(accept_res);
						if constexpr (decays_to_instance_of<decltype(accept_res), result>)
						{
							if (accept_res.has_error()) [[unlikely]]
							{
								_op->complete_error(accept_res.error());
								return;
							}
							accept = bool(*accept_res);
						}
						if (!accept) [[unlikely]]
							continue;
					}

					{ /* Start subdirectory traversal operation. */
						const auto _ = _op->incref_guard();
						_next = new next_state_t(connect(schedule(empty_base<Sch>::value()), type(_next, this, std::move(*subdir), _level + 1)));
					}
					start(*_next);
				}
				break;
			}

			/* Successfully enumerated all files & subdirectories. */
			_op->complete_value();
		}

		struct traverse_t
		{

		};
	}

	namespace fs
	{
		using _traverse::traverse_t;
	}
}

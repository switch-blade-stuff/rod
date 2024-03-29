/*
 * Created by switchblade on 2023-09-25.
 */

#pragma once

#include "../queries/completion.hpp"
#include "../adaptors/closure.hpp"
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

		template<typename V, typename Sch, typename Env, template<typename...> typename Tuple = std::tuple>
		using errors_list = unique_tuple_t<_detail::apply_tuple_t<Tuple, _detail::concat_tuples_t<error_types_of_t<schedule_result_t<Sch>, Env, std::tuple>, visitor_errors<V>, std::tuple<handle_error, std::exception_ptr>>>>;
		template<typename V, template<typename...> typename Tuple = std::tuple>
		using values_list = unique_tuple_t<_detail::apply_tuple_t<Tuple, visitor_values<V>>>;

		/* Visitor ignore & accept functions may either return a boolean or a boolean result to indicate an explicit error. */
		template<typename T>
		concept boolean_result = (instance_of<T, result> && std::convertible_to<typename T::value_type, bool>) || std::convertible_to<T, bool>;
	}

	namespace fs
	{
		template<typename V>
		concept traverse_visitor = requires(std::decay_t<V> &v, _traverse::handle_error e, const directory_handle &h, path_view p, stat s, stat::query q, std::size_t l)
		{
			{ v.accept_entry(h, p, s, q, l) } noexcept -> _traverse::boolean_result;
			{ v.ignore_error(e, h, p, l) } noexcept -> _traverse::boolean_result;
			{ v.accept_handle(h, l) } noexcept -> _traverse::boolean_result;
			{ v.result() };
		};
	}

	namespace _traverse
	{
		template<typename Sch>
		struct env { class type; };
		template<typename Sch, typename Vst>
		struct sender { class type; };
		template<typename Sch, typename Rcv, typename Vst>
		struct receiver { class type; };
		template<typename Sch, typename Rcv, typename Vst>
		struct operation { class type; };

		template<typename Sch>
		class env<Sch>::type : empty_base<Sch>
		{
		public:
			template<typename Sch2>
			constexpr explicit type(Sch2 &&sch) noexcept(std::is_nothrow_constructible_v<Sch, Sch2>) : empty_base<Sch>(std::forward<Sch2>(sch)) {}

			template<decays_to_same<type> E, _detail::completion_channel C>
			friend constexpr Sch tag_invoke(get_completion_scheduler_t<C>, E &&e) noexcept(std::is_nothrow_copy_constructible_v<Sch>) { return e.sch_base::value(); }
		};

		template<typename, typename...>
		struct channel_result
		{
			constexpr channel_result() noexcept = default;

			constexpr std::tuple<> operator()() const noexcept { return {}; }
		};
		template<typename Vst>
		struct channel_result<set_value_t, Vst> : empty_base<Vst>
		{
			channel_result() = delete;
			template<typename Vst2>
			explicit channel_result(Vst2 &&vst) noexcept(std::is_nothrow_constructible_v<Vst, Vst2>) : empty_base<Vst>(std::forward<Vst2>(vst)) {}

			decltype(auto) operator()() noexcept(noexcept(empty_base<Vst>::value().result())) { return empty_base<Vst>::value().result(); }
		};
		template<typename Err>
		struct channel_result<set_error_t, Err> : empty_base<Err>
		{
			channel_result() = delete;
			template<typename Err2>
			explicit channel_result(Err2 &&err) noexcept(std::is_nothrow_constructible_v<Err, Err2>) : empty_base<Err>(std::forward<Err2>(err)) {}

			decltype(auto) operator()() && noexcept { return std::forward_as_tuple(std::move(empty_base<Err>::value())); }
		};

		template<typename Sch, typename Rcv, typename Vst>
		class receiver<Sch, Rcv, Vst>::type : public receiver_adaptor<type>
		{
			friend receiver_adaptor<type>;

			using operation_t = typename operation<Sch, Rcv, Vst>::type;
			using next_state_t = connect_result_t<schedule_result_t<Sch>, type>;

		public:
			type(const type &) = delete;
			type &operator=(const type &) = delete;

			constexpr type(type &&other) noexcept : type(std::exchange(other.next, {}), other._op, std::move(other._dir), other.level) {}

			template<typename Dir2>
			explicit type(operation_t *op, Dir2 &&dir) : type(nullptr, op, std::forward<Dir2>(dir), 0) {}
			template<typename Dir2>
			explicit type(next_state_t *next, operation_t *op, Dir2 &&dir, std::size_t level) : _next(next), _op(op), _dir(std::forward<Dir2>(dir)), _level(level) {}

			~type() { delete _next; }

		private:
			inline env_of_t<Rcv> do_get_env() const noexcept;

			void do_set_value() noexcept { run(); }

			/* These are propagated to downstream. */
			inline void do_set_stopped() noexcept;
			template<typename Err>
			inline void do_set_error(Err &&err) noexcept;

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

			fs::directory_handle _dir;
			std::size_t _level;
		};

		template<typename Sch, typename Rcv, typename Vst>
		class operation<Sch, Rcv, Vst>::type : empty_base<Sch>, empty_base<Rcv>, connect_result_t<schedule_result_t<Sch>, typename receiver<Sch, Rcv, Vst>::type>
		{
			friend class receiver<Sch, Rcv, Vst>::type;

			using worker_base = connect_result_t<schedule_result_t<Sch>, typename receiver<Sch, Rcv, Vst>::type>;

			template<typename... Errs>
			using make_error_variant = std::variant<channel_result<set_error_t, std::decay_t<Errs>>...>;
			using data_t = _detail::concat_tuples_t<std::variant<channel_result<set_value_t, Vst>, channel_result<set_stopped_t>>, errors_list<Vst, Sch, env_of_t<schedule_result_t<Sch>>, make_error_variant>>;

		public:
			type(type &&) = delete;
			type &operator=(type &&) = delete;

			template<typename Sch2, typename Rcv2, typename Vst2, typename Dir2>
			type(Sch2 &&sch, Rcv2 &&rcv, Vst2 &&vst, Dir2 &&dir) : empty_base<Sch>(std::forward<Sch2>(sch)), empty_base<Rcv>(std::forward<Rcv2>(rcv)), worker_base(connect(schedule(sch()), receiver_t(this, std::forward<Dir2>(dir)))), _workers(1), _data(std::forward<Vst2>(vst)) {}

			friend void tag_invoke(start_t, type &op) noexcept
			{
				/* If the constructor has failed to allocate a worker, immediately complete with an error. */
				if (op._workers.load(std::memory_order_relaxed) != 0)
					start(*op._worker_state);
				else
					op.complete_next();
			}

		private:
			[[nodiscard]] auto &rcv() noexcept { return empty_base<Rcv>::value(); }
			[[nodiscard]] const auto &rcv() const noexcept { return empty_base<Rcv>::value(); }
			[[nodiscard]] const auto &sch() const noexcept { return empty_base<Sch>::value(); }

			[[nodiscard]] auto &visitor() noexcept { return std::get<channel_result<set_value_t, Vst>>(_data).empty_base<Vst>::value(); }
			[[nodiscard]] auto &visitor() const noexcept { return std::get<channel_result<set_value_t, Vst>>(_data).empty_base<Vst>::value(); }

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

			[[nodiscard]] auto decref_guard() noexcept
			{
				const auto workers = lock();
				return std::make_pair(workers, defer_invoke([this, workers]() noexcept { unlock(workers - 1); }));
			}
			[[nodiscard]] auto incref_guard() noexcept
			{
				const auto workers = lock();
				return std::make_pair(workers, defer_invoke([this, workers]() noexcept { unlock(workers + 1); }));
			}

			template<typename C, typename Res>
			void apply_channel(Res &&res) noexcept
			{
				invoke_channel<C>(std::forward<Res>(res));
			}
			template<typename C, typename Res> requires tuple_like<std::decay_t<Res>>
			void apply_channel(Res &&res) noexcept
			{
				std::apply([&]<typename... Args>(Args &&...args) { invoke_channel<C>(std::forward<Args>(args)...); }, std::forward<Res>(res));
			}
			template<typename C, typename Res> requires decays_to_instance_of<Res, result>
			void apply_channel(Res &&res) noexcept
			{
				if (res.has_error())
					set_error(std::move(rcv()), res.error());
				else
					apply_channel<C>(std::move(*res));
			}
			template<typename C, typename... Args>
			void invoke_channel(Args &&...args) noexcept
			{
				if constexpr (!_detail::nothrow_callable<C, Rcv &&, Args...> && !std::same_as<C, set_error_t>)
					try { C{}(std::move(rcv()), std::forward<Args>(args)...); } catch(...) { invoke_channel<set_error_t>(std::current_exception()); }
				else
					C{}(std::move(rcv()), std::forward<Args>(args)...);
			}

			void complete_next() noexcept
			{
				/* Complete the selected channel using the result provided by channel_result::operator() */
				std::visit([&]<typename C, typename... Ts>(channel_result<C, Ts...> &res) noexcept
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
				}, _data);
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
			template<typename Err>
			void complete_error(Err &&err) noexcept
			{
				using err_value = channel_result<set_error_t, std::decay_t<Err>>;
				static_assert(requires { _data.template emplace<err_value>(std::forward<Err>(err)); });

				const auto [workers, _] = decref_guard();
				/* Switch to the error state on first error. */
				if (_data.index() == 0) [[likely]]
					_data.template emplace<err_value>(std::forward<Err>(err));
				/* Last worker completes the operation. */
				if (workers == 1) [[unlikely]]
					complete_next();
			}

			std::atomic<std::size_t> _workers;
			data_t _data;
		};

		template<typename Sch, typename Rcv, typename Vst>
		env_of_t<Rcv> receiver<Sch, Rcv, Vst>::type::do_get_env() const noexcept { return get_env(_op->rcv()); }

		template<typename Sch, typename Rcv, typename Vst>
		void receiver<Sch, Rcv, Vst>::type::do_set_stopped() noexcept { _op->complete_stop(); }
		template<typename Sch, typename Rcv, typename Vst>
		template<typename Err>
		void receiver<Sch, Rcv, Vst>::type::do_set_error(Err &&err) noexcept { _op->complete_error(std::forward<Err>(err)); }

		template<typename Sch, typename Rcv, typename Vst>
		void receiver<Sch, Rcv, Vst>::type::run() noexcept
		{
			auto buffs = std::vector<io_buffer_t<fs::directory_handle, read_some_t>>(64);
			auto req = read_some_request_t<fs::directory_handle>{.buffs = buffs};
			for (;;)
			{
				auto read_res = read_some(_dir, std::move(req));
				if (read_res.has_error()) [[unlikely]]
				{
					_op->complete_error(read_res.error());
					return;
				}

				/* Expand the buffer if full and attempt another read. */
				if (read_res->second || read_res->first.size() < buffs.size()) [[likely]]
					break;

				buffs.resize(buffs.size() * 2);
				req.buffs = {std::move(read_res->first), buffs};
			}

			/* Enumerate directory entries. */
			for (auto &entry : req.buffs)
			{
				auto [st, mask] = entry.st();
#if !defined(ROD_WIN32) && !defined(ROD_POSIX) /* Explicit query check is only needed for unknown systems where entry type is not guaranteed. */
				if (!bool(mask & stat::query::type))
				{
					auto st_res = get_stat(st, _dir, entry.path(), stat::query::type);
					if (st_res.has_error()) [[unlikely]]
					{
						_op->complete_error(st_res.error());
						return;
					}
					mask |= *st_res;
				}
#endif

				{ /* Report entry to the visitor. */
					auto accept_res = _op->visitor().accept_entry(_dir, entry.path(), st, mask, _level);
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

				/* FIXME: How should out-of-descriptors condition be handled (ex. add entry to a list and process later)? */
				/* Attempt to open the subdirectory and report errors. */
				auto subdir = fs::directory_handle::open(_dir, entry.path());
				if (subdir.has_error()) [[unlikely]]
				{
					/* Ignore the subdirectory as requested by the user. */
					auto ignore_res = _op->visitor().ignore_error(subdir.error(), _dir, entry.path(), _level);
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
					const auto g = _op->incref_guard();
					/* Make sure we are not scheduling if a non-value completion has been requested. */
					if (_op->_data.index() != 0) [[unlikely]]
						break;

					try { _next = new next_state_t(connect(schedule(_op->sch()), type(_next, this, std::move(*subdir), _level + 1))); }
					catch (...)
					{
						_op->complete_error(handle_error(_detail::current_error()));
						return;
					}
				}
				start(*_next);
			}

			/* Successfully enumerated all files & subdirectories. */
			_op->complete_value();
		}

		template<typename Sch, typename Vst>
		class sender<Sch, Vst>::type : empty_base<Sch>, empty_base<Vst>
		{
			using vst_base = empty_base<Vst>;

		public:
			using is_sender = std::true_type;

		private:
			template<typename Rcv>
			using operation_t = typename operation<Sch, std::decay_t<Rcv>, Vst>::type;
			template<typename Rcv>
			using receiver_t = typename receiver<Sch, std::decay_t<Rcv>, Vst>::type;

			template<typename... Ts>
			using bind_set_stopped = set_stopped_t();
			template<typename... Ts>
			using make_error_signs = completion_signatures<set_error_t(std::decay_t<Ts>)...>;
			template<typename... Ts>
			using make_value_signs = completion_signatures<set_value_t(std::decay_t<Ts>...)>;

			template<typename S, typename E>
			using stop_signs_t = _detail::gather_signatures_t<set_stopped_t, S, E, bind_set_stopped, completion_signatures>;
			template<typename S, typename E>
			using error_signs_t = errors_list<Vst, S, E, make_error_signs>;
			using value_signs_t = values_list<Vst, make_value_signs>;

			template<typename Env>
			using signs_t = _detail::concat_tuples_t<value_signs_t, error_signs_t<Sch, Env>, stop_signs_t<Sch, Env>>;
			using env_t = env_of_t<schedule_result_t<Sch>>;

		public:
			template<typename Sch2, typename Vst2, typename Dir2>
			explicit type(Sch2 &&sch, Vst2 &&vst, Dir2 &&dir) noexcept(std::is_nothrow_constructible_v<Sch, Sch2> && std::is_nothrow_constructible_v<Vst, Vst2> && std::is_nothrow_constructible_v<fs::directory_handle, Dir2>)
					: empty_base<Sch>(std::forward<Sch2>(sch)), empty_base<Vst>(std::forward<Vst2>(vst)), _dir(std::forward<Dir2>(dir)) {}

		public:
			friend constexpr env_t tag_invoke(get_env_t, const type &s) noexcept(noexcept(get_env(schedule(s.empty_base<Sch>::value())))) { return get_env(schedule(s.empty_base<Sch>::value())); }
			template<typename E>
			friend constexpr signs_t<E> tag_invoke(get_completion_signatures_t, type &&, E) noexcept { return {}; }

			/* Since directory_handle is not copyable, connect should not accept a const-qualified sender. */
			template<rod::receiver Rcv> requires receiver_of<Rcv, signs_t<env_of_t<receiver_t<Rcv>>>>
			friend operation_t<Rcv> tag_invoke(connect_t, type &&s, Rcv &&rcv) noexcept(std::is_nothrow_constructible_v<operation_t<Rcv>, Sch, Rcv, Vst, fs::directory_handle>)
			{
				return operation_t<Rcv>(std::move(s.empty_base<Sch>::value()), std::forward<Rcv>(rcv), std::move(s.vst_base::value()), std::move(s._dir));
			}

		private:
			fs::directory_handle _dir;
		};

		class traverse_t
		{
			template<typename Sch, typename Vst>
			using sender_t = typename sender<Sch, std::decay_t<Vst>>::type;

		public:
			template<rod::scheduler Sch, fs::traverse_visitor Vst, std::convertible_to<fs::directory_handle> Dir> requires tag_invocable<traverse_t, Sch, Vst, Dir>
			[[nodiscard]] rod::sender auto operator()(Sch &&sch, Vst &&vst, Dir &&dir) const noexcept(nothrow_tag_invocable<traverse_t, Sch, Vst, Dir>)
			{
				return tag_invoke(*this, std::forward<Sch>(sch), std::forward<Vst>(vst), std::forward<Dir>(dir));
			}
			template<rod::scheduler Sch, fs::traverse_visitor Vst, std::convertible_to<fs::directory_handle> Dir> requires(!tag_invocable<traverse_t, Sch, Vst, Dir>)
			[[nodiscard]] sender_t<Sch, Vst> operator()(Sch &&sch, Vst &&vst, Dir &&dir) const noexcept(std::is_nothrow_constructible_v<sender_t<Sch, Vst>, Sch, Vst, Dir>)
			{
				return sender_t<Sch, Vst>(std::forward<Sch>(sch), std::forward<Vst>(vst), std::forward<Dir>(dir));
			}
		};
	}

	namespace fs
	{
		using _traverse::traverse_t;

		/* TODO: Document usage. */
		inline constexpr auto traverse = traverse_t{};
	}
}

/*
 * Created by switchblade on 2023-08-27.
 */

#pragma once

#include "queries/scheduler.hpp"
#include "io_status_code.hpp"
#include "handle_base.hpp"

namespace rod
{
	namespace _extent
	{
		template<typename Hnd>
		concept has_extent = handle<Hnd> && requires { typename Hnd::extent_type; };
		template<typename Hnd>
		concept decay_has_extent = has_extent<std::decay_t<Hnd>>;

		template<typename Res, typename Hnd>
		concept extent_result = instance_of<Res, result> && std::constructible_from<typename Res::template rebind_value<handle_extent_t<Hnd>>, Res>;

		struct endpos_t
		{
			template<decay_has_extent Hnd> requires tag_invocable<endpos_t, const Hnd &>
			[[nodiscard]] extent_result<std::decay_t<Hnd>> auto operator()(const Hnd &hnd) const noexcept { return tag_invoke(*this, hnd); }
			template<decay_has_extent Hnd> requires(!tag_invocable<endpos_t, const Hnd &> && _detail::callable<get_stat_t, stat &, const Hnd &, stat::query>)
			[[nodiscard]] extent_result<std::decay_t<Hnd>> auto operator()(const Hnd &hnd) const noexcept
			{
				using extent_type = handle_extent_t<std::decay_t<Hnd>>;
				using result_type = typename decltype(get_stat(std::declval<stat &>(), hnd, stat::query::size))::template rebind_value<extent_type>;

				result_type res;
				stat st;

				if (auto stat_res = get_stat(st, hnd, stat::query::size); stat_res.has_error()) [[unlikely]]
					res.emplace_error(std::move(stat_res.error()));
				else if ((*stat_res & stat::query::size) != stat::query::size) [[unlikely]]
					res.emplace_error(std::make_error_code(std::errc::not_supported));
				else
					res.emplace_value(extent_type(st.size));

				return res;
			}
		};
		struct truncate_t
		{
			template<decay_has_extent Hnd, typename Ext = handle_extent_t<std::decay_t<Hnd>>> requires tag_invocable<truncate_t, Hnd, Ext>
			extent_result<std::decay_t<Hnd>> auto operator()(Hnd &&hnd, Ext endp) const noexcept { return tag_invoke(*this, std::forward<Hnd>(hnd), endp); }
		};
	}

	using _extent::endpos_t;
	using _extent::truncate_t;

	/* TODO: Document usage */
	inline constexpr auto endpos = endpos_t{};
	/* TODO: Document usage */
	inline constexpr auto truncate = truncate_t{};

	/** Concept used to define a handle that supports size queries and resizing. */
	template<typename Hnd>
	concept sized_handle = _extent::has_extent<Hnd> && _detail::callable<endpos_t, const Hnd &> && _detail::callable<truncate_t, Hnd &, handle_extent_t<Hnd>>;

	inline namespace _get_io_scheduler
	{
		struct get_io_scheduler_t
		{
			[[nodiscard]] constexpr friend bool tag_invoke(forwarding_query_t, get_io_scheduler_t) noexcept { return true; }

			template<typename Q> requires tag_invocable<get_io_scheduler_t, const std::remove_cvref_t<Q> &>
			[[nodiscard]] constexpr decltype(auto) operator()(Q &&q) const noexcept { return tag_invoke(*this, std::as_const(q)); }
			template<typename Q> requires(!tag_invocable<get_io_scheduler_t, const std::remove_cvref_t<Q> &> && _detail::callable<get_completion_scheduler_t<set_value_t>, const std::remove_cvref_t<Q> &>)
			[[nodiscard]] constexpr decltype(auto) operator()(Q &&q) const noexcept { return get_completion_scheduler<set_value_t>(std::as_const(q)); }
			[[nodiscard]] constexpr decltype(auto) operator()() const noexcept { return read(*this); }
		};
	}

	/** Customization point object used to obtain a scheduler that can be used to schedule IO operations. Falls back to `get_completion_scheduler&lt;set_value_t&gt;`. */
	inline constexpr auto get_io_scheduler = get_io_scheduler_t{};

	namespace _handle
	{
		template<typename Child, typename Base, template <typename, typename> typename BaseAdaptor>
		struct io_handle_adaptor { class type; };

		template<typename Hnd, typename Op>
		struct io_request_impl;
		template<typename Hnd, typename Op> requires(requires { typename Hnd::template io_request<Op>; })
		struct io_request_impl<Hnd, Op> { using type = typename Hnd::template io_request<Op>; };

		template<typename Hnd, typename Op>
		struct io_result_impl;
		template<typename Hnd, typename Op> requires(requires { typename Hnd::template io_result<Op>; })
		struct io_result_impl<Hnd, Op> { using type = typename Hnd::template io_result<Op>; };

		template<typename Hnd, typename Op>
		struct io_buffer_sequence_impl;
		template<typename Hnd, typename Op> requires(requires { typename Hnd::template io_buffer_sequence<Op>; })
		struct io_buffer_sequence_impl<Hnd, Op> { using type = typename Hnd::template io_buffer_sequence<Op>; };

		template<typename Hnd, typename Op>
		struct io_buffer_impl;
		template<typename Hnd, typename Op> requires(requires { typename Hnd::template io_buffer<Op>; })
		struct io_buffer_impl<Hnd, Op> { using type = typename Hnd::template io_buffer<Op>; };
	}

	/** Type trait used to obtain the IO buffer sequence type of handle \a Hnd for operation \a Op. */
	template<typename Hnd, typename Op>
	struct io_buffer_sequence : _handle::io_buffer_sequence_impl<Hnd, Op> {};
	/** Alias for `typename io_buffer_sequence&lt;Hnd, Op&gt;::type` */
	template<typename Hnd, typename Op>
	using io_buffer_sequence_t = typename io_buffer_sequence<Hnd, Op>::type;

	/** Type trait used to obtain the IO buffer type of handle \a Hnd for operation \a Op. */
	template<typename Hnd, typename Op>
	struct io_buffer : _handle::io_buffer_impl<Hnd, Op> {};
	/** Alias for `typename io_buffer&lt;Hnd, Op&gt;::type` */
	template<typename Hnd, typename Op>
	using io_buffer_t = typename io_buffer<Hnd, Op>::type;

	/** Type trait used to obtain the IO request type of handle \a Hnd for operation \a Op. */
	template<typename Hnd, typename Op>
	struct io_request : _handle::io_request_impl<Hnd, Op> {};
	/** Alias for `typename io_request&lt;Hnd, Op&gt;::type` */
	template<typename Hnd, typename Op>
	using io_request_t = typename io_request<Hnd, Op>::type;

	/** Type trait used to obtain the IO result type of handle \a Hnd for operation \a Op. */
	template<typename Hnd, typename Op>
	struct io_result : _handle::io_result_impl<Hnd, Op> {};
	/** Alias for `typename io_result&lt;Hnd, Op&gt;::type` */
	template<typename Hnd, typename Op>
	using io_result_t = typename io_result<Hnd, Op>::type;

	/** Type trait used to obtain the value type of the IO result of handle \a Hnd for operation \a Op. */
	template<typename Hnd, typename Op>
	struct io_value { using type = typename io_result_t<Hnd, Op>::value_type; };
	/** Alias for `typename io_value&lt;Hnd, Op&gt;::type` */
	template<typename Hnd, typename Op>
	using io_value_t = typename io_value<Hnd, Op>::type;

	/** Type trait used to obtain the error type of the IO result of handle \a Hnd for operation \a Op. */
	template<typename Hnd, typename Op>
	struct io_error { using type = typename io_result_t<Hnd, Op>::error_type; };
	/** Alias for `typename io_value&lt;Hnd, Op&gt;::type` */
	template<typename Hnd, typename Op>
	using io_error_t = typename io_error<Hnd, Op>::type;

	namespace _io_operation
	{
		template<typename Hnd, typename Op>
		concept has_io_buffer_sequence = requires { typename Hnd::template io_buffer_sequence<Op>; };
		template<typename Hnd, typename Op>
		concept has_io_buffer = requires { typename Hnd::template io_buffer<Op>; };

		template<typename Hnd, typename Op>
		concept has_io_request = requires { typename Hnd::template io_request<Op>; };
		template<typename Hnd, typename Op>
		concept has_io_result = requires { typename Hnd::template io_result<Op>; };

		template<typename Hnd>
		concept has_timeout = requires { typename Hnd::timeout_type; };

		template<typename Hnd, typename Op>
		concept has_io_definitions = _extent::has_extent<Hnd> && has_timeout<Hnd> && has_io_buffer_sequence<Hnd, Op> && has_io_buffer<Hnd, Op> && has_io_request<Hnd, Op> && has_io_result<Hnd, Op>;
		template<typename Hnd, typename Op>
		concept decay_has_io_definitions = has_io_definitions<std::decay_t<Hnd>, Op>;

		template<typename T, typename Hnd>
		concept buffer_io_result = instance_of<std::decay_t<T>, result> && std::constructible_from<typename T::template rebind_value<handle_size_t<std::decay_t<Hnd>>>, std::decay_t<T>>;
		template<typename T, typename Hnd, typename Op>
		concept buffer_range = std::ranges::sized_range<std::decay_t<T>> && decays_to_same<std::ranges::range_value_t<std::decay_t<T>>, io_buffer_t<std::decay_t<Hnd>, Op>>;

		template<typename Op>
		struct adaptor { class type; };
		template<typename Op>
		class adaptor<Op>::type
		{
			template<typename Hnd> requires has_io_request<std::decay_t<Hnd>, Op>
			using request_t = io_request_t<std::decay_t<Hnd>, Op>;
			template<typename Hnd> requires has_io_result<std::decay_t<Hnd>, Op>
			using result_t = io_result_t<std::decay_t<Hnd>, Op>;
			template<typename Hnd> requires has_io_buffer<std::decay_t<Hnd>, Op>
			using buffer_t = io_buffer_t<std::decay_t<Hnd>, Op>;

			template<typename Hnd> requires has_timeout<std::decay_t<Hnd>>
			using timeout_t = handle_timeout_t<std::decay_t<Hnd>>;
			template<typename Hnd> requires _extent::decay_has_extent<Hnd>
			using extent_t = handle_extent_t<std::decay_t<Hnd>>;

		public:
			template<decay_has_io_definitions<Op> Hnd, std::convertible_to<request_t<Hnd>> Req = request_t<Hnd>, std::convertible_to<timeout_t<Hnd>> To = timeout_t<Hnd>> requires tag_invocable<Op, Hnd, Req, To>
			constexpr result_t<Hnd> operator()(Hnd &&hnd, Req &&req, To &&to = To()) const noexcept { return tag_invoke(Op{}, std::forward<Hnd>(hnd), std::forward<Req>(req), std::forward<To>(to)); }
		};
	}

	/** Concept used to check if \a Hnd is a handle compatible with IO operation \a Op with timeout \a To. */
	template<typename Hnd, typename Op, typename To = handle_timeout_t<Hnd>>
	concept io_handle = _io_operation::has_io_definitions<Hnd, Op> && _detail::callable_r<io_result_t<Hnd, Op>, Op, Hnd, io_request_t<Hnd, Op>, To>;

	namespace _sync { struct sync_at_t : _io_operation::adaptor<sync_at_t>::type {}; }

	using _sync::sync_at_t;

	/* TODO: Document usage. */
	inline constexpr auto sync_at = sync_at_t{};

	template<typename Hnd>
	using sync_at_buffer_sequence_t = io_buffer_sequence_t<Hnd, sync_at_t>;
	template<typename Hnd>
	using sync_at_buffer_t = io_buffer_t<Hnd, sync_at_t>;

	template<typename Hnd>
	using sync_at_request_t = io_request_t<Hnd, sync_at_t>;
	template<typename Hnd>
	using sync_at_result_t = io_result_t<Hnd, sync_at_t>;

	template<typename Hnd>
	using sync_at_value_t = io_value_t<Hnd, sync_at_t>;
	template<typename Hnd>
	using sync_at_error_t = io_error_t<Hnd, sync_at_t>;

	namespace _extent
	{
		/* NOTE: Extent functions do not have async versions as they require multiple dependant IO calls. */
		struct clone_extents_to_t : _io_operation::adaptor<clone_extents_to_t>::type {};
		struct zero_extents_t : _io_operation::adaptor<zero_extents_t>::type {};
		struct list_extents_t : _io_operation::adaptor<list_extents_t>::type {};
	}

	using _extent::clone_extents_to_t;
	using _extent::zero_extents_t;
	using _extent::list_extents_t;

	namespace _read_some
	{
		struct read_some_at_t : _io_operation::adaptor<read_some_at_t>::type {};
		struct read_some_t : _io_operation::adaptor<read_some_t>::type {};
	}

	using _read_some::read_some_at_t;
	using _read_some::read_some_t;

	/** Customization point object used to preform a synchronous sparse input using an IO handle.
	 * @param hnd Handle to preform the IO operation on.
	 * @param req Value of type `io_request_t&lt;decltype(hnd), read_some_at_t&gt;` used to specify parameters of the IO operation.
	 * @param to Optional value of type `handle_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Value of type `io_result_t&lt;decltype(hnd), read_some_at_t&gt;` indicating either a success or a status code on failure.
	 * @note It is recommended to use the value of the result, as it may own internal buffers or contain important information about the operation.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note See definition of the individual handle type for further information about it's IO request and result types. */
	inline constexpr auto read_some_at = read_some_at_t{};

	template<typename Hnd>
	using read_some_at_buffer_sequence_t = io_buffer_sequence_t<Hnd, read_some_at_t>;
	template<typename Hnd>
	using read_some_at_buffer_t = io_buffer_t<Hnd, read_some_at_t>;

	template<typename Hnd>
	using read_some_at_request_t = io_request_t<Hnd, read_some_at_t>;
	template<typename Hnd>
	using read_some_at_result_t = io_result_t<Hnd, read_some_at_t>;

	/** Customization point object used to preform a synchronous stream input using an IO handle.
	 * @param hnd Handle to preform the IO operation on.
	 * @param req Value of type `io_request_t&lt;decltype(hnd), read_some_t&gt;` used to specify parameters of the IO operation.
	 * @param to Optional value of type `handle_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Value of type `io_result_t&lt;decltype(hnd), read_some_t&gt;` indicating either a success or a status code on failure.
	 * @note It is recommended to use the value of the result, as it may own internal buffers or contain important information about the operation.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note See definition of the individual handle type for further information about it's IO request and result types. */
	inline constexpr auto read_some = read_some_t{};

	template<typename Hnd>
	using read_some_buffer_sequence_t = io_buffer_sequence_t<Hnd, read_some_t>;
	template<typename Hnd>
	using read_some_buffer_t = io_buffer_t<Hnd, read_some_t>;

	template<typename Hnd>
	using read_some_request_t = io_request_t<Hnd, read_some_t>;
	template<typename Hnd>
	using read_some_result_t = io_result_t<Hnd, read_some_t>;

	template<typename Hnd>
	using read_some_value_t = io_value_t<Hnd, read_some_t>;
	template<typename Hnd>
	using read_some_error_t = io_error_t<Hnd, read_some_t>;

	/** Concept used to define an IO handle that supports synchronous sparse input. */
	template<typename Hnd>
	concept sparse_input_handle = io_handle<Hnd, read_some_at_t>;
	/** Concept used to define an IO handle that supports synchronous stream input. */
	template<typename Hnd>
	concept stream_input_handle = io_handle<Hnd, read_some_t>;

	namespace _write_some
	{
		struct write_some_at_t : _io_operation::adaptor<write_some_at_t>::type {};
		struct write_some_t : _io_operation::adaptor<write_some_t>::type {};
	}

	using _write_some::write_some_at_t;
	using _write_some::write_some_t;

	/** Customization point object used to preform a synchronous sparse output using an IO handle.
	 * @param hnd Handle to preform the IO operation on.
	 * @param req Value of type `io_request_t&lt;decltype(hnd), write_some_at_t&gt;` used to specify parameters of the IO operation.
	 * @param to Optional value of type `handle_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Value of type `io_result_t&lt;decltype(hnd), write_some_at_t&gt;` indicating either a success or a status code on failure.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note See definition of the individual handle type for further information about it's IO request and result types. */
	inline constexpr auto write_some_at = write_some_at_t{};
	/** Customization point object used to preform a synchronous stream output using an IO handle.
	 * @param hnd Handle to preform the IO operation on.
	 * @param src Value of type `io_request_t&lt;decltype(hnd), write_some_t&gt;` used to specify parameters of the IO operation.
	 * @param to Optional value of type `handle_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Value of type `io_result_t&lt;decltype(hnd), write_some_t&gt;` indicating either a success or a status code on failure.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note See definition of the individual handle type for further information about it's IO request and result types. */
	inline constexpr auto write_some = write_some_t{};

	template<typename Hnd>
	using write_some_at_buffer_sequence_t = io_buffer_sequence_t<Hnd, write_some_at_t>;
	template<typename Hnd>
	using write_some_at_buffer_t = io_buffer_t<Hnd, write_some_at_t>;

	template<typename Hnd>
	using write_some_at_request_t = io_request_t<Hnd, write_some_at_t>;
	template<typename Hnd>
	using write_some_at_result_t = io_result_t<Hnd, write_some_at_t>;

	template<typename Hnd>
	using write_some_buffer_sequence_t = io_buffer_sequence_t<Hnd, write_some_t>;
	template<typename Hnd>
	using write_some_buffer_t = io_buffer_t<Hnd, write_some_t>;

	template<typename Hnd>
	using write_some_request_t = io_request_t<Hnd, write_some_t>;
	template<typename Hnd>
	using write_some_result_t = io_result_t<Hnd, write_some_t>;

	template<typename Hnd>
	using write_some_value_t = io_value_t<Hnd, write_some_t>;
	template<typename Hnd>
	using write_some_error_t = io_error_t<Hnd, write_some_t>;

	/** Concept used to define an IO handle that supports synchronous sparse output. */
	template<typename Hnd>
	concept sparse_output_handle = io_handle<Hnd, write_some_at_t>;
	/** Concept used to define an IO handle that supports synchronous stream output. */
	template<typename Hnd>
	concept stream_output_handle = io_handle<Hnd, write_some_t>;

	/** Concept used to define an IO handle that supports synchronous sparse IO operations. */
	template<typename Hnd>
	concept sparse_io_handle = sparse_input_handle<Hnd> && sparse_output_handle<Hnd>;
	/** Concept used to define an IO handle that supports synchronous stream IO operations. */
	template<typename Hnd>
	concept stream_io_handle = stream_input_handle<Hnd> && stream_output_handle<Hnd>;

	/* TODO: Implement async IO operations. */

	namespace _extent
	{
		template<typename Hnd>
		concept has_offset = has_extent<Hnd> && requires { typename Hnd::offset_type; };
		template<typename Hnd>
		concept decay_has_offset = has_offset<std::decay_t<Hnd>>;

		/** Enumeration defining seek direction for the `seekpos` CBO. */
		enum class seek_dir
		{
			/** Seek from the starting position. */
			beg = SEEK_SET,
			/** Seek from the current position. */
			cur = SEEK_CUR,
			/** Seek from the end position. */
			end = SEEK_END,
		};

		struct getpos_t
		{
			template<decay_has_offset Hnd> requires tag_invocable<getpos_t, const Hnd &>
			[[nodiscard]] extent_result<std::decay_t<Hnd>> auto operator()(const Hnd &hnd) const noexcept { return tag_invoke(*this, hnd); }
		};
		struct setpos_t
		{
			template<decay_has_offset Hnd, typename Ext = handle_extent_t<std::decay_t<Hnd>>> requires tag_invocable<setpos_t, Hnd, Ext>
			extent_result<std::decay_t<Hnd>> auto operator()(Hnd &&hnd, Ext pos) const noexcept { return tag_invoke(*this, std::forward<Hnd>(hnd), pos); }
		};
		struct seekpos_t
		{
			template<decay_has_offset Hnd, typename Off = handle_offset_t<std::decay_t<Hnd>>> requires tag_invocable<seekpos_t, Hnd, Off, seek_dir>
			extent_result<std::decay_t<Hnd>> auto operator()(Hnd &&hnd, Off off, seek_dir dir) const noexcept { return tag_invoke(*this, std::forward<Hnd>(hnd), off, dir); }
		};
	}

	using _extent::seek_dir;
	using _extent::getpos_t;
	using _extent::setpos_t;
	using _extent::seekpos_t;

	/* TODO: Document usage */
	inline constexpr auto getpos = getpos_t{};
	/* TODO: Document usage */
	inline constexpr auto setpos = setpos_t{};
	/* TODO: Document usage */
	inline constexpr auto seekpos = seekpos_t{};

	/** Concept used to define a sized handle that supports seeking IO operations. */
	template<typename Hnd>
	concept seekable_handle = sized_handle<Hnd> && _extent::has_offset<Hnd> && _detail::callable<getpos_t, const Hnd &> && _detail::callable<setpos_t, Hnd &, handle_extent_t<Hnd>> && _detail::callable<seekpos_t, Hnd &, handle_offset_t<Hnd>, seek_dir>;

	namespace _handle
	{
		template<typename Child, typename Base, template <typename, typename> typename BaseAdaptor>
		class io_handle_adaptor<Child, Base, BaseAdaptor>::type : public BaseAdaptor<Child, Base>
		{
			using adp_base = BaseAdaptor<Child, Base>;

			friend BaseAdaptor<Child, Base>;
			friend Child;

		public:
			template<typename Op>
			using io_buffer_sequence = io_buffer_sequence_t<Base, Op>;
			template<typename Op>
			using io_buffer = io_buffer_t<Base, Op>;

			template<typename Op>
			using io_request = io_request_t<Base, Op>;
			template<typename Op>
			using io_result = io_result_t<Base, Op>;

		private:
			template<typename T>
			static constexpr decltype(auto) get_adaptor(T &&value) noexcept { return static_cast<copy_cvref_t<std::add_rvalue_reference_t<T>, type>>(std::forward<T>(value)); }

		public:
			type(const type &) = delete;
			type &operator=(const type &) = delete;

			type() noexcept = default;
			type(type &&other) noexcept : adp_base(std::forward<adp_base>(other)) {}
			type &operator=(type &&other) noexcept { return (adp_base::operator=(std::forward<adp_base>(other)), *this); }

			template<typename... Args> requires std::constructible_from<adp_base, Base, Args...>
			explicit type(Base &&other, Args &&...args) noexcept : adp_base(std::forward<Base>(other), std::forward<Args>(args)...) {}
			template<typename... Args> requires std::constructible_from<adp_base, typename adp_base::native_handle_type, Args...>
			explicit type(typename adp_base::native_handle_type hnd, Args &&...args) noexcept : adp_base(hnd, std::forward<Args>(args)...) {}

			using adp_base::release;
			using adp_base::is_open;
			using adp_base::native_handle;

			template<decays_to_same_or_derived<Child> Hnd = Child>
			constexpr void swap(Hnd &&other) noexcept { adl_swap(static_cast<adp_base &>(*this), static_cast<adp_base &>(get_adaptor(other))); }
			template<decays_to_same_or_derived<Child> Hnd0 = Child, decays_to_same_or_derived<Child> Hnd1 = Child>
			friend constexpr void swap(Hnd0 &&a, Hnd1 &&b) noexcept { get_adaptor(std::forward<Hnd0>(a)).swap(std::forward<Hnd1>(b)); }

		private:
			static constexpr int do_sync = 1;
			static constexpr int do_sync_at = 1;

			template<typename Hnd>
			static constexpr bool has_sync() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_sync)); }; }
			template<typename Hnd>
			static constexpr bool has_sync_at() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_sync_at)); }; }

			template<typename Hnd>
			constexpr static auto dispatch_sync(Hnd &&hnd, sync_mode mode) noexcept -> decltype(std::forward<Hnd>(hnd).do_sync(mode)) { return std::forward<Hnd>(hnd).do_sync(mode); }
			template<typename Hnd, typename Req>
			constexpr static auto dispatch_sync_at(Hnd &&hnd, Req &&req) noexcept -> decltype(std::forward<Hnd>(hnd).do_sync_at(std::forward<Req>(req))) { return std::forward<Hnd>(hnd).do_sync_at(std::forward<Req>(req)); }

		public:
			template<decays_to_same<sync_t> Op, decays_to_same_or_derived<Child> Hnd> requires(has_sync<Hnd>())
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, sync_mode mode) noexcept { return dispatch_sync(std::forward<Hnd>(hnd), mode); }
			template<decays_to_same<sync_t> Op, decays_to_same_or_derived<Child> Hnd> requires(!has_sync<Hnd>() && _detail::callable<Op, copy_cvref_t<Hnd, Base>, sync_mode>)
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, sync_mode mode) noexcept { return Op{}(get_adaptor(std::forward<Hnd>(hnd)).base(), mode); }

			template<decays_to_same<sync_at_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req> requires(has_sync<Hnd>())
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req) noexcept { return dispatch_sync_at(std::forward<Hnd>(hnd), std::forward<Req>(req)); }
			template<decays_to_same<sync_at_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req> requires(!has_sync<Hnd>() && _detail::callable<Op, copy_cvref_t<Hnd, Base>, Req>)
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req) noexcept { return Op{}(get_adaptor(std::forward<Hnd>(hnd)).base(), std::forward<Req>(req)); }

		private:
			static constexpr int do_endpos = 1;
			static constexpr int do_truncate = 1;

			template<typename Hnd>
			static constexpr bool has_endpos() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_endpos)); }; }
			template<typename Hnd>
			static constexpr bool has_truncate() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_truncate)); }; }

			template<typename Hnd>
			constexpr static auto dispatch_endpos(Hnd &&hnd) noexcept -> decltype(std::forward<Hnd>(hnd).do_endpos()) { return std::forward<Hnd>(hnd).do_endpos(); }
			template<typename Hnd, typename Ext>
			constexpr static auto dispatch_truncate(Hnd &&hnd, Ext &&endp) noexcept -> decltype(std::forward<Hnd>(hnd).do_truncate(std::forward<Ext>(endp))) { return std::forward<Hnd>(hnd).do_truncate(std::forward<Ext>(endp)); }

		public:
			template<decays_to_same<endpos_t> Op, decays_to_same_or_derived<Child> Hnd> requires(has_endpos<Hnd>())
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd) noexcept { return dispatch_endpos(std::forward<Hnd>(hnd)); }
			template<decays_to_same<endpos_t> Op, decays_to_same_or_derived<Child> Hnd> requires(!has_endpos<Hnd>() && _detail::callable<Op, copy_cvref_t<Hnd, Base>>)
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd) noexcept { return Op{}(get_adaptor(std::forward<Hnd>(hnd)).base()); }

			template<decays_to_same<truncate_t> Op, decays_to_same_or_derived<Child> Hnd, std::convertible_to<handle_extent_t<std::decay_t<Hnd>>> Ext> requires(has_truncate<Hnd>())
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Ext &&endp) noexcept { return dispatch_truncate(std::forward<Hnd>(hnd), std::forward<Ext>(endp)); }
			template<decays_to_same<truncate_t> Op, decays_to_same_or_derived<Child> Hnd, std::convertible_to<handle_extent_t<std::decay_t<Hnd>>> Ext> requires(!has_truncate<Hnd>() && _detail::callable<Op, copy_cvref_t<Hnd, Base>, Ext>)
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Ext &&endp) noexcept { return Op{}(get_adaptor(std::forward<Hnd>(hnd)).base(), std::forward<Ext>(endp)); }

		private:
			static constexpr int do_getpos = 1;
			static constexpr int do_setpos = 1;
			static constexpr int do_seekpos = 1;

			template<typename Hnd>
			static constexpr bool has_getpos() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_getpos)); }; }
			template<typename Hnd>
			static constexpr bool has_setpos() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_setpos)); }; }
			template<typename Hnd>
			static constexpr bool has_seekpos() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_seekpos)); }; }

			template<typename Hnd>
			constexpr static auto dispatch_getpos(Hnd &&hnd) noexcept -> decltype(std::forward<Hnd>(hnd).do_getpos()) { return std::forward<Hnd>(hnd).do_getpos(); }
			template<typename Hnd>
			constexpr static auto dispatch_setpos(Hnd &&hnd, auto pos) noexcept -> decltype(std::forward<Hnd>(hnd).do_setpos(pos)) { return std::forward<Hnd>(hnd).do_setpos(pos); }
			template<typename Hnd>
			constexpr static auto dispatch_seekpos(Hnd &&hnd, auto off, auto dir) noexcept -> decltype(std::forward<Hnd>(hnd).do_seekpos(off, dir)) { return std::forward<Hnd>(hnd).do_seekpos(off, dir); }

		public:
			template<decays_to_same<getpos_t> Op, decays_to_same_or_derived<Child> Hnd> requires(has_getpos<Hnd>())
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd) noexcept { return dispatch_getpos(std::forward<Hnd>(hnd)); }
			template<decays_to_same<getpos_t> Op, decays_to_same_or_derived<Child> Hnd> requires(!has_getpos<Hnd>() && _detail::callable<Op, copy_cvref_t<Hnd, Base>>)
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd) noexcept { return Op{}(get_adaptor(std::forward<Hnd>(hnd)).base()); }

			template<decays_to_same<setpos_t> Op, decays_to_same_or_derived<Child> Hnd, std::convertible_to<handle_extent_t<std::decay_t<Hnd>>> Ext> requires(has_setpos<Hnd>())
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Ext &&pos) noexcept { return dispatch_setpos(std::forward<Hnd>(hnd), std::forward<Ext>(pos)); }
			template<decays_to_same<setpos_t> Op, decays_to_same_or_derived<Child> Hnd, std::convertible_to<handle_extent_t<std::decay_t<Hnd>>> Ext> requires(!has_setpos<Hnd>() && _detail::callable<Op, copy_cvref_t<Hnd, Base>, Ext>)
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Ext &&pos) noexcept { return Op{}(get_adaptor(std::forward<Hnd>(hnd)).base(), std::forward<Ext>(pos)); }

			template<decays_to_same<seekpos_t> Op, decays_to_same_or_derived<Child> Hnd, std::convertible_to<handle_offset_t<std::decay_t<Hnd>>> Off> requires(has_seekpos<Hnd>())
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Off &&off, seek_dir dir) noexcept { return dispatch_seekpos(std::forward<Hnd>(hnd), std::forward<Off>(off), dir); }
			template<decays_to_same<seekpos_t> Op, decays_to_same_or_derived<Child> Hnd, std::convertible_to<handle_offset_t<std::decay_t<Hnd>>> Off> requires(!has_seekpos<Hnd>() && _detail::callable<Op, copy_cvref_t<Hnd, Base>, Off, seek_dir>)
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Off &&off, seek_dir dir) noexcept { return Op{}(get_adaptor(std::forward<Hnd>(hnd)).base(), std::forward<Off>(off), dir); }

		private:
			static constexpr int do_list_extents = 1;
			static constexpr int do_clone_extents_to = 1;

			template<typename Hnd>
			static constexpr bool has_list_extents() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_list_extents)); }; }
			template<typename Hnd>
			static constexpr bool has_clone_extents_to() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_clone_extents_to)); }; }

		public:
			template<decays_to_same<list_extents_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req) noexcept requires(has_list_extents<Hnd>() && requires { std::forward<Hnd>(hnd).do_list_extents(std::forward<Req>(req)); })
			{
				return std::forward<Hnd>(hnd).do_list_extents(std::forward<Req>(req));
			}
			template<decays_to_same<list_extents_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req) noexcept requires(!has_list_extents<Hnd>() && _detail::callable<Op, copy_cvref_t<Hnd, Base>, Req>)
			{
				return Op{}(get_adaptor(std::forward<Hnd>(hnd)).base(), std::forward<Req>(req));
			}

			template<decays_to_same<clone_extents_to_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req) noexcept requires(has_clone_extents_to<Hnd>() && requires { std::forward<Hnd>(hnd).do_clone_extents_to(std::forward<Req>(req)); })
			{
				return std::forward<Hnd>(hnd).do_clone_extents_to(std::forward<Req>(req));
			}
			template<decays_to_same<clone_extents_to_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req) noexcept requires(!has_clone_extents_to<Hnd>() && _detail::callable<Op, copy_cvref_t<Hnd, Base>, Req>)
			{
				return Op{}(get_adaptor(std::forward<Hnd>(hnd)).base(), std::forward<Req>(req));
			}

			template<decays_to_same<list_extents_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req, std::convertible_to<handle_timeout_t<std::decay_t<Hnd>>> To>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req, const To &to) noexcept requires(has_list_extents<Hnd>() && requires { std::forward<Hnd>(hnd).do_list_extents(std::forward<Req>(req), to); })
			{
				return std::forward<Hnd>(hnd).do_list_extents(std::forward<Req>(req), to);
			}
			template<decays_to_same<list_extents_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req, std::convertible_to<handle_timeout_t<std::decay_t<Hnd>>> To>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req, const To &to) noexcept requires(!has_list_extents<Hnd>() && _detail::callable<Op, copy_cvref_t<Hnd, Base>, Req, const To &>)
			{
				return Op{}(get_adaptor(std::forward<Hnd>(hnd)).base(), std::forward<Req>(req), to);
			}

			template<decays_to_same<clone_extents_to_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req, std::convertible_to<handle_timeout_t<std::decay_t<Hnd>>> To>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req, const To &to) noexcept requires(has_clone_extents_to<Hnd>() && requires { std::forward<Hnd>(hnd).do_clone_extents_to(std::forward<Req>(req), to); })
			{
				return std::forward<Hnd>(hnd).do_clone_extents_to(std::forward<Req>(req), to);
			}
			template<decays_to_same<clone_extents_to_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req, std::convertible_to<handle_timeout_t<std::decay_t<Hnd>>> To>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req, const To &to) noexcept requires(!has_clone_extents_to<Hnd>() && _detail::callable<Op, copy_cvref_t<Hnd, Base>, Req, const To &>)
			{
				return Op{}(get_adaptor(std::forward<Hnd>(hnd)).base(), std::forward<Req>(req), to);
			}

		private:
			static constexpr int do_read_some = 1;
			static constexpr int do_read_some_at = 1;

			template<typename Hnd>
			static constexpr bool has_read_some() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_read_some)); }; }
			template<typename Hnd>
			static constexpr bool has_read_some_at() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_read_some_at)); }; }

		public:
			template<decays_to_same<read_some_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req) noexcept requires(has_read_some<Hnd>() && requires { std::forward<Hnd>(hnd).do_read_some(std::forward<Req>(req)); })
			{
				return std::forward<Hnd>(hnd).do_read_some(std::forward<Req>(req));
			}
			template<decays_to_same<read_some_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req) noexcept requires(!has_read_some<Hnd>() && _detail::callable<Op, copy_cvref_t<Hnd, Base>, Req>)
			{
				return Op{}(get_adaptor(std::forward<Hnd>(hnd)).base(), std::forward<Req>(req));
			}

			template<decays_to_same<read_some_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req, std::convertible_to<handle_timeout_t<std::decay_t<Hnd>>> To>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req, const To &to) noexcept requires(has_read_some<Hnd>() && requires { std::forward<Hnd>(hnd).do_read_some(std::forward<Req>(req), to); })
			{
				return std::forward<Hnd>(hnd).do_read_some(std::forward<Req>(req), to);
			}
			template<decays_to_same<read_some_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req, std::convertible_to<handle_timeout_t<std::decay_t<Hnd>>> To>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req, const To &to) noexcept requires(!has_read_some<Hnd>() && _detail::callable<Op, copy_cvref_t<Hnd, Base>, Req, const To &>)
			{
				return Op{}(get_adaptor(std::forward<Hnd>(hnd)).base(), std::forward<Req>(req), to);
			}

			template<decays_to_same<read_some_at_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req) noexcept requires(has_read_some_at<Hnd>() && requires { std::forward<Hnd>(hnd).do_read_some_at(std::forward<Req>(req)); })
			{
				return std::forward<Hnd>(hnd).do_read_some_at(std::forward<Req>(req));
			}
			template<decays_to_same<read_some_at_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req) noexcept requires(!has_read_some_at<Hnd>() && _detail::callable<Op, copy_cvref_t<Hnd, Base>, Req>)
			{
				return Op{}(get_adaptor(std::forward<Hnd>(hnd)).base(), std::forward<Req>(req));
			}

			template<decays_to_same<read_some_at_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req, std::convertible_to<handle_timeout_t<std::decay_t<Hnd>>> To>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req, const To &to) noexcept requires(has_read_some_at<Hnd>() && requires { std::forward<Hnd>(hnd).do_read_some_at(std::forward<Req>(req), to); })
			{
				return std::forward<Hnd>(hnd).do_read_some_at(std::forward<Req>(req), to);
			}
			template<decays_to_same<read_some_at_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req, std::convertible_to<handle_timeout_t<std::decay_t<Hnd>>> To>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req, const To &to) noexcept requires(!has_read_some_at<Hnd>() && _detail::callable<Op, copy_cvref_t<Hnd, Base>, Req, const To &>)
			{
				return Op{}(get_adaptor(std::forward<Hnd>(hnd)).base(), std::forward<Req>(req), to);
			}

		private:
			static constexpr int do_write_some = 1;
			static constexpr int do_write_some_at = 1;

			template<typename Hnd>
			static constexpr bool has_write_some() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_write_some)); }; }
			template<typename Hnd>
			static constexpr bool has_write_some_at() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_write_some_at)); }; }

		public:
			template<decays_to_same<write_some_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req) noexcept requires(has_write_some<Hnd>() && requires { std::forward<Hnd>(hnd).do_write_some(std::forward<Req>(req)); })
			{
				return std::forward<Hnd>(hnd).do_write_some(std::forward<Req>(req));
			}
			template<decays_to_same<write_some_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req) noexcept requires(!has_write_some<Hnd>() && _detail::callable<Op, copy_cvref_t<Hnd, Base>, Req>)
			{
				return Op{}(get_adaptor(std::forward<Hnd>(hnd)).base(), std::forward<Req>(req));
			}

			template<decays_to_same<write_some_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req, std::convertible_to<handle_timeout_t<std::decay_t<Hnd>>> To>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req, const To &to) noexcept requires(has_write_some<Hnd>() && requires { std::forward<Hnd>(hnd).do_write_some(std::forward<Req>(req), to); })
			{
				return std::forward<Hnd>(hnd).do_write_some(std::forward<Req>(req), to);
			}
			template<decays_to_same<write_some_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req, std::convertible_to<handle_timeout_t<std::decay_t<Hnd>>> To>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req, const To &to) noexcept requires(!has_write_some<Hnd>() && _detail::callable<Op, copy_cvref_t<Hnd, Base>, Req, const To &>)
			{
				return Op{}(get_adaptor(std::forward<Hnd>(hnd)).base(), std::forward<Req>(req), to);
			}

			template<decays_to_same<write_some_at_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req) noexcept requires(has_write_some_at<Hnd>() && requires { std::forward<Hnd>(hnd).do_write_some_at(std::forward<Req>(req)); })
			{
				return std::forward<Hnd>(hnd).do_write_some_at(std::forward<Req>(req));
			}
			template<decays_to_same<write_some_at_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req) noexcept requires(!has_write_some_at<Hnd>() && _detail::callable<Op, copy_cvref_t<Hnd, Base>, Req>)
			{
				return Op{}(get_adaptor(std::forward<Hnd>(hnd)).base(), std::forward<Req>(req));
			}

			template<decays_to_same<write_some_at_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req, std::convertible_to<handle_timeout_t<std::decay_t<Hnd>>> To>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req, const To &to) noexcept requires(has_write_some_at<Hnd>() && requires { std::forward<Hnd>(hnd).do_write_some_at(std::forward<Req>(req), to); })
			{
				return std::forward<Hnd>(hnd).do_write_some_at(std::forward<Req>(req), to);
			}
			template<decays_to_same<write_some_at_t> Op, decays_to_same_or_derived<Child> Hnd, decays_to_same<io_request_t<std::decay_t<Hnd>, Op>> Req, std::convertible_to<handle_timeout_t<std::decay_t<Hnd>>> To>
			friend decltype(auto) tag_invoke(Op, Hnd &&hnd, Req &&req, const To &to) noexcept requires(!has_write_some_at<Hnd>() && _detail::callable<Op, copy_cvref_t<Hnd, Base>, Req, const To &>)
			{
				return Op{}(get_adaptor(std::forward<Hnd>(hnd)).base(), std::forward<Req>(req), to);
			}

			/* TODO: Implement async IO operations. */
		};
	}

	/** Handle adaptor used to implement IO handles. */
	template<typename Child, handle Base = basic_handle, template<typename, typename> typename BaseAdaptor = handle_adaptor>
	using io_handle_adaptor = typename _handle::io_handle_adaptor<Child, Base, BaseAdaptor>::type;
}

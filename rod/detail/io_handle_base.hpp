/*
 * Created by switch_blade on 2023-08-27.
 */

#pragma once

#include "io_status_code.hpp"
#include "handle_base.hpp"

namespace rod
{
	namespace _handle
	{
		template<typename Hnd>
		struct handle_extent_impl;
		template<typename Hnd> requires(requires { typename Hnd::extent_type; })
		struct handle_extent_impl<Hnd> { using type = typename Hnd::extent_type; };

		template<typename Hnd>
		struct handle_size_impl;
		template<typename Hnd> requires(requires { typename Hnd::size_type; })
		struct handle_size_impl<Hnd> { using type = typename Hnd::size_type; };

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

	/** Type trait used to obtain the extent type of handle \a Hnd. */
	template<typename Hnd>
	struct handle_extent : _handle::handle_extent_impl<Hnd> {};
	/** Alias for `typename handle_extent&lt;Hnd&gt;::type` */
	template<typename Hnd>
	using handle_extent_t = typename handle_extent<Hnd>::type;

	/** Type trait used to obtain the size type of handle \a Hnd. */
	template<typename Hnd>
	struct handle_size : _handle::handle_size_impl<Hnd> {};
	/** Alias for `typename handle_size&lt;Hnd&gt;::type` */
	template<typename Hnd>
	using handle_size_t = typename handle_size<Hnd>::type;

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

	namespace _io_operation
	{
		template<typename T, typename Hnd>
		concept buffer_io_result = instance_of<std::decay_t<T>, result> && std::constructible_from<typename T::template rebind_value<handle_size_t<std::decay_t<Hnd>>>, std::decay_t<T>>;
		template<typename T, typename Hnd, typename Op>
		concept buffer_range = std::ranges::sized_range<std::decay_t<T>> && decays_to_same<std::ranges::range_value_t<std::decay_t<T>>, io_buffer_t<std::decay_t<Hnd>, Op>>;

		template<typename Op>
		struct adaptor { struct type; };
		template<typename Op>
		struct sparse_adaptor { class type; };
		template<typename Op>
		struct stream_adaptor { class type; };

		template<typename Op>
		struct adaptor<Op>::type
		{
			template<typename Hnd>
			using request_t = io_request_t<std::decay_t<Hnd>, Op>;
			template<typename Hnd>
			using result_t = io_result_t<std::decay_t<Hnd>, Op>;
			template<typename Hnd>
			using buffer_t = io_buffer_t<std::decay_t<Hnd>, Op>;

			template<typename Hnd>
			using timeout_t = handle_timeout_t<std::decay_t<Hnd>>;
			template<typename Hnd>
			using extent_t = handle_extent_t<std::decay_t<Hnd>>;

			template<typename Hnd, decays_to_same<request_t<Hnd>> Req = request_t<Hnd>, decays_to_same<timeout_t<Hnd>> To = timeout_t<Hnd>> requires tag_invocable<Op, Hnd, Req, To>
			result_t<Hnd> operator()(Hnd &&hnd, Req &&req, To &&to = To()) const noexcept { return tag_invoke(Op{}, std::forward<Hnd>(hnd), std::forward<Req>(req), to); }
		};
		template<typename Op>
		class sparse_adaptor<Op>::type : adaptor<Op>::type
		{
			template<typename Hnd>
			using timeout_t = typename adaptor<Op>::type::template timeout_t<Hnd>;
			template<typename Hnd>
			using extent_t = typename adaptor<Op>::type::template extent_t<Hnd>;
			template<typename Hnd>
			using buffer_t = typename adaptor<Op>::type::template buffer_t<Hnd>;

		public:
			using adaptor<Op>::type::operator();
			template<typename Hnd, buffer_range<std::decay_t<Hnd>, Op> Buff = std::initializer_list<buffer_t<Hnd>>, decays_to_same<extent_t<Hnd>> Ext = extent_t<Hnd>, decays_to_same<timeout_t<Hnd>> To = timeout_t<Hnd>>
			buffer_io_result<Hnd> auto operator()(Hnd &&hnd, Buff &&buff, const Ext &pos, To &&to = To()) const noexcept requires tag_invocable<Op, Hnd, Buff, const Ext &, To>
			{
				return tag_invoke(Op{}, std::forward<Buff>(hnd), std::forward<Buff>(buff), pos, std::forward<To>(to));
			}
			template<typename Hnd, decays_to_same<buffer_t<Hnd>> Buff = buffer_t<Hnd>, decays_to_same<extent_t<Hnd>> Ext = extent_t<Hnd>, decays_to_same<timeout_t<Hnd>> To = timeout_t<Hnd>>
			buffer_io_result<Hnd> auto operator()(Hnd &&hnd, Buff &&buff, const Ext &pos, To &&to = To()) const noexcept requires tag_invocable<Op, Hnd, Buff, const Ext &, To>
			{
				return tag_invoke(Op{}, std::forward<Hnd>(hnd), std::forward<Buff>(buff), pos, std::forward<To>(to));
			}
		};
		template<typename Op>
		class stream_adaptor<Op>::type : adaptor<Op>::type
		{
			template<typename Hnd>
			using timeout_t = typename adaptor<Op>::type::template timeout_t<Hnd>;
			template<typename Hnd>
			using buffer_t = typename adaptor<Op>::type::template buffer_t<Hnd>;

		public:
			using adaptor<Op>::type::operator();
			template<typename Hnd, buffer_range<std::decay_t<Hnd>, Op> Buff = std::initializer_list<buffer_t<Hnd>>, decays_to_same<timeout_t<Hnd>> To = timeout_t<Hnd>>
			buffer_io_result<Hnd> auto operator()(Hnd &&hnd, Buff &&buff, To &&to = To()) const noexcept requires tag_invocable<Op, Hnd, Buff, To>
			{
				return tag_invoke(static_cast<const Op &>(*this), std::forward<Hnd>(hnd), std::forward<Buff>(buff), std::forward<To>(to));
			}
			template<typename Hnd, decays_to_same<buffer_t<Hnd>> Buff = buffer_t<Hnd>, decays_to_same<timeout_t<Hnd>> To = timeout_t<Hnd>>
			buffer_io_result<Hnd> auto operator()(Hnd &&hnd, Buff &&buff, To &&to = To()) const noexcept requires tag_invocable<Op, Hnd, Buff, To>
			{
				return tag_invoke(static_cast<const Op &>(*this), std::forward<Hnd>(hnd), std::forward<Buff>(buff), std::forward<To>(to));
			}
		};
	}

	/** Concept used to check if \a Hnd is a handle compatible with IO operation \a Op. */
	template<typename Hnd, typename Op>
	concept io_handle = handle<Hnd> && requires
	{
		typename Hnd::template io_buffer_sequence<Op>;
		typename Hnd::template io_buffer<Op>;

		typename Hnd::template io_request<Op>;
		typename Hnd::template io_result<Op>;

		typename Hnd::timeout_type;
		typename Hnd::extent_type;
		typename Hnd::size_type;

		_detail::callable_r<io_result_t<Hnd, Op>, Op, Hnd, io_request_t<Hnd, Op>, handle_timeout_t<Hnd>>;
	};

	namespace _read_some
	{
		struct read_some_at_t : _io_operation::sparse_adaptor<read_some_at_t>::type {};
		struct read_some_t : _io_operation::stream_adaptor<read_some_t>::type {};
	}

	using _read_some::read_some_at_t;
	using _read_some::read_some_t;

	/** Customization point object used to preform a synchronous sparse input using an IO handle.
	 *
	 * @overload Preforms a sparse input operation using an IO request.
	 * @param hnd Handle to preform the IO operation on.
	 * @param req Value of type `io_request_t&lt;decltype(hnd), read_some_at_t&gt;` used to specify parameters of the IO operation.
	 * @param to Optional value of type `io_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Value of type `io_result_t&lt;decltype(hnd), read_some_at_t&gt;` indicating either a success or a status code on failure.
	 * @note It is recommended to use the value of the result, as it may own internal buffers or contain important information about the operation.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note See definition of the individual handle type for further information about it's IO request and result types.
	 *
	 * @overload Preforms a sparse input operation using a range of IO buffers.
	 * @param hnd Handle to preform the IO operation on.
	 * @param dst Sized range of `io_buffer_t&lt;decltype(hnd), read_some_at_t&gt;` containing requested IO buffers.
	 * @param pos Value of type `handle_extent_t&lt;decltype(hnd)&gt;` specifying the target offset within the handle.
	 * @param to Optional value of type `io_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Result containing the amount of bytes transferred or a status code on failure.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note Not all handle types support buffer range IO operations.
	 *
	 * @overload Preforms a sparse input operation using a single IO buffer.
	 * @param hnd Handle to preform the IO operation on.
	 * @param dst Requested IO buffer of type `io_buffer_t&lt;decltype(hnd), read_some_at_t&gt;`.
	 * @param pos Value of type `handle_extent_t&lt;decltype(hnd)&gt;` specifying the target offset within the handle.
	 * @param to Optional value of type `io_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Result containing the amount of bytes transferred or a status code on failure.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note Not all handle types support single-buffer IO operations. */
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
	 *
	 * @overload Preforms an input operation using an IO request.
	 * @param hnd Handle to preform the IO operation on.
	 * @param req Value of type `io_request_t&lt;decltype(hnd), read_some_t&gt;` used to specify parameters of the IO operation.
	 * @param to Optional value of type `io_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Value of type `io_result_t&lt;decltype(hnd), read_some_t&gt;` indicating either a success or a status code on failure.
	 * @note It is recommended to use the value of the result, as it may own internal buffers or contain important information about the operation.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note See definition of the individual handle type for further information about it's IO request and result types.
	 *
	 * @overload Preforms a stream input operation using a range of IO buffers.
	 * @param hnd Handle to preform the IO operation on.
	 * @param dst Sized range of `io_buffer_t&lt;decltype(hnd), read_some_t&gt;` containing requested IO buffers.
	 * @param to Optional value of type `io_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Result containing the amount of bytes transferred or a status code on failure.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note Not all handle types support buffer range IO operations.
	 *
	 * @overload Preforms a stream input operation using a single IO buffer.
	 * @param hnd Handle to preform the IO operation on.
	 * @param dst Requested IO buffer of type `io_buffer_t&lt;decltype(hnd), read_some_t&gt;`.
	 * @param to Optional value of type `io_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Result containing the amount of bytes transferred or a status code on failure.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note Not all handle types support single-buffer IO operations. */
	inline constexpr auto read_some = read_some_t{};

	template<typename Hnd>
	using read_some_buffer_sequence_t = io_buffer_sequence_t<Hnd, read_some_t>;
	template<typename Hnd>
	using read_some_buffer_t = io_buffer_t<Hnd, read_some_t>;

	template<typename Hnd>
	using read_some_request_t = io_request_t<Hnd, read_some_t>;
	template<typename Hnd>
	using read_some_result_t = io_result_t<Hnd, read_some_t>;

	/** Concept used to define an IO handle that supports synchronous sparse input. */
	template<typename Hnd>
	concept sparse_input_handle = io_handle<Hnd, read_some_at_t>;
	/** Concept used to define an IO handle that supports synchronous stream input. */
	template<typename Hnd>
	concept stream_input_handle = io_handle<Hnd, read_some_t>;

	namespace _write_some
	{
		struct write_some_at_t : _io_operation::sparse_adaptor<write_some_at_t>::type {};
		struct write_some_t : _io_operation::stream_adaptor<write_some_t>::type {};
	}

	using _write_some::write_some_at_t;
	using _write_some::write_some_t;

	/** Customization point object used to preform a synchronous sparse output using an IO handle.
	 *
	 * @overload Preforms a sparse output operation using an IO request.
	 * @param hnd Handle to preform the IO operation on.
	 * @param req Value of type `io_request_t&lt;decltype(hnd), write_some_at_t&gt;` used to specify parameters of the IO operation.
	 * @param to Optional value of type `io_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Value of type `io_result_t&lt;decltype(hnd), write_some_at_t&gt;` indicating either a success or a status code on failure.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note See definition of the individual handle type for further information about it's IO request and result types.
	 *
	 * @overload Preforms a sparse output operation using a range of buffers.
	 * @param hnd Handle to preform the IO operation on.
	 * @param src Sized range of `io_buffer_t&lt;decltype(hnd), write_some_at_t&gt;` containing requested IO buffers.
	 * @param pos Value of type `handle_extent_t&lt;decltype(hnd)&gt;` specifying the target offset within the handle.
	 * @param to Optional value of type `io_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Result containing the amount of bytes transferred or a status code on failure.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note Not all handle types support buffer range IO operations.
	 *
	 * @overload Preforms a sparse output operation using a single IO buffer.
	 * @param hnd Handle to preform the IO operation on.
	 * @param src Requested IO buffer of type `io_buffer_t&lt;decltype(hnd), write_some_at_t&gt;`.
	 * @param pos Value of type `handle_extent_t&lt;decltype(hnd)&gt;` specifying the target offset within the handle.
	 * @param to Optional value of type `io_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Result containing the amount of bytes transferred or a status code on failure.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note Not all handle types support single-buffer IO operations. */
	inline constexpr auto write_some_at = write_some_at_t{};
	/** Customization point object used to preform a synchronous stream output using an IO handle.
	 *
	 * @overload Preforms a stream output operation using an IO request.
	 * @param hnd Handle to preform the IO operation on.
	 * @param src Value of type `io_request_t&lt;decltype(hnd), write_some_t&gt;` used to specify parameters of the IO operation.
	 * @param to Optional value of type `io_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Value of type `io_result_t&lt;decltype(hnd), write_some_t&gt;` indicating either a success or a status code on failure.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note See definition of the individual handle type for further information about it's IO request and result types.
	 *
	 * @overload Preforms a stream output operation using a range of IO buffers.
	 * @param hnd Handle to preform the IO operation on.
	 * @param src Sized range of `io_buffer_t&lt;decltype(hnd), write_some_t&gt;` containing requested IO buffers.
	 * @param to Optional value of type `io_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Result containing the amount of bytes transferred or a status code on failure.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note Not all handle types support buffer range IO operations.
	 *
	 * @overload Preforms a stream output operation using a single IO buffer.
	 * @param hnd Handle to preform the IO operation on.
	 * @param src Requested IO buffer of type `io_buffer_t&lt;decltype(hnd), write_some_t&gt;`.
	 * @param to Optional value of type `io_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Result containing the amount of bytes transferred or a status code on failure.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note Not all handle types support single-buffer IO operations. */
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
}

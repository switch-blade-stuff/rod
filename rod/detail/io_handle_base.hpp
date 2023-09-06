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
		struct handle_timeout_impl;
		template<typename Hnd> requires(requires { typename Hnd::timeout_type; })
		struct handle_timeout_impl<Hnd> { using type = typename Hnd::timeout_type; };

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

	/** Type trait used to obtain the timeout type of handle \a Hnd. */
	template<typename Hnd>
	struct handle_timeout : _handle::handle_timeout_impl<Hnd> {};
	/** Alias for `typename handle_timeout&lt;Hnd&gt;::type` */
	template<typename Hnd>
	using handle_timeout_t = typename handle_timeout<Hnd>::type;

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

	namespace _io_operation
	{
		template<typename T, typename Hnd>
		concept buffer_io_result = is_result_v<std::decay_t<T>> && std::constructible_from<typename T::template rebind_value<handle_size_t<std::decay_t<Hnd>>>, std::decay_t<T>>;
		template<typename T, typename Hnd, typename Op>
		concept buffer_range = std::ranges::sized_range<std::decay_t<T>> && decays_to_same<std::ranges::range_value_t<std::decay_t<T>>, io_buffer_t<std::decay_t<Hnd>, Op>>;

		template<typename Op>
		struct adaptor { struct type; };
		template<typename Op>
		struct sparse_adaptor { struct type; };
		template<typename Op>
		struct stream_adaptor { struct type; };

		template<typename Op>
		struct adaptor<Op>::type
		{
			template<io_handle<Op> Hnd, decays_to_same<io_request_t<Hnd, Op>> Req, decays_to_same<handle_timeout_t<Hnd>> To> requires tag_invocable<Op, Hnd, Req, const To &>
			io_result_t<Hnd, Op> operator()(Hnd &&hnd, Req &&req, const To &to = To()) noexcept { return tag_invoke(static_cast<const Op &>(*this), std::forward<Hnd>(hnd), std::forward<Req>(req), to); }
		};
		template<typename Op>
		struct sparse_adaptor<Op>::type : adaptor<Op>::type
		{
			template<io_handle<Op> Hnd, decays_to_same<io_buffer_t<Hnd, Op>> Src, decays_to_same<handle_extent_t<Hnd>> Ext, decays_to_same<handle_timeout_t<Hnd>> To> requires tag_invocable<Op, Hnd, Src, const Ext &, const To &>
			buffer_io_result<Hnd> auto operator()(Hnd &&hnd, Src &&src, const Ext &pos, const To &to = To()) noexcept { return tag_invoke(*this, std::forward<Hnd>(hnd), std::forward<Src>(src), pos, to); }
			template<io_handle<Op> Hnd, buffer_range<Hnd, Op> Src, decays_to_same<handle_extent_t<Hnd>> Ext, decays_to_same<handle_timeout_t<Hnd>> To> requires tag_invocable<Op, Hnd, Src, const Ext &, const To &>
			buffer_io_result<Hnd> auto operator()(Hnd &&hnd, Src &&src, const Ext &pos, const To &to = To()) noexcept { return tag_invoke(*this, std::forward<Hnd>(hnd), std::forward<Src>(src), pos, to); }
		};
		template<typename Op>
		struct stream_adaptor<Op>::type : adaptor<Op>::type
		{
			template<io_handle<Op> Hnd, decays_to_same<io_buffer_t<Hnd, Op>> Dst, decays_to_same<handle_timeout_t<Hnd>> To> requires tag_invocable<Op, Hnd, Dst, const To &>
			buffer_io_result<Hnd> auto operator()(Hnd &&hnd, Dst &&dst, const To &to = To()) noexcept { return tag_invoke(*this, std::forward<Hnd>(hnd), std::forward<Dst>(dst), to); }
			template<io_handle<Op> Hnd, buffer_range<Hnd, Op> Dst, decays_to_same<handle_timeout_t<Hnd>> To> requires tag_invocable<Op, Hnd, Dst, const To &>
			buffer_io_result<Hnd> auto operator()(Hnd &&hnd, Dst &&dst, const To &to = To()) noexcept { return tag_invoke(*this, std::forward<Hnd>(hnd), std::forward<Dst>(dst), to); }
		};
	}

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
	/** Customization point object used to preform a synchronous stream input using an IO handle.
	 *
	 * @overload Preforms an input operation using an IO request.
	 * @param hnd Handle to preform the IO operation on.
	 * @param req Value of type `io_request_t&lt;decltype(hnd), read_some_t&gt;` used to specify parameters of the IO operation.
	 * @param to Optional value of type `io_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Value of type `io_result_t&lt;decltype(hnd), read_some_t&gt;` indicating either a success or a status code on failure.
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
	 * @param dst Sized range of `io_buffer_t&lt;decltype(hnd), write_some_at_t&gt;` containing requested IO buffers.
	 * @param pos Value of type `handle_extent_t&lt;decltype(hnd)&gt;` specifying the target offset within the handle.
	 * @param to Optional value of type `io_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Result containing the amount of bytes transferred or a status code on failure.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note Not all handle types support buffer range IO operations.
	 *
	 * @overload Preforms a sparse output operation using a single IO buffer.
	 * @param hnd Handle to preform the IO operation on.
	 * @param dst Requested IO buffer of type `io_buffer_t&lt;decltype(hnd), write_some_at_t&gt;`.
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
	 * @param req Value of type `io_request_t&lt;decltype(hnd), write_some_t&gt;` used to specify parameters of the IO operation.
	 * @param to Optional value of type `io_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Value of type `io_result_t&lt;decltype(hnd), write_some_t&gt;` indicating either a success or a status code on failure.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note See definition of the individual handle type for further information about it's IO request and result types.
	 *
	 * @overload Preforms a stream output operation using a range of IO buffers.
	 * @param hnd Handle to preform the IO operation on.
	 * @param dst Sized range of `io_buffer_t&lt;decltype(hnd), write_some_t&gt;` containing requested IO buffers.
	 * @param to Optional value of type `io_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Result containing the amount of bytes transferred or a status code on failure.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note Not all handle types support buffer range IO operations.
	 *
	 * @overload Preforms a stream output operation using a single IO buffer.
	 * @param hnd Handle to preform the IO operation on.
	 * @param dst Requested IO buffer of type `io_buffer_t&lt;decltype(hnd), write_some_t&gt;`.
	 * @param to Optional value of type `io_timeout_t&lt;decltype(hnd)&gt;` used to specify the timeout for the IO operation.
	 * @return Result containing the amount of bytes transferred or a status code on failure.
	 * @note If the handle supports partial IO, the result type's status code will indicate the partial amount of bytes transferred.
	 * @note Not all handle types support single-buffer IO operations. */
	inline constexpr auto write_some = write_some_t{};

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

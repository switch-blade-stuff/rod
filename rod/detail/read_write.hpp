/*
 * Created by switchblade on 2023-05-18.
 */

#pragma once

#include <concepts>
#include <ranges>
#include <span>

#include "../scheduling.hpp"

ROD_TOPLEVEL_NAMESPACE_OPEN
namespace rod::io
{
	/** Concept used to check if type \a T is a buffer type (contiguous range of `std::byte` or integral values) that can participate in read & write operations. */
	template<typename T>
	concept buffer = std::ranges::contiguous_range<T> && (std::same_as<std::ranges::range_value_t<T>, std::byte> || std::integral<std::ranges::range_value_t<T>>);

	namespace _read
	{
		struct read_t
		{
			template<typename Src, buffer Buff> requires tag_invocable<read_t, Src, Buff, std::error_code &>
			[[nodiscard]] std::size_t operator()(Src &&src, Buff &&buff, std::error_code &err) const noexcept(nothrow_tag_invocable<read_t, Src, Buff, std::error_code &>)
			{
				return tag_invoke(*this, std::forward<Src>(src), std::forward<Buff>(buff), err);
			}
			template<typename Src, buffer Buff> requires tag_invocable<read_t, Src, Buff>
			[[nodiscard]] std::size_t operator()(Src &&src, Buff &&buff) const noexcept(nothrow_tag_invocable<read_t, Src, Buff>)
			{
				return tag_invoke(*this, std::forward<Src>(src), std::forward<Buff>(buff));
			}
			template<typename Src, buffer Buff> requires(!tag_invocable<read_t, Src, Buff>)
			[[nodiscard]] std::size_t operator()(Src &&src, Buff &&buff) const
			{
				static_assert(tag_invocable<read_t, Src, Buff, std::error_code &>);

				std::error_code err = {};
				if (const auto res = operator()(std::forward<Src>(src), std::forward<Buff>(buff), err); err)
					throw std::system_error(err);
				else
					return res;
			}
		};

		struct read_at_t
		{
			template<typename Src, std::integral Pos, buffer Buff> requires tag_invocable<read_at_t, Src, Pos, Buff, std::error_code &>
			[[nodiscard]] std::size_t operator()(Src &&src, Pos &&pos, Buff &&buff, std::error_code &err) const noexcept(nothrow_tag_invocable<read_at_t, Src, Pos, Buff, std::error_code &>)
			{
				return tag_invoke(*this, std::forward<Src>(src), std::forward<Pos>(pos), std::forward<Buff>(buff), err);
			}
			template<typename Src, std::integral Pos, buffer Buff> requires tag_invocable<read_at_t, Src, Pos, Buff>
			[[nodiscard]] std::size_t operator()(Src &&src, Pos &&pos, Buff &&buff) const noexcept(nothrow_tag_invocable<read_at_t, Src, Pos, Buff>)
			{
				return tag_invoke(*this, std::forward<Src>(src), std::forward<Pos>(pos), std::forward<Buff>(buff));
			}
			template<typename Src, std::integral Pos, buffer Buff> requires(!tag_invocable<read_at_t, Src, Pos, Buff>)
			[[nodiscard]] std::size_t operator()(Src &&src, Pos &&pos, Buff &&buff) const
			{
				static_assert(tag_invocable<read_at_t, Src, Pos, Buff, std::error_code &>);

				std::error_code err = {};
				if (const auto res = operator()(std::forward<Src>(src), std::forward<Pos>(pos), std::forward<Buff>(buff), err); err)
					throw std::system_error(err);
				else
					return res;
			}
		};

		template<typename Snd>
		concept read_sender = sender_of<Snd, set_value_t(std::size_t)> || sender_of<Snd, set_value_t(std::size_t, std::error_code)>;

		class async_read_t
		{
			template<typename Snd>
			using value_completion = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Src, typename Buff>
			using back_adaptor = detail::back_adaptor<async_read_t, Src, std::decay_t<Buff>>;

		public:
			template<sender Snd, typename Src, buffer Buff> requires detail::tag_invocable_with_completion_scheduler<async_read_t, set_value_t, Snd, Snd, Src, Buff>
			[[nodiscard]] read_sender decltype(auto) operator()(Snd &&snd, Src &&src, Buff &&buff) const noexcept(nothrow_tag_invocable<async_read_t, value_completion<Snd>, Snd, Src, Buff>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::forward<Src>(src), std::forward<Buff>(buff));
			}
			template<sender Snd, typename Src, buffer Buff> requires(!detail::tag_invocable_with_completion_scheduler<async_read_t, set_value_t, Snd, Snd, Src, Buff> && tag_invocable<async_read_t, Snd, Src, Buff>)
			[[nodiscard]] read_sender decltype(auto) operator()(Snd &&snd, Src &&src, Buff &&buff) const noexcept(nothrow_tag_invocable<async_read_t, Snd, Src, Buff>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<Src>(src), std::forward<Buff>(buff));
			}

			template<typename Src, buffer Buff>
			[[nodiscard]] back_adaptor<Src, Buff> operator()(Src &&src, Buff &&buff) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Src, Buff>, async_read_t, Src, Buff>)
			{
				return back_adaptor<Src, Buff>{*this, std::forward_as_tuple(std::forward<Src>(src), std::forward<Buff>(buff))};
			}
		};

		class async_read_at_t
		{
			template<typename Snd>
			using value_completion = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Src, typename Pos, typename Buff>
			using back_adaptor = detail::back_adaptor<async_read_t, Src, std::decay_t<Pos>, std::decay_t<Buff>>;

		public:
			template<sender Snd, typename Src, std::integral Pos, buffer Buff> requires detail::tag_invocable_with_completion_scheduler<async_read_at_t, set_value_t, Snd, Snd, Src, Pos, Buff>
			[[nodiscard]] read_sender decltype(auto) operator()(Snd &&snd, Src &&src, Pos &&pos, Buff &&buff) const noexcept(nothrow_tag_invocable<async_read_at_t, value_completion<Snd>, Snd, Src, Pos, Buff>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::forward<Src>(src), std::forward<Pos>(pos), std::forward<Buff>(buff));
			}
			template<sender Snd, typename Src, std::integral Pos, buffer Buff> requires(!detail::tag_invocable_with_completion_scheduler<async_read_at_t, set_value_t, Snd, Snd, Src, Pos, Buff> && tag_invocable<async_read_at_t, Snd, Src, Pos, Buff>)
			[[nodiscard]] read_sender decltype(auto) operator()(Snd &&snd, Src &&src, Pos &&pos, Buff &&buff) const noexcept(nothrow_tag_invocable<async_read_at_t, Snd, Src, Pos, Buff>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<Src>(src), std::forward<Pos>(pos), std::forward<Buff>(buff));
			}

			template<typename Src, std::integral Pos, buffer Buff>
			[[nodiscard]] back_adaptor<Src, Pos, Buff> operator()(Src &&src, Pos &&pos, Buff &&buff) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Src, Pos, Buff>, async_read_t, Src, Pos, Buff>)
			{
				return back_adaptor<Src, Pos, Buff>{*this, std::forward_as_tuple(std::forward<Src>(src), std::forward<Pos>(pos), std::forward<Buff>(buff))};
			}
		};
	}

	using _read::read_t;

	/** Concept used to check if type \a T implements the `read` CBO for buffer type \a Buff. */
	template<typename T, typename Buff = std::span<std::byte>>
	concept readable = detail::callable<read_t, T, Buff, std::error_code &>;

	/** Customization point object used to read a contiguous buffer of integral values from a readable source object.
	 * @note Not to be confused with `rod::read` factory.
	 * @param[in] src Source object to read the data from.
	 * @param[out] buff Contiguous output range of integral values.
	 * @param[out] err Reference to the error code set on failure to complete the operation. If omitted, an exception is thrown instead.
	 * @throw std::system_error If an error has occurred and no output error code has been specified.
	 * @return Amount of elements read. */
	inline constexpr auto read = read_t{};

	using _read::read_at_t;

	/** Concept used to check if type \a T is a readable type that implements the `read_at` CBO for offset type \a Pos and buffer type \a Buff. */
	template<typename T, typename Pos = std::size_t, typename Buff = std::span<std::byte>>
	concept readable_at = detail::callable<read_at_t, T, Pos, Buff>;

	/** Customization point object used to read a contiguous buffer of integral values from a readable source object at the specified offset.
	 * @param[in] src Source object to read the data from.
	 * @param[in] pos Offset into the source object at which to read the values.
	 * @param[out] buff Contiguous output range of integral values.
	 * @param[out] err Reference to the error code set on failure to complete the operation. If omitted, an exception is thrown instead.
	 * @throw std::system_error If an error has occurred and no output error code has been specified.
	 * @return Amount of elements read. */
	inline constexpr auto read_at = read_at_t{};

	using _read::async_read_t;

	/** Concept used to check if type \a Src is a readable source type that implements the `async_read` CBO for input sender \a Snd and buffer type \a Buff. */
	template<typename Snd, typename Src, typename Buff = std::span<std::byte>>
	concept async_readable = detail::callable<async_read_t, Snd, Src, Buff>;

	/** Customization point object returning a sender used to read a contiguous buffer of integral values from an async-readable source object.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] snd Input sender who's value completion channel will be used for the async read operation. If omitted, creates a pipe-able sender adaptor.
	 * @param[in] src Source object used to read the data from.
	 * @param[out] buff Contiguous output range of integral values.
	 * @return Sender completing either with the amount of elements read and an optional error code, or an error on read failure. */
	inline constexpr auto async_read = async_read_t{};

	using _read::async_read_at_t;

	/** Concept used to check if type \a Src is an readable source type that implements the `async_read_at` CBO for input sender \a Snd and buffer type \a Buff. */
	template<typename Snd, typename Src, typename Pos = std::size_t, typename Buff = std::span<std::byte>>
	concept async_readable_at = async_readable<Snd, Src, Buff> && detail::callable<async_read_at_t, Snd, Src, Pos, Buff>;

	/** Customization point object returning a sender used to read a contiguous buffer of integral values from an async-readable source object.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] snd Input sender who's value completion channel will be used for the async read operation. If omitted, creates a pipe-able sender adaptor.
	 * @param[in] src Source object used to read the data from.
	 * @param[out] buff Contiguous output range of integral values.
	 * @return Sender completing either with the amount of elements read and an optional error code, or an error on read failure. */
	inline constexpr auto async_read_at = async_read_at_t{};

	namespace _write
	{
		struct write_t
		{
			template<typename Dst, typename Buff> requires tag_invocable<write_t, Dst, Buff, std::error_code &>
			[[nodiscard]] std::size_t operator()(Dst &&dst, Buff &&buff, std::error_code &err) const noexcept(nothrow_tag_invocable<write_t, Dst, Buff, std::error_code &>)
			{
				return tag_invoke(*this, std::forward<Dst>(dst), std::forward<Buff>(buff), err);
			}
			template<typename Dst, typename Buff> requires tag_invocable<write_t, Dst, Buff>
			[[nodiscard]] std::size_t operator()(Dst &&dst, Buff &&buff) const noexcept(nothrow_tag_invocable<write_t, Dst, Buff>)
			{
				return tag_invoke(*this, std::forward<Dst>(dst), std::forward<Buff>(buff));
			}
			template<typename Dst, typename Buff> requires(!tag_invocable<write_t, Dst, Buff>)
			[[nodiscard]] std::size_t operator()(Dst &&dst, Buff &&buff) const
			{
				static_assert(tag_invocable<write_t, Dst, Buff, std::error_code &>);

				std::error_code err = {};
				if (const auto res = operator()(std::forward<Dst>(dst), std::forward<Buff>(buff), err); err)
					throw std::system_error(err);
				else
					return res;
			}
		};

		struct write_at_t
		{
			template<typename Dst, std::integral Pos, buffer Buff> requires tag_invocable<write_at_t, Dst, Pos, Buff, std::error_code &>
			[[nodiscard]] std::size_t operator()(Dst &&dst, Pos &&pos, Buff &&buff, std::error_code &err) const noexcept(nothrow_tag_invocable<write_at_t, Dst, Pos, Buff, std::error_code &>)
			{
				return tag_invoke(*this, std::forward<Dst>(dst), std::forward<Pos>(pos), std::forward<Buff>(buff), err);
			}
			template<typename Dst, std::integral Pos, buffer Buff> requires tag_invocable<write_at_t, Dst, Pos, Buff>
			[[nodiscard]] std::size_t operator()(Dst &&dst, Pos &&pos, Buff &&buff) const noexcept(nothrow_tag_invocable<write_at_t, Dst, Pos, Buff>)
			{
				return tag_invoke(*this, std::forward<Dst>(dst), std::forward<Pos>(pos), std::forward<Buff>(buff));
			}
			template<typename Dst, std::integral Pos, buffer Buff> requires(!tag_invocable<write_at_t, Dst, Pos, Buff>)
			[[nodiscard]] std::size_t operator()(Dst &&dst, Pos &&pos, Buff &&buff) const
			{
				static_assert(tag_invocable<write_at_t, Dst, Pos, Buff, std::error_code &>);

				std::error_code err = {};
				if (const auto res = operator()(std::forward<Dst>(dst), std::forward<Pos>(pos), std::forward<Buff>(buff), err); err)
					throw std::system_error(err);
				else
					return res;
			}
		};

		template<typename Snd>
		concept write_sender = sender_of<Snd, set_value_t(std::size_t)> || sender_of<Snd, set_value_t(std::size_t, std::error_code)>;

		class async_write_t
		{
			template<typename Snd>
			using value_completion = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Dst, typename Buff>
			using back_adaptor = detail::back_adaptor<async_write_t, Dst, std::decay_t<Buff>>;

		public:
			template<sender Snd, typename Dst, buffer Buff> requires detail::tag_invocable_with_completion_scheduler<async_write_t, set_value_t, Snd, Snd, Dst, Buff>
			[[nodiscard]] write_sender decltype(auto) operator()(Snd &&snd, Dst &&dst, Buff &&buff) const noexcept(nothrow_tag_invocable<async_write_t, value_completion<Snd>, Snd, Dst, Buff>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::forward<Dst>(dst), std::forward<Buff>(buff));
			}
			template<sender Snd, typename Dst, buffer Buff> requires(!detail::tag_invocable_with_completion_scheduler<async_write_t, set_value_t, Snd, Snd, Dst, Buff> && tag_invocable<async_read_t, Snd, Dst, Buff>)
			[[nodiscard]] write_sender decltype(auto) operator()(Snd &&snd, Dst &&dst, Buff &&buff) const noexcept(nothrow_tag_invocable<async_write_t, Snd, Dst, Buff>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<Dst>(dst), std::forward<Buff>(buff));
			}

			template<typename Dst, buffer Buff>
			[[nodiscard]] back_adaptor<Dst, Buff> operator()(Dst &&dst, Buff &&buff) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Dst, Buff>, async_write_t, Dst, Buff>)
			{
				return back_adaptor<Dst, Buff>{*this, std::forward_as_tuple(std::forward<Dst>(dst), std::forward<Buff>(buff))};
			}
		};

		class async_write_at_t
		{
			template<typename Snd>
			using value_completion = decltype(get_completion_scheduler<set_value_t>(get_env(std::declval<Snd>())));
			template<typename Dst, typename Pos, typename Buff>
			using back_adaptor = detail::back_adaptor<async_write_at_t, Dst, std::decay_t<Pos>, std::decay_t<Buff>>;

		public:
			template<sender Snd, typename Dst, std::integral Pos, buffer Buff> requires detail::tag_invocable_with_completion_scheduler<async_write_at_t, set_value_t, Snd, Snd, Dst, Pos, Buff>
			[[nodiscard]] write_sender decltype(auto) operator()(Snd &&snd, Dst &&dst, Pos &&pos, Buff &&buff) const noexcept(nothrow_tag_invocable<async_write_at_t, value_completion<Snd>, Snd, Dst, Pos, Buff>)
			{
				return tag_invoke(*this, get_completion_scheduler<set_value_t>(get_env(snd)), std::forward<Snd>(snd), std::forward<Dst>(dst), std::forward<Pos>(pos), std::forward<Buff>(buff));
			}
			template<sender Snd, typename Dst, std::integral Pos, buffer Buff> requires(!detail::tag_invocable_with_completion_scheduler<async_write_at_t, set_value_t, Snd, Snd, Dst, Pos, Buff> && tag_invocable<async_write_at_t, Snd, Dst, Pos, Buff>)
			[[nodiscard]] write_sender decltype(auto) operator()(Snd &&snd, Dst &&dst, Pos &&pos, Buff &&buff) const noexcept(nothrow_tag_invocable<async_write_at_t, Snd, Dst, Pos, Buff>)
			{
				return tag_invoke(*this, std::forward<Snd>(snd), std::forward<Dst>(dst), std::forward<Pos>(pos), std::forward<Buff>(buff));
			}

			template<typename Dst, std::integral Pos, buffer Buff>
			[[nodiscard]] back_adaptor<Dst, Pos, Buff> operator()(Dst &&dst, Pos &&pos, Buff &&buff) const noexcept(std::is_nothrow_constructible_v<back_adaptor<Dst, Pos, Buff>, async_write_at_t, Dst, Pos, Buff>)
			{
				return back_adaptor<Dst, Pos, Buff>{*this, std::forward_as_tuple(std::forward<Dst>(dst), std::forward<Pos>(pos), std::forward<Buff>(buff))};
			}
		};
	}

	using _write::write_t;

	/** Concept used to check if type \a T implements the `write` CBO for buffer type \a Buff. */
	template<typename T, typename Buff = std::span<const std::byte>>
	concept writeable = detail::callable<write_t, T, Buff>;

	/** Customization point object used to write a contiguous buffer of integral values to a writeable destination object.
	 * @param[in] dst Destination object to write the data into.
	 * @param[in] buff Contiguous input range of integral values.
	 * @param[out] err Reference to the error code set on failure to complete the operation. If omitted, an exception is thrown instead.
	 * @throw std::system_error If an error has occurred and no output error code has been specified.
	 * @return Amount of elements written. */
	inline constexpr auto write = write_t{};

	using _write::write_at_t;

	/** Concept used to check if type \a T is a writeable type that implements the `write_at` CBO for offset type \a Pos and buffer type \a Buff. */
	template<typename T, typename Pos = std::size_t, typename Buff = std::span<const std::byte>>
	concept writeable_at = detail::callable<write_at_t, T, Pos, Buff>;

	/** Customization point object used to write a contiguous buffer of integral values to a writeable destination object at the specified offset.
	 * @param[in] dst Destination object to write the data into.
	 * @param[in] pos Offset into the destination object at which to write the values.
	 * @param[in] buff Contiguous input range of integral values.
	 * @param[out] err Reference to the error code set on failure to complete the operation. If omitted, an exception is thrown instead.
	 * @throw std::system_error If an error has occurred and no output error code has been specified.
	 * @return Amount of elements written. */
	inline constexpr auto write_at = write_at_t{};

	using _write::async_write_t;

	/** Concept used to check if type \a T is a writeable type implements the `async_write` CBO for buffer type \a Buff. */
	template<typename T, typename Buff = std::span<const std::byte>>
	concept async_writeable = writeable<T, Buff> && tag_invocable<async_write_t, T, Buff>;

	/** Customization point object returning a sender used to write a contiguous buffer of integral values to an async-writeable destination object.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] dst Destination object used to write the data into.
	 * @param[in] buff Contiguous input range of integral values.
	 * @return Sender completing either with the amount of elements read and an optional error code, or an error on read failure. */
	inline constexpr auto async_write = async_write_t{};

	using _write::async_write_at_t;

	/** Concept used to check if type \a T is an async writeable type implements the `async_write_at` CBO for buffer type \a Buff. */
	template<typename T, typename Pos = std::size_t, typename Buff = std::span<const std::byte>>
	concept async_writeable_at = writeable_at<T, Pos, Buff> && tag_invocable<async_write_at_t, T, Pos, Buff>;

	/** Customization point object returning a sender used to write a contiguous buffer of integral values to an async-writeable destination object.
	 * @note Resulting sender might complete with an optional error code as part of the value channel to indicate partial success.
	 * @param[in] dst Destination object used to write the data into.
	 * @param[in] pos Offset into the destination object at which to write the values.
	 * @param[in] buff Contiguous input range of integral values.
	 * @return Sender completing either with the amount of elements read and an optional error code, or an error on read failure. */
	inline constexpr auto async_write_at = async_write_at_t{};

	/** Creates a mutable span of bytes created from the specified raw pointer and size. */
	[[nodiscard]] inline std::span<std::byte> make_byte_buffer(void *ptr, std::size_t n) noexcept
	{
		const auto bytes = static_cast<std::byte *>(ptr);
		return {bytes, bytes + n};
	}
	/** Creates an immutable span of bytes created from the specified raw pointer and size. */
	[[nodiscard]] inline std::span<const std::byte> make_byte_buffer(const void *ptr, std::size_t n) noexcept
	{
		const auto bytes = static_cast<const std::byte *>(ptr);
		return {bytes, bytes + n};
	}

	/** Returns a mutable span of bytes created from the specified pointer to integral and size. */
	template<std::integral T>
	[[nodiscard]] inline std::span<std::byte> make_byte_buffer(T *ptr, std::size_t n) noexcept
	{
		return make_byte_buffer(static_cast<void *>(ptr), n * sizeof(T));
	}
	/** Returns an immutable span of bytes created from the specified pointer to integral and size. */
	template<std::integral T>
	[[nodiscard]] inline std::span<const std::byte> make_byte_buffer(const T *ptr, std::size_t n) noexcept
	{
		return make_byte_buffer(static_cast<const void *>(ptr), n * sizeof(T));
	}
}
ROD_TOPLEVEL_NAMESPACE_CLOSE

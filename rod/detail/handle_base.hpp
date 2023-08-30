/*
 * Created by switch_blade on 2023-08-15.
 */

#pragma once

#include "handle_stat.hpp"

namespace rod
{
	namespace _handle
	{
		class basic_handle;

		/** Enumeration used to control behavior of handle open functions. */
		enum class open_mode : std::uint8_t
		{
			/** Open or create if missing. */
			always,
			/** Create and fail if exists. */
			create,
			/** Open if exists and fail if missing. */
			existing,
			/** Open if exists and atomically truncate. */
			truncate,
			/** Create if missing or replace if exists. */
			supersede,
		};

		ROD_API_PUBLIC auto do_close(basic_handle &) noexcept -> result<>;
		ROD_API_PUBLIC auto do_clone(const basic_handle &) noexcept -> result<basic_handle>;

		template<typename Res>
		concept close_result = is_result_with_value_v<Res, void>;
		template<typename Res, typename Hnd>
		concept clone_result = is_result_v<Res> && std::constructible_from<typename Res::template rebind_value<Hnd>, Res>;

		struct close_t
		{
			template<typename Hnd> requires tag_invocable<close_t, Hnd>
			close_result auto operator()(Hnd &&hnd) const noexcept { return tag_invoke(*this, std::forward<Hnd>(hnd)); }
			template<typename Hnd> requires(!tag_invocable<close_t, Hnd>)
			close_result auto operator()(Hnd &&hnd) const noexcept { return do_close(std::forward<Hnd>(hnd)); }
		};
		struct clone_t
		{
			template<typename Hnd> requires tag_invocable<clone_t, const Hnd &>
			[[nodiscard]] clone_result<Hnd> auto operator()(const Hnd &hnd) const noexcept { return tag_invoke(*this, hnd); }
			template<typename Hnd> requires(!tag_invocable<clone_t, const Hnd &>)
			[[nodiscard]] clone_result<Hnd> auto operator()(const Hnd &hnd) const noexcept { return do_clone(hnd); }
		};

		/** Enumeration used to select representation of the path returned by `to_native_path`. */
		enum class native_path_format
		{
			/** Use any valid implementation-defined path representation. */
			any = 0,
			/** Use a generic path representation (ex. use DOS path format under Windows). */
			generic,
			/** Use system-native path representation (ex. use Win32 path format under Windows).
			 * @note Some systems (ex. Linux) do not support different path representations, in which case `system` is same as `generic`. */
			system,

			/** Use a unique volume ID path representation (Windows only). */
			volume_id,
			/** Use a unique object ID path representation (Windows only). */
			object_id,
		};

		/* Default implementations for basic_handle. */
		ROD_API_PUBLIC result<path> do_to_object_path(const basic_handle &) noexcept;
		ROD_API_PUBLIC result<path> do_to_native_path(const basic_handle &, native_path_format) noexcept;

		template<typename Res>
		concept path_result = is_result_v<Res> && std::constructible_from<typename Res::template rebind_value<path>, result<path>>;

		struct to_object_path_t
		{
			template<typename Hnd> requires tag_invocable<to_object_path_t, const Hnd &>
			[[nodiscard]] path_result auto operator()(const Hnd &hnd) const noexcept { return tag_invoke(*this, hnd); }
			template<typename Hnd> requires(!tag_invocable<to_object_path_t, const Hnd &>)
			[[nodiscard]] path_result auto operator()(const Hnd &hnd) const noexcept { return do_to_object_path(hnd); }
		};
		struct to_native_path_t
		{
			template<typename Hnd> requires tag_invocable<to_native_path_t, const Hnd &, native_path_format>
			[[nodiscard]] path_result auto operator()(const Hnd &hnd, native_path_format fmt) const noexcept { return tag_invoke(*this, hnd, fmt); }
			template<typename Hnd> requires(!tag_invocable<to_native_path_t, const Hnd &, native_path_format>)
			[[nodiscard]] path_result auto operator()(const Hnd &hnd, native_path_format fmt) const noexcept { return do_to_native_path(hnd, fmt); }
		};

		/** Basic system handle type. */
		class basic_handle
		{
		public:
#if defined(ROD_WIN32)
			using native_handle_type = void *;
#elif defined(ROD_POSIX)
			using native_handle_type = int;
#endif

		private:
#if defined(ROD_WIN32)
			static native_handle_type sentinel() noexcept { return std::bit_cast<native_handle_type>(std::intptr_t(-1)); }
#elif defined(ROD_POSIX)
			static native_handle_type sentinel() noexcept { return -1; }
#endif

		public:
			basic_handle(const basic_handle &) = delete;
			basic_handle &operator=(const basic_handle &) = delete;

			basic_handle() noexcept = default;
			basic_handle(basic_handle &&other) noexcept : _hnd(std::exchange(other._hnd, sentinel())) {}
			basic_handle &operator=(basic_handle &&other) noexcept { return (std::swap(_hnd, other._hnd), *this); }

			explicit basic_handle(native_handle_type hnd) noexcept : _hnd(hnd) {}
			~basic_handle() { if (is_open()) do_close(*this); }

			/** Checks if the handle is open. */
			[[nodiscard]] bool is_open() const noexcept { return _hnd != sentinel(); }
			/** @copydoc is_open */
			[[nodiscard]] explicit operator bool() const noexcept { return is_open(); }
			/** Returns the underlying native handle. */
			[[nodiscard]] native_handle_type native_handle() const noexcept { return _hnd; }

			/** Releases the underlying native handle. */
			native_handle_type release() noexcept { return release(sentinel()); }
			/** Releases the underlying native handle and replaces it with \a hnd. */
			native_handle_type release(native_handle_type hnd) noexcept { return std::exchange(_hnd, hnd); }

			constexpr void swap(basic_handle &other) noexcept { std::swap(_hnd, other._hnd); }
			friend constexpr void swap(basic_handle &a, basic_handle &b) noexcept { a.swap(b); }

		public:
			[[nodiscard]] friend bool operator==(const basic_handle &a, const basic_handle &b) noexcept { return a.native_handle() == b.native_handle(); }
			[[nodiscard]] friend bool operator!=(const basic_handle &a, const basic_handle &b) noexcept { return a.native_handle() != b.native_handle(); }

		private:
			native_handle_type _hnd = sentinel();
		};

		template<typename Child, typename Base>
		struct handle_adaptor { class type; };
		template<typename Child, typename Base>
		class handle_adaptor<Child, Base>::type : public Base
		{
		public:
			using native_handle_type = typename Base::native_handle_type;

		private:
			template<typename T>
			static decltype(auto) get_adaptor(T &&value) noexcept { return ((copy_cvref_t<T, type>) std::forward<T>(value)); }

		public:
			type(const type &) = delete;
			type &operator=(const type &) = delete;

			type() noexcept = default;
			type(type &&) noexcept = default;
			type &operator=(type &&) noexcept = default;

			explicit type(native_handle_type hnd) noexcept : Base(hnd) {}
			explicit type(Base &&other) noexcept : Base(std::forward<Base>(other)) {}

			using Base::release;
			using Base::is_open;
			using Base::operator bool;
			using Base::native_handle;

			constexpr void swap(type &other) noexcept { adl_swap(static_cast<Base &>(*this), static_cast<Base &>(other)); }
			friend constexpr void swap(type &a, type &b) noexcept { adl_swap(static_cast<Base &>(a), static_cast<Base &>(b)); }

		public:
			[[nodiscard]] friend bool operator==(const type &, const type &) noexcept = default;
			[[nodiscard]] friend bool operator!=(const type &, const type &) noexcept = default;

		private:
			static constexpr int close = 1;
			static constexpr int clone = 1;

			template<typename Hnd>
			static constexpr bool has_close() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::close)); }; }
			template<typename Hnd>
			static constexpr bool has_clone() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::clone)); }; }

			template<typename Hnd>
			using clone_result = typename decltype(clone_t{}(std::declval<const Base &>()))::template rebind_value<Hnd>;

			template<typename Hnd>
			constexpr static auto dispatch_close(Hnd &&hnd) noexcept -> decltype(std::forward<Hnd>(hnd).close()) { return hnd.close(); }
			template<typename Hnd>
			constexpr static auto dispatch_clone(const Hnd &hnd) noexcept -> decltype(hnd.clone()) { return hnd.clone(); }

			static constexpr int to_object_path = 1;
			static constexpr int to_native_path = 1;

			template<typename Hnd>
			static constexpr bool has_to_object_path() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::to_object_path)); }; }
			template<typename Hnd>
			static constexpr bool has_to_native_path() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::to_native_path)); }; }

			template<typename Hnd>
			constexpr static auto dispatch_to_object_path(const Hnd &hnd) noexcept -> decltype(hnd.to_object_path()) { return hnd.to_object_path(); }
			template<typename Hnd>
			constexpr static auto dispatch_to_native_path(const Hnd &hnd, native_path_format fmt) noexcept -> decltype(hnd.to_native_path(fmt)) { return hnd.to_native_path(fmt); }

			static constexpr int get_stat = 1;
			static constexpr int set_stat = 1;

			template<typename Hnd>
			static constexpr bool has_get_stat() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::get_stat)); }; }
			template<typename Hnd>
			static constexpr bool has_set_stat() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::set_stat)); }; }

			template<typename Hnd>
			constexpr static auto dispatch_get_stat(stat &st, const Hnd &hnd, stat::query q) noexcept -> decltype(hnd.get_stat(st, hnd, q)) { return hnd.get_stat(st, hnd, q); }
			template<typename Hnd>
			constexpr static auto dispatch_set_stat(const stat &st, Hnd &hnd, stat::query q) noexcept -> decltype(hnd.set_stat(st, hnd, q)) { return hnd.set_stat(st, hnd, q); }

		public:
			template<std::same_as<close_t> T> requires(has_close<Child>())
			friend auto tag_invoke(T, Child &&hnd) noexcept { return dispatch_close(std::forward<Child>(hnd)); }
			template<std::same_as<close_t> T, typename Hnd = Child> requires(!has_close<Hnd>())
			friend auto tag_invoke(T, Hnd &&hnd) noexcept { return close_t{}(get_adaptor(std::forward<Hnd>(hnd)).base()); }

			template<std::same_as<clone_t> T> requires(has_clone<Child>())
			friend auto tag_invoke(T, const Child &hnd) noexcept { return dispatch_clone(hnd); }
			template<std::same_as<clone_t> T, typename Hnd = Child> requires(!has_clone<Hnd>())
			friend auto tag_invoke(T, const Hnd &hnd) noexcept { return clone_result<Hnd>(clone_t{}(get_adaptor(hnd).base())); }

			template<std::same_as<to_native_path_t> T> requires(has_to_native_path<Child>())
			friend auto tag_invoke(T, const Child &hnd, native_path_format fmt) noexcept { return dispatch_to_native_path(hnd, fmt); }
			template<std::same_as<to_native_path_t> T, typename Hnd = Child> requires(!has_to_native_path<Hnd>())
			friend auto tag_invoke(T, const Hnd &hnd, native_path_format fmt) noexcept { return to_native_path_t{}(get_adaptor(hnd).base(), fmt); }

		protected:
			[[nodiscard]] constexpr decltype(auto) base() & noexcept { return static_cast<Base &>(*this); }
			[[nodiscard]] constexpr decltype(auto) base() const & noexcept { return static_cast<const Base &>(*this); }
			[[nodiscard]] constexpr decltype(auto) base() && noexcept { return static_cast<Base &&>(*this); }
			[[nodiscard]] constexpr decltype(auto) base() const && noexcept { return static_cast<const Base &&>(*this); }
		};
	}

	/** Concept used to define a handle type. */
	template<typename Hnd>
	concept handle = !std::copyable<Hnd> && std::movable<Hnd> && std::destructible<Hnd> && std::swappable<Hnd> && std::equality_comparable<Hnd> && requires(Hnd &mut_hnd, const Hnd &hnd)
	{
		typename Hnd::native_handle_type;
		std::constructible_from<Hnd, typename Hnd::native_handle_type>;

		{ hnd.is_open() } -> std::convertible_to<bool>;
		{ hnd.native_handle() } -> std::convertible_to<typename Hnd::native_handle_type>;

		{ mut_hnd.release() } -> std::convertible_to<typename Hnd::native_handle_type>;
		{ mut_hnd.release(std::declval<typename Hnd::native_handle_type>()) } -> std::convertible_to<typename Hnd::native_handle_type>;

		_detail::callable<_handle::close_t, Hnd &>;
		_detail::callable<_handle::clone_t, const Hnd &>;

		_detail::callable<_handle::to_object_path_t, const Hnd &>;
		_detail::callable<_handle::to_native_path_t, const Hnd &, _handle::native_path_format>;

		_detail::callable<get_stat_t, stat &, const Hnd &, stat::query>;
		_detail::callable<set_stat_t, const stat &, Hnd &, stat::query>;
	};

	namespace _handle
	{
		struct dummy_handle : handle_adaptor<dummy_handle, basic_handle>::type
		{
			using adaptor_base = handle_adaptor<dummy_handle, basic_handle>::type;
			using adaptor_base::adaptor_base;
		};

		static_assert(handle<dummy_handle>, "Child of `hande_adaptor` must satisfy `handle`");
		static_assert(handle<basic_handle>, "`basic_handle` must satisfy `handle`");
	}

	/** Handle adaptor used to implement basic handle functionality. */
	template<typename Child, handle Base = basic_handle>
	using handle_adaptor = typename _handle::handle_adaptor<Child, Base>::type;

	using _handle::basic_handle;
	using _handle::open_mode;

	using _handle::close_t;
	using _handle::clone_t;

	/** Customization point object used to close an open handle.
	 * @param hnd Handle to close.
	 * @return `void` result on success or a status code on failure. */
	inline constexpr auto close = close_t{};
	/** Customization point object used to clone a handle.
	 * @param hnd Handle to clone.
	 * @return Result containing the cloned handle on success or a status code on failure. */
	inline constexpr auto clone = clone_t{};

	using _handle::native_path_format;
	using _handle::to_object_path_t;
	using _handle::to_native_path_t;
}

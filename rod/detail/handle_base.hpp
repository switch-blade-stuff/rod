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
			/** Open if exists or create if missing. */
			always,
			/** Create only if does not exist and fail otherwise. */
			create,

			/** Open only if already exists and fail otherwise. */
			existing,
			/** Open and overwrite contents only if already exists and fail otherwise. */
			truncate,
			/** Replace if already exists or create if missing. */
			supersede,
		};

		template<typename Res>
		concept close_result = is_result_with_value_v<Res, void>;
		template<typename Res, typename Hnd>
		concept clone_result = instance_of<Res, result> && std::constructible_from<typename Res::template rebind_value<Hnd>, Res>;

		struct close_t
		{
			template<typename Hnd> requires tag_invocable<close_t, Hnd>
			close_result auto operator()(Hnd &&hnd) const noexcept { return tag_invoke(*this, std::forward<Hnd>(hnd)); }
		};
		struct clone_t
		{
			template<typename Hnd> requires tag_invocable<clone_t, const Hnd &>
			[[nodiscard]] clone_result<Hnd> auto operator()(const Hnd &hnd) const noexcept { return tag_invoke(*this, hnd); }
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

		template<typename Res>
		concept path_result = instance_of<Res, result> && std::constructible_from<typename Res::template rebind_value<path>, result<path>>;

		struct to_object_path_t
		{
			template<typename Hnd> requires tag_invocable<to_object_path_t, const Hnd &>
			[[nodiscard]] path_result auto operator()(const Hnd &hnd) const noexcept { return tag_invoke(*this, hnd); }
		};
		struct to_native_path_t
		{
			template<typename Hnd> requires tag_invocable<to_native_path_t, const Hnd &, native_path_format, dev_t, ino_t>
			[[nodiscard]] auto operator()(const Hnd &hnd, native_path_format fmt, dev_t dev, ino_t ino) const noexcept -> tag_invoke_result_t<to_native_path_t, const Hnd &, native_path_format, dev_t, ino_t> { return tag_invoke(*this, hnd, fmt, dev, ino); }
			template<typename Hnd> requires(!tag_invocable<to_native_path_t, const Hnd &, native_path_format> && tag_invocable<to_native_path_t, const Hnd &, native_path_format, dev_t, ino_t>)
			[[nodiscard]] auto operator()(const Hnd &hnd, native_path_format fmt = native_path_format::any) const noexcept -> tag_invoke_result_t<to_native_path_t, const Hnd &, native_path_format, dev_t, ino_t>
			{
				stat st;
				if (auto res = get_stat(st, hnd, stat::query::dev | stat::query::ino); res.has_value()) [[likely]]
					return tag_invoke(*this, hnd, fmt, st.dev, st.ino);
				else
					return res.error();
			}

			template<typename Hnd> requires tag_invocable<to_native_path_t, const Hnd &, native_path_format>
			[[nodiscard]] path_result auto operator()(const Hnd &hnd, native_path_format fmt = native_path_format::any) const noexcept { return tag_invoke(*this, hnd, fmt); }
		};

		/** Basic system handle type. */
		class basic_handle
		{
		public:
			/** Structure representing an annotated native handle. */
			struct native_handle_type
			{
#if defined(ROD_WIN32)
				using value_type = void *;
#elif defined(ROD_POSIX)
				using value_type = int;
#endif

#if defined(ROD_WIN32)
				native_handle_type() noexcept : value(std::bit_cast<value_type>(std::intptr_t(-1))) {}
#elif defined(ROD_POSIX)
				native_handle_type() noexcept : value(-1) {}
#endif

				native_handle_type(const native_handle_type &) noexcept = default;
				native_handle_type &operator=(const native_handle_type &) noexcept = default;

				native_handle_type(native_handle_type &&other) noexcept { swap(other); }
				native_handle_type &operator=(native_handle_type &&other) noexcept { return (swap(other), *this); }

				constexpr native_handle_type(value_type value) : value(value) {}
				constexpr native_handle_type(value_type value, std::uint32_t flags) : value(value), flags(flags) {}

				[[nodiscard]] constexpr operator value_type &() & noexcept { return value; }
				[[nodiscard]] constexpr operator value_type &&() && noexcept { return std::move(value); }
				[[nodiscard]] constexpr operator const value_type &() const & noexcept { return value; }
				[[nodiscard]] constexpr operator const value_type &&() const && noexcept { return std::move(value); }

				[[nodiscard]] explicit operator bool() const noexcept { return value != native_handle_type().value; }

				constexpr void swap(native_handle_type &other) noexcept
				{
					std::swap(value, other.value);
					std::swap(flags, other.flags);
				}
				friend constexpr void swap(native_handle_type &a, native_handle_type &b) noexcept { a.swap(b); }

				[[nodiscard]] friend constexpr bool operator==(const native_handle_type &a, const native_handle_type &b) noexcept { return a.value == b.value; }
				[[nodiscard]] friend constexpr bool operator!=(const native_handle_type &a, const native_handle_type &b) noexcept { return a.value != b.value; }

				value_type value;
				std::uint32_t flags = 0;
			};

		public:
			basic_handle(const basic_handle &) = delete;
			basic_handle &operator=(const basic_handle &) = delete;

			basic_handle() noexcept = default;
			basic_handle(basic_handle &&other) noexcept : _hnd(std::move(other._hnd)) {}
			basic_handle &operator=(basic_handle &&other) noexcept { return (_hnd = std::move(other._hnd), *this); }

			explicit basic_handle(native_handle_type hnd) noexcept : _hnd(hnd) {}
			inline ~basic_handle() { if (is_open()) do_close(); }

			/** Checks if the handle is open. */
			[[nodiscard]] bool is_open() const noexcept { return static_cast<bool>(_hnd); }
			/** Returns the underlying native handle. */
			[[nodiscard]] native_handle_type native_handle() const noexcept { return _hnd; }

			/** Releases the underlying native handle. */
			native_handle_type release() noexcept { return std::exchange(_hnd, {}); }
			/** Releases the underlying native handle and replaces it with \a hnd. */
			native_handle_type release(native_handle_type hnd) noexcept { return std::exchange(_hnd, hnd); }

			constexpr void swap(basic_handle &other) noexcept { _hnd.swap(other._hnd); }
			friend constexpr void swap(basic_handle &a, basic_handle &b) noexcept { a.swap(b); }

		public:
			[[nodiscard]] friend constexpr bool operator==(const basic_handle &a, const basic_handle &b) noexcept { return a._hnd == b._hnd; }
			[[nodiscard]] friend constexpr bool operator!=(const basic_handle &a, const basic_handle &b) noexcept { return a._hnd != b._hnd; }

			template<typename Hnd>
			friend auto tag_invoke(close_t, Hnd &&hnd) noexcept { return hnd.do_close(); }
			template<typename Hnd>
			friend auto tag_invoke(clone_t, const Hnd &hnd) noexcept { return hnd.do_clone(); }

			template<typename Hnd>
			friend auto tag_invoke(get_stat_t, stat &st, const Hnd &hnd, stat::query q) noexcept { return hnd.do_get_stat(st, q); }
			template<typename Hnd>
			friend auto tag_invoke(set_stat_t, const stat &st, Hnd &&hnd, stat::query q) noexcept { return hnd.do_set_stat(st, q); }
			template<typename Hnd>
			friend auto tag_invoke(get_fs_stat_t, fs_stat &st, const Hnd &hnd, fs_stat::query q) noexcept { return hnd.do_get_fs_stat(st, q); }

			template<typename Hnd>
			friend auto tag_invoke(to_object_path_t, const Hnd &hnd) noexcept { return hnd.do_to_object_path(); }
			template<typename Hnd>
			friend auto tag_invoke(to_native_path_t, const Hnd &hnd, native_path_format fmt, dev_t dev, ino_t ino) noexcept { return hnd.do_to_native_path(fmt, dev, ino); }

		private:
			ROD_API_PUBLIC auto do_close() noexcept -> result<>;
			ROD_API_PUBLIC auto do_clone() const noexcept -> result<basic_handle>;

			ROD_API_PUBLIC result<stat::query> do_get_stat(stat &, stat::query) const noexcept;
			ROD_API_PUBLIC result<stat::query> do_set_stat(const stat &, stat::query) noexcept;
			ROD_API_PUBLIC result<fs_stat::query> do_get_fs_stat(fs_stat &, fs_stat::query) const noexcept;

			ROD_API_PUBLIC result<path> do_to_object_path() const noexcept;
			ROD_API_PUBLIC result<path> do_to_native_path(native_path_format, dev_t, ino_t) const noexcept;

			native_handle_type _hnd;
		};

		template<typename Child, typename Base>
		struct handle_adaptor { class type; };
		template<typename Child, typename Base>
		class handle_adaptor<Child, Base>::type
		{
			friend Child;

		public:
			using native_handle_type = typename Base::native_handle_type;

		private:
			template<typename T>
			static constexpr decltype(auto) get_adaptor(T &&value) noexcept { return static_cast<copy_cvref_t<std::add_rvalue_reference_t<T>, type>>(std::forward<T>(value)); }

		public:
			type(const type &) = delete;
			type &operator=(const type &) = delete;

			type() noexcept = default;
			type(type &&) noexcept = default;
			type &operator=(type &&) noexcept = default;

			explicit type(native_handle_type hnd) noexcept : _base(hnd) {}
			explicit type(Base &&other) noexcept : _base(std::forward<Base>(other)) {}

			/** Checks if the handle is open. */
			[[nodiscard]] bool is_open() const noexcept { return _base.is_open(); }
			/** Returns the underlying native handle. */
			[[nodiscard]] native_handle_type native_handle() const noexcept { return _base.native_handle(); }

			/** Releases the underlying native handle. */
			native_handle_type release() noexcept { return _base.release(); }
			/** Releases the underlying native handle and replaces it with \a hnd. */
			native_handle_type release(native_handle_type hnd) noexcept { return _base.release(hnd); }

			template<decays_to_same_or_derived<Child> Hnd = Child>
			constexpr void swap(Hnd &&other) noexcept { adl_swap(base(), get_adaptor(other).base()); }
			template<decays_to_same_or_derived<Child> Hnd0 = Child, decays_to_same_or_derived<Child> Hnd1 = Child>
			friend constexpr void swap(Hnd0 &&a, Hnd1 &&b) noexcept { get_adaptor(std::forward<Hnd0>(a)).swap(std::forward<Hnd1>(b)); }

		public:
			[[nodiscard]] friend constexpr bool operator==(const type &, const type &) noexcept = default;
			[[nodiscard]] friend constexpr bool operator!=(const type &, const type &) noexcept = default;

		private:
			static constexpr int do_close = 1;
			static constexpr int do_clone = 1;

			template<typename Hnd>
			static constexpr bool has_close() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_close)); }; }
			template<typename Hnd>
			static constexpr bool has_clone() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_clone)); }; }

			template<typename Hnd>
			using clone_result = typename decltype(clone_t{}(std::declval<const Base &>()))::template rebind_value<std::decay_t<Hnd>>;

			template<typename Hnd>
			constexpr static auto dispatch_close(Hnd &&hnd) noexcept -> decltype(std::forward<Hnd>(hnd).do_close()) { return hnd.do_close(); }
			template<typename Hnd>
			constexpr static auto dispatch_clone(const Hnd &hnd) noexcept -> decltype(hnd.do_clone()) { return hnd.do_clone(); }

			static constexpr int do_to_object_path = 1;
			static constexpr int do_to_native_path = 1;

			template<typename Hnd>
			static constexpr bool has_to_object_path() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_to_object_path)); }; }
			template<typename Hnd>
			static constexpr bool has_to_native_path() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_to_native_path)); }; }

			template<typename Hnd>
			constexpr static auto dispatch_to_object_path(const Hnd &hnd) noexcept -> decltype(hnd.do_to_object_path()) { return hnd.do_to_object_path(); }
			template<typename Hnd>
			constexpr static auto dispatch_to_native_path(const Hnd &hnd, native_path_format fmt) noexcept -> decltype(hnd.do_to_native_path(fmt)) { return hnd.do_to_native_path(fmt); }

		public:
			template<std::same_as<close_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(has_close<Hnd>())
			friend auto tag_invoke(T, Hnd &&hnd) noexcept { return dispatch_close(std::forward<Hnd>(hnd)); }
			template<std::same_as<close_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(!has_close<Hnd>())
			friend auto tag_invoke(T, Hnd &&hnd) noexcept { return T{}(get_adaptor(std::forward<Hnd>(hnd)).base()); }

			template<std::same_as<clone_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(has_clone<Hnd>())
			friend auto tag_invoke(T, Hnd &&hnd) noexcept { return dispatch_clone(hnd); }
			template<std::same_as<clone_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(!has_clone<Hnd>())
			friend auto tag_invoke(T, Hnd &&hnd) noexcept { return clone_result<Hnd>(T{}(get_adaptor(hnd).base())); }

			template<std::same_as<to_object_path_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(has_to_object_path<Hnd>())
			friend auto tag_invoke(T, const Hnd &hnd) noexcept { return dispatch_to_object_path(hnd); }
			template<std::same_as<to_object_path_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(!has_to_object_path<Hnd>())
			friend auto tag_invoke(T, const Hnd &hnd) noexcept { return T{}(get_adaptor(hnd).base()); }

			template<std::same_as<to_native_path_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(has_to_native_path<Hnd>())
			friend auto tag_invoke(T, const Hnd &hnd, native_path_format fmt) noexcept { return dispatch_to_native_path(hnd, fmt); }
			template<std::same_as<to_native_path_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(!has_to_native_path<Hnd>())
			friend auto tag_invoke(T, const Hnd &hnd, native_path_format fmt) noexcept { return T{}(get_adaptor(hnd).base(), fmt); }

		private:
			static constexpr int do_get_stat = 1;
			static constexpr int do_set_stat = 1;
			static constexpr int do_get_fs_stat = 1;

			template<typename Hnd>
			static constexpr bool has_get_stat() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_get_stat)); }; }
			template<typename Hnd>
			static constexpr bool has_set_stat() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_set_stat)); }; }
			template<typename Hnd>
			static constexpr bool has_get_fs_stat() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_get_fs_stat)); }; }

			template<typename Hnd>
			constexpr static auto dispatch_get_stat(stat &st, const Hnd &hnd, stat::query q) noexcept -> decltype(hnd.do_get_stat(st, hnd, q)) { return hnd.do_get_stat(st, hnd, q); }
			template<typename Hnd>
			constexpr static auto dispatch_set_stat(const stat &st, Hnd &hnd, stat::query q) noexcept -> decltype(hnd.do_set_stat(st, hnd, q)) { return hnd.do_set_stat(st, hnd, q); }
			template<typename Hnd>
			constexpr static auto dispatch_get_fs_stat(fs_stat &st, const Hnd &hnd, fs_stat::query q) noexcept -> decltype(hnd.do_get_fs_stat(st, hnd, q)) { return hnd.do_get_fs_stat(st, hnd, q); }

		public:
			template<std::same_as<get_stat_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(has_get_stat<Hnd>())
			friend auto tag_invoke(T, stat &st, const Hnd &hnd, stat::query q) noexcept { return dispatch_get_stat(st, hnd, q); }
			template<std::same_as<get_stat_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(!has_get_stat<Hnd>())
			friend auto tag_invoke(T, stat &st, const Hnd &hnd, stat::query q) noexcept { return T{}(st, get_adaptor(hnd).base(), q); }

			template<std::same_as<set_stat_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(has_set_stat<Hnd>())
			friend auto tag_invoke(T, const stat &st, Hnd &&hnd, stat::query q) noexcept { return dispatch_set_stat(st, std::forward<Hnd>(hnd), q); }
			template<std::same_as<set_stat_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(!has_set_stat<Hnd>())
			friend auto tag_invoke(T, const stat &st, Hnd &&hnd, stat::query q) noexcept { return T{}(st, get_adaptor(std::forward<Hnd>(hnd)).base(), q); }

			template<std::same_as<get_fs_stat_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(has_get_fs_stat<Hnd>())
			friend auto tag_invoke(T, fs_stat &st, const Hnd &hnd, fs_stat::query q) noexcept { return dispatch_get_fs_stat(st, hnd, q); }
			template<std::same_as<get_fs_stat_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(!has_get_fs_stat<Hnd>())
			friend auto tag_invoke(T, fs_stat &st, const Hnd &hnd, fs_stat::query q) noexcept { return T{}(st, get_adaptor(hnd).base(), q); }

		protected:
			[[nodiscard]] constexpr Base &base() & noexcept { return _base; }
			[[nodiscard]] constexpr Base &&base() && noexcept { return std::move(_base); }
			[[nodiscard]] constexpr const Base &base() const & noexcept { return _base; }
			[[nodiscard]] constexpr const Base &&base() const && noexcept { return std::move(_base); }

		private:
			Base _base;
		};
	}

	/** Concept used to define a handle type. */
	template<typename Hnd>
	concept handle = !std::copyable<Hnd> && std::movable<Hnd> && std::destructible<Hnd> && std::swappable<Hnd> && std::equality_comparable<Hnd> && requires(Hnd &mut_hnd, const Hnd &hnd)
	{
		typename Hnd::native_handle_type;

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

	/** Handle adaptor used to implement basic handle functionality. */
	template<typename Child, handle Base = _handle::basic_handle>
	using handle_adaptor = typename _handle::handle_adaptor<Child, Base>::type;

	using _handle::basic_handle;
	using _handle::open_mode;

	namespace _handle
	{
		struct dummy_handle : rod::handle_adaptor<dummy_handle, basic_handle>
		{
			using adp_base = rod::handle_adaptor<dummy_handle, basic_handle>;
			using adp_base::adp_base;
		};

		static_assert(handle<dummy_handle>, "Child of `hande_adaptor` must satisfy `handle`");
		static_assert(handle<basic_handle>, "`basic_handle` must satisfy `handle`");
	}

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

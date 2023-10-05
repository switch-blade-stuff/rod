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

		template<typename Hnd>
		struct handle_timeout_impl;
		template<typename Hnd> requires(requires { typename Hnd::timeout_type; })
		struct handle_timeout_impl<Hnd> { using type = typename Hnd::timeout_type; };

		template<typename T>
		struct select_extent_type { using type = extent_type; };
		template<typename T>
		struct select_size_type { using type = size_type; };

		template<typename Hnd>
		struct handle_extent_impl;
		template<typename Hnd> requires(requires { typename Hnd::extent_type; })
		struct handle_extent_impl<Hnd> { using type = typename Hnd::extent_type; };

		template<typename Hnd>
		struct handle_size_impl;
		template<typename Hnd> requires(requires { typename Hnd::size_type; })
		struct handle_size_impl<Hnd> { using type = typename Hnd::size_type; };
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

	namespace _close
	{
		template<typename Res>
		concept close_result = is_result_with_value_v<Res, void>;

		struct close_t
		{
			template<typename Hnd> requires tag_invocable<close_t, Hnd>
			close_result auto operator()(Hnd &&hnd) const noexcept { return tag_invoke(*this, std::forward<Hnd>(hnd)); }
		};
	}

	using _close::close_t;

	/** Customization point object used to close an open handle.
	 * @param hnd Handle to close.
	 * @return `void` result on success or a status code on failure.
	 * @errors Error codes are implementation-defined. Default implementation forwards the errors returned by `close` on POSIX or `CloseHandle` on Windows. */
	inline constexpr auto close = close_t{};

	namespace _clone
	{
		template<typename Res, typename Hnd>
		concept clone_result = instance_of<Res, result> && std::constructible_from<typename Res::template rebind_value<Hnd>, Res>;

		struct clone_t
		{
			template<typename Hnd> requires tag_invocable<clone_t, const Hnd &>
			[[nodiscard]] clone_result<Hnd> auto operator()(const Hnd &hnd) const noexcept { return tag_invoke(*this, hnd); }
		};
	}

	using _clone::clone_t;

	/** Customization point object used to clone a handle.
	 * @param hnd Handle to clone.
	 * @return Result containing the cloned handle on success or a status code on failure.
	 * @errors Error codes are implementation-defined. Default implementation forwards the errors returned by `fcntl(F_DUPFD_CLOEXEC)`, `fcntl(F_DUPFD)`, `fcntl(F_GETFL)`, and `fcntl(F_SETFL)` on POSIX or `DuplicateHandle` on Windows. */
	inline constexpr auto clone = clone_t{};

	namespace _sync
	{
		/** Flags used to control handle synchronization behavior. */
		enum class sync_mode : int
		{
			none = 0,
			/** Synchronize everything. */
			all = -1,
			/** Synchronize handle data only. */
			data = 1,
			/** Synchronize handle metadata only. */
			metadata = 2,
		};

		[[nodiscard]] constexpr sync_mode operator~(sync_mode h) noexcept { return sync_mode(~int(h)); }
		[[nodiscard]] constexpr sync_mode operator&(sync_mode a, sync_mode b) noexcept { return sync_mode(int(a) & int(b)); }
		[[nodiscard]] constexpr sync_mode operator|(sync_mode a, sync_mode b) noexcept { return sync_mode(int(a) | int(b)); }
		[[nodiscard]] constexpr sync_mode operator^(sync_mode a, sync_mode b) noexcept { return sync_mode(int(a) ^ int(b)); }
		constexpr sync_mode &operator&=(sync_mode &a, sync_mode b) noexcept { return a = a & b; }
		constexpr sync_mode &operator|=(sync_mode &a, sync_mode b) noexcept { return a = a | b; }
		constexpr sync_mode &operator^=(sync_mode &a, sync_mode b) noexcept { return a = a ^ b; }

		template<typename Res>
		concept sync_result = instance_of<Res, result> && std::same_as<typename Res::value_type, void>;

		struct sync_t
		{
			template<typename Hnd, typename To = handle_timeout_t<std::decay_t<Hnd>>> requires tag_invocable<sync_t, Hnd, sync_mode, const To &>
			sync_result auto operator()(Hnd &&hnd, sync_mode mode = sync_mode::all, const To &to = To()) const noexcept { return tag_invoke(*this, std::forward<Hnd>(hnd), mode, to); }
		};
	}

	using _sync::sync_mode;
	using _sync::sync_t;

	/* TODO: Document usage. */
	inline constexpr auto sync = sync_t{};

	namespace _to_path
	{
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
		concept path_result = instance_of<Res, result> && std::constructible_from<typename Res::template rebind_value<fs::path>, result<fs::path>>;

		struct to_object_path_t
		{
			template<typename Hnd> requires tag_invocable<to_object_path_t, const Hnd &>
			[[nodiscard]] path_result auto operator()(const Hnd &hnd) const noexcept { return tag_invoke(*this, hnd); }
		};
		struct to_native_path_t
		{
			template<typename Hnd> requires tag_invocable<to_native_path_t, const Hnd &, native_path_format, fs::dev_t, fs::ino_t>
			[[nodiscard]] auto operator()(const Hnd &hnd, native_path_format fmt, fs::dev_t dev, fs::ino_t ino) const noexcept -> tag_invoke_result_t<to_native_path_t, const Hnd &, native_path_format, fs::dev_t, fs::ino_t>
			{
				return tag_invoke(*this, hnd, fmt, dev, ino);
			}
			template<typename Hnd> requires(tag_invocable<to_native_path_t, const Hnd &, native_path_format, fs::dev_t, fs::ino_t> && !tag_invocable<to_native_path_t, const Hnd &, native_path_format>)
			[[nodiscard]] auto operator()(const Hnd &hnd, native_path_format fmt = native_path_format::any) const noexcept -> tag_invoke_result_t<to_native_path_t, const Hnd &, native_path_format, fs::dev_t, fs::ino_t>
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
	}

	using _to_path::native_path_format;
	using _to_path::to_object_path_t;
	using _to_path::to_native_path_t;

	/* TODO: Document usage */
	inline constexpr auto to_object_path = to_object_path_t{};
	/* TODO: Document usage */
	inline constexpr auto to_native_path = to_native_path_t{};

	namespace _extent
	{
		template<typename Res, typename Hnd>
		concept extent_result = instance_of<Res, result> && std::constructible_from<typename Res::template rebind_value<handle_extent_t<Hnd>>, Res>;

		struct endpos_t
		{
			template<typename Hnd, typename To = handle_timeout_t<std::decay_t<Hnd>>> requires tag_invocable<endpos_t, const Hnd &>
			[[nodiscard]] extent_result<Hnd> auto operator()(const Hnd &hnd) const noexcept { return tag_invoke(*this, hnd); }
			template<typename Hnd, typename To = handle_timeout_t<std::decay_t<Hnd>>> requires(!tag_invocable<endpos_t, const Hnd &>)
			[[nodiscard]] result<handle_extent_t<std::decay_t<Hnd>>> operator()(const Hnd &hnd) const noexcept
			{
				stat st;
				if (auto res = get_stat(st, hnd, stat::query::size); res.has_error()) [[unlikely]]
					return res.error();
				else if ((*res & stat::query::size) != stat::query::size) [[unlikely]]
					return std::make_error_code(std::errc::not_supported);
				else
					return handle_extent_t<Hnd>(st.size);
			}
		};
		struct truncate_t
		{
			template<typename Hnd, typename Ext = handle_extent_t<std::decay_t<Hnd>>> requires tag_invocable<truncate_t, Hnd, Ext>
			extent_result<std::decay_t<Hnd>> auto operator()(Hnd &&hnd, Ext new_endpos) const noexcept { return tag_invoke(*this, std::forward<Hnd>(hnd), new_endpos); }
		};
	}

	using _extent::endpos_t;
	using _extent::truncate_t;

	/* TODO: Document usage */
	inline constexpr auto endpos = endpos_t{};
	/* TODO: Document usage */
	inline constexpr auto truncate = truncate_t{};

	namespace _handle
	{
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

		/** Basic system handle type. */
		class basic_handle
		{
		public:
			/** Structure representing an annotated native handle. */
			struct native_handle_type
			{
				using flags_type = std::uint32_t;
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

				constexpr native_handle_type(value_type value) noexcept : value(value), flags() {}
				constexpr native_handle_type(value_type value, std::uint32_t flags) noexcept : value(value), flags(flags) {}
				template<typename T> requires std::is_enum_v<T> && std::convertible_to<std::underlying_type_t<T>, flags_type>
				constexpr native_handle_type(value_type value, T flags) noexcept : native_handle_type(value, flags_type(flags)) {}

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
				flags_type flags;
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
			friend auto tag_invoke(to_native_path_t, const Hnd &hnd, native_path_format fmt, fs::dev_t dev, fs::ino_t ino) noexcept { return hnd.do_to_native_path(fmt, dev, ino); }

		private:
			ROD_API_PUBLIC auto do_close() noexcept -> result<>;
			ROD_API_PUBLIC auto do_clone() const noexcept -> result<basic_handle>;

			ROD_API_PUBLIC result<stat::query> do_get_stat(stat &, stat::query) const noexcept;
			ROD_API_PUBLIC result<stat::query> do_set_stat(const stat &, stat::query) noexcept;
			ROD_API_PUBLIC result<fs_stat::query> do_get_fs_stat(fs_stat &, fs_stat::query) const noexcept;

			ROD_API_PUBLIC result<fs::path> do_to_object_path() const noexcept;
			ROD_API_PUBLIC result<fs::path> do_to_native_path(native_path_format, fs::dev_t, fs::ino_t) const noexcept;

			native_handle_type _hnd;
		};

		template<typename T>
		struct define_timeout_type {};
		template<typename T> requires(requires { typename T::timeout_type; })
		struct define_timeout_type<T>
		{
			/** Timeout type used for handle operations. */
			using timeout_type = typename T::timeout_type;
		};

		template<typename T> requires(requires { typename T::extent_type; })
		struct select_extent_type<T> { using type = typename T::extent_type; };
		template<typename T> requires(requires { typename T::size_type; })
		struct select_size_type<T> { using type = typename T::size_type; };

		template<typename Child, typename Base>
		struct handle_adaptor { class type; };
		template<typename Child, typename Base>
		class handle_adaptor<Child, Base>::type : public define_timeout_type<Base>
		{
			friend Child;

		public:
			using native_handle_type = typename Base::native_handle_type;
			/** Integer type used for handle offsets. */
			using extent_type = typename select_extent_type<Base>::type;
			/** Integer type used for handle buffers. */
			using size_type = typename select_size_type<Base>::type;

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
			[[nodiscard]] friend constexpr bool operator==(const type &a, const type &b) noexcept { return a._base == b._base; }
			[[nodiscard]] friend constexpr bool operator!=(const type &a, const type &b) noexcept { return a._base != b._base; }

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

		_detail::callable<close_t, Hnd &>;
		_detail::callable<clone_t, const Hnd &>;

		_detail::callable<to_object_path_t, const Hnd &>;
		_detail::callable<to_native_path_t, const Hnd &, native_path_format>;

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
}

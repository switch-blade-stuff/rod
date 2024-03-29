/*
 * Created by switchblade on 2023-08-15.
 */

#pragma once

#include "../timeout.hpp"
#include "../result.hpp"

namespace rod
{
	namespace _handle
	{
		using extent_type = std::uint64_t;
		using size_type = std::size_t;

		class basic_handle;

		template<typename Hnd>
		struct handle_timeout_impl;
		template<typename Hnd> requires(requires { typename Hnd::timeout_type; })
		struct handle_timeout_impl<Hnd> { using type = typename Hnd::timeout_type; };

		template<typename T>
		struct select_extent_type { using type = std::make_unsigned_t<extent_type>; };
		template<typename T>
		struct select_offset_type { using type = std::make_signed_t<extent_type>; };
		template<typename T>
		struct select_size_type { using type = size_type; };

		template<typename Hnd>
		struct handle_extent_impl;
		template<typename Hnd> requires(requires { typename Hnd::extent_type; })
		struct handle_extent_impl<Hnd> { using type = typename Hnd::extent_type; };

		template<typename Hnd>
		struct handle_offset_impl;
		template<typename Hnd> requires(requires { typename Hnd::offset_type; })
		struct handle_offset_impl<Hnd> { using type = typename Hnd::offset_type; };

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

	/** Type trait used to obtain the offset type of handle \a Hnd. */
	template<typename Hnd>
	struct handle_offset : _handle::handle_offset_impl<Hnd> {};
	/** Alias for `typename handle_offset&lt;Hnd&gt;::type` */
	template<typename Hnd>
	using handle_offset_t = typename handle_offset<Hnd>::type;

	/** Type trait used to obtain the size type of handle \a Hnd. */
	template<typename Hnd>
	struct handle_size : _handle::handle_size_impl<Hnd> {};
	/** Alias for `typename handle_size&lt;Hnd&gt;::type` */
	template<typename Hnd>
	using handle_size_t = typename handle_size<Hnd>::type;

	namespace _handle
	{
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
	}

	using _handle::close_t;
	using _handle::clone_t;

	/** Customization point object used to close an open handle.
	 * @param hnd Handle to close.
	 * @return `void` result on success or a status code on failure.
	 * @errors Error codes are implementation-defined. Default implementation forwards the errors returned by `close` on POSIX or `CloseHandle` on Windows. */
	inline constexpr auto close = close_t{};
	/** Customization point object used to clone a handle.
	 * @param hnd Handle to clone.
	 * @return Result containing the cloned handle on success or a status code on failure.
	 * @errors Error codes are implementation-defined. Default implementation forwards the errors returned by `fcntl(F_DUPFD_CLOEXEC)`, `fcntl(F_DUPFD)`, `fcntl(F_GETFL)`, and `fcntl(F_SETFL)` on POSIX or `DuplicateHandle` on Windows. */
	inline constexpr auto clone = clone_t{};

	namespace _handle
	{
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

				constexpr native_handle_type(const native_handle_type &) noexcept = default;
				constexpr native_handle_type &operator=(const native_handle_type &) noexcept = default;

				native_handle_type(native_handle_type &&other) noexcept : native_handle_type() { swap(other); }
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
			~basic_handle() { if (is_open()) do_close(); }

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

		private:
			ROD_API_PUBLIC auto do_close() noexcept -> result<>;
			ROD_API_PUBLIC auto do_clone() const noexcept -> result<basic_handle>;

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
		template<typename T> requires(requires { typename T::offset_type; })
		struct select_offset_type<T> { using type = typename T::offset_type; };
		template<typename T> requires(requires { typename T::size_type; })
		struct select_size_type<T> { using type = typename T::size_type; };

		template<typename Child, typename Base>
		struct handle_adaptor { class type; };
		template<typename Child, typename Base>
		class handle_adaptor<Child, Base>::type : public define_timeout_type<Base>, empty_base<Base>
		{
			friend Child;

		public:
			using native_handle_type = typename Base::native_handle_type;
			/** Integer type used for absolute handle offsets. */
			using extent_type = typename select_extent_type<Base>::type;
			/** Integer type used for relative handle offsets. */
			using offset_type = typename select_offset_type<Base>::type;
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

			template<typename... Args> requires std::constructible_from<Base, native_handle_type, Args...>
			explicit type(native_handle_type hnd, Args &&...args) noexcept : empty_base<Base>(hnd, std::forward<Args>(args)...) {}
			template<typename... Args> requires std::constructible_from<Base, Base, Args...>
			explicit type(Base &&hnd, Args &&...args) noexcept : empty_base<Base>(std::forward<Base>(hnd), std::forward<Args>(args)...) {}

			/** Checks if the handle is open. */
			[[nodiscard]] bool is_open() const noexcept { return base().is_open(); }
			/** Returns the underlying native handle. */
			[[nodiscard]] constexpr native_handle_type native_handle() const noexcept { return base().native_handle(); }

			/** Releases the underlying native handle. */
			native_handle_type release() noexcept { return base().release(); }
			/** Releases the underlying native handle and replaces it with \a hnd. */
			native_handle_type release(native_handle_type hnd) noexcept { return base().release(hnd); }

			constexpr void swap(type &other) noexcept { adl_swap(base(), get_adaptor(other).base()); }
			friend constexpr void swap(type &a, type &b) noexcept { a.swap(b); }

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

		public:
			template<std::same_as<close_t> T, decays_to_same<Child> Hnd> requires(has_close<Hnd>())
			friend auto tag_invoke(T, Hnd &&hnd) noexcept { return dispatch_close(std::forward<Hnd>(hnd)); }
			template<std::same_as<close_t> T, decays_to_same<Child> Hnd> requires(!has_close<Hnd>())
			friend auto tag_invoke(T, Hnd &&hnd) noexcept { return T{}(get_adaptor(std::forward<Hnd>(hnd)).base()); }

			template<std::same_as<clone_t> T, decays_to_same<Child> Hnd> requires(has_clone<Hnd>())
			friend auto tag_invoke(T, Hnd &&hnd) noexcept { return dispatch_clone(hnd); }
			template<std::same_as<clone_t> T, decays_to_same<Child> Hnd> requires(!has_clone<Hnd>())
			friend auto tag_invoke(T, Hnd &&hnd) noexcept { return clone_result<Hnd>(T{}(get_adaptor(hnd).base())); }

		protected:
			[[nodiscard]] constexpr Base &base() & noexcept { return empty_base<Base>::value(); }
			[[nodiscard]] constexpr Base &&base() && noexcept { return std::move(empty_base<Base>::value()); }
			[[nodiscard]] constexpr const Base &base() const & noexcept { return empty_base<Base>::value(); }
			[[nodiscard]] constexpr const Base &&base() const && noexcept { return std::move(empty_base<Base>::value()); }
		};
	}

	/** Concept used to define a handle type. */
	template<typename Hnd>
	concept handle = !std::copyable<Hnd> && std::movable<Hnd> && std::destructible<Hnd> && std::swappable<Hnd> && requires(Hnd &mut_hnd, const Hnd &hnd)
	{
		typename Hnd::native_handle_type;

		{ hnd.is_open() } -> std::convertible_to<bool>;
		{ hnd.native_handle() } -> std::convertible_to<typename Hnd::native_handle_type>;

		{ mut_hnd.release() } -> std::convertible_to<typename Hnd::native_handle_type>;
		{ mut_hnd.release(std::declval<typename Hnd::native_handle_type>()) } -> std::convertible_to<typename Hnd::native_handle_type>;
	} && _detail::callable<close_t, Hnd &> && _detail::callable<clone_t, const Hnd &>;

	/* _detail::callable<to_object_path_t, const Hnd &> &&
	 * _detail::callable<to_native_path_t, const Hnd &, native_path_format> &&
	 * _detail::callable<get_stat_t, stat &, const Hnd &, stat::query> &&
	 * _detail::callable<set_stat_t, const stat &, Hnd &, stat::query> */

	/** Handle adaptor used to implement basic handle functionality. */
	template<typename Child, handle Base = _handle::basic_handle>
	using handle_adaptor = typename _handle::handle_adaptor<Child, Base>::type;

	using _handle::basic_handle;

	namespace _handle
	{
		struct dummy_handle : rod::handle_adaptor<dummy_handle, basic_handle>
		{
			using adaptor = rod::handle_adaptor<dummy_handle, basic_handle>;
			using adaptor::adaptor;
		};

		static_assert(handle<dummy_handle>, "Child of `hande_adaptor` must satisfy `handle`");
		static_assert(handle<basic_handle>, "`basic_handle` must satisfy `handle`");
	}
}

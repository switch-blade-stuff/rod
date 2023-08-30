/*
 * Created by switch_blade on 2023-08-24.
 */

#pragma once

#include "handle_base.hpp"
#include "handle_stat.hpp"

namespace rod
{
	namespace _handle
	{
		[[nodiscard]] constexpr file_perm operator~(file_perm h) noexcept { return file_perm(~std::uint16_t(h)); }
		[[nodiscard]] constexpr file_perm operator&(file_perm a, file_perm b) noexcept { return file_perm(std::uint16_t(a) & std::uint16_t(b)); }
		[[nodiscard]] constexpr file_perm operator|(file_perm a, file_perm b) noexcept { return file_perm(std::uint16_t(a) | std::uint16_t(b)); }
		[[nodiscard]] constexpr file_perm operator^(file_perm a, file_perm b) noexcept { return file_perm(std::uint16_t(a) ^ std::uint16_t(b)); }
		[[nodiscard]] constexpr file_perm &operator&=(file_perm &a, file_perm b) noexcept { return a = a & b; }
		[[nodiscard]] constexpr file_perm &operator|=(file_perm &a, file_perm b) noexcept { return a = a | b; }
		[[nodiscard]] constexpr file_perm &operator^=(file_perm &a, file_perm b) noexcept { return a = a ^ b; }

		/** Flags used to control behavior of filesystem handles. */
		enum class file_flags : std::uint16_t
		{
			none = 0,

			/** Allow reading of handle contents. */
			data_read = 0x1,
			/** Allow reading of handle attributes. */
			attr_read = 0x2,
			/** Allow reading of handle contents & attributes. */
			read = data_read | attr_read,
			/** Allow writing of handle contents. */
			data_write = 0x4,
			/** Allow writing of handle attributes. */
			attr_write = 0x8,
			/** Allow appending of handle contents. */
			append = 0x10,
			/** Allow writing and appending of handle data & attributes. */
			write = data_write | attr_write | append,

			/** Enables case sensitivity on platforms where it is optional (ex. Windows). */
			case_sensitive = 0x20,
			/** File referenced by the handle will be unlinked once the handle is closed. */
			unlink_on_close = 0x40,
			/** Disable creation of native sparse files, and instead emulate extents via padding. */
			no_sparse_files = 0x80,

			/** File will be opened in non-blocking mode (`O_NONBLOCK` or `OVERLAPPED`).
			 * @note This flag is implied for handles opened using an IO scheduler. */
			non_blocking = 0x100,
		};

		[[nodiscard]] constexpr file_flags operator~(file_flags h) noexcept { return file_flags(~std::uint16_t(h)); }
		[[nodiscard]] constexpr file_flags operator&(file_flags a, file_flags b) noexcept { return file_flags(std::uint16_t(a) & std::uint16_t(b)); }
		[[nodiscard]] constexpr file_flags operator|(file_flags a, file_flags b) noexcept { return file_flags(std::uint16_t(a) | std::uint16_t(b)); }
		[[nodiscard]] constexpr file_flags operator^(file_flags a, file_flags b) noexcept { return file_flags(std::uint16_t(a) ^ std::uint16_t(b)); }
		[[nodiscard]] constexpr file_flags &operator&=(file_flags &a, file_flags b) noexcept { return a = a & b; }
		[[nodiscard]] constexpr file_flags &operator|=(file_flags &a, file_flags b) noexcept { return a = a | b; }
		[[nodiscard]] constexpr file_flags &operator^=(file_flags &a, file_flags b) noexcept { return a = a ^ b; }

		/** Flags used to control kernel caching behavior of filesystem handles. */
		enum class file_caching : std::uint8_t
		{
			/** Do not preform any caching. IO must be aligned to system-specific boundary (ex. 4Kb). */
			none = 0,
			/** Enable caching of metadata. IO must be aligned to system-specific boundary (ex. 4Kb). */
			meta = 0x1,
			/** Enable caching of data reads. */
			read = 0x2,
			/** Enable caching of data writes. */
			write = 0x4,
			/** Enable all caching without any additional synchronization (default OS behavior). */
			all = meta | read | write,

			/** Synchronize cached data and metadata only once the last handle to the file has been closed. */
			temporary = 0x8,
			/** Hint to the OS to avoid precaching of data. Mutually exclusive with `avoid_precache`. */
			avoid_precache = 0x10,
			/** Hint to the OS to prefer precaching of data. Mutually exclusive with `force_precache`. */
			force_precache = 0x20,
			/** Enable additional synchronization for some IO operations to prevent data races in certain caching modes. */
			sanity_buffers = 0x40,
		};

		[[nodiscard]] constexpr file_caching operator~(file_caching h) noexcept { return file_caching(~std::uint8_t(h)); }
		[[nodiscard]] constexpr file_caching operator&(file_caching a, file_caching b) noexcept { return file_caching(std::uint8_t(a) & std::uint8_t(b)); }
		[[nodiscard]] constexpr file_caching operator|(file_caching a, file_caching b) noexcept { return file_caching(std::uint8_t(a) | std::uint8_t(b)); }
		[[nodiscard]] constexpr file_caching operator^(file_caching a, file_caching b) noexcept { return file_caching(std::uint8_t(a) ^ std::uint8_t(b)); }
		[[nodiscard]] constexpr file_caching &operator&=(file_caching &a, file_caching b) noexcept { return a = a & b; }
		[[nodiscard]] constexpr file_caching &operator|=(file_caching &a, file_caching b) noexcept { return a = a | b; }
		[[nodiscard]] constexpr file_caching &operator^=(file_caching &a, file_caching b) noexcept { return a = a ^ b; }

		template<typename Res>
		concept link_result = is_result_with_value_v<Res, void>;
		template<typename Res, typename Hnd>
		concept reopen_result = is_result_v<Res> && std::constructible_from<typename Res::template rebind_value<Hnd>, Res>;

		struct link_t
		{
			template<typename Hnd> requires tag_invocable<link_t, Hnd>
			link_result auto operator()(Hnd &&hnd) const noexcept { return tag_invoke(*this, std::forward<Hnd>(hnd)); }
		};
		struct relink_t
		{
			template<typename Hnd> requires tag_invocable<relink_t, Hnd>
			link_result auto operator()(Hnd &&hnd) const noexcept { return tag_invoke(*this, std::forward<Hnd>(hnd)); }
		};
		struct unlink_t
		{
			template<typename Hnd> requires tag_invocable<unlink_t, Hnd>
			link_result auto operator()(Hnd &&hnd) const noexcept { return tag_invoke(*this, std::forward<Hnd>(hnd)); }
		};

		/* TODO: Implement */
		/* Default implementations for basic_handle used by fs_handle_adaptor. */
		ROD_API_PUBLIC result<> do_link(basic_handle & /*, const path_handle &, path_view */) noexcept;
		ROD_API_PUBLIC result<> do_relink(basic_handle & /*, const path_handle &, path_view */) noexcept;
		ROD_API_PUBLIC result<> do_unlink(basic_handle &) noexcept;

		struct reopen_t
		{
			template<typename Hnd> requires tag_invocable<reopen_t, const Hnd &>
			reopen_result<Hnd> auto operator()(const Hnd &hnd) const noexcept { return tag_invoke(*this, hnd); }
		};

		/* TODO: Implement */
		ROD_API_PUBLIC result<basic_handle> do_reopen(const basic_handle &) noexcept;

		template<typename Child, typename Base>
		struct fs_handle_adaptor { class type; };
		template<typename Child, typename Base>
		class fs_handle_adaptor<Child, Base>::type : public handle_adaptor<Child, Base>::type
		{
			friend class handle_adaptor<Child, Base>::type;

			using adaptor_base = typename handle_adaptor<Child, Base>::type;

		public:
			using native_handle_type = typename adaptor_base::native_handle_type;

			/** Integer type used for handle offsets. */
			using extent_type = _handle::extent_type;
			/** Integer type used for handle buffers. */
			using size_type = _handle::size_type;

		private:
			template<typename T>
			static decltype(auto) get_adaptor(T &&value) noexcept { return ((copy_cvref_t<std::add_rvalue_reference_t<T>, type>) std::forward<T>(value)); }

		public:
			type(const type &) = delete;
			type &operator=(const type &) = delete;

			type() noexcept = default;
			type(type &&other) noexcept : adaptor_base(std::forward<Base>(other)), _dev(std::exchange(other._dev, 0)), _ino(std::exchange(other._ino, 0)) {}
			type &operator=(type &&other) noexcept { return (adaptor_base::operator=(std::forward<Base>(other)), std::swap(_dev, other._dev), std::swap(_ino, other._ino), *this); }

			explicit type(native_handle_type hnd) noexcept : adaptor_base(hnd) {}
			explicit type(native_handle_type hnd, dev_t dev, ino_t ino) noexcept : adaptor_base(hnd), _dev(dev), _ino(ino) {}

			explicit type(Base &&other) noexcept : adaptor_base(std::forward<Base>(other)) {}
			explicit type(Base &&other, dev_t dev, ino_t ino) noexcept : adaptor_base(std::forward<Base>(other)), _dev(dev), _ino(ino) {}

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
			static constexpr int link = 1;
			static constexpr int relink = 1;
			static constexpr int unlink = 1;

			template<typename Hnd>
			static constexpr bool has_link() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::link)); }; }
			template<typename Hnd>
			static constexpr bool has_relink() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::relink)); }; }
			template<typename Hnd>
			static constexpr bool has_unlink() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::unlink)); }; }

			template<typename Hnd>
			constexpr static auto dispatch_link(Hnd &&hnd) noexcept -> decltype(std::forward<Hnd>(hnd).link()) { return std::forward<Hnd>(hnd).link(); }
			template<typename Hnd>
			constexpr static auto dispatch_relink(Hnd &&hnd) noexcept -> decltype(std::forward<Hnd>(hnd).relink()) { return std::forward<Hnd>(hnd).relink(); }
			template<typename Hnd>
			constexpr static auto dispatch_unlink(Hnd &&hnd) noexcept -> decltype(std::forward<Hnd>(hnd).unlink()) { return std::forward<Hnd>(hnd).unlink(); }

			static constexpr int reopen = 1;

			template<typename Hnd>
			static constexpr bool has_reopen() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::reopen)); }; }
			template<typename Hnd>
			constexpr static auto dispatch_reopen(Hnd &&hnd) noexcept -> decltype(std::forward<Hnd>(hnd).reopen()) { return std::forward<Hnd>(hnd).reopen(); }

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
			template<std::same_as<link_t> T> requires(has_link<Child>())
			friend auto tag_invoke(T, Child &&hnd) noexcept { return dispatch_link(std::forward<Child>(hnd)); }
			template<std::same_as<link_t> T, typename Hnd = Child> requires(!has_link<Hnd>() && _detail::callable<link_t, copy_cvref_t<Hnd, Base>>)
			friend auto tag_invoke(T, Hnd &&hnd) noexcept { return link_t{}(get_adaptor(std::forward<Hnd>(hnd)).base()); }
			template<std::same_as<link_t> T, typename Hnd = Child> requires(!has_link<Hnd>() && !_detail::callable<link_t, copy_cvref_t<Hnd, Base>>)
			friend auto tag_invoke(T, Hnd &&hnd) noexcept { return do_link(get_adaptor(std::forward<Hnd>(hnd)).base()); }

			template<std::same_as<relink_t> T> requires(has_relink<Child>())
			friend auto tag_invoke(T, Child &&hnd) noexcept { return dispatch_relink(std::forward<Child>(hnd)); }
			template<std::same_as<relink_t> T, typename Hnd = Child> requires(!has_relink<Hnd>() && _detail::callable<relink_t, copy_cvref_t<Hnd, Base>>)
			friend auto tag_invoke(T, Hnd &&hnd) noexcept { return relink_t{}(get_adaptor(std::forward<Hnd>(hnd)).base()); }
			template<std::same_as<relink_t> T, typename Hnd = Child> requires(!has_relink<Hnd>() && !_detail::callable<relink_t, copy_cvref_t<Hnd, Base>>)
			friend auto tag_invoke(T, Hnd &&hnd) noexcept { return do_relink(get_adaptor(std::forward<Hnd>(hnd)).base()); }

			template<std::same_as<unlink_t> T> requires(has_unlink<Child>())
			friend auto tag_invoke(T, Child &&hnd) noexcept { return dispatch_unlink(std::forward<Child>(hnd)); }
			template<std::same_as<unlink_t> T, typename Hnd = Child> requires(!has_unlink<Hnd>() && _detail::callable<unlink_t, copy_cvref_t<Hnd, Base>>)
			friend auto tag_invoke(T, Hnd &&hnd) noexcept { return unlink_t{}(get_adaptor(std::forward<Hnd>(hnd)).base()); }
			template<std::same_as<unlink_t> T, typename Hnd = Child> requires(!has_unlink<Hnd>() && !_detail::callable<unlink_t, copy_cvref_t<Hnd, Base>>)
			friend auto tag_invoke(T, Hnd &&hnd) noexcept { return do_unlink(get_adaptor(std::forward<Hnd>(hnd)).base()); }

			template<std::same_as<reopen_t> T> requires(has_reopen<Child>())
			friend auto tag_invoke(T, const Child &hnd) noexcept { return dispatch_reopen(hnd); }
			template<std::same_as<reopen_t> T, typename Hnd = Child> requires(!has_reopen<Hnd>() && _detail::callable<reopen_t, const Base &>)
			friend auto tag_invoke(T, const Hnd &hnd) noexcept { return reopen_t{}(get_adaptor(hnd).base()); }
			template<std::same_as<reopen_t> T, typename Hnd = Child> requires(!has_reopen<Hnd>() && !_detail::callable<reopen_t, const Base &>)
			friend auto tag_invoke(T, const Hnd &hnd) noexcept { return do_reopen(get_adaptor(hnd).base()); }

		private:
			template<typename Hnd, typename F>
			[[nodiscard]] auto cache_stat(stat &st, const Hnd &hnd, stat::query q, F &&get) const noexcept
			{
				stat::query m = {};
				if (bool(q & stat::query::dev) && _dev)
				{
					m |= stat::query::dev;
					st.dev = _dev;
				}
				if (bool(q & stat::query::ino) && _ino)
				{
					m |= stat::query::ino;
					st.ino = _ino;
				}
				auto res = get(st, hnd, q ^ m);
				if (res.has_value()) [[likely]]
				{
					if ((*res & stat::query::dev) > (m & stat::query::dev))
						_dev = st.dev;
					if ((*res & stat::query::ino) > (m & stat::query::ino))
						_ino = st.ino;
					*res |= m;
				}
				return res;
			}

		public:
			template<std::same_as<get_stat_t> T> requires(has_get_stat<Child>())
			friend auto tag_invoke(T, stat &st, const Child &hnd, stat::query q) noexcept { return get_adaptor(hnd).cache_stat(st, hnd, q, dispatch_get_stat<Child>); }
			template<std::same_as<get_stat_t> T, typename Hnd = Child> requires(!has_get_stat<Hnd>())
			friend auto tag_invoke(T, stat &st, const Hnd &hnd, stat::query q) noexcept { return get_adaptor(hnd).cache_stat(st, get_adaptor(hnd).base(), q, get_stat_t{}); }

			template<std::same_as<set_stat_t> T> requires(has_set_stat<Child>())
			friend auto tag_invoke(T, const stat &st, Child &&hnd, stat::query q) noexcept { return dispatch_set_stat(st, std::forward<Child>(hnd), q); }
			template<std::same_as<set_stat_t> T, typename Hnd = Child> requires(!has_set_stat<Hnd>())
			friend auto tag_invoke(T, const stat &st, Hnd &&hnd, stat::query q) noexcept { return set_stat_t{}(st, get_adaptor(std::forward<Hnd>(hnd)).base(), q); }

		private:
			mutable dev_t _dev = 0;
			mutable ino_t _ino = 0;
		};
	}

	/** Concept used to define a filesystem handle type. */
	template<typename Hnd>
	concept fs_handle = handle<Hnd> && requires (const Hnd &hnd, Hnd &mut_hnd)
	{
		typename Hnd::extent_type;
		typename Hnd::size_type;

		_detail::callable<_handle::link_t, Hnd &>;
		_detail::callable<_handle::relink_t, Hnd &>;
		_detail::callable<_handle::unlink_t, Hnd &>;
		_detail::callable<_handle::reopen_t, const Hnd &>;
	};

	namespace _handle
	{
		struct dummy_fs_handle : fs_handle_adaptor<dummy_fs_handle, basic_handle>::type
		{
			using adaptor_base = fs_handle_adaptor<dummy_fs_handle, basic_handle>::type;
			using adaptor_base::adaptor_base;
		};

		static_assert(fs_handle<dummy_fs_handle>, "Child of `fs_hande_adaptor` must satisfy `fs_handle`");
	}

	/** Handle adaptor used to implement filesystem object handles. */
	template<typename Child, handle Base = basic_handle>
	using fs_handle_adaptor = typename _handle::fs_handle_adaptor<Child, Base>::type;

	using _handle::file_caching;
	using _handle::file_flags;

	using _handle::link_t;
	using _handle::relink_t;
	using _handle::unlink_t;
	using _handle::reopen_t;

	/** Returns current path of the object referenced by the handle.
	 * @note Object referenced by the handle may not have a valid path, in which case an empty path is returned instead.
	 * @note Returned path may not be a valid filesystem path and may instead represent some internal object ID. */

	/** Returns current path of the object referenced by the handle using native path format \a fmt.
	 * @note Certain path formats (such as volume & object ID) may not be supported by either the underlying filesystem or the OS, in which case `std::errc::not_supported` is returned. */
}

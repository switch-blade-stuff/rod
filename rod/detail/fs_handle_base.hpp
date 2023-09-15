/*
 * Created by switch_blade on 2023-08-24.
 */

#pragma once

#include "handle_base.hpp"
#include "path_handle.hpp"

namespace rod
{
	namespace fs
	{
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

			/** Disable creation of native sparse files, and instead emulate extents via padding. */
			no_sparse_files = 0x20,
			/** File referenced by the handle will be unlinked once the last handle to it is closed. */
			unlink_on_close = 0x40,

			/** File will be opened in non-blocking mode (`O_NONBLOCK` or `OVERLAPPED`).
			 * @note This flag is implied for handles opened using an IO scheduler. */
			non_blocking = 0x80,
			/** Makes NTFS directories to preform case-sensitive path lookup as opposed to system default.
			 * @note This flag will affect other code accessing the directory. */
			case_sensitive = 0x100,
		};

		[[nodiscard]] constexpr file_flags operator~(file_flags h) noexcept { return file_flags(~std::uint16_t(h)); }
		[[nodiscard]] constexpr file_flags operator&(file_flags a, file_flags b) noexcept { return file_flags(std::uint16_t(a) & std::uint16_t(b)); }
		[[nodiscard]] constexpr file_flags operator|(file_flags a, file_flags b) noexcept { return file_flags(std::uint16_t(a) | std::uint16_t(b)); }
		[[nodiscard]] constexpr file_flags operator^(file_flags a, file_flags b) noexcept { return file_flags(std::uint16_t(a) ^ std::uint16_t(b)); }
		constexpr file_flags &operator&=(file_flags &a, file_flags b) noexcept { return a = a & b; }
		constexpr file_flags &operator|=(file_flags &a, file_flags b) noexcept { return a = a | b; }
		constexpr file_flags &operator^=(file_flags &a, file_flags b) noexcept { return a = a ^ b; }

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
		constexpr file_caching &operator&=(file_caching &a, file_caching b) noexcept { return a = a & b; }
		constexpr file_caching &operator|=(file_caching &a, file_caching b) noexcept { return a = a | b; }
		constexpr file_caching &operator^=(file_caching &a, file_caching b) noexcept { return a = a ^ b; }
	}

	namespace _handle
	{
		[[nodiscard]] inline static typename fs::path::string_type generate_unique_name()
		{
			constexpr typename fs::path::value_type alphabet[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

			auto str = typename fs::path::string_type(64, '\0');
			auto gen = system_random(str.data(), str.size() * sizeof(typename fs::path::value_type));
			for (std::size_t i = 0; i < str.size(); ++i)
			{
				/* std::rand fallback is fine since it's not used it for cryptography. */
				if (i >= gen) [[unlikely]]
					str[i] = alphabet[std::rand() % 16];
				else
					str[i] = alphabet[str[i] % 16];
			}
			return str;
		}

		template<typename Res>
		concept link_result = instance_of<Res, result> && std::constructible_from<typename Res::template rebind_value<void>, Res>;

		class link_t
		{
			template<typename Hnd>
			using timeout_t = handle_timeout_t<std::decay_t<Hnd>>;

		public:
			template<typename Hnd, std::convertible_to<const fs::path_handle &> Base = const path_handle &, fs::path_like Path = fs::path_view, decays_to_same<timeout_t<Hnd>> To = timeout_t<Hnd>> requires tag_invocable<link_t, Hnd, Base, Path, bool, To>
			link_result auto operator()(Hnd &&hnd, Base &&base, Path &&path, bool replace = false, To &&to = To()) const noexcept { return tag_invoke(*this, std::forward<Hnd>(hnd), base, path, replace, std::forward<To>(to)); }
		};
		class relink_t
		{
			template<typename Hnd>
			using timeout_t = handle_timeout_t<std::decay_t<Hnd>>;

		public:
			template<typename Hnd, std::convertible_to<const fs::path_handle &> Base = const path_handle &, fs::path_like Path = fs::path_view, decays_to_same<timeout_t<Hnd>> To = timeout_t<Hnd>> requires tag_invocable<relink_t, Hnd, Base, Path, bool, To>
			link_result auto operator()(Hnd &&hnd, Base &&base, Path path, bool replace = false, To &&to = To()) const noexcept { return tag_invoke(*this, std::forward<Hnd>(hnd), base, path, replace, std::forward<To>(to)); }
		};
		class unlink_t
		{
			template<typename Hnd>
			using timeout_t = handle_timeout_t<std::decay_t<Hnd>>;

		public:
			template<typename Hnd, decays_to_same<timeout_t<Hnd>> To = timeout_t<Hnd>> requires tag_invocable<unlink_t, Hnd, To>
			link_result auto operator()(Hnd &&hnd, To &&to = To()) const noexcept { return tag_invoke(*this, std::forward<Hnd>(hnd), std::forward<To>(to)); }
		};

		template<typename T>
		struct select_extent_type { using type = extent_type; };
		template<typename T>
		struct select_size_type { using type = size_type; };

		template<typename T> requires(requires { typename T::extent_type; })
		struct select_extent_type<T> { using type = typename T::extent_type; };
		template<typename T> requires(requires { typename T::size_type; })
		struct select_size_type<T> { using type = typename T::size_type; };

		template<typename T>
		struct fs_handle_timeout { using type = fs::file_timeout; };
		template<typename T> requires(requires { typename T::timeout_type; })
		struct fs_handle_timeout<T> { using type = typename T::timeout_type; };

		template<typename Child, typename Base>
		struct fs_handle_adaptor { class type; };
		template<typename Child, typename Base>
		class fs_handle_adaptor<Child, Base>::type : public handle_adaptor<Child, Base>::type
		{
			using adp_base = typename handle_adaptor<Child, Base>::type;

			friend adp_base;
			friend Child;

		public:
			/** Timeout type used for handle operations. */
			using timeout_type = typename fs_handle_timeout<adp_base>::type;
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
			type(type &&other) noexcept : adp_base(std::forward<adp_base>(other)), _dev(std::exchange(other._dev, 0)), _ino(std::exchange(other._ino, 0)) {}
			type &operator=(type &&other) noexcept { return (adp_base::operator=(std::forward<adp_base>(other)), std::swap(_dev, other._dev), std::swap(_ino, other._ino), *this); }

			explicit type(typename adp_base::native_handle_type hnd) noexcept : adp_base(hnd) {}
			explicit type(typename adp_base::native_handle_type hnd, dev_t dev, ino_t ino) noexcept : adp_base(hnd), _dev(dev), _ino(ino) {}

			explicit type(Base &&other) noexcept : adp_base(std::forward<Base>(other)) {}
			explicit type(Base &&other, dev_t dev, ino_t ino) noexcept : adp_base(std::forward<Base>(other)), _dev(dev), _ino(ino) {}

			using adp_base::release;
			using adp_base::is_open;
			using adp_base::native_handle;

			constexpr void swap(type &other) noexcept
			{
				adl_swap(adp_base::base(), other.base());
				std::swap(_dev, other._dev);
				std::swap(_ino, other._ino);
			}
			friend constexpr void swap(type &a, type &b) noexcept { a.swap(b); }

		private:
			static constexpr int do_link = 1;
			static constexpr int do_relink = 1;
			static constexpr int do_unlink = 1;

			template<typename Hnd>
			static constexpr bool has_link() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_link)); }; }
			template<typename Hnd>
			static constexpr bool has_relink() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_relink)); }; }
			template<typename Hnd>
			static constexpr bool has_unlink() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_unlink)); }; }

			template<typename Hnd>
			constexpr static auto dispatch_link(Hnd &&hnd, const fs::path_handle &base, fs::path_view path, bool replace, const timeout_type &to) noexcept -> decltype(std::forward<Hnd>(hnd).do_link(base, path, replace, to))
			{
				return std::forward<Hnd>(hnd).do_link(base, path, replace, to);
			}
			template<typename Hnd>
			constexpr static auto dispatch_relink(Hnd &&hnd, const fs::path_handle &base, fs::path_view path, bool replace, const timeout_type &to) noexcept -> decltype(std::forward<Hnd>(hnd).do_relink(base, path, replace, to))
			{
				return std::forward<Hnd>(hnd).do_relink(base, path, replace, to);
			}
			template<typename Hnd>
			constexpr static auto dispatch_unlink(Hnd &&hnd, const timeout_type &to) noexcept -> decltype(std::forward<Hnd>(hnd).do_unlink(to))
			{
				return std::forward<Hnd>(hnd).do_unlink(to);
			}

		public:
			template<std::same_as<link_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(has_link<Hnd>())
			friend auto tag_invoke(T, Hnd &&hnd, const fs::path_handle &base, fs::path_view path, bool replace, const timeout_type &to) noexcept { return dispatch_link(std::forward<Hnd>(hnd), base, path, replace, to); }
			template<std::same_as<link_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(!has_link<Hnd>())
			friend auto tag_invoke(T, Hnd &&hnd, const fs::path_handle &base, fs::path_view path, bool replace, const timeout_type &to) noexcept { return T{}(get_adaptor(std::forward<Hnd>(hnd)).base(), base, path, replace, to); }

			template<std::same_as<relink_t> T, decays_to_same_or_derived<Child> Hnd> requires(has_relink<Hnd>())
			friend auto tag_invoke(T, Hnd &&hnd, const fs::path_handle &base, fs::path_view path, bool replace, const timeout_type &to) noexcept { return dispatch_relink(std::forward<Hnd>(hnd), base, path, replace, to); }
			template<std::same_as<relink_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(!has_relink<Hnd>())
			friend auto tag_invoke(T, Hnd &&hnd, const fs::path_handle &base, fs::path_view path, bool replace, const timeout_type &to) noexcept { return T{}(get_adaptor(std::forward<Hnd>(hnd)).base(), base, path, replace, to); }

			template<std::same_as<unlink_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(has_unlink<Hnd>())
			friend auto tag_invoke(T, Hnd &&hnd, const timeout_type &to) noexcept { return dispatch_unlink(std::forward<Hnd>(hnd), to); }
			template<std::same_as<unlink_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(!has_unlink<Hnd>())
			friend auto tag_invoke(T, Hnd &&hnd, const timeout_type &to) noexcept { return T{}(get_adaptor(std::forward<Hnd>(hnd)).base(), to); }

		private:
			template<typename Res, typename Ret = typename std::decay_t<Res>::template rebind_value<Child>>
			constexpr static auto convert_handle_result(Res &&res) noexcept(std::is_nothrow_constructible_v<Ret, Res>) { return Ret(std::forward<Res>(res)); }

			static constexpr int do_get_stat = 1;

			template<typename Hnd>
			static constexpr bool has_get_stat() noexcept { return !requires { requires bool(int(std::decay_t<Hnd>::do_get_stat)); }; }
			template<typename Hnd>
			constexpr static auto dispatch_get_stat(stat &st, const Hnd &hnd, stat::query q) noexcept -> decltype(hnd.get_stat(st, hnd, q)) { return hnd.do_get_stat(st, hnd, q); }

		public:
			template<std::same_as<get_stat_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(has_get_stat<Hnd>())
			friend auto tag_invoke(T, stat &st, Hnd &&hnd, stat::query q) noexcept { return get_adaptor(std::forward<Hnd>(hnd)).cache_stat(st, std::forward<Hnd>(hnd), q, dispatch_get_stat<Hnd>); }
			template<std::same_as<get_stat_t> T, decays_to_same_or_derived<Child> Hnd = Child> requires(!has_get_stat<Hnd>())
			friend auto tag_invoke(T, stat &st, Hnd &&hnd, stat::query q) noexcept { return get_adaptor(std::forward<Hnd>(hnd)).cache_stat(st, get_adaptor(std::forward<Hnd>(hnd)).base(), q, get_stat_t{}); }

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

		private:
			mutable dev_t _dev = 0;
			mutable ino_t _ino = 0;
		};
	}

	namespace fs
	{
		/** Concept used to define a filesystem handle type. */
		template<typename Hnd>
		concept fs_handle = handle<Hnd> && requires(const Hnd &hnd, Hnd &mut_hnd)
		{
			typename Hnd::timeout_type;
			typename Hnd::extent_type;
			typename Hnd::size_type;

			_detail::callable<_handle::link_t, Hnd &, const path_handle &, path_view, bool, const typename Hnd::timeout_type &>;
			_detail::callable<_handle::relink_t, Hnd &, const path_handle &, path_view, bool, const typename Hnd::timeout_type &>;
			_detail::callable<_handle::unlink_t, Hnd &, const typename Hnd::timeout_type &>;
		};

		/** Handle adaptor used to implement filesystem object handles. */
		template<typename Child, handle Base = basic_handle>
		using fs_handle_adaptor = typename _handle::fs_handle_adaptor<Child, Base>::type;
	}

	namespace _handle
	{
		struct dummy_fs_handle : fs::fs_handle_adaptor<dummy_fs_handle, basic_handle>
		{
			using adp_base = fs::fs_handle_adaptor<dummy_fs_handle, basic_handle>;
			using adp_base::adp_base;

			result<> do_link(const fs::path_handle &, fs::path_view, bool, const typename adp_base::timeout_type &) noexcept { return {}; };
			result<> do_relink(const fs::path_handle &, fs::path_view, bool, const typename adp_base::timeout_type &) noexcept { return {}; };
			result<> do_unlink(const typename adp_base::timeout_type &) noexcept { return {}; };
		};

		static_assert(fs::fs_handle<dummy_fs_handle>, "Child of `fs_hande_adaptor` must satisfy `fs_handle`");
	}

	namespace fs
	{
		using _handle::link_t;
		using _handle::relink_t;
		using _handle::unlink_t;

		/* TODO: Document usage */
		inline constexpr auto link = link_t{};
		/* TODO: Document usage */
		inline constexpr auto relink = relink_t{};
		/* TODO: Document usage */
		inline constexpr auto unlink = unlink_t{};
	}
}

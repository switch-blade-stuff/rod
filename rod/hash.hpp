/*
 * Created by switch_blade on 2023-08-05.
 */

#pragma once

#include "utility.hpp"
#include "tag.hpp"

namespace rod
{
	/** Concept used to define a hash builder type. */
	template<typename T>
	concept hash_builder = requires (T &builder)
	{
		{ builder.write(std::declval<int>()) } -> std::convertible_to<T &>;
		{ builder.write(std::declval<int *>(), std::declval<std::size_t>()) } -> std::convertible_to<T &>;
		{ builder.write(std::declval<void *>(), std::declval<std::size_t>()) } -> std::convertible_to<T &>;
		{ builder.finish() } -> std::convertible_to<std::size_t>;
	};
	/** Concept used to define a hash builder type that supports hashing of type `V`. */
	template<typename T, typename V>
	concept hash_builder_for = hash_builder<T> && requires (T &builder, const V &value) { builder.write(value); };

	namespace _hash
	{
		/** SDBM hash builder. */
		class sdbm_builder
		{
		public:
			/** Initializes SDBM hash builder with a default seed. */
			constexpr sdbm_builder() noexcept = default;
			/** Initializes SDBM hash builder with the specified seed. */
			constexpr sdbm_builder(std::size_t seed) noexcept : _seed(seed) {}

			/** Writes a scalar value to the resulting hash.
			 * @param value Scalar value to be added to the resulting hash.
			 * @return Reference to this hash builder. */
			template<typename T> requires std::is_scalar_v<std::decay_t<T>>
			constexpr sdbm_builder &write(const T &value) noexcept { return write(&value, 1); }
			/** Writes a buffer of scalar values to the resulting hash.
			 * @param data Pointer to the source buffer to be added to the resulting hash.
			 * @param n Size of the source buffer.
			 * @return Reference to this hash builder. */
			template<typename T> requires std::is_scalar_v<std::decay_t<T>>
			constexpr sdbm_builder &write(const T *data, std::size_t n) noexcept
			{
				if (!std::is_constant_evaluated())
					return write(static_cast<const void *>(data), n * sizeof(T));
				else
				{
					for (std::size_t i = 0; i < n; ++i)
					{
						const auto bytes = std::bit_cast<std::array<std::byte, sizeof(T)>>(data[i]);
						push(bytes.data(), bytes.size());
					}
					return *this;
				}
			}
			/** Writes a buffer of bytes to the resulting hash.
			 * @param data Pointer to the source buffer to be added to the resulting hash.
			 * @param n Size of the source buffer in bytes.
			 * @return Reference to this hash builder. */
			sdbm_builder &write(const void *data, std::size_t n) noexcept
			{
				push(static_cast<const std::byte *>(data), n);
				return *this;
			}

			/** Finalizes the hash and returns it's value. */
			[[nodiscard]] constexpr std::size_t finish() noexcept { return _seed; }

		private:
			constexpr void push(const std::byte *bytes, std::size_t n) noexcept
			{
				std::size_t result = _seed;
				for (std::size_t i = 0; i < n; ++i)
				{
					const auto word = static_cast<std::size_t>(bytes[i]);
					result = word + (result << 6) + (result << 16) - result;
				}
				_seed = result;
			}

			std::size_t _seed = 0;
		};

		static_assert(hash_builder<sdbm_builder>);
		static_assert(sdbm_builder().write(0).finish() == sdbm_builder().write(0).finish());
		static_assert(sdbm_builder().write(0).finish() != sdbm_builder().write(1).finish());

		/** FNV1a hash builder. */
		class fnv1a_builder
		{
#if SIZE_MAX > UINT32_MAX
			constexpr static std::size_t prime = 0x00000100000001b3;
			constexpr static std::size_t offset = 0xcbf29ce484222325;
#else
			constexpr static std::size_t prime = 0x01000193;
			constexpr static std::size_t offset = 0x811c9dc5;
#endif

		public:
			/** Initializes FNV1a hash builder with a default seed. */
			constexpr fnv1a_builder() noexcept = default;
			/** Initializes FNV1a hash builder with the specified seed. */
			constexpr fnv1a_builder(std::size_t seed) noexcept : _seed(seed) {}

			/** Writes a scalar value to the resulting hash.
			 * @param value Scalar value to be added to the resulting hash.
			 * @return Reference to this hash builder. */
			template<typename T> requires std::is_scalar_v<std::decay_t<T>>
			constexpr fnv1a_builder &write(const T &value) noexcept { return write(&value, 1); }
			/** Writes a buffer of scalar values to the resulting hash.
			 * @param data Pointer to the source buffer to be added to the resulting hash.
			 * @param n Size of the source buffer.
			 * @return Reference to this hash builder. */
			template<typename T> requires std::is_scalar_v<std::decay_t<T>>
			constexpr fnv1a_builder &write(const T *data, std::size_t n) noexcept
			{
				if (!std::is_constant_evaluated())
					return write(static_cast<const void *>(data), n * sizeof(T));
				else
				{
					for (std::size_t i = 0; i < n; ++i)
					{
						const auto bytes = std::bit_cast<std::array<std::byte, sizeof(T)>>(data[i]);
						push(bytes.data(), bytes.size());
					}
					return *this;
				}
			}
			/** Writes a buffer of bytes to the resulting hash.
			 * @param data Pointer to the source buffer to be added to the resulting hash.
			 * @param n Size of the source buffer in bytes.
			 * @return Reference to this hash builder. */
			fnv1a_builder &write(const void *data, std::size_t n) noexcept
			{
				push(static_cast<const std::byte *>(data), n);
				return *this;
			}

			/** Finalizes the hash and returns it's value. */
			[[nodiscard]] constexpr std::size_t finish() noexcept { return _seed; }

		private:
			constexpr void push(const std::byte *bytes, std::size_t n) noexcept
			{
				std::size_t result = _seed;
				for (std::size_t i = 0; i < n; ++i)
				{
					result ^= static_cast<std::size_t>(bytes[i]);
					result *= prime;
				}
				_seed = result;
			}

			std::size_t _seed = offset;
		};

		static_assert(hash_builder<fnv1a_builder>);
		static_assert(fnv1a_builder().write(0).finish() == fnv1a_builder().write(0).finish());
		static_assert(fnv1a_builder().write(0).finish() != fnv1a_builder().write(1).finish());

		/** Compile-time hash builder type. */
		using const_hash_builder = fnv1a_builder;
		/** Default hash builder type. */
		using default_hash_builder = const_hash_builder;
	}

	using _hash::sdbm_builder;
	using _hash::fnv1a_builder;
	using _hash::const_hash_builder;
	using _hash::default_hash_builder;

	/** Combines two hash values. */
	[[nodiscard]] constexpr std::size_t hash_combine(std::size_t a, std::size_t b) noexcept { return a = b + 0x9e3779b9 + (a << 6) + (a >> 2); }

	/** Hashes a range of elements defined by [\a first, \a last) starting at seed \a seed using hash function \a h. */
	template<std::forward_iterator I, std::sentinel_for<I> S, typename H> requires std::invocable<H, std::iter_reference_t<I>> && std::convertible_to<std::invoke_result_t<H, std::iter_reference_t<I>>, std::size_t>
	[[nodiscard]] constexpr std::size_t hash_range(I first, S last, std::size_t seed, H &&h) noexcept(std::is_nothrow_invocable_v<H, std::iter_reference_t<I>>)
	{
		std::size_t result = seed;
		std::for_each(first, last, [&](auto &v)
		{
			const auto hash = std::invoke(h, v);
			result = hash_combine(result, hash);
		});
		return result;
	}
	/** Hashes a range of elements defined by [\a first, \a last) starting at seed \a seed using `std::hash`. */
	template<std::forward_iterator I, std::sentinel_for<I> S>
	[[nodiscard]] constexpr std::size_t hash_range(I first, S last, std::size_t seed) noexcept(std::is_nothrow_invocable_v<std::hash<std::iter_value_t<I>>, std::iter_reference_t<I>>)
	{
		return hash_range(first, last, seed, std::hash<std::iter_value_t<I>>{});
	}

	/** Hashes a range of elements defined by [\a first, \a last) using hash builder \a b. */
	template<std::forward_iterator I, std::sentinel_for<I> S, hash_builder_for<std::iter_value_t<I>> B>
	[[nodiscard]] constexpr std::size_t hash_range(I first, S last, B &&b) noexcept(noexcept(b.write(*first)) && noexcept(b.finish()))
	{
		std::for_each(first, last, [&](auto &v) { b.write(v); });
		return static_cast<std::size_t>(b.finish());
	}
}

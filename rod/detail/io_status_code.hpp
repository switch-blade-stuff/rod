/*
 * Created by switch_blade on 2023-08-08.
 */

#pragma once

#include <string>

#include "../result.hpp"

namespace rod
{
	/** Status code type used by IO operations. */
	class io_status_code
	{
	public:
		/** Initializes IO status code from an error code. */
		io_status_code(std::error_code err) noexcept : _err(err) {}
		/** Initializes IO status code from an error code and number of bytes transferred. */
		io_status_code(std::error_code err, std::size_t bytes) noexcept : _err(err), _bytes(bytes) {}

		/** Initializes IO status code from a value and error category. */
		template<typename V> requires std::constructible_from<std::error_code, V, const std::error_category &>
		io_status_code(V value, const std::error_category &cat) noexcept : _err(value, cat) {}
		/** Initializes IO status code from a value, error category, and number of bytes transferred. */
		template<typename V> requires std::constructible_from<std::error_code, V, const std::error_category &>
		io_status_code(V value, const std::error_category &cat, std::size_t bytes) noexcept : _err(value, cat), _bytes(bytes) {}

		/** Resets the status code to an empty state. */
		void clear() noexcept
		{
			_err.clear();
			_bytes = {};
		}

		/** Returns the underlying value of the status code. */
		[[nodiscard]] int value() const noexcept { return _err.value(); }
		/** Returns the number of bytes partially processed by the IO operation. */
		[[nodiscard]] constexpr std::size_t partial_bytes() const noexcept { return _bytes; }

		/** Returns the underlying category of the status code. */
		[[nodiscard]] const std::error_category &category() const noexcept { return _err.category(); }
		/** Returns an error condition created from the underlying value and category. */
		[[nodiscard]] std::error_condition default_error_condition() const noexcept { return _err.default_error_condition(); }

		/** Checks if the status code is not empty. */
		[[nodiscard]] explicit operator bool() const noexcept { return static_cast<bool>(_err); }
		/** Converts status code to an instance of `std::error_code`. */
		[[nodiscard]] explicit operator std::error_code() const noexcept { return _err; }

		/** Generates a message string from the status code. */
		[[nodiscard]] std::string message() const
		{
			auto result = _err.message();
			result.append(" [bytes = ");
			result.append(std::to_string(_bytes));
			result.append("]");
			return result;
		}
		/** Throws an exception produced from the status code. */
		[[noreturn]] void throw_exception() { _detail::throw_error_code(_err, message()); }

		[[nodiscard]] friend bool operator==(const io_status_code &a, const io_status_code &b) noexcept { return a._err == b._err; }
		[[nodiscard]] friend auto operator<=>(const io_status_code &a, const io_status_code &b) noexcept { return a._err <=> b._err; }

		[[nodiscard]] friend bool operator==(const io_status_code &a, const std::error_code &b) noexcept { return a._err == b; }
		[[nodiscard]] friend bool operator==(const std::error_code &a, const io_status_code &b) noexcept { return a == b._err; }
		[[nodiscard]] friend bool operator==(const io_status_code &a, const std::error_condition &b) noexcept { return a._err == b; }
		[[nodiscard]] friend bool operator==(const std::error_condition &a, const io_status_code &b) noexcept { return a == b._err; }

		[[nodiscard]] friend auto operator<=>(const io_status_code &a, const std::error_code &b) noexcept { return a._err <=> b; }
		[[nodiscard]] friend auto operator<=>(const std::error_code &a, const io_status_code &b) noexcept { return a <=> b._err; }

	private:
		std::error_code _err;
		std::size_t _bytes = 0;
	};

	/** Result type with `io_status_code` error type. */
	template<typename T>
	using io_result = result<T, io_status_code>;
}

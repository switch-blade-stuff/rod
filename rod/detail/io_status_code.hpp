/*
 * Created by switchblade on 2023-08-08.
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
		io_status_code(std::error_code err, _handle::extent_type prt) noexcept : _err(err), _prt(prt) {}

		/** Initializes IO status code from a value and error category. */
		template<typename V> requires std::constructible_from<std::error_code, V, const std::error_category &>
		io_status_code(V value, const std::error_category &cat) noexcept : _err(value, cat) {}
		/** Initializes IO status code from a value, error category, and number of bytes transferred. */
		template<typename V> requires std::constructible_from<std::error_code, V, const std::error_category &>
		io_status_code(V value, const std::error_category &cat, _handle::extent_type prt) noexcept : _err(value, cat), _prt(prt) {}

		/** Resets the status code to an empty state. */
		void clear() noexcept
		{
			_err.clear();
			_prt = {};
		}

		/** Checks if the operation has completed partially. */
		[[nodiscard]] constexpr bool is_partial() const noexcept { return _prt != 0; }
		/** Returns the number of bytes partially processed by the IO operation. */
		[[nodiscard]] constexpr _handle::extent_type partial_bytes() const noexcept { return _prt; }

		/** Returns the underlying value of the status code. */
		[[nodiscard]] auto value() const noexcept { return _err.value(); }
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
			result.append(" [partial bytes = ");
			result.append(std::to_string(_prt));
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
		_handle::extent_type _prt = 0;
	};
}

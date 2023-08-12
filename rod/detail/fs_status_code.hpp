/*
 * Created by switch_blade on 2023-08-08.
 */

#pragma once

#include "../result.hpp"
#include "path_view.hpp"

namespace rod::fs
{
	/** Status code type used by filesystem facilities. */
	class fs_status_code
	{
	public:
		/** Initializes filesystem status code from an error code. */
		fs_status_code(std::error_code err) : _err(err) {}
		/** Initializes filesystem status code from an error code and one path via copy. */
		fs_status_code(std::error_code err, const path &p) : _err(err), _paths{p} {}
		/** Initializes filesystem status code from an error code code and one path via move. */
		fs_status_code(std::error_code err, path &&p) : _err(err), _paths{std::move(p)} {}
		/** Initializes filesystem status code from an error code and two paths via copy. */
		fs_status_code(std::error_code err, const path &p1, const path &p2) : _err(err), _paths{p1, p2} {}
		/** Initializes filesystem status code from an error code and two paths via move. */
		fs_status_code(std::error_code err, path &&p1, path &&p2) : _err(err), _paths{std::move(p1), std::move(p2)} {}

		/** Initializes filesystem status code from a value and error category. */
		fs_status_code(auto value, const std::error_category &cat) : _err(value, cat) {}
		/** Initializes filesystem status code from a value, error category, and one path via copy. */
		fs_status_code(auto value, const std::error_category &cat, const path &p) : _err(value, cat), _paths{p} {}
		/** Initializes filesystem status code from a value, error category, code and one path via move. */
		fs_status_code(auto value, const std::error_category &cat, path &&p) : _err(value, cat), _paths{std::move(p)} {}
		/** Initializes filesystem status code from a value, error category, and two paths via copy. */
		fs_status_code(auto value, const std::error_category &cat, const path &p1, const path &p2) : _err(value, cat), _paths{p1, p2} {}
		/** Initializes filesystem status code from a value, error category, and two paths via move. */
		fs_status_code(auto value, const std::error_category &cat, path &&p1, path &&p2) : _err(value, cat), _paths{std::move(p1), std::move(p2)} {}

		/** Resets the status code to an empty state. */
		void clear() noexcept
		{
			_err.clear();
			_paths[0].clear();
			_paths[1].clear();
		}

		/** Returns the underlying value of the status code. */
		[[nodiscard]] int value() const noexcept { return _err.value(); }
		/** Returns the underlying category of the status code. */
		[[nodiscard]] const std::error_category &category() const noexcept { return _err.category(); }
		/** Returns an error condition created from the underlying value and category. */
		[[nodiscard]] std::error_condition default_error_condition() const noexcept { return _err.default_error_condition(); }

		/** Returns reference to the first path involved in the operation. */
		[[nodiscard]] constexpr const path &path1() const noexcept { return _paths[0]; }
		/** Returns reference to the second path involved in the operation. */
		[[nodiscard]] constexpr const path &path2() const noexcept { return _paths[1]; }

		/** Checks if the status code is not empty. */
		[[nodiscard]] explicit operator bool() const noexcept { return static_cast<bool>(_err); }
		/** Converts status code to an instance of `std::error_code`. */
		[[nodiscard]] explicit operator std::error_code() const noexcept { return _err; }

		/** Generates a message string from the status code. */
		[[nodiscard]] std::string message() const
		{
			auto result = _err.message();
			if (!path1().empty() && !path2().empty())
			{
				result.append(" [path1 = \"");
				result.append(path1().string());
				result.append("\", path2 = \"");
				result.append(path2().string());
				result.append("\"]");
			}
			else if (!path1().empty())
			{
				result.append(" [path = \"");
				result.append(path1().string());
				result.append("\"]");
			}
			return result;
		}
		/** Throws an exception produced from the status code. */
		[[noreturn]] void throw_exception() { _detail::throw_error_code(_err, message()); }

		[[nodiscard]] friend bool operator==(const fs_status_code &a, const fs_status_code &b) noexcept { return a._err == b._err; }
		[[nodiscard]] friend auto operator<=>(const fs_status_code &a, const fs_status_code &b) noexcept { return a._err <=> b._err; }

		[[nodiscard]] friend bool operator==(const fs_status_code &a, const std::error_code &b) noexcept { return a._err == b; }
		[[nodiscard]] friend bool operator==(const std::error_code &a, const fs_status_code &b) noexcept { return a == b._err; }
		[[nodiscard]] friend bool operator==(const fs_status_code &a, const std::error_condition &b) noexcept { return a._err == b; }
		[[nodiscard]] friend bool operator==(const std::error_condition &a, const fs_status_code &b) noexcept { return a == b._err; }

		[[nodiscard]] friend auto operator<=>(const fs_status_code &a, const std::error_code &b) noexcept { return a._err <=> b; }
		[[nodiscard]] friend auto operator<=>(const std::error_code &a, const fs_status_code &b) noexcept { return a <=> b._err; }

	private:
		std::error_code _err;
		path _paths[2];
	};
}

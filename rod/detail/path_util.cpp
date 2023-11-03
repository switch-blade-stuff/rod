/*
 * Created by switchblade on 2023-09-15.
 */

#include "path_util.hpp"

namespace rod::fs
{
	result<bool> equivalent(path_view a, path_view b) noexcept
	{
		dev_t dev_a, dev_b;
		ino_t ino_a, ino_b;
		stat st;

		if (auto res = get_stat(st, {}, a, stat::query::dev | stat::query::ino); res.has_error()) [[unlikely]]
			return {in_place_error, res.error()};
		else if (*res != (stat::query::dev | stat::query::ino))
			return false;
		dev_a = st.dev;
		ino_a = st.ino;

		if (auto res = get_stat(st, {}, b, stat::query::dev | stat::query::ino); res.has_error()) [[unlikely]]
			return {in_place_error, res.error()};
		else if (*res != (stat::query::dev | stat::query::ino))
			return false;
		dev_b = st.dev;
		ino_b = st.ino;

		return dev_a == dev_b && ino_a == ino_b;
	}

	result<path> relative(path_view path) noexcept
	{
		if (auto curr = current_path(); curr.has_value()) [[likely]]
			return relative(path, *curr);
		else
			return curr.error();
	}
	result<path> proximate(path_view path) noexcept
	{
		if (auto curr = current_path(); curr.has_value()) [[likely]]
			return proximate(path, *curr);
		else
			return curr.error();
	}
	result<path> relative(path_view path, path_view base) noexcept
	{
		auto path_canon = weakly_canonical(path);
		if (path_canon.has_error()) [[unlikely]]
			return path_canon.error();

		auto base_canon = weakly_canonical(base);
		if (base_canon.has_error()) [[unlikely]]
			return base_canon.error();

		try { return path_canon->lexically_relative(*base_canon); }
		catch (...) { return _detail::current_error(); }
	}
	result<path> proximate(path_view path, path_view base) noexcept
	{
		auto path_canon = weakly_canonical(path);
		if (path_canon.has_error()) [[unlikely]]
			return path_canon.error();

		auto base_canon = weakly_canonical(base);
		if (base_canon.has_error()) [[unlikely]]
			return base_canon.error();

		try { return path_canon->lexically_proximate(*base_canon); }
		catch (...) { return _detail::current_error(); }
	}
}

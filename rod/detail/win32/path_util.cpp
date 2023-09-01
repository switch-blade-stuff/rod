/*
 * Created by switch_blade on 2023-08-31.
 */

#include "../path_util.hpp"

namespace rod
{
	result<bool> equivalent(path_view a, path_view b) noexcept
	{
		dev_t dev_a, dev_b;
		ino_t ino_a, ino_b;
		stat st;

		if (auto res = get_stat(st, a, stat::query::dev | stat::query::ino); res.has_error()) [[unlikely]]
			return result(in_place_error, res.error());
		dev_a = st.dev;
		ino_a = st.ino;

		if (auto res = get_stat(st, b, stat::query::dev | stat::query::ino); res.has_error()) [[unlikely]]
			return result(in_place_error, res.error());
		dev_b = st.dev;
		ino_b = st.ino;

		return dev_a == dev_b && ino_a == ino_b;
	}
}

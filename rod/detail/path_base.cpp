/*
 * Created by switch_blade on 2023-07-30.
 */

#include "path_base.hpp"
#include "../array_list.hpp"

namespace rod::_path
{
#ifdef ROD_WIN32
	constexpr typename path::string_view_type curr_dir = L".";
	constexpr typename path::string_view_type prev_dir = L"..";
#else
	constexpr typename path::string_view_type curr_dir = ".";
	constexpr typename path::string_view_type prev_dir = "..";
#endif

	result<cvt_status> utf16_to_utf32(const char16_t *src_first, const char16_t *src_last, char32_t *dst_first, char32_t *dst_last) noexcept
	{
		auto src_pos = src_first;
		auto dst_pos = dst_first;
		for (char32_t cp;; *dst_pos = cp, ++src_pos, ++dst_pos)
		{
			if (src_pos == src_last || dst_pos == dst_last)
			{
				cvt_status status;
				status.n_src = src_pos - src_first;
				status.n_dst = dst_pos - dst_first;
				status.done = src_pos == src_last;
				return status;
			}
			else if (cp = *src_pos; cp > 0xdfff) [[unlikely]]
				break;
			else if (cp > 0xd7ff && cp <= 0xdbff)
			{
				if (++src_pos == src_last) [[unlikely]]
					break;
				if (const auto lh = *src_pos; lh >= 0xdc00 && lh <= 0xdfff)
					[[likely]]
							cp = (cp << 10) + lh + 0xfca02400;
				else
					break;
			}
		}
		return std::make_error_code(std::errc::invalid_argument);
	}
	result<cvt_status> utf32_to_utf16(const char32_t *src_first, const char32_t *src_last, char16_t *dst_first, char16_t *dst_last) noexcept
	{
		auto src_pos = src_first;
		auto dst_pos = dst_first;
		for (;;)
		{
			if (src_pos == src_last || dst_pos == dst_last)
			{
				cvt_status status;
				status.n_src = src_pos - src_first;
				status.n_dst = dst_pos - dst_first;
				status.done = src_pos == src_last;
				return status;
			}

			if (const auto cp = *src_pos++; (cp > 0xd7ff && cp <= 0xdfff) || cp > 0x10ffff) [[unlikely]]
				return std::make_error_code(std::errc::invalid_argument);
			else if (cp > 0xffff)
			{
				*dst_pos++ = static_cast<char16_t>(0xd7c0 + (cp >> 10));
				*dst_pos++ = static_cast<char16_t>(0xdc00 + (cp & 0x3ff));
			}
			else
				*dst_pos++ = static_cast<char16_t>(cp);
		}
	}

	path path::make_relative(const path &self, const path &base)
	{
		const auto self_root_name = root_name_substr<value_type>(self._string, self.format());
		const auto base_root_name = root_name_substr<value_type>(base._string, base.format());

		path result;
		if (self_root_name != base_root_name || self.is_absolute() != base.is_absolute() || (!self.has_root_directory() && base.has_root_directory()) ||
		    contains_root<value_type>(self._string, self.format()) || contains_root<value_type>(base._string, base.format()))
			return result;

		const auto self_begin = self.begin(), self_end = self.end();
		const auto base_begin = base.begin(), base_end = base.end();

		auto [a_iter, b_iter] = std::mismatch(self_begin, self_end, base_begin, base_end);
		if (a_iter == self_end && b_iter == base_end)
		{
			result = curr_dir;
			return result;
		}

		const auto base_root_dist = base.has_root_name() + base.has_root_directory();
		for (auto dist = std::distance(base_begin, b_iter); dist < base_root_dist; ++dist)
			++b_iter;

		std::ptrdiff_t diff = 0;
		for (; b_iter != base_end; ++b_iter)
		{
			const auto &val = *b_iter;
			if (val.empty() || val == curr_dir)
				continue;
			else if (val == prev_dir)
				--diff;
			else
				++diff;
		}

		if (diff < 0)
			return result;
		if (diff == 0 && (a_iter == self_end || a_iter->empty()))
		{
			result = curr_dir;
			return result;
		}

		for (; diff > 0; --diff)
			result /= prev_dir;
		for (; a_iter != self_end; ++a_iter)
			result /= *a_iter;

		return result;
	}

	path path::lexically_normal() const
	{
		if (empty() || format() == binary_format)
			return *this;

		size_type filename_pos = root_name_size<value_type>(_string, format());
		size_type result_len = filename_pos;
		array_list<string_view_type> comps;
		string_type result_string;

		/* Separate path string into component list & find the approximate string length. */
		while (result_len < _string.size())
		{
			if (!is_separator(_string[result_len], format()))
			{
				const auto tail = string_view_type(_string.data() + result_len, _string.size() - result_len);
				const auto comp = tail.substr(0, lfind_separator(tail, format()));
				result_len += comp.size();
				comps.emplace_back(comp);
			}
			else if (comps.empty() || !comps.back().empty())
			{
				comps.emplace_back();
				result_len += 1;
			}
		}

		const auto erase_wildcards = [&](string_view_type str)
		{
			for (auto pos = comps.begin(); pos != comps.end();)
			{
				if (*pos != str)
				{
					pos = std::next(pos);
					continue;
				}

				result_len -= pos->size();
				pos = comps.erase(pos);

				if (pos != comps.end())
				{
					result_len -= pos->size();
					pos = comps.erase(pos);
				}
			}
		};

		/* Remove single-dot wildcards. */
		erase_wildcards(curr_dir);

		/* Resolve non-leading dot-dot wildcards. */
		for (auto pos = comps.begin(); pos != comps.end();)
			if (auto curr = pos++; *curr == prev_dir && curr != comps.begin() && --curr != comps.begin() && *(--curr) != prev_dir)
			{
				if (pos != comps.end())
					++pos;

				while (curr != pos)
				{
					result_len -= curr->size();
					curr = comps.erase(curr);
				}
			}

		/* Remove leading dot-dot wildcards if there is a root. */
		if (!comps.empty() && comps.front().empty())
			erase_wildcards(prev_dir);

		/* Remove the trailing dot-dot separator. */
		if (comps.size() >= 2 && comps.back().empty() && *(std::prev(comps.end(), 2)) == prev_dir)
		{
			result_len -= comps.back().size();
			comps.pop_back();
		}

		/* Build the result string. */
		result_string.reserve(result_len);
		result_string = string_view_type(_string.data(), filename_pos);
#ifdef ROD_WIN32
		std::ranges::replace(result_string, '/', preferred_separator);
#endif

		for (auto &comp: comps)
		{
			if (comp.empty())
				result_string.push_back(preferred_separator);
			else
				result_string.append(comp);
		}

		if (result_string.empty())
			result_string = '.';

		return std::move(result_string);
	}
	path path::lexically_relative(const path &base) const
	{
		if (format() == binary_format || base.format() == binary_format)
			return {};

#ifdef ROD_WIN32
		const auto both_unc = is_unc_drive_path<value_type>(_string, format()) && is_unc_drive_path<value_type>(base._string, base.format());
		return make_relative(both_unc ? relative_path() : *this, both_unc ? base.relative_path() : base);
#else
		return make_relative(*this, base);
#endif
	}
}

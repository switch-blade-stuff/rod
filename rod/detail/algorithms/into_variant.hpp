/*
 * Created by switchblade on 2023-04-18.
 */

#pragma once

#include "../queries/completion.hpp"

namespace rod
{
	namespace detail
	{
		template<class S, class E> requires sender_in<S, E>
		using into_variant_type = value_types_of_t<S, E>;
	}
}

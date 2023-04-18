/*
 * Created by switchblade on 2023-04-09.
 */

#pragma once

#include "detail/concepts.hpp"

#include "detail/queries/signatures.hpp"
#include "detail/queries/allocator.hpp"
#include "detail/queries/scheduler.hpp"
#include "detail/queries/may_block.hpp"
#include "detail/queries/progress.hpp"

#include "detail/algorithms/transfer.hpp"
#include "detail/algorithms/schedule.hpp"
#include "detail/algorithms/connect.hpp"
#include "detail/algorithms/opstate.hpp"
#include "detail/algorithms/just.hpp"
#include "detail/algorithms/read.hpp"

#include "detail/awaitable.hpp"
#include "stop_token.hpp"

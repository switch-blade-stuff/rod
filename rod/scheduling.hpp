/*
 * Created by switchblade on 2023-04-09.
 */

#pragma once

#include "detail/concepts.hpp"

#include "detail/factories/read.hpp"
#include "detail/factories/just.hpp"

#include "detail/queries/completion.hpp"
#include "detail/queries/scheduler.hpp"
#include "detail/queries/allocator.hpp"
#include "detail/queries/may_block.hpp"
#include "detail/queries/progress.hpp"

#include "detail/adaptors/closure.hpp"
#include "detail/adaptors/schedule_from.hpp"
#include "detail/adaptors/transfer.hpp"
#include "detail/adaptors/split.hpp"
#include "detail/adaptors/bulk.hpp"
#include "detail/adaptors/then.hpp"
#include "detail/adaptors/let.hpp"
#include "detail/adaptors/on.hpp"

#include "detail/sync_wait.hpp"
#include "detail/awaitable.hpp"
#include "detail/run_loop.hpp"
#include "stop_token.hpp"


include(${CMAKE_CURRENT_LIST_DIR}/queries/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/adaptors/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/factories/CMakeLists.txt)

list(APPEND ROD_HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/config.hpp
        ${CMAKE_CURRENT_LIST_DIR}/shared_ref.hpp
        ${CMAKE_CURRENT_LIST_DIR}/environment.hpp
        ${CMAKE_CURRENT_LIST_DIR}/atomic_queue.hpp
        ${CMAKE_CURRENT_LIST_DIR}/basic_queue.hpp
        ${CMAKE_CURRENT_LIST_DIR}/priority_queue.hpp
        ${CMAKE_CURRENT_LIST_DIR}/concepts.hpp
        ${CMAKE_CURRENT_LIST_DIR}/run_loop.hpp
        ${CMAKE_CURRENT_LIST_DIR}/awaitable.hpp
        ${CMAKE_CURRENT_LIST_DIR}/sync_wait.hpp
        ${CMAKE_CURRENT_LIST_DIR}/ensure_started.hpp
        ${CMAKE_CURRENT_LIST_DIR}/start_detached.hpp
        ${CMAKE_CURRENT_LIST_DIR}/io_buffer.hpp
        ${CMAKE_CURRENT_LIST_DIR}/file_fwd.hpp)

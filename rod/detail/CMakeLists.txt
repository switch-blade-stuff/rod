
include(${CMAKE_CURRENT_LIST_DIR}/queries/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/adaptors/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/factories/CMakeLists.txt)

target_sources(${PROJECT_NAME} INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/config.hpp
        ${CMAKE_CURRENT_LIST_DIR}/environment.hpp
        ${CMAKE_CURRENT_LIST_DIR}/atomic_queue.hpp
        ${CMAKE_CURRENT_LIST_DIR}/concepts.hpp
        ${CMAKE_CURRENT_LIST_DIR}/run_loop.hpp
        ${CMAKE_CURRENT_LIST_DIR}/awaitable.hpp
        ${CMAKE_CURRENT_LIST_DIR}/sync_wait.hpp)
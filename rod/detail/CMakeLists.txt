
include(${CMAKE_CURRENT_LIST_DIR}/queries/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/algorithms/CMakeLists.txt)

target_sources(${PROJECT_NAME} INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/utility.hpp
        ${CMAKE_CURRENT_LIST_DIR}/environment.hpp
        ${CMAKE_CURRENT_LIST_DIR}/concepts.hpp
        ${CMAKE_CURRENT_LIST_DIR}/awaitable.hpp
        ${CMAKE_CURRENT_LIST_DIR}/atomic_queue.hpp)
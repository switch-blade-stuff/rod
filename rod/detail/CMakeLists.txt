
include(${CMAKE_CURRENT_LIST_DIR}/query/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/algorithm/CMakeLists.txt)

include(${CMAKE_CURRENT_LIST_DIR}/unix/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/win32/CMakeLists.txt)

target_sources(${PROJECT_NAME} INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/utility.hpp
        ${CMAKE_CURRENT_LIST_DIR}/environment.hpp
        ${CMAKE_CURRENT_LIST_DIR}/concepts.hpp
        ${CMAKE_CURRENT_LIST_DIR}/awaitable.hpp)
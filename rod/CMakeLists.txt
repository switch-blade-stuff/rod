
include(${CMAKE_CURRENT_LIST_DIR}/detail/CMakeLists.txt)

# Platform subdirectories
include(${CMAKE_CURRENT_LIST_DIR}/unix/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/linux/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/win32/CMakeLists.txt)

list(APPEND ROD_HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/tag.hpp
        ${CMAKE_CURRENT_LIST_DIR}/utility.hpp
        ${CMAKE_CURRENT_LIST_DIR}/delegate.hpp
        ${CMAKE_CURRENT_LIST_DIR}/packed_pair.hpp
        ${CMAKE_CURRENT_LIST_DIR}/async_mutex.hpp
        ${CMAKE_CURRENT_LIST_DIR}/stop_token.hpp
        ${CMAKE_CURRENT_LIST_DIR}/scheduling.hpp
        ${CMAKE_CURRENT_LIST_DIR}/generator.hpp
        ${CMAKE_CURRENT_LIST_DIR}/result.hpp
        ${CMAKE_CURRENT_LIST_DIR}/signal.hpp
        ${CMAKE_CURRENT_LIST_DIR}/task.hpp
        ${CMAKE_CURRENT_LIST_DIR}/file.hpp
        ${CMAKE_CURRENT_LIST_DIR}/io.hpp)
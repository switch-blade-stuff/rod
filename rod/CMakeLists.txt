
include(${CMAKE_CURRENT_LIST_DIR}/detail/CMakeLists.txt)

# Platform subdirectories
if (WIN32)
    include(${CMAKE_CURRENT_LIST_DIR}/win32/CMakeLists.txt)
endif ()
if (UNIX)
    include(${CMAKE_CURRENT_LIST_DIR}/unix/CMakeLists.txt)
endif ()
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(${CMAKE_CURRENT_LIST_DIR}/linux/CMakeLists.txt)
endif ()

list(APPEND ROD_HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/tag.hpp
        ${CMAKE_CURRENT_LIST_DIR}/utility.hpp
        ${CMAKE_CURRENT_LIST_DIR}/delegate.hpp
        ${CMAKE_CURRENT_LIST_DIR}/packed_pair.hpp
        ${CMAKE_CURRENT_LIST_DIR}/async_mutex.hpp
        ${CMAKE_CURRENT_LIST_DIR}/scheduling.hpp
        ${CMAKE_CURRENT_LIST_DIR}/generator.hpp
        ${CMAKE_CURRENT_LIST_DIR}/signal.hpp
        ${CMAKE_CURRENT_LIST_DIR}/task.hpp
        ${CMAKE_CURRENT_LIST_DIR}/file.hpp
        ${CMAKE_CURRENT_LIST_DIR}/io.hpp)
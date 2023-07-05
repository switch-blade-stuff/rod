
include(${CMAKE_CURRENT_LIST_DIR}/detail/CMakeLists.txt)

list(APPEND ROD_HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/iocp_context.hpp)

list(APPEND ROD_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/iocp_context.cpp)


include(${CMAKE_CURRENT_LIST_DIR}/detail/CMakeLists.txt)

list(APPEND ROD_HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/monotonic_clock.hpp)

list(APPEND ROD_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/monotonic_clock.cpp)

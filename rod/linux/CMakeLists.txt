
list(APPEND ROD_HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/io_uring_context.hpp
        ${CMAKE_CURRENT_LIST_DIR}/epoll_context.hpp)

list(APPEND ROD_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/io_uring_context.cpp
        ${CMAKE_CURRENT_LIST_DIR}/epoll_context.cpp)

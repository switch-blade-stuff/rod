
list(APPEND ROD_HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/descriptor.hpp
        ${CMAKE_CURRENT_LIST_DIR}/file.hpp
        ${CMAKE_CURRENT_LIST_DIR}/mmap.hpp
        ${CMAKE_CURRENT_LIST_DIR}/shm.hpp)

list(APPEND ROD_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/descriptor.cpp
        ${CMAKE_CURRENT_LIST_DIR}/file.cpp
        ${CMAKE_CURRENT_LIST_DIR}/mmap.cpp
        ${CMAKE_CURRENT_LIST_DIR}/shm.cpp)

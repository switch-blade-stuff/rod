
include(${CMAKE_CURRENT_LIST_DIR}/detail/CMakeLists.txt)

target_sources(${PROJECT_NAME} INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/tag.hpp
        ${CMAKE_CURRENT_LIST_DIR}/utility.hpp
        ${CMAKE_CURRENT_LIST_DIR}/delegate.hpp
        ${CMAKE_CURRENT_LIST_DIR}/packed_pair.hpp
        ${CMAKE_CURRENT_LIST_DIR}/async_mutex.hpp
        ${CMAKE_CURRENT_LIST_DIR}/scheduling.hpp
        ${CMAKE_CURRENT_LIST_DIR}/generator.hpp
        ${CMAKE_CURRENT_LIST_DIR}/signal.hpp
        ${CMAKE_CURRENT_LIST_DIR}/task.hpp)
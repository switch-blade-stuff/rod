
include(${CMAKE_CURRENT_LIST_DIR}/detail/CMakeLists.txt)

target_sources(${PROJECT_NAME} INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/tag.hpp
        ${CMAKE_CURRENT_LIST_DIR}/scheduling.hpp
        ${CMAKE_CURRENT_LIST_DIR}/this_thread.hpp
        ${CMAKE_CURRENT_LIST_DIR}/run_loop.hpp
        ${CMAKE_CURRENT_LIST_DIR}/signal.hpp
        ${CMAKE_CURRENT_LIST_DIR}/timer.hpp)
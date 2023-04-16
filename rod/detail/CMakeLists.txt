
include(${CMAKE_CURRENT_LIST_DIR}/unix/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/win32/CMakeLists.txt)

target_sources(${PROJECT_NAME} INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/utility.hpp
        ${CMAKE_CURRENT_LIST_DIR}/exec_env.hpp
        ${CMAKE_CURRENT_LIST_DIR}/operation.hpp
        ${CMAKE_CURRENT_LIST_DIR}/stop_token.hpp
        ${CMAKE_CURRENT_LIST_DIR}/completion.hpp
        ${CMAKE_CURRENT_LIST_DIR}/sender.hpp
        ${CMAKE_CURRENT_LIST_DIR}/receiver.hpp
        ${CMAKE_CURRENT_LIST_DIR}/awaitable.hpp)
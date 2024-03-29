cmake_minimum_required(VERSION 3.19)
project(rod-tests)
enable_testing()

function(make_test NAME FILE)
    set(TEST_PROJECT "${PROJECT_NAME}-${NAME}")

    add_executable(${TEST_PROJECT} ${FILE})
    target_link_libraries(${TEST_PROJECT} PRIVATE rod)

    # Enable max error reporting
    if (MSVC)
        target_compile_options(${TEST_PROJECT} PRIVATE /W3 /WX)
    else ()
        target_compile_options(${TEST_PROJECT} PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-unknown-pragmas -Wno-ignored-attributes)
#        target_compile_options(${TEST_PROJECT} PRIVATE -fsanitize=address)
#        target_link_options(${TEST_PROJECT} PRIVATE -fsanitize=address)
    endif ()

    # Add as test
    add_test(NAME ${NAME} COMMAND "$<TARGET_FILE:${TEST_PROJECT}>")
endfunction()

make_test(path ${CMAKE_CURRENT_LIST_DIR}/test_path.cpp)
make_test(file ${CMAKE_CURRENT_LIST_DIR}/test_file.cpp)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    make_test(iocp ${CMAKE_CURRENT_LIST_DIR}/test_iocp.cpp)
endif ()
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    make_test(epoll ${CMAKE_CURRENT_LIST_DIR}/test_epoll.cpp)
    make_test(io_uring ${CMAKE_CURRENT_LIST_DIR}/test_io_uring.cpp)
endif ()

make_test(signal ${CMAKE_CURRENT_LIST_DIR}/test_signal.cpp)
make_test(coroutine ${CMAKE_CURRENT_LIST_DIR}/test_coroutine.cpp)
make_test(stop-token ${CMAKE_CURRENT_LIST_DIR}/test_stop_token.cpp)
make_test(scheduling ${CMAKE_CURRENT_LIST_DIR}/test_scheduling.cpp)
make_test(thread-pool ${CMAKE_CURRENT_LIST_DIR}/test_thread_pool.cpp)

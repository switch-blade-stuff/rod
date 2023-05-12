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
        target_compile_options(${TEST_PROJECT} PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-unknown-pragmas -Wno-ignored-attributes -ftemplate-backtrace-limit=0)
    endif ()

    # Add as test
    add_test(NAME ${NAME} COMMAND "$<TARGET_FILE:${TEST_PROJECT}>")
endfunction()

make_test(run-loop ${CMAKE_CURRENT_LIST_DIR}/test_run_loop.cpp)
make_test(coroutine ${CMAKE_CURRENT_LIST_DIR}/test_coroutine.cpp)
make_test(stop-token ${CMAKE_CURRENT_LIST_DIR}/test_stop_token.cpp)

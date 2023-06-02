cmake_minimum_required(VERSION 3.19)
project(rod)

# Set CMake module search path
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake ${CMAKE_MODULE_PATH})

# Library options
option(ROD_NO_COROUTINES "Disables support for C++20 coroutines" OFF)
if (${ROD_NO_COROUTINES})
    add_compile_definitions(ROD_NO_COROUTINES)
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    option(ROD_NO_LIBURING "Disables support for io_uring" OFF)

    find_package(liburing)
    if (NOT LIBURING_FOUND)
        set(ROD_NO_LIBURING ON CACHE BOOL "liburing was not found" FORCE)
    else ()
        get_filename_component(LIBURING_PATH ${LIBURING_LIBRARY} DIRECTORY)
    endif ()
    if (${ROD_NO_LIBURING})
        add_compile_definitions(ROD_NO_LIBURING)
    endif ()
endif ()

option(ROD_BUILD_SHARED "Toggles build of shared library target" ON)
option(ROD_BUILD_STATIC "Toggles build of static library target" ON)

# Make sure either static or shared target is enabled
if (NOT (${ROD_BUILD_SHARED} OR ${ROD_BUILD_STATIC}))
    message(FATAL_ERROR "Neither shared nor static target is enabled, aborting")
endif ()

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib CACHE STRING "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE STRING "")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE STRING "")

# Include library source subdirectory
set(ROD_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/rod)
include(${ROD_SOURCE_DIR}/CMakeLists.txt)

# Add shared & static library targets
function(rod_configure_target NAME TYPE PREFIX)
    add_library(${NAME} ${TYPE})

    # Add sources & includes
    target_sources(${NAME} PUBLIC ${ROD_HEADERS})
    target_sources(${NAME} PRIVATE ${ROD_SOURCES})
    target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

    # Link dependencies
    if (${LIBURING_FOUND})
        target_include_directories(${NAME} PRIVATE ${LIBURING_INCLUDE_DIR})
        target_link_libraries(${NAME} PUBLIC ${LIBURING_LIBRARY})
    endif ()

    # Set target properties
    set_target_properties(${NAME} PROPERTIES PREFIX "${PREFIX}")
    set_target_properties(${NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")

    # Set C++ version
    target_compile_features(${NAME} PUBLIC cxx_std_20)

    # Set default symbol visibility
    set_target_properties(${NAME} PROPERTIES C_VISIBILITY_PRESET hidden)
    set_target_properties(${NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
    set_target_properties(${NAME} PROPERTIES VISIBILITY_INLINES_HIDDEN ON)

    # Set definitions
    set_target_properties(${NAME} PROPERTIES DEFINE_SYMBOL "ROD_EXPORT")
    target_compile_definitions(${NAME} PUBLIC "ROD_LIB_${TYPE}")

    # Enable max error reporting
    if (MSVC)
        target_compile_options(${NAME} PRIVATE /W3 /WX)
    else ()
        target_compile_options(${NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-unknown-pragmas -Wno-ignored-attributes)
    endif ()
endfunction()

if (${ROD_BUILD_SHARED})
    rod_configure_target(${PROJECT_NAME}-shared SHARED "")
endif ()
if (${ROD_BUILD_STATIC})
    rod_configure_target(${PROJECT_NAME}-static STATIC "lib")
endif ()

# Include library source subdirectory
set(ROD_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/rod)
include(${ROD_SOURCE_DIR}/CMakeLists.txt)

# Unit tests
option(ROD_TESTS "Enable unit tests" OFF)
if (${ROD_TESTS})
    include(${CMAKE_CURRENT_LIST_DIR}/test/CMakeLists.txt)
endif ()
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

option(BUILD_SHARED_LIBS "Toggles build as a shared library" ON)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib CACHE STRING "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE STRING "")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE STRING "")

# Add library target
add_library(${PROJECT_NAME})

# Include library source subdirectory
set(ROD_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/rod)
include(${ROD_SOURCE_DIR}/CMakeLists.txt)

# Generate CMake export header to be included by config.hpp
include(GenerateExportHeader)
set(ROD_EXPORT_HEADER ${ROD_SOURCE_DIR}/detail/export.gen.hpp)
generate_export_header(${PROJECT_NAME} BASE_NAME ROD EXPORT_FILE_NAME ${ROD_EXPORT_HEADER})
list(APPEND ROD_HEADERS ${ROD_EXPORT_HEADER})

# Add sources & includes
target_sources(${PROJECT_NAME} PUBLIC ${ROD_HEADERS})
target_sources(${PROJECT_NAME} PRIVATE ${ROD_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# Set C++ version
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# Set default symbol visibility
set_target_properties(${PROJECT_NAME} PROPERTIES C_VISIBILITY_PRESET hidden)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(${PROJECT_NAME} PROPERTIES VISIBILITY_INLINES_HIDDEN ON)

# Link dependencies
if (${LIBURING_FOUND})
    target_include_directories(${PROJECT_NAME} PRIVATE ${LIBURING_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBURING_LIBRARY})
endif ()

# Enable max error reporting
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-unknown-pragmas -Wno-ignored-attributes)
endif ()

# Unit tests
option(ROD_TESTS "Enable unit tests" OFF)
if (${ROD_TESTS})
    include(${CMAKE_CURRENT_LIST_DIR}/test/CMakeLists.txt)
endif ()
cmake_minimum_required(VERSION 3.19)
project(rod)

# Library options
# ...

# Create library target
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# Set C++ version
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

# Include library source subdirectory
set(ROD_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/rod)
include(${ROD_SOURCE_DIR}/CMakeLists.txt)

# Unit tests
option(ROD_TESTS "Enable unit tests" OFF)
if (${ROD_TESTS})
    include(${CMAKE_CURRENT_LIST_DIR}/test/CMakeLists.txt)
endif ()
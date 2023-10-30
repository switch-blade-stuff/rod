cmake_minimum_required(VERSION 3.23)
project(rod VERSION 0.0.0 LANGUAGES CXX)

# Set CMake module search path & system name
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake ${CMAKE_MODULE_PATH})
string(TOLOWER ${CMAKE_SYSTEM_NAME} ROD_SYSTEM_NAME)

# Library options
option(ROD_NO_COROUTINES "Disables support for C++20 coroutines" OFF)
if (${ROD_NO_COROUTINES})
    add_compile_definitions(ROD_NO_COROUTINES)
endif ()

if (${ROD_SYSTEM_NAME} MATCHES "linux|android")
    option(ROD_NO_LIBURING "Disables support for io_uring" OFF)
    find_package(liburing)

    if (NOT LIBURING_FOUND)
        set(ROD_NO_LIBURING ON CACHE BOOL "liburing was not found" FORCE)
    endif ()
    if (ROD_NO_LIBURING)
        add_compile_definitions(ROD_NO_LIBURING)
    endif ()
endif ()

option(BUILD_SHARED_LIBS "Toggles build as a shared library" ON)

# Add library target
add_library(${PROJECT_NAME})

# Set C++ version
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# Set default symbol visibility
set_target_properties(${PROJECT_NAME} PROPERTIES C_VISIBILITY_PRESET hidden)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(${PROJECT_NAME} PROPERTIES VISIBILITY_INLINES_HIDDEN ON)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_include_directories(${PROJECT_NAME} PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Link dependencies
if (LIBURING_FOUND AND NOT ROD_NO_LIBURING)
    target_include_directories(${PROJECT_NAME} PRIVATE ${LIBURING_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBURING_LIBRARY})
endif ()

# Enable max error reporting
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-unknown-pragmas -Wno-ignored-attributes)
endif ()

# Generate CMake export header
include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} BASE_NAME ROD EXPORT_FILE_NAME rod/detail/export.gen.hpp)

# Platform-independent headers
target_sources(${PROJECT_NAME} PUBLIC FILE_SET HEADERS BASE_DIRS rod FILES
        # Configuration
        rod/detail/export.gen.hpp
        rod/detail/config.hpp

        # Utilities
        rod/detail/receiver_adaptor.hpp
        rod/detail/priority_queue.hpp
        rod/detail/atomic_queue.hpp
        rod/detail/basic_queue.hpp
        rod/detail/byte_buffer.hpp
        rod/detail/shared_ref.hpp
        rod/detail/file_clock.hpp
        rod/detail/timeout.hpp
        rod/packed_pair.hpp
        rod/array_list.hpp
        rod/delegate.hpp
        rod/property.hpp
        rod/signal.hpp
        rod/result.hpp
        rod/hash.hpp
        rod/tag.hpp

        # Scheduling base
        rod/detail/async_base.hpp
        rod/stop_token.hpp
        rod/scheduling.hpp

        # Scheduling factories
        rod/detail/factories/read.hpp
        rod/detail/factories/just.hpp

        # Scheduling queries
        rod/detail/queries/completion.hpp
        rod/detail/queries/scheduler.hpp
        rod/detail/queries/allocator.hpp
        rod/detail/queries/may_block.hpp
        rod/detail/queries/progress.hpp

        # Scheduling adaptors
        rod/detail/adaptors/closure.hpp
        rod/detail/adaptors/into_variant.hpp
        rod/detail/adaptors/with_stop_token.hpp
        rod/detail/adaptors/schedule_from.hpp
        rod/detail/adaptors/stopped_as.hpp
        rod/detail/adaptors/transfer.hpp
        rod/detail/adaptors/when_all.hpp
        rod/detail/adaptors/split.hpp
        rod/detail/adaptors/bulk.hpp
        rod/detail/adaptors/then.hpp
        rod/detail/adaptors/let.hpp
        rod/detail/adaptors/on.hpp

        # Async algorithms
        rod/detail/algorithm/ensure_started.hpp
        rod/detail/algorithm/start_detached.hpp
        rod/detail/algorithm/sync_wait.hpp
        rod/detail/algorithm/execute.hpp

        # Execution contexts
        rod/detail/run_loop.hpp
        rod/detail/thread_pool.hpp

        # Coroutines
        rod/detail/awaitable.hpp
        rod/async_mutex.hpp
        rod/generator.hpp
        rod/task.hpp

        # Basic handles
        rod/detail/handle_stat.hpp
        rod/detail/handle_base.hpp
        rod/detail/io_status_code.hpp
        rod/detail/io_handle_base.hpp
        rod/detail/fs_handle_base.hpp

        # Filesystem base
        rod/detail/path_base.hpp
        rod/detail/path_view.hpp
        rod/detail/path_util.hpp
        rod/detail/path_handle.hpp
        rod/detail/link_handle.hpp
        rod/detail/path_discovery.hpp
        rod/detail/directory_handle.hpp

        # Filesystem IO
        rod/detail/file_handle.hpp
        # rod/detail/mapping_source.hpp
        # rod/detail/mapping_handle.hpp
        # rod/detail/file_mapping.hpp

        # Filesystem algorithms
        rod/detail/algorithm/traverse.hpp

        # Linux headers
        # rod/linux/io_uring_context.hpp
        # rod/linux/epoll_context.hpp

        # Win32 headers
        # rod/win32/iocp_context.hpp

        # API headers
        rod/path.hpp
        rod/file.hpp
        #[[rod/io.hpp]])

# Platform-independent sources
target_sources(${PROJECT_NAME} PRIVATE
        rod/detail/path_base.cpp
        rod/detail/path_util.cpp
        rod/detail/path_discovery.cpp
        rod/detail/thread_pool.cpp
        rod/detail/run_loop.cpp
        rod/utility.cpp
        rod/result.cpp)

# Posix implementation
# if (POSIX)
#
# endif ()

# Linux implementation
if (${ROD_SYSTEM_NAME} MATCHES "linux|android")
    target_sources(${PROJECT_NAME} PRIVATE
            #[[rod/linux/io_uring_context.cpp]]
            #[[rod/linux/epoll_context.cpp]])
endif ()

# Win32 implementation
if (${ROD_SYSTEM_NAME} MATCHES "windows|msys" OR CYGWIN)
    target_sources(${PROJECT_NAME} PRIVATE
            rod/detail/win32/ntapi.hpp
            rod/detail/win32/ntapi.cpp
            rod/detail/win32/path_base.hpp
            rod/detail/win32/path_base.cpp
            rod/detail/win32/path_util.hpp
            rod/detail/win32/path_util.cpp
            rod/detail/win32/file_clock.cpp
            rod/detail/win32/handle_base.hpp
            rod/detail/win32/handle_base.cpp
            rod/detail/win32/handle_stat.cpp
            rod/detail/win32/path_handle.hpp
            rod/detail/win32/path_handle.cpp
            rod/detail/win32/link_handle.hpp
            rod/detail/win32/link_handle.cpp
            rod/detail/win32/path_discovery.hpp
            rod/detail/win32/path_discovery.cpp
            rod/detail/win32/directory_handle.hpp
            rod/detail/win32/directory_handle.cpp
            rod/detail/win32/file_handle.hpp
            rod/detail/win32/file_handle.cpp
            #[[rod/win32/iocp_context.cpp]])
endif ()

# Unit tests
option(ROD_TESTS "Enable unit tests" OFF)
if (${ROD_TESTS})
    include(${CMAKE_CURRENT_LIST_DIR}/test/CMakeLists.txt)
endif ()

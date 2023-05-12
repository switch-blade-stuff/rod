
target_sources(${PROJECT_NAME} INTERFACE
        ../adaptors/on.hpp
        ../factories/read.hpp
        ../factories/just.hpp
        ../adaptors/then.hpp
        ../adaptors/closure.hpp
        ${CMAKE_CURRENT_LIST_DIR}/connect.hpp
        ${CMAKE_CURRENT_LIST_DIR}/opstate.hpp
        ../factories/schedule.hpp
        ../adaptors/transfer.hpp
        ${CMAKE_CURRENT_LIST_DIR}/into_variant.hpp)
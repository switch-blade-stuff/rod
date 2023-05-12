
target_sources(${PROJECT_NAME} INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/completion.hpp
        ${CMAKE_CURRENT_LIST_DIR}/allocator.hpp
        ${CMAKE_CURRENT_LIST_DIR}/scheduler.hpp
        ${CMAKE_CURRENT_LIST_DIR}/may_block.hpp
        ${CMAKE_CURRENT_LIST_DIR}/progress.hpp)
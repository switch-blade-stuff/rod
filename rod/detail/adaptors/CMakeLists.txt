
target_sources(${PROJECT_NAME} INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/on.hpp
        ${CMAKE_CURRENT_LIST_DIR}/let.hpp
        ${CMAKE_CURRENT_LIST_DIR}/then.hpp
        ${CMAKE_CURRENT_LIST_DIR}/closure.hpp
        ${CMAKE_CURRENT_LIST_DIR}/transfer.hpp
        ${CMAKE_CURRENT_LIST_DIR}/into_variant.hpp
        ${CMAKE_CURRENT_LIST_DIR}/schedule_from.hpp)
# SIERA Core Module

set(SIERA_CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/data_structures/list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/data_structures/queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/event/event.c
    ${CMAKE_CURRENT_SOURCE_DIR}/event/event_subscription.c
)

add_library(siera_core ${SIERA_CORE_SOURCES})
add_library(siera::core ALIAS siera_core)

target_include_directories(siera_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/data_structures>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/event>
)

target_compile_options(siera_core
    PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
)

message(STATUS "SIERA: Core module enabled")

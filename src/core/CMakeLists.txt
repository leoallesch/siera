# SIERA Core Module

file(GLOB_RECURSE SIERA_CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_library(siera_core ${SIERA_CORE_SOURCES})
add_library(siera::core ALIAS siera_core)

target_include_directories(siera_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/data_structures>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/database>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/event>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/timer>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/state_machine>
)

target_compile_options(siera_core
    PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
)

message(STATUS "SIERA: Core module enabled")

if(SIERA_ENABLE_COVERAGE)
  target_compile_options(siera_core PRIVATE --coverage)
  target_link_options(siera_core PUBLIC --coverage)
  message(STATUS "SIERA: Coverage instrumentation enabled for core")
endif()

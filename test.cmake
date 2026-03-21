cmake_minimum_required(3.16)

if(NOT ESP_PLATFORM)
    project(
        siera
        LANGUAGES C
        VERSION "0.0.1"
    )
endif()

set(SIERA_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

# OPTIONS

option(SIERA_ENABLE_UI "Build SIERA UI package" OFF)
option(SIERA_ENABLE_DRIVERS "Enable device specifc drivers, please define SIERA_PLATFORM" OFF)

set(SIERA_PLATFORM "" CACHE STRING "SIERA device platform")
set_property(CACHE SIERA_PLATFORM PROPERTY STRINGS "simulator" "esp")

# BUILD

file(GLOB_RECURSE SIERA_CORE_SOURCES "${SIERA_ROOT_DIR}/src/core/*.c")
file(GLOB_RECURSE SIERA_UI_SOURCES "${SIERA_ROOT_DIR}/src/ui" EXCLUDE REGEX ".*/components/.*")
set(SIERA_REQUIRES "")

if(SIERA_ENABLE_DRIVERS)
    if(NOT SIERA_PLATFORM)
        message(FATAL_ERROR "SIERA_PLATFORM must be define to use SIERA_ENABLE_DRIVERS")
    endif()

    if(SIERA_PLATFORM STREQUAL "simulator")
        file(GLOB_RECURSE SIERA_DRIVER_SOURCES "${SIERA_ROOT_DIR}/src/driver/simulator")
        list(APPEND SIERA_REQUIRES )
    endif()

endif()

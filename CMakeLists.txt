cmake_minimum_required(VERSION 3.14)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(SieraModule)

# ──────────────────────────────────────────────────────────────
# IDF early-expansion pass: collect component dependencies and exit.
# CMAKE_BUILD_EARLY_EXPANSION is set by IDF during its first-pass dependency
# scan; add_library and add_subdirectory are unavailable at this point.
# Sources are registered in the normal build below (ESP_PLATFORM guard).
# ──────────────────────────────────────────────────────────────

if(CMAKE_BUILD_EARLY_EXPANSION)
    file(GLOB _drv_requires_files
        "${CMAKE_CURRENT_LIST_DIR}/src/drivers/*/idf_requires.cmake")
    set(_idf_requires "")
    foreach(_f IN LISTS _drv_requires_files)
        include(${_f})
        list(APPEND _idf_requires ${SIERA_IDF_REQUIRES})
        unset(SIERA_IDF_REQUIRES)
    endforeach()
    idf_component_register(REQUIRES ${_idf_requires})
    return()
endif()

# ──────────────────────────────────────────────────────────────
# Options (host + ESP-IDF normal build)
# ──────────────────────────────────────────────────────────────

option(SIERA_ENABLE_DRIVERS "Build platform drivers"  OFF)
option(SIERA_BUILD_TESTS    "Build unit tests"         OFF)
option(SIERA_BUILD_EXAMPLES "Build examples"           OFF)

# ──────────────────────────────────────────────────────────────
# SIERA_PLATFORM — required when SIERA_ENABLE_DRIVERS=ON.
# On ESP-IDF this is set implicitly; validation is skipped.
# Valid values: ESP32  POSIX  SIMULATOR  PICO
# ──────────────────────────────────────────────────────────────

set(SIERA_VALID_PLATFORMS ESP32 POSIX SIMULATOR PICO)
set(SIERA_PLATFORM "" CACHE STRING "Target platform (ESP32 | POSIX | SIMULATOR | PICO)")
set_property(CACHE SIERA_PLATFORM PROPERTY STRINGS ${SIERA_VALID_PLATFORMS})

if(SIERA_ENABLE_DRIVERS)
    if(NOT SIERA_PLATFORM)
        message(FATAL_ERROR
            "SIERA_PLATFORM must be set when SIERA_ENABLE_DRIVERS=ON.\n"
            "Pass -DSIERA_PLATFORM=<platform>. Valid: ${SIERA_VALID_PLATFORMS}")
    endif()
    if(NOT SIERA_PLATFORM IN_LIST SIERA_VALID_PLATFORMS)
        message(FATAL_ERROR
            "SIERA_PLATFORM='${SIERA_PLATFORM}' is not valid.\n"
            "Valid platforms: ${SIERA_VALID_PLATFORMS}")
    endif()
endif()

# ──────────────────────────────────────────────────────────────
# Host-only setup
# ──────────────────────────────────────────────────────────────

if(NOT ESP_PLATFORM)
    project(siera
        VERSION 0.2.0
        DESCRIPTION "Simple Interface for Embedded Runtime Applications"
        LANGUAGES C CXX
    )

    set(CMAKE_C_STANDARD        11)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_STANDARD      20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    include(SieraTest)

    if(SIERA_BUILD_TESTS)
        enable_testing()
        include(FetchContent)
        FetchContent_Declare(cpputest
            GIT_REPOSITORY https://github.com/cpputest/cpputest.git
            GIT_TAG        master)
        set(TESTS OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(cpputest)
    endif()
endif()

# ──────────────────────────────────────────────────────────────
# siera — INTERFACE library
#
# Sources are compiled by the consumer so that consumer-supplied
# config headers (e.g. siera_ds_conf.h) are on the include path.
# ──────────────────────────────────────────────────────────────

add_library(siera INTERFACE)
target_compile_options(siera INTERFACE -Wall -Wextra)

set(SIERA_SRCS "")
set(SIERA_INCLUDE_DIRS "")

add_subdirectory(src/core/siera_common)
add_subdirectory(src/core/siera_hal)
add_subdirectory(src/core/siera_slist)
add_subdirectory(src/core/siera_event)
add_subdirectory(src/core/siera_timer)
add_subdirectory(src/core/siera_fsm)
add_subdirectory(src/core/siera_ds)

# ──────────────────────────────────────────────────────────────
# Platform driver
# ──────────────────────────────────────────────────────────────

if(SIERA_ENABLE_DRIVERS)
    if(SIERA_PLATFORM STREQUAL "ESP32")
        add_subdirectory(src/drivers/siera_drv_esp32)
    elseif(SIERA_PLATFORM STREQUAL "POSIX" OR SIERA_PLATFORM STREQUAL "SIMULATOR")
        add_subdirectory(src/drivers/posix)
    endif()
endif()

# ──────────────────────────────────────────────────────────────
# ESP-IDF: register all accumulated sources as the siera component
# ──────────────────────────────────────────────────────────────

if(ESP_PLATFORM)
    idf_component_register(
        SRCS         ${SIERA_SRCS}
        INCLUDE_DIRS ${SIERA_INCLUDE_DIRS}
    )
    return()
endif()

# ──────────────────────────────────────────────────────────────
# Tests and examples
# ──────────────────────────────────────────────────────────────

if(SIERA_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(SIERA_BUILD_EXAMPLES)
    add_subdirectory(examples/hello_siera)
endif()

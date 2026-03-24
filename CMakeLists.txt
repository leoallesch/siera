cmake_minimum_required(VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(SieraModule)

# ── Platform bootstrap (before project()) ─────────────────────────────────────
# Pass -DSIERA_PLATFORM=posix or -DSIERA_PLATFORM=esp32 at configure time.
# Omit when building SIERA as a pure library (tests only).
#
# Each cmake/platforms/*/<name>.cmake appends its name to SIERA_PLATFORMS and
# runs its SDK/toolchain bootstrap when SIERA_PLATFORM matches.
#
# When siera is used as a subdirectory the top-level CMakeLists.txt must run
# the bootstrap and call project() before add_subdirectory(siera).

if(ESP_PLATFORM AND NOT DEFINED SIERA_PLATFORM)
    set(SIERA_PLATFORM esp32)
endif()


if(DEFINED SIERA_PLATFORM)
    string(TOLOWER "${SIERA_PLATFORM}" _platform)
endif()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    if(DEFINED SIERA_PLATFORM)
        file(GLOB _platform_files
            "${CMAKE_CURRENT_LIST_DIR}/cmake/platforms/*/*.cmake")
        foreach(_f IN LISTS _platform_files)
            include("${_f}")
        endforeach()

        if(NOT "${_platform}" IN_LIST SIERA_PLATFORMS)
            message(FATAL_ERROR
                "SIERA: unknown platform '${SIERA_PLATFORM}'.\n"
                "Available: ${SIERA_PLATFORMS}")
        endif()
    endif()

    project(siera
        VERSION 0.2.0
        DESCRIPTION "Simple Interface for Embedded Runtime Applications"
        LANGUAGES C CXX
    )
endif()

set(CMAKE_C_STANDARD         11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD       20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Options ───────────────────────────────────────────────────────────────────

option(SIERA_BUILD_TESTS "Build unit tests" OFF)

include(SieraTest)

if(SIERA_BUILD_TESTS)
    enable_testing()
    set(TESTS OFF CACHE BOOL "" FORCE)
    add_subdirectory(lib/cpputest)
endif()

# ── siera INTERFACE library ───────────────────────────────────────────────────

add_library(siera INTERFACE)
target_compile_options(siera INTERFACE -Wall -Wextra)

add_subdirectory(src/core/siera_common)
add_subdirectory(src/core/siera_hal)
add_subdirectory(src/core/siera_slist)
add_subdirectory(src/core/siera_event)
add_subdirectory(src/core/siera_timer)
add_subdirectory(src/core/siera_fsm)
add_subdirectory(src/core/siera_ds)

# ── Platform driver (auto-selected) ──────────────────────────────────────────

if(DEFINED SIERA_PLATFORM)
    add_subdirectory(src/drivers/${_platform})
endif()

# ── Tests ─────────────────────────────────────────────────────────────────────

if(SIERA_BUILD_TESTS)
    add_subdirectory(tests)
endif()

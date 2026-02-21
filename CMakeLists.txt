cmake_minimum_required(VERSION 3.20)

# ──────────────────────────────────────────────────────────────
# Project metadata
# ──────────────────────────────────────────────────────────────

project(siera
    VERSION 0.1.0
    DESCRIPTION "Simple Interface for Embedded Runtime Applications"
    LANGUAGES C CXX
)

set(CMAKE_C_STANDARD   11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ──────────────────────────────────────────────────────────────
# Options
# ──────────────────────────────────────────────────────────────

option(SIERA_ENABLE_UI          "Enable UI module (agnostic display/view interfaces)" OFF)
option(SIERA_ENABLE_LVGL        "Enable LVGL integration (implies SIERA_ENABLE_UI)"    OFF)
option(SIERA_DRIVER_SIMULATOR   "Build simulator drivers for host testing"            OFF)
option(SIERA_BUILD_TESTS        "Build unit tests"                                    OFF)
option(SIERA_BUILD_EXAMPLES     "Build example applications"                         OFF)
option(SIERA_ENABLE_COVERAGE    "Enable gcov code coverage instrumentation"           OFF)

# LVGL implies UI
if(SIERA_ENABLE_LVGL)
    set(SIERA_ENABLE_UI ON CACHE BOOL "Forced by SIERA_ENABLE_LVGL" FORCE)
endif()

# ──────────────────────────────────────────────────────────────
# Source & include collection
# ──────────────────────────────────────────────────────────────

set(SRC_ROOT "${CMAKE_CURRENT_LIST_DIR}/src")

file(GLOB_RECURSE CORE_SOURCES    "${SRC_ROOT}/core/*.c")
file(GLOB_RECURSE UI_SOURCES      "${SRC_ROOT}/ui/*.c"   EXCLUDE REGEX ".*/lvgl/.*")
file(GLOB_RECURSE LVGL_WRAPPER_SOURCES "${SRC_ROOT}/ui/lvgl/*.c" EXCLUDE REGEX ".*/lvgl/lvgl/.*")
file(GLOB         SIMULATOR_SOURCES "${SRC_ROOT}/driver/simulator/*.c")

set(SIERA_SOURCES ${CORE_SOURCES})

set(SIERA_INCLUDE_DIRS
    ${SRC_ROOT}/core
    ${SRC_ROOT}/core/data_structures
    ${SRC_ROOT}/core/datastream
    ${SRC_ROOT}/core/event
    ${SRC_ROOT}/core/timer
    ${SRC_ROOT}/core/state_machine
)

if(SIERA_ENABLE_UI)
    list(APPEND SIERA_SOURCES      ${UI_SOURCES})
    list(APPEND SIERA_INCLUDE_DIRS
        ${SRC_ROOT}/ui
        ${SRC_ROOT}/ui/view
        ${SRC_ROOT}/ui/display
    )
    message(STATUS "SIERA: UI module enabled")
endif()

if(SIERA_DRIVER_SIMULATOR)
    list(APPEND SIERA_SOURCES      ${SIMULATOR_SOURCES})
    list(APPEND SIERA_INCLUDE_DIRS ${SRC_ROOT}/driver/simulator)
    message(STATUS "SIERA: Simulator drivers enabled")
endif()

# ──────────────────────────────────────────────────────────────
# ESP-IDF component mode
# ──────────────────────────────────────────────────────────────

if(ESP_PLATFORM)
    # ESP-IDF names the component after the folder → expected name = siera

    if(SIERA_ENABLE_UI)
        # We already have UI_SOURCES without lvgl/ subfolder
        list(APPEND SIERA_SOURCES ${UI_SOURCES})

        list(APPEND SIERA_INCLUDE_DIRS
            ${SRC_ROOT}/ui
            ${SRC_ROOT}/ui/view
            ${SRC_ROOT}/ui/display
            ${SRC_ROOT}/ui/lvgl
            ${SRC_ROOT}/ui/lvgl/components
        )
    endif()

    # LVGL is expected to come from REQUIRES
    idf_component_register(
        SRCS            ${SIERA_SOURCES}
        INCLUDE_DIRS    ${SIERA_INCLUDE_DIRS}
        REQUIRES        lvgl    # only added when actually used
    )

    return()   # ── stop processing here for ESP-IDF ──
endif()

# ──────────────────────────────────────────────────────────────
# Standalone / Simulator / Desktop build
# ──────────────────────────────────────────────────────────────

add_library(siera STATIC ${SIERA_SOURCES})

target_include_directories(siera
    PUBLIC  ${SIERA_INCLUDE_DIRS}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}   # useful for generated files if any
)

target_compile_options(siera PRIVATE
    -Wall
    -Wextra
    # -Wpedantic   # consider adding later, many embedded projects disable it
)

# ──────────────────────────────────────────────────────────────
# LVGL integration (standalone only)
# ──────────────────────────────────────────────────────────────

if(SIERA_ENABLE_LVGL)
    # LVGL configuration before adding subdirectory
    set(CONFIG_LV_BUILD_DEMOS       OFF CACHE BOOL "" FORCE)
    set(CONFIG_LV_BUILD_EXAMPLES    OFF CACHE BOOL "" FORCE)
    set(CONFIG_LV_USE_THORVG_INTERNAL OFF CACHE BOOL "" FORCE)

    add_subdirectory(${SRC_ROOT}/ui/lvgl/lvgl)

    list(APPEND SIERA_SOURCES ${LVGL_WRAPPER_SOURCES})
    target_sources(siera PRIVATE ${LVGL_WRAPPER_SOURCES})

    target_include_directories(siera PUBLIC
        ${SRC_ROOT}/ui/lvgl
        ${SRC_ROOT}/ui/lvgl/components
    )

    target_link_libraries(siera PUBLIC lvgl)

    message(STATUS "SIERA: LVGL integration enabled")
endif()

# ──────────────────────────────────────────────────────────────
# Test suite
# ──────────────────────────────────────────────────────────────

if(SIERA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ──────────────────────────────────────────────────────────────
# Optional: examples (not implemented yet)
# ──────────────────────────────────────────────────────────────

# if(SIERA_BUILD_EXAMPLES)
#     add_subdirectory(examples)
# endif()
cmake_minimum_required(VERSION 3.20)

project(siera
    VERSION 0.1.0
    LANGUAGES C CXX
    DESCRIPTION "Simple Interface for Embedded Runtime Applications"
)

# Global Settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Module Options
option(SIERA_ENABLE_UI "Build the UI module (agnostic display/view interfaces)" OFF)
option(SIERA_ENABLE_LVGL "Build the LVGL module (fetches LVGL, implies SIERA_ENABLE_UI)" OFF)

# Build Options
option(SIERA_BUILD_TESTS "Build unit tests" OFF)
option(SIERA_BUILD_EXAMPLES "Build examples" OFF)
option(SIERA_ENABLE_COVERAGE "Enable gcov code coverage instrumentation" OFF)

# LVGL module implies UI module
if(SIERA_ENABLE_LVGL)
    set(SIERA_ENABLE_UI ON CACHE BOOL "" FORCE)
endif()

# Enable simulator drivers when building examples
if(SIERA_BUILD_EXAMPLES)
    set(SIERA_DRIVER_SIMULATOR ON CACHE BOOL "" FORCE)
endif()

# Core is always built
add_subdirectory(src/core)

# Drivers (platform-specific implementations)
add_subdirectory(src/driver)

# UI module (agnostic interfaces)
if(SIERA_ENABLE_UI)
    add_subdirectory(src/ui)
endif()

# LVGL module (fetches and wraps LVGL)
if(SIERA_ENABLE_LVGL)
    add_subdirectory(src/lvgl)
endif()

# Testing
if(SIERA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(SIERA_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

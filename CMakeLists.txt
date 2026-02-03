cmake_minimum_required(VERSION 3.14)

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
option(SIERA_ENABLE_UI "Build the UI module (includes LVGL)" OFF)

# Build Options
option(SIERA_BUILD_TESTS "Build unit tests" OFF)
option(SIERA_BUILD_EXAMPLES "Build examples" OFF)

# Core is always built
add_subdirectory(src/core)

# UI is optional
if(SIERA_ENABLE_UI)
    add_subdirectory(src/ui)
endif()

# Testing
if(SIERA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(SIERA_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

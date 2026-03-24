cmake_minimum_required(VERSION 3.16)

set(_siera_root ${CMAKE_CURRENT_SOURCE_DIR}/../../)

include(${_siera_root}/cmake/SieraPlatform.cmake)

project(multi_target C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LVGL_VERSION "v9.4.0" CACHE STRING "LVGL version to fetch")
set(LV_BUILD_CONF_PATH "${CMAKE_SOURCE_DIR}/platform/${_platform}/lv_conf_simulator.h" CACHE STRING "" FORCE)

    # LVGL configuration before adding subdirectory
    set(CONFIG_LV_BUILD_DEMOS         OFF CACHE BOOL "" FORCE)
    set(CONFIG_LV_BUILD_EXAMPLES      OFF CACHE BOOL "" FORCE)
    set(CONFIG_LV_USE_THORVG_INTERNAL OFF CACHE BOOL "" FORCE)

    include(FetchContent)
    FetchContent_Declare(lvgl
        GIT_REPOSITORY https://github.com/lvgl/lvgl.git
        GIT_TAG        ${LVGL_VERSION}
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(lvgl)


set(SIERA_ENABLE_DRIVERS ON CACHE BOOL "" FORCE)
set(SIERA_ENABLE_UI ON CACHE BOOL "" FORCE)
add_subdirectory(${_siera_root} ${CMAKE_BINARY_DIR}/siera)

add_subdirectory(app)
add_subdirectory(platform/${_platform})

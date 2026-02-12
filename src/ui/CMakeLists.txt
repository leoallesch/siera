# SIERA UI Module (fetches and contains LVGL)

include(FetchContent)

set(LVGL_VERSION "v9.4.0" CACHE STRING "LVGL version to fetch")

set(CONFIG_LV_BUILD_DEMOS OFF)
set(CONFIG_LV_BUILD_EXAMPLES OFF)
set(CONFIG_LV_USE_THORVG_INTERNAL OFF)
set(LV_BUILD_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h" CACHE STRING "" FORCE)

FetchContent_Declare(
    lvgl
    GIT_REPOSITORY https://github.com/lvgl/lvgl.git
    GIT_TAG        ${LVGL_VERSION}
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(lvgl)

file(GLOB_RECURSE SIERA_UI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_library(siera_ui ${SIERA_UI_SOURCES})
add_library(siera::ui ALIAS siera_ui)

target_link_libraries(siera_ui
    PUBLIC
        lvgl
        siera::core
)

target_include_directories(siera_ui
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/view>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/display>
)

message(STATUS "SIERA: UI module enabled (includes LVGL ${LVGL_VERSION})")

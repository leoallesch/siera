# SIERA LVGL Module (fetches and wraps LVGL)

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

add_library(siera_lvgl INTERFACE)
add_library(siera::lvgl ALIAS siera_lvgl)

target_link_libraries(siera_lvgl
    INTERFACE
        lvgl
        siera::ui
)

target_include_directories(siera_lvgl
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

message(STATUS "SIERA: LVGL module enabled (LVGL ${LVGL_VERSION})")

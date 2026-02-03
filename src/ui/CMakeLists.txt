# SIERA UI Module (fetches and contains LVGL)

include(FetchContent)

set(LVGL_VERSION "v9.4.0" CACHE STRING "LVGL version to fetch")

# Fetch LVGL
set(CONFIG_LV_BUILD_DEMOS OFF) # for demos
set(CONFIG_LV_BUILD_EXAMPLES OFF) # for examples
set(CONFIG_LV_USE_THORVG_INTERNAL OFF) # for ThorVG
set(LV_BUILD_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h" CACHE STRING "" FORCE)

FetchContent_Declare(
    lvgl
    GIT_REPOSITORY https://github.com/lvgl/lvgl.git
    GIT_TAG        ${LVGL_VERSION}
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(lvgl)

# UI module (currently interface-only, will add sources as wrappers are created)
add_library(siera_ui INTERFACE)
add_library(siera::ui ALIAS siera_ui)

target_link_libraries(siera_ui
    INTERFACE
        lvgl
        siera::core
)

target_include_directories(siera_ui
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

message(STATUS "SIERA: UI module enabled (includes LVGL ${LVGL_VERSION})")

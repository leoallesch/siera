# SIERA UI Module (toolkit-agnostic interfaces)

file(GLOB_RECURSE SIERA_UI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_library(siera_ui ${SIERA_UI_SOURCES})
add_library(siera::ui ALIAS siera_ui)

target_link_libraries(siera_ui
    PUBLIC
        siera::core
)

target_include_directories(siera_ui
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/view>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/display>
)

message(STATUS "SIERA: UI module enabled")

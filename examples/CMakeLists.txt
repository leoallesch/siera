# SIERA Examples

add_subdirectory(hello_world)

if(SIERA_ENABLE_LVGL)
    add_subdirectory(simulator)
endif()
# SIERA Examples

add_subdirectory(hello_world)

if(SIERA_ENABLE_UI)
    add_subdirectory(simulator)
endif()
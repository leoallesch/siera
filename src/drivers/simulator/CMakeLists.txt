siera_add_module(
    SRCS     sim_timesource.c
             sim_rtc.c
    INCLUDES include
)

if(TARGET lvgl)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)

    target_sources(siera INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/sim.c
        ${CMAKE_CURRENT_SOURCE_DIR}/sim_ui.c
        ${CMAKE_CURRENT_SOURCE_DIR}/sim_display.c
    )
    target_link_libraries(siera INTERFACE lvgl ${SDL2_LIBRARIES})
    target_include_directories(siera INTERFACE ${SDL2_INCLUDE_DIRS})
endif()

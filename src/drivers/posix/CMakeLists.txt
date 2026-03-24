add_library(siera_drv_posix STATIC
    hal_timesource.c
)
add_library(siera::posix ALIAS siera_drv_posix)

target_include_directories(siera_drv_posix PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(siera_drv_posix PUBLIC siera)

if(TARGET lvgl)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)

    target_sources(siera_drv_posix PRIVATE posix_sim.c posix_display.c)
    target_link_libraries(siera_drv_posix PUBLIC lvgl ${SDL2_LIBRARIES})
    target_include_directories(siera_drv_posix PUBLIC ${SDL2_INCLUDE_DIRS})
endif()

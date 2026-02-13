# Simulator Example (LVGL + SDL)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

add_subdirectory(ui)

add_executable(simulator
    main.c
    hello_view.c
)

target_link_libraries(simulator
    PRIVATE
        siera::lvgl
        siera::driver
        lib-ui
        ${SDL2_LIBRARIES}
)

target_include_directories(simulator
    PRIVATE
        ${SDL2_INCLUDE_DIRS}
)

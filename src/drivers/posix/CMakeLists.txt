add_library(siera_drv_posix STATIC
    hal_timesource.c
)
add_library(siera::posix ALIAS siera_drv_posix)

target_include_directories(siera_drv_posix PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(siera_drv_posix PUBLIC siera)

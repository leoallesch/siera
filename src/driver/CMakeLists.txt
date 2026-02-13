# SIERA Driver Module

# Driver type options
option(SIERA_DRIVER_SIMULATOR "Build simulator drivers for host testing" OFF)

# Collect driver sources based on selected options
set(SIERA_DRIVER_SOURCES "")
set(SIERA_DRIVER_INCLUDE_DIRS "")

# Simulator drivers (for host-based testing)
if(SIERA_DRIVER_SIMULATOR)
    file(GLOB SIMULATOR_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/simulator/*.c")
    list(APPEND SIERA_DRIVER_SOURCES ${SIMULATOR_SOURCES})
    list(APPEND SIERA_DRIVER_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/simulator")
    message(STATUS "SIERA: Simulator drivers enabled")
endif()

# Only create the library if at least one driver is enabled
if(SIERA_DRIVER_SOURCES)
    add_library(siera_driver ${SIERA_DRIVER_SOURCES})
    add_library(siera::driver ALIAS siera_driver)

    target_include_directories(siera_driver
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<BUILD_INTERFACE:${SIERA_DRIVER_INCLUDE_DIRS}>
    )

    target_link_libraries(siera_driver
        PUBLIC
            siera::core
            siera::lvgl
    )

    target_compile_options(siera_driver
        PRIVATE
            $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
    )

    if(SIERA_DRIVER_SIMULATOR)
        target_compile_definitions(siera_driver PRIVATE SIERA_PLATFORM_SIMULATOR)
    endif()
else()
    message(STATUS "SIERA: No drivers enabled (use -DSIERA_DRIVER_SIMULATOR=ON)")
endif()

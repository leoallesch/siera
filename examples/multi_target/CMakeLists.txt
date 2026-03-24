cmake_minimum_required(VERSION 3.16)

# Select platform at configure time:
#   cmake -B build/posix -DSIERA_PLATFORM=posix
#   cmake -B build/esp32 -DSIERA_PLATFORM=esp32 -DIDF_TARGET=esp32 -G Ninja

set(_siera_root ${CMAKE_CURRENT_SOURCE_DIR}/../../)

include(${_siera_root}/cmake/SieraPlatform.cmake)

project(multi_target C CXX ASM)

add_subdirectory(${_siera_root} ${CMAKE_BINARY_DIR}/siera)

add_subdirectory(app)
add_subdirectory(platform/${_platform})

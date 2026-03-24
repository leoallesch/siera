cmake_minimum_required(VERSION 3.16)

set(_siera_root ${CMAKE_CURRENT_SOURCE_DIR}/../../)

include(${_siera_root}/cmake/SieraPlatform.cmake)

project(multi_target C CXX)

add_subdirectory(${_siera_root} ${CMAKE_BINARY_DIR}/siera)

add_subdirectory(app)
add_subdirectory(platform/${_platform})

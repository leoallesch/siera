cmake_minimum_required(VERSION 3.16)

set(_siera_root ${CMAKE_CURRENT_SOURCE_DIR}/../../)

include(${_siera_root}/cmake/SieraPlatform.cmake)

project(hello_siera C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SIERA_ENABLE_DRIVERS ON CACHE BOOL "" FORCE)

add_subdirectory(${_siera_root} ${CMAKE_BINARY_DIR}/siera)

add_executable(hello_siera main.c)
target_link_libraries(hello_siera PRIVATE siera)

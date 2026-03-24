cmake_minimum_required(VERSION 3.14)

project(hello_siera)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../ ${CMAKE_BINARY_DIR}/siera)

add_executable(hello_siera main.c)
target_include_directories(hello_siera PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(hello_siera PRIVATE siera)

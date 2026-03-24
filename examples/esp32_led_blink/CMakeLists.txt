cmake_minimum_required(VERSION 3.16)

# Build with: cmake -B build -DSIERA_PLATFORM=esp32 -DIDF_TARGET=esp32 -G Ninja

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../ ${CMAKE_BINARY_DIR}/siera)

project(esp32_led_blink C)

add_subdirectory(main)

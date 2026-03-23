cmake_minimum_required(VERSION 3.16)

# Tell ESP-IDF where to find the siera core component (repo root) and the
# ESP32 driver component.  Both must be listed before include(project.cmake)
# so the build system can resolve component dependencies.
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../")

set(SIERA_ENABLE_DRIVERS ON  CACHE BOOL "" FORCE)
set(SIERA_PLATFORM       ESP32 CACHE STRING "" FORCE)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(esp32_led_blink)

list(APPEND SIERA_PLATFORMS "esp32")

if(SIERA_PLATFORM STREQUAL "esp32")
    message(STATUS "SIERA: building for esp32 (ESP-IDF bootstrap)")

    if(NOT DEFINED ENV{IDF_PATH})
        message(FATAL_ERROR
            "SIERA platform esp32 requires IDF_PATH to be set.\n"
            "Run: source \$IDF_PATH/export.sh  before configuring.")
    endif()

    if(NOT DEFINED IDF_TARGET)
        message(STATUS "IDF_TARGET not set — defaulting to esp32")
        set(IDF_TARGET "esp32" CACHE STRING "IDF target chip" FORCE)
    endif()

    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
endif()

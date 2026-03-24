SIERA_ROOT ?= ../..
BUILD_DIR  ?= build

# --- posix target ---
POSIX_PLATFORM    := posix
POSIX_EXECUTABLE  := platform/posix/multi_target

# --- esp32 target ---
ESP32_PLATFORM   := esp32
ESP32_IDF_PATH   ?= $(HOME)/dev/lib/esp/esp-idf
ESP32_IDF_TARGET ?= esp32c6
ESP32_PORT       ?= /dev/ttyUSB0
# ESP32_CMAKE_ARGS := -DSOME_OPTION=ON

include $(SIERA_ROOT)/siera.mk

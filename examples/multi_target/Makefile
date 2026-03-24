BUILD_DIR := build

IDF_PATH  ?= $(HOME)/dev/lib/esp/esp-idf
IDF_TOOLS ?= $(HOME)/.espressif


IDF_ENV := IDF_PATH=$(IDF_PATH) \
           IDF_PYTHON_ENV_PATH=$(IDF_VENV) \
           PATH="$(IDF_VENV)/bin:$(XTENSA_BIN):$(PATH)"

PORT ?= /dev/ttyUSB0

.PHONY: posix esp32 menuconfig flash monitor clean

posix:
	cmake -B $(BUILD_DIR)/posix -DSIERA_PLATFORM=posix
	cmake --build $(BUILD_DIR)/posix -- -j$(shell nproc)

esp32:
	$(IDF_ENV) cmake -B $(BUILD_DIR)/esp32 -DSIERA_PLATFORM=esp32 -DIDF_TARGET=esp32 -G Ninja
	$(IDF_ENV) cmake --build $(BUILD_DIR)/esp32 -- -j$(shell nproc)

flash: $(BUILD_DIR)/esp32/build.ninja
	$(IDF_ENV) cmake --build $(BUILD_DIR)/esp32 --target flash -- ESPPORT=$(PORT)

monitor: $(BUILD_DIR)/esp32/build.ninja
	$(IDF_ENV) cmake --build $(BUILD_DIR)/esp32 --target monitor -- ESPPORT=$(PORT)

menuconfig: $(BUILD_DIR)/esp32/build.ninja
	$(IDF_ENV) cmake --build $(BUILD_DIR)/esp32 --target menuconfig

$(BUILD_DIR)/esp32/build.ninja:
	$(IDF_ENV) cmake -B $(BUILD_DIR)/esp32 -DSIERA_PLATFORM=esp32 -DIDF_TARGET=esp32 -G Ninja

clean:
	rm -rf $(BUILD_DIR)

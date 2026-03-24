# SIERA Build Shortcuts
# Wraps cmake commands for convenience

BUILD_DIR := build

.PHONY: all tests examples simulator clean rebuild

# Default: configure and build core library
all:
	cmake -B $(BUILD_DIR) -DSIERA_BUILD_TESTS=OFF
	cmake --build $(BUILD_DIR) -- -j$(shell nproc)

# Build and run unit tests
tests:
	cmake -B $(BUILD_DIR) -DSIERA_BUILD_TESTS=ON
	cmake --build $(BUILD_DIR) -- -j$(shell nproc)
	ctest --test-dir $(BUILD_DIR) --output-on-failure --verbose -j$(shell nproc)

# Build multi_target example (simulator / posix host)
examples:
	cmake -B build/multi_target -S examples/multi_target -DSIERA_PLATFORM=posix
	cmake --build build/multi_target -- -j$(shell nproc)

# Build simulator example
simulator:
	cmake -B build/simulator -S examples/simulator -DSIERA_PLATFORM=posix
	cmake --build build/simulator -- -j$(shell nproc)

# Clean build directories
clean:
	rm -rf $(BUILD_DIR) build/multi_target build/simulator build/esp32

# Clean and rebuild
rebuild: clean all

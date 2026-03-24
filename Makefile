# SIERA Build Shortcuts
# Wraps cmake commands for convenience

BUILD_DIR := build

.PHONY: all tests examples clean rebuild

# Default: configure and build core library
all:
	cmake -B $(BUILD_DIR) -DSIERA_BUILD_TESTS=OFF
	cmake --build $(BUILD_DIR) -- -j$(shell nproc)

# Build and run unit tests
tests:
	cmake -B $(BUILD_DIR) -DSIERA_BUILD_TESTS=ON
	cmake --build $(BUILD_DIR) -- -j$(shell nproc)
	ctest --test-dir $(BUILD_DIR) --output-on-failure --verbose -j$(shell nproc)

# Build POSIX example
examples:
	cmake -B build/posix -S examples/multi_target -DSIERA_PLATFORM=posix
	cmake --build build/posix -- -j$(shell nproc)

# Clean build directories
clean:
	rm -rf $(BUILD_DIR) build/posix build/esp32

# Clean and rebuild
rebuild: clean all

# SIERA Build Shortcuts
# Wraps cmake commands for convenience

BUILD_DIR := build
MAKEFLAGS += -j$(shell nproc)

.PHONY: all tests examples clean rebuild

# Default: configure and build all core libraries
all:
	cmake -B $(BUILD_DIR) -DSIERA_BUILD_TESTS=OFF -DSIERA_BUILD_EXAMPLES=OFF
	cmake --build $(BUILD_DIR) -- -j$(shell nproc)

# Build and run unit tests
tests:
	cmake -B $(BUILD_DIR) -DSIERA_BUILD_TESTS=ON -DSIERA_BUILD_EXAMPLES=OFF
	cmake --build $(BUILD_DIR) -- -j$(shell nproc)
	ctest --test-dir $(BUILD_DIR) --output-on-failure --verbose -j$(shell nproc)

examples:
	cmake -B $(BUILD_DIR) -DSIERA_BUILD_EXAMPLES=ON -DSIERA_BUILD_TESTS=OFF
	cmake --build $(BUILD_DIR) -- -j$(shell nproc)

# Clean build directory
clean:
	rm -rf $(BUILD_DIR)

# Clean and rebuild
rebuild: clean all

# SIERA Build Shortcuts
# Wraps cmake commands for convenience

BUILD_DIR := build

.PHONY: all core ui tests clean rebuild examples

# Default: build core only
all: core

# Build core module only
core:
	cmake -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR)

# Build with UI module (includes LVGL)
ui:
	cmake -B $(BUILD_DIR) -DSIERA_ENABLE_UI=ON
	cmake --build $(BUILD_DIR)

# Build examples
examples:
	cmake -B $(BUILD_DIR) -DSIERA_BUILD_EXAMPLES=ON
	cmake --build $(BUILD_DIR)

# Build with tests
tests:
	cmake -B $(BUILD_DIR) -DSIERA_BUILD_TESTS=ON
	cmake --build $(BUILD_DIR)
	ctest --test-dir $(BUILD_DIR) --output-on-failure

# Clean build directory
clean:
	rm -rf $(BUILD_DIR)

# Clean and rebuild
rebuild: clean all

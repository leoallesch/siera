# SIERA Build Shortcuts
# Wraps cmake commands for convenience

BUILD_DIR := build
COVERAGE_DIR := $(BUILD_DIR)/coverage-report

.PHONY: all core ui tests clean rebuild examples simulator coverage

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
	ctest --test-dir $(BUILD_DIR) --output-on-failure --verbose

# Build simulator example (LVGL + SDL)
simulator:
	cmake -B $(BUILD_DIR) -DSIERA_BUILD_EXAMPLES=ON -DSIERA_ENABLE_UI=ON
	cmake --build $(BUILD_DIR)

# Build and run tests with gcov coverage, generate HTML report via lcov
coverage:
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -DSIERA_BUILD_TESTS=ON -DSIERA_ENABLE_COVERAGE=ON
	cmake --build $(BUILD_DIR)
	ctest --test-dir $(BUILD_DIR) --output-on-failure
	lcov --capture --directory $(BUILD_DIR) --output-file $(BUILD_DIR)/coverage.info \
	     --ignore-errors inconsistent,inconsistent
	lcov --remove $(BUILD_DIR)/coverage.info \
	     '*/_deps/*' \
	     '/usr/*' \
	     --output-file $(BUILD_DIR)/coverage.info \
	     --ignore-errors inconsistent,inconsistent,unused
	genhtml $(BUILD_DIR)/coverage.info --output-directory $(COVERAGE_DIR)
	@echo "Coverage report: $(COVERAGE_DIR)/index.html"

# Clean build directory
clean:
	rm -rf $(BUILD_DIR)

# Clean and rebuild
rebuild: clean all

# Siera Build System

Siera follows target-centric modern CMake (3.20+). Every artifact is a named target with
explicit `target_*` properties — no global `include_directories`, no `add_compile_options`,
no `link_libraries`. Siera is a library collection consumed via `add_subdirectory`, never
installed or found with `find_package`. It provides reusable modules and shared platform
drivers; consumer projects provide the platform-specific executables.

## 1. Project Structure

### 1.1 Siera (library)

```text
siera/
├── CMakeLists.txt              # Root: project setup, options, conditional subdirs
├── Makefile                    # Convenience wrapper for common cmake invocations
├── src/
│   ├── core/                   # siera::core  (always built)
│   │   ├── CMakeLists.txt
│   │   ├── data_structures/
│   │   ├── database/
│   │   ├── event/
│   │   ├── hal/
│   │   ├── state_machine/
│   │   └── timer/
│   ├── driver/                 # siera::driver (conditional per platform)
│   │   ├── CMakeLists.txt
│   │   └── simulator/          # SDL2 display + timesource (host)
│   ├── ui/                     # siera::ui    (optional, UI-agnostic interfaces)
│   │   ├── CMakeLists.txt
│   │   ├── display/
│   │   └── view/
│   └── lvgl/                   # siera::lvgl  (optional, fetches LVGL)
│       ├── CMakeLists.txt
│       └── lv_conf.h
├── tests/
│   ├── CMakeLists.txt          # test_core executable (CppUTest)
│   ├── core/                   # Test source files
│   ├── doubles/                # Test doubles / fakes
│   └── TestRunner.cpp
├── examples/
│   ├── CMakeLists.txt
│   ├── hello_world/            # Core-only example (no UI)
│   └── simulator/              # UI + SDL2 example
├── doc/
└── scripts/
```

### 1.2 Consumer Project (versevault-ui)

```text
versevault-ui/
├── CMakeLists.txt              # Root: pulls in siera, defines app + platform targets
├── Makefile                    # Convenience wrapper
├── lib/
│   └── siera/                  # Git submodule
├── src/
│   ├── app/                    # Platform-agnostic application library
│   │   ├── application.c/.h
│   │   ├── presenter.c/.h
│   │   ├── key_manager/
│   │   └── view/               # View implementations (home, browse, ...)
│   ├── platform/
│   │   ├── simulator/          # Host simulator executable
│   │   │   ├── main.c
│   │   │   └── bsp_config.h
│   │   └── esp32/              # Embedded target executable
│   │       ├── main.c
│   │       └── bsp_config.h
│   └── ui/                     # XML-defined UI, code-generated sources
│       ├── CMakeLists.txt
│       ├── globals.xml
│       ├── file_list_gen.cmake
│       └── components/
├── tests/                      # Consumer test sources (compiled into siera test executable)
│   ├── app/                    # Tests for application logic
│   │   ├── test_presenter.cpp
│   │   └── test_key_manager.cpp
│   └── doubles/                # Consumer-specific test doubles
│       └── ...
└── scripts/
```

The split is deliberate: siera owns the framework (core data structures, HAL interfaces,
LVGL integration, platform drivers). The consumer owns the application logic, UI
definitions, and platform-specific entry points.


## 2. Siera Root CMakeLists.txt

The root file declares the project, sets language standards, defines user-facing options,
and conditionally includes subdirectories.

```cmake
cmake_minimum_required(VERSION 3.20)

project(siera
    VERSION 0.1.0
    LANGUAGES C CXX
    DESCRIPTION "Simple Interface for Embedded Runtime Applications"
)

# Language standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Module options
option(SIERA_ENABLE_UI "Build the UI module (agnostic display/view interfaces)" OFF)
option(SIERA_ENABLE_LVGL "Build the LVGL module (fetches LVGL, implies SIERA_ENABLE_UI)" OFF)

# Build options
option(SIERA_BUILD_TESTS "Build unit tests" OFF)
option(SIERA_BUILD_EXAMPLES "Build examples" OFF)

# LVGL module implies UI module
if(SIERA_ENABLE_LVGL)
    set(SIERA_ENABLE_UI ON CACHE BOOL "" FORCE)
endif()

# Building examples automatically enables the simulator driver
if(SIERA_BUILD_EXAMPLES)
    set(SIERA_DRIVER_SIMULATOR ON CACHE BOOL "" FORCE)
endif()

# Core is always built
add_subdirectory(src/core)

# Drivers (platform-specific implementations, conditional on SIERA_DRIVER_* options)
add_subdirectory(src/driver)

# UI module (agnostic interfaces)
if(SIERA_ENABLE_UI)
    add_subdirectory(src/ui)
endif()

# LVGL module (fetches and wraps LVGL)
if(SIERA_ENABLE_LVGL)
    add_subdirectory(src/lvgl)
endif()

# Tests
if(SIERA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
if(SIERA_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

Key points:

- **No `GNUInstallDirs`, no `install()` commands.** Siera is not installed; it lives
  inside consumer build trees via `add_subdirectory`.
- **Options are namespaced** with `SIERA_` to avoid collisions when embedded in a larger
  project.
- **Driver subdirectory is always added** but builds nothing unless a `SIERA_DRIVER_*`
  option is enabled. This keeps the option definition inside `src/driver/CMakeLists.txt`
  where it belongs.
- **Implicit dependencies** between options (examples need simulator driver, LVGL
  implies UI) are handled with `set(... CACHE BOOL "" FORCE)`.
- **UI and LVGL are separate modules.** `siera::ui` provides toolkit-agnostic
  interfaces (display, view). `siera::lvgl` fetches LVGL and re-exports it alongside
  `siera::ui`. Code that does not need LVGL links only `siera::ui`.
- **Consumer testing** is supported by populating `SIERA_CONSUMER_TEST_*` variables
  before `add_subdirectory`. Consumer test sources are compiled into the siera test
  executable so everything runs in a single `ctest` invocation.


## 3. Defining Library Modules

Each module is a separate static library with a namespaced alias.

### 3.1 Core Module (`src/core/CMakeLists.txt`)

The core module is always built. It contains data structures, events, timers, state
machines, the database abstraction, and HAL interfaces.

```cmake
file(GLOB_RECURSE SIERA_CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_library(siera_core ${SIERA_CORE_SOURCES})
add_library(siera::core ALIAS siera_core)

target_include_directories(siera_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/data_structures>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/database>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/event>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/timer>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/state_machine>
)

target_compile_options(siera_core
    PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
)
```

Notes:

- `PUBLIC` include directories propagate to anything that links `siera::core`.
- `$<BUILD_INTERFACE:...>` ensures paths are only used during build, not if the target
  were ever exported (future-proofing).
- Compiler warnings are `PRIVATE` so they do not leak to consumers.

### 3.2 Driver Module (`src/driver/CMakeLists.txt`)

Drivers are platform-specific implementations of siera HAL interfaces. Each platform
has its own `SIERA_DRIVER_*` option and subdirectory. The library is only created
when at least one driver is enabled.

```cmake
# Per-platform driver options
option(SIERA_DRIVER_SIMULATOR "Build simulator drivers for host testing" OFF)

# Collect sources and include paths from enabled drivers
set(SIERA_DRIVER_SOURCES "")
set(SIERA_DRIVER_INCLUDE_DIRS "")

# --- Simulator drivers (host-based testing with SDL2) ---
if(SIERA_DRIVER_SIMULATOR)
    file(GLOB SIMULATOR_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/simulator/*.c")
    list(APPEND SIERA_DRIVER_SOURCES ${SIMULATOR_SOURCES})
    list(APPEND SIERA_DRIVER_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/simulator")
    message(STATUS "SIERA: Simulator drivers enabled")
endif()

# --- Future: ESP32 drivers ---
# if(SIERA_DRIVER_ESP32)
#     file(GLOB ESP32_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/esp32/*.c")
#     list(APPEND SIERA_DRIVER_SOURCES ${ESP32_SOURCES})
#     list(APPEND SIERA_DRIVER_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/esp32")
#     message(STATUS "SIERA: ESP32 drivers enabled")
# endif()

# Only create the library if at least one driver is enabled
if(SIERA_DRIVER_SOURCES)
    add_library(siera_driver ${SIERA_DRIVER_SOURCES})
    add_library(siera::driver ALIAS siera_driver)

    target_include_directories(siera_driver
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<BUILD_INTERFACE:${SIERA_DRIVER_INCLUDE_DIRS}>
    )

    target_link_libraries(siera_driver
        PUBLIC
            siera::core
            siera::lvgl
    )

    target_compile_options(siera_driver
        PRIVATE
            $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
    )

    if(SIERA_DRIVER_SIMULATOR)
        target_compile_definitions(siera_driver PRIVATE SIERA_PLATFORM_SIMULATOR)
    endif()
else()
    message(STATUS "SIERA: No drivers enabled (use -DSIERA_DRIVER_SIMULATOR=ON)")
endif()
```

This pattern scales naturally: adding ESP32 or any new platform means adding another
`if(SIERA_DRIVER_<PLATFORM>)` block that appends sources and includes.

### 3.3 UI Module (`src/ui/CMakeLists.txt`)

The UI module provides toolkit-agnostic interfaces for displays and views. It has no
dependency on LVGL or any concrete graphics library.

```cmake
file(GLOB_RECURSE SIERA_UI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_library(siera_ui ${SIERA_UI_SOURCES})
add_library(siera::ui ALIAS siera_ui)

target_link_libraries(siera_ui
    PUBLIC
        siera::core
)

target_include_directories(siera_ui
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/view>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/display>
)
```

This module defines `i_view_t` (load/unload lifecycle) and `i_display_t` (display
update abstraction). Application code that only needs these interfaces links
`siera::ui` without pulling in LVGL.

### 3.4 LVGL Module (`src/lvgl/CMakeLists.txt`)

The LVGL module fetches LVGL via `FetchContent` and re-exports it alongside the
agnostic UI interfaces. Enabling `SIERA_ENABLE_LVGL` automatically enables
`SIERA_ENABLE_UI`.

```cmake
include(FetchContent)

set(LVGL_VERSION "v9.4.0" CACHE STRING "LVGL version to fetch")

# Disable LVGL extras we do not need
set(CONFIG_LV_BUILD_DEMOS OFF)
set(CONFIG_LV_BUILD_EXAMPLES OFF)
set(CONFIG_LV_USE_THORVG_INTERNAL OFF)
set(LV_BUILD_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h" CACHE STRING "" FORCE)

FetchContent_Declare(
    lvgl
    GIT_REPOSITORY https://github.com/lvgl/lvgl.git
    GIT_TAG        ${LVGL_VERSION}
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(lvgl)

add_library(siera_lvgl INTERFACE)
add_library(siera::lvgl ALIAS siera_lvgl)

target_link_libraries(siera_lvgl
    INTERFACE
        lvgl
        siera::ui
)

target_include_directories(siera_lvgl
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

message(STATUS "SIERA: LVGL module enabled (LVGL ${LVGL_VERSION})")
```

`siera::lvgl` is an `INTERFACE` library — it has no source files of its own. It bundles
the fetched `lvgl` target with `siera::ui` so that consumers get everything through a
single link. The `lv_conf.h` file in this directory controls LVGL's compile-time
configuration for all consumers.

Targets that need LVGL (drivers, consumer UI code) link `siera::lvgl`. Targets that
only need the abstract display/view interfaces link `siera::ui`.


## 4. Tests and Examples

Tests and examples are the only executables defined inside siera.

### 4.1 Tests (`tests/CMakeLists.txt`)

CppUTest is fetched via `FetchContent`. Test doubles live alongside test sources.
Consumer projects can inject their own test sources, doubles, include paths, and link
libraries into the test executable by populating variables before enabling
`SIERA_BUILD_TESTS`.

```cmake
include(FetchContent)

FetchContent_Declare(
    cpputest
    GIT_REPOSITORY https://github.com/cpputest/cpputest.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)
set(TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(cpputest)

# Siera's own test sources
file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp")
file(GLOB_RECURSE DOUBLE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/doubles/*.c")

add_executable(test_core
    ${TEST_SOURCES}
    ${DOUBLE_SOURCES}
    ${SIERA_CONSUMER_TEST_SOURCES}
    ${SIERA_CONSUMER_DOUBLE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/TestRunner.cpp
)

target_include_directories(test_core
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/doubles/core/timer
        ${SIERA_CONSUMER_TEST_INCLUDE_DIRS}
)

target_link_libraries(test_core
    PRIVATE
        CppUTest
        CppUTestExt
        siera::core
        ${SIERA_CONSUMER_TEST_LINK_LIBRARIES}
)

add_test(NAME test_core COMMAND test_core)
```

The `set(TESTS OFF ...)` before `FetchContent_MakeAvailable` prevents CppUTest from
building its own internal tests.

**Consumer test variables.** Before `add_subdirectory(lib/siera)` (with tests enabled),
the consumer populates these variables:

| Variable | Purpose |
|---|---|
| `SIERA_CONSUMER_TEST_SOURCES` | Consumer `.cpp` test files |
| `SIERA_CONSUMER_DOUBLE_SOURCES` | Consumer `.c` test double files |
| `SIERA_CONSUMER_TEST_INCLUDE_DIRS` | Include paths for consumer doubles |
| `SIERA_CONSUMER_TEST_LINK_LIBRARIES` | Consumer libraries to link (e.g., the app library) |

When these variables are empty (siera built standalone), the test executable contains
only siera's own tests. When a consumer populates them, CppUTest's `RunAllTests`
auto-discovers all TEST_GROUPs at link time — both siera and consumer tests run in a
single executable with unified output.

### 4.2 Examples (`examples/CMakeLists.txt`)

```cmake
add_subdirectory(hello_world)

if(SIERA_ENABLE_LVGL)
    add_subdirectory(simulator)
endif()
```

**hello_world** is a minimal core-only example:

```cmake
add_executable(hello_world main.c)

target_link_libraries(hello_world
    PRIVATE
        siera::core
        siera::driver
)
```

**simulator** demonstrates the full UI stack with SDL2:

```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

add_executable(simulator main.c hello_view.c)

target_link_libraries(simulator
    PRIVATE
        siera::lvgl
        siera::driver
        ${SDL2_LIBRARIES}
)

target_include_directories(simulator
    PRIVATE
        ${SDL2_INCLUDE_DIRS}
)
```

Examples serve as reference implementations for consumer projects. The simulator example
in particular mirrors the pattern used by consumer projects like versevault-ui.


## 5. Consumer Project Integration

A consumer project pulls in siera as a git submodule and uses `add_subdirectory`.
The consumer forces the siera options it needs, defines its own application library,
and creates platform-specific executables.

### 5.1 Root CMakeLists.txt (versevault-ui)

```cmake
cmake_minimum_required(VERSION 3.20)

project(versevault-ui
    VERSION 0.1.0
    LANGUAGES C CXX
    DESCRIPTION "VerseVault UI Application"
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

# Force siera options before pulling it in
set(SIERA_ENABLE_LVGL ON CACHE BOOL "" FORCE)
set(SIERA_DRIVER_SIMULATOR ON CACHE BOOL "" FORCE)

# --- Testing (consumer tests compiled into siera test executable) ---
option(VERSEVAULT_BUILD_TESTS "Build unit tests" OFF)

if(VERSEVAULT_BUILD_TESTS)
    set(SIERA_BUILD_TESTS ON CACHE BOOL "" FORCE)

    file(GLOB_RECURSE SIERA_CONSUMER_TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/*.cpp")
    file(GLOB_RECURSE SIERA_CONSUMER_DOUBLE_SOURCES "${CMAKE_SOURCE_DIR}/tests/doubles/*.c")

    set(SIERA_CONSUMER_TEST_INCLUDE_DIRS
        "${CMAKE_SOURCE_DIR}/tests/doubles"
    )

    # Populated after versevault-app is defined (below)
    set(SIERA_CONSUMER_TEST_LINK_LIBRARIES versevault-app)
endif()

add_subdirectory(lib/siera)

# --- UI Code Generation (XML -> C) ---
find_program(NODE_EXECUTABLE node REQUIRED)
file(GLOB_RECURSE UI_XML_SOURCES "${CMAKE_SOURCE_DIR}/src/ui/*.xml")
set(UI_GENERATE_STAMP "${CMAKE_BINARY_DIR}/ui-generate.stamp")

add_custom_command(
    OUTPUT ${UI_GENERATE_STAMP}
    COMMAND ${NODE_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/lved-cli.js
            generate ${CMAKE_SOURCE_DIR}/src/ui
    COMMAND ${CMAKE_COMMAND} -E touch ${UI_GENERATE_STAMP}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS ${UI_XML_SOURCES}
    COMMENT "Generating UI sources from XML"
)
add_custom_target(ui-generate DEPENDS ${UI_GENERATE_STAMP})

add_subdirectory(src/ui)
add_dependencies(lib-ui ui-generate)

# --- Application Library (platform-agnostic) ---
add_library(versevault-app
    src/app/view/home_view.c
    src/app/view/browse_view.c
    src/app/presenter.c
    src/app/application.c
    src/app/key_manager/key_manager.c
)

target_link_libraries(versevault-app
    PUBLIC
        siera::lvgl
        lib-ui
)

target_include_directories(versevault-app
    PUBLIC
        src/app
        src/app/view
        src/app/key_manager
)

# --- Simulator Executable ---
add_executable(versevault-simulator
    src/platform/simulator/main.c
)

target_link_libraries(versevault-simulator
    PRIVATE
        versevault-app
        siera::driver
        ${SDL2_LIBRARIES}
)

target_include_directories(versevault-simulator
    PRIVATE
        ${SDL2_INCLUDE_DIRS}
        src/platform/simulator
)
```

### 5.2 Key Patterns

**Forcing submodule options.** The consumer sets siera options before `add_subdirectory`
using `set(... CACHE BOOL "" FORCE)`. This overrides the defaults defined in siera's
own `option()` calls.

**Application library.** `versevault-app` contains all platform-agnostic logic (views,
presenter, application state machine, key manager). It links `siera::ui` and the
project's own `lib-ui` (generated UI code). Platform executables link this library
rather than duplicating application sources.

**Platform executables.** Each platform has one executable target with its own `main.c`
and `bsp_config.h`. The `bsp_config.h` file adapts the abstract database keys to
concrete hardware (simulator widgets, GPIO pins, etc.). The executable links the
application library plus the appropriate siera driver.

**UI code generation.** A custom target (`ui-generate`) runs a Node.js script that
converts XML component definitions into C source files. The `lib-ui` library compiles
these generated sources and depends on `ui-generate` so they are regenerated when XML
files change.

**Consumer testing.** When `VERSEVAULT_BUILD_TESTS` is enabled, the consumer populates
`SIERA_CONSUMER_TEST_*` variables and forces `SIERA_BUILD_TESTS` on before
`add_subdirectory(lib/siera)`. Siera's `tests/CMakeLists.txt` compiles both siera and
consumer test sources into a single `test_core` executable. CppUTest auto-discovers all
`TEST_GROUP`s at link time, so no test registration boilerplate is needed — just write
test files and they appear in the output. Consumer tests link `versevault-app` through
`SIERA_CONSUMER_TEST_LINK_LIBRARIES`, giving them access to all application logic
without duplicating sources.


## 6. CMakePresets.json

Presets replace ad-hoc `-D` flags with named configurations. Define them in
`CMakePresets.json` at the project root (and optionally `CMakeUserPresets.json` for
local overrides).

### 6.1 Siera Standalone

```json
{
  "version": 6,
  "configurePresets": [
    {
      "name": "default",
      "hidden": true,
      "generator": "Ninja",
      "cacheVariables": {
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
      }
    },
    {
      "name": "debug",
      "displayName": "Debug (tests + examples)",
      "inherits": "default",
      "binaryDir": "${sourceDir}/build/debug",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "SIERA_BUILD_TESTS": "ON",
        "SIERA_BUILD_EXAMPLES": "ON",
        "SIERA_ENABLE_LVGL": "ON"
      }
    },
    {
      "name": "release",
      "displayName": "Release",
      "inherits": "default",
      "binaryDir": "${sourceDir}/build/release",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release"
      }
    }
  ],
  "buildPresets": [
    { "name": "debug",   "configurePreset": "debug" },
    { "name": "release", "configurePreset": "release" }
  ],
  "testPresets": [
    {
      "name": "debug",
      "configurePreset": "debug",
      "output": { "outputOnFailure": true, "verbosity": "default" }
    }
  ]
}
```

Usage:

```text
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

### 6.2 Consumer Project (versevault-ui)

```json
{
  "version": 6,
  "configurePresets": [
    {
      "name": "default",
      "hidden": true,
      "generator": "Ninja",
      "cacheVariables": {
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
      }
    },
    {
      "name": "simulator-debug",
      "displayName": "Simulator Debug",
      "inherits": "default",
      "binaryDir": "${sourceDir}/build/simulator-debug",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "SIERA_ENABLE_LVGL": "ON",
        "SIERA_DRIVER_SIMULATOR": "ON"
      }
    },
    {
      "name": "simulator-debug-test",
      "displayName": "Simulator Debug + Tests",
      "inherits": "simulator-debug",
      "binaryDir": "${sourceDir}/build/simulator-debug-test",
      "cacheVariables": {
        "VERSEVAULT_BUILD_TESTS": "ON"
      }
    },
    {
      "name": "simulator-release",
      "displayName": "Simulator Release",
      "inherits": "default",
      "binaryDir": "${sourceDir}/build/simulator-release",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release",
        "SIERA_ENABLE_LVGL": "ON",
        "SIERA_DRIVER_SIMULATOR": "ON"
      }
    },
    {
      "name": "esp32-debug",
      "displayName": "ESP32 Debug",
      "inherits": "default",
      "binaryDir": "${sourceDir}/build/esp32-debug",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "SIERA_ENABLE_LVGL": "ON",
        "SIERA_DRIVER_ESP32": "ON",
        "CMAKE_TOOLCHAIN_FILE": "${sourceDir}/cmake/toolchain-esp32.cmake"
      }
    },
    {
      "name": "esp32-release",
      "displayName": "ESP32 Release",
      "inherits": "default",
      "binaryDir": "${sourceDir}/build/esp32-release",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release",
        "SIERA_ENABLE_LVGL": "ON",
        "SIERA_DRIVER_ESP32": "ON",
        "CMAKE_TOOLCHAIN_FILE": "${sourceDir}/cmake/toolchain-esp32.cmake"
      }
    }
  ],
  "buildPresets": [
    { "name": "simulator-debug",      "configurePreset": "simulator-debug" },
    { "name": "simulator-debug-test", "configurePreset": "simulator-debug-test" },
    { "name": "simulator-release",    "configurePreset": "simulator-release" },
    { "name": "esp32-debug",          "configurePreset": "esp32-debug" },
    { "name": "esp32-release",        "configurePreset": "esp32-release" }
  ],
  "testPresets": [
    {
      "name": "simulator-debug-test",
      "configurePreset": "simulator-debug-test",
      "output": { "outputOnFailure": true, "verbosity": "default" }
    }
  ]
}
```

The ESP32 presets reference a toolchain file that sets the cross-compiler. Each preset
gets its own `binaryDir` so configurations do not clobber each other.


## 7. Convenience Makefile

A thin `Makefile` wrapper saves typing for common workflows. It does not replace CMake;
it just invokes it with the right arguments.

### 7.1 Siera Makefile

```makefile
BUILD_DIR := build

.PHONY: all core ui tests clean rebuild examples simulator

all: core

core:
	cmake -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR)

ui:
	cmake -B $(BUILD_DIR) -DSIERA_ENABLE_UI=ON
	cmake --build $(BUILD_DIR)

tests:
	cmake -B $(BUILD_DIR) -DSIERA_BUILD_TESTS=ON
	cmake --build $(BUILD_DIR)
	ctest --test-dir $(BUILD_DIR) --output-on-failure --verbose

examples:
	cmake -B $(BUILD_DIR) -DSIERA_BUILD_EXAMPLES=ON
	cmake --build $(BUILD_DIR)

simulator:
	cmake -B $(BUILD_DIR) -DSIERA_BUILD_EXAMPLES=ON -DSIERA_ENABLE_LVGL=ON
	cmake --build $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean all
```

### 7.2 Consumer Makefile (versevault-ui)

```makefile
BUILD_DIR := build

.PHONY: all configure build run generate tests clean

all: build

configure:
	cmake -DCMAKE_BUILD_TYPE=Debug -B $(BUILD_DIR)

build: configure
	cmake --build $(BUILD_DIR) -j$$(nproc)

run: build
	./$(BUILD_DIR)/versevault-simulator

generate:
	cmake --build $(BUILD_DIR) --target ui-generate

tests:
	cmake -DCMAKE_BUILD_TYPE=Debug -DVERSEVAULT_BUILD_TESTS=ON -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR) -j$$(nproc)
	ctest --test-dir $(BUILD_DIR) --output-on-failure --verbose

clean:
	rm -rf $(BUILD_DIR)
```

When `CMakePresets.json` is in place, the Makefile targets can be simplified to use
`cmake --preset <name>` and `cmake --build --preset <name>` instead of passing
explicit `-D` flags.


## 8. Adding a New Platform

Adding a platform (e.g., ESP32, STM32, a new simulator backend) follows a repeatable
pattern across both siera and the consumer project.

### 8.1 In Siera

1. **Create a driver directory:** `src/driver/<platform>/` with the HAL
   implementations (display driver, timesource, etc.).

2. **Add a driver option** in `src/driver/CMakeLists.txt`:

   ```cmake
   option(SIERA_DRIVER_ESP32 "Build ESP32 drivers" OFF)

   if(SIERA_DRIVER_ESP32)
       file(GLOB ESP32_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/esp32/*.c")
       list(APPEND SIERA_DRIVER_SOURCES ${ESP32_SOURCES})
       list(APPEND SIERA_DRIVER_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/esp32")
       message(STATUS "SIERA: ESP32 drivers enabled")
   endif()
   ```

3. **Add a compile definition** (optional, for `#ifdef` guards):

   ```cmake
   if(SIERA_DRIVER_ESP32)
       target_compile_definitions(siera_driver PRIVATE SIERA_PLATFORM_ESP32)
   endif()
   ```

### 8.2 In the Consumer Project

1. **Create a platform directory:** `src/platform/<platform>/` with:
   - `main.c` — entry point, hardware init, main loop
   - `bsp_config.h` — maps abstract database keys to concrete peripherals

2. **Add an executable target** in the root `CMakeLists.txt`:

   ```cmake
   add_executable(versevault-esp32
       src/platform/esp32/main.c
   )

   target_link_libraries(versevault-esp32
       PRIVATE
           versevault-app
           siera::driver
   )

   target_include_directories(versevault-esp32
       PRIVATE
           src/platform/esp32
   )
   ```

3. **Add CMakePresets entries** for the new platform (debug and release), including
   any required toolchain file.

4. **Update the Makefile** with convenience targets if desired.


## 9. Best Practices

**Use STATIC libraries.** Siera targets embedded systems where shared libraries are
not available. All `add_library()` calls produce static libraries by default (CMake's
default when `BUILD_SHARED_LIBS` is not set).

**Prefer explicit source lists for production code.** `file(GLOB)` is convenient during
prototyping but does not automatically re-run CMake when files are added or removed.
For stable modules, list sources explicitly. For rapidly changing modules (generated
code, early development), `file(GLOB_RECURSE)` is acceptable when paired with a
re-configure step.

**Use INTERFACE libraries for shared compiler flags.** If multiple targets need the
same warning flags, define an `INTERFACE` library and link it `PRIVATE`:

```cmake
add_library(siera_warnings INTERFACE)
target_compile_options(siera_warnings INTERFACE
    $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

# In each module:
target_link_libraries(siera_core PRIVATE siera_warnings)
```

**Use FetchContent for external dependencies.** LVGL and CppUTest are fetched at
configure time. This avoids requiring consumers to pre-install anything and pins
exact versions. Set dependency options (like `CONFIG_LV_BUILD_DEMOS OFF`) before
calling `FetchContent_MakeAvailable` to control what gets built.

**Isolate platform-specific code.** Platform code lives exclusively in
`src/platform/<name>/` (consumer) or `src/driver/<name>/` (siera). The application
library and core modules never `#include` platform headers directly; they depend on
abstract HAL interfaces defined in `src/core/hal/`.

**Use namespaced aliases.** Always link against `siera::core`, `siera::driver`,
`siera::ui` rather than the raw target names (`siera_core`, etc.). The `::` syntax
makes typos a hard error (CMake treats `::` names as imported targets and fails
immediately if they do not exist) and communicates dependency intent clearly.

**Keep build directories separate.** Use distinct `binaryDir` values per configuration
(via CMakePresets or manual `-B` flags). Never mix Debug and Release artifacts in the
same build tree. Never build in-source.

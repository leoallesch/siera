# Multi-Platform / Multi-Toolchain CMake Project Design

## Overview

This document defines the architecture, conventions, and build system design for embedded C projects that target multiple hardware platforms and toolchains from a single codebase. The goal is a clean separation between application logic, hardware abstraction, and platform integration — with zero platform-specific guards in shared code.

---

## Principles

**1. The application layer knows nothing about the platform.**
`app/` contains only business logic. It calls into an abstract HAL interface and never includes platform headers, vendor SDKs, or RTOS APIs directly.

**2. The HAL interface is fixed. Implementations are swapped by CMake.**
`lib/hal/hal_*.h` defines the contract. Each platform provides its own implementation directory. CMake selects the correct one at configure time — there are no `#ifdef PLATFORM` guards in source.

**3. The entry point belongs to the platform.**
`app_main()`, `main()`, or any RTOS task wrapper lives in `platform/`. The app layer exposes a single function (`app_run()`) that the platform calls. This makes the app layer fully portable and independently testable.

**4. All platform selection happens in CMake, not in source.**
Passing `-DPLATFORM=<name>` at configure time is the only switch. No source file changes, no manual header swaps.

**5. Each layer only knows about the layer directly below it.**
Dependencies flow strictly downward. Nothing in a lower layer references a higher layer.

---

## Directory Structure

```
project/
├── CMakeLists.txt              # Top-level: selects platform, wires subdirectories
├── cmake/
│   ├── esp32.cmake             # IDF bootstrap + toolchain config
│   └── cortex_m0.cmake         # arm-none-eabi toolchain config
├── app/
│   ├── CMakeLists.txt
│   ├── app.h                   # Declares app_run()
│   └── app.c                   # Platform-agnostic application logic
├── lib/
│   └── hal/
│       ├── hal_gpio.h          # HAL interface — shared across all platforms
│       ├── esp32/
│       │   ├── CMakeLists.txt
│       │   └── hal_gpio.c      # IDF + FreeRTOS implementation
│       └── cortex_m0/
│           ├── CMakeLists.txt
│           └── hal_gpio.c      # Bare-metal CMSIS register implementation
└── platform/
    ├── esp32/
    │   ├── CMakeLists.txt      # Links app + hal, calls idf_build_executable
    │   └── main.c              # app_main() { app_run(); }
    └── cortex_m0/
        ├── CMakeLists.txt      # Links app + hal, sets linker script, post-build
        ├── main.c              # main() { app_run(); }
        └── device.ld           # Linker script for target MCU
```

---

## Layer Responsibilities

### `app/`

The application layer. Contains all product logic. Has no knowledge of which platform it runs on.

- Includes only `hal_gpio.h` (and other HAL headers) — never vendor or RTOS headers
- Exposes `app_run()` as its sole entry point
- Compiled as a static library: `add_library(app STATIC ...)`
- Linked against `hal_gpio` via `target_link_libraries(app PUBLIC hal_gpio)`

### `lib/hal/`

The hardware abstraction layer. Defines the interface in headers at the root of `lib/hal/`. Each platform subdirectory provides a concrete implementation.

- Headers live at `lib/hal/hal_*.h` — one interface, shared by all platforms
- Implementations live at `lib/hal/<platform>/hal_*.c`
- Each implementation is a static library named `hal_gpio` (or the appropriate HAL module name)
- The include path is set to `lib/hal/` so consumers include `"hal_gpio.h"` — not a platform-specific path
- Implementations may link against platform SDKs (`idf::driver`, CMSIS, etc.) — the app layer never sees these

### `platform/`

The platform integration layer. Owns the entry point and any platform-specific startup concerns.

- Provides the entry point (`app_main` for ESP-IDF, `main` for bare-metal)
- Calls `app_run()` — nothing else
- Owns the linker script (bare-metal platforms)
- Owns flash and post-build targets
- For IDF: calls `idf_build_executable()`

### `cmake/`

Toolchain and SDK configuration files, included by the top-level `CMakeLists.txt` before `project()`.

- `esp32.cmake` — bootstraps IDF via `idf.cmake` (not `project.cmake`), calls `idf_build_process()`
- `cortex_m0.cmake` — sets `CMAKE_SYSTEM_NAME`, `CMAKE_C_COMPILER`, compiler flags, and `CMAKE_TRY_COMPILE_TARGET_TYPE`

---

## Dependency Graph

```
platform/esp32/main.c          platform/cortex_m0/main.c
        │                               │
        └──────────────┬────────────────┘
                       ▼
                   app/app.c              ← shared, no platform knowledge
                       │
                       ▼
              lib/hal/hal_gpio.h          ← interface only
               │              │
               ▼              ▼
   lib/hal/esp32/        lib/hal/cortex_m0/
   hal_gpio.c            hal_gpio.c
        │
        ▼
   idf::driver
   idf::freertos
```

Dependencies flow downward only. No layer references anything above it.

---

## CMake Design

### Top-Level `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED PLATFORM)
    message(FATAL_ERROR
        "PLATFORM not set. Use -DPLATFORM=esp32 or -DPLATFORM=cortex_m0")
endif()

# Must be included before project() — sets toolchain and SDK
include(cmake/${PLATFORM}.cmake)

project(myproject C)

add_subdirectory(app)
add_subdirectory(lib/hal/${PLATFORM})
add_subdirectory(platform/${PLATFORM})
```

### IDF Bootstrap (`cmake/esp32.cmake`)

Use `idf.cmake`, not `project.cmake`. The distinction is critical: `project.cmake` overrides CMake's `project()` function and forces script mode, which prevents `add_library()` and `add_executable()` from working. `idf.cmake` bootstraps the IDF build system while leaving CMake's normal command set intact.

```cmake
include($ENV{IDF_PATH}/tools/cmake/idf.cmake)

idf_build_process(esp32
    SDKCONFIG ${CMAKE_BINARY_DIR}/sdkconfig
    BUILD_DIR  ${CMAKE_BINARY_DIR}
    COMPONENTS freertos driver esptool_py
)
```

### Bare-Metal Toolchain (`cmake/cortex_m0.cmake`)

```cmake
set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m0)

set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_OBJCOPY      arm-none-eabi-objcopy)

set(CMAKE_C_FLAGS_INIT
    "-mcpu=cortex-m0 -mthumb -mfloat-abi=soft \
     -ffunction-sections -fdata-sections -Wall -O2"
)

# Prevents CMake from linking a test executable during configuration
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
```

### HAL Library Convention

Each platform's HAL `CMakeLists.txt` follows this pattern:

```cmake
add_library(hal_gpio STATIC
    hal_gpio.c
)

target_include_directories(hal_gpio PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/..   # exposes lib/hal/hal_gpio.h
)

# Platform deps (IDF only — bare-metal has none)
target_link_libraries(hal_gpio PUBLIC
    idf::driver
    idf::freertos
)
```

The library name `hal_gpio` is identical on both platforms. The top-level only ever refers to `hal_gpio` — it never knows which implementation it got.

### Platform Executable Convention

```cmake
# ESP32
add_executable(myproject.elf main.c)
target_link_libraries(myproject.elf PRIVATE app hal_gpio idf::freertos idf::spi_flash)
idf_build_executable(myproject.elf)

# Cortex-M0
add_executable(myproject.elf main.c)
target_link_libraries(myproject.elf PRIVATE app hal_gpio)
target_link_options(myproject.elf PRIVATE
    -T${CMAKE_CURRENT_LIST_DIR}/device.ld
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_BINARY_DIR}/myproject.map
)
add_custom_command(TARGET myproject.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary myproject.elf myproject.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex   myproject.elf myproject.hex
)
```

---

## Build Commands

### ESP32

```bash
source $IDF_PATH/export.sh

cmake -S . -B build/esp32 \
    -G Ninja \
    -DPLATFORM=esp32 \
    -DIDF_TARGET=esp32

cmake --build build/esp32
ESPPORT=/dev/ttyUSB0 cmake --build build/esp32 --target flash
ESPPORT=/dev/ttyUSB0 cmake --build build/esp32 --target monitor
```

### Cortex-M0

```bash
cmake -S . -B build/m0 \
    -G Ninja \
    -DPLATFORM=cortex_m0

cmake --build build/m0

openocd -f interface/stlink.cfg \
        -f target/stm32f0x.cfg \
        -c "program build/m0/myproject.elf verify reset exit"
```

---

## Adding a New Platform

1. **Create `cmake/<platform>.cmake`** — set toolchain, SDK, or any pre-`project()` configuration needed.

2. **Create `lib/hal/<platform>/`** — implement every function declared in `lib/hal/hal_*.h`. Name the CMake target identically to the existing platforms (`hal_gpio`, etc.).

3. **Create `platform/<platform>/`** — write the entry point that calls `app_run()`. Add the `CMakeLists.txt` with `add_executable`, `target_link_libraries`, and any platform-specific link options or post-build steps.

4. **Build** with `-DPLATFORM=<platform>`. No other files change.

The app layer and HAL interface require no modification.

---

## Adding a New HAL Module

1. **Define the interface** in `lib/hal/hal_<module>.h` — pure function declarations, no platform types.

2. **Implement for each platform** in `lib/hal/<platform>/hal_<module>.c`.

3. **Add the source** to each platform's `lib/hal/<platform>/CMakeLists.txt`:
   ```cmake
   add_library(hal_gpio STATIC
       hal_gpio.c
       hal_<module>.c    # ← add here
   )
   ```

4. **Include in `app/`** via `#include "hal_<module>.h"`.

---

## Rules and Constraints

| Rule | Rationale |
|---|---|
| No `#ifdef PLATFORM` in `app/` or `lib/hal/*.h` | Platform selection is a build concern, not a source concern |
| No vendor headers in `app/` | App layer must compile against any HAL implementation |
| HAL interface functions use only stdint types | Prevents leaking platform type systems into the interface |
| Library target names are identical across platforms | Top-level CMake stays platform-agnostic |
| `idf.cmake` not `project.cmake` for IDF | `project.cmake` hijacks `project()` and breaks standard CMake |
| `CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY` for bare-metal | Prevents configure-time link failure when no OS is present |
| Entry point lives in `platform/`, not `app/` | `app_run()` is called by the platform — not the other way around |
| One build directory per platform | Avoids CMake cache collisions between toolchain configurations |

---

## Interface Contract

Every HAL header defines only what the app layer needs. Types are restricted to `stdint.h` primitives. No RTOS types, no SDK enumerations, no vendor structs cross the boundary.

```c
// lib/hal/hal_gpio.h
#pragma once
#include <stdint.h>

void hal_gpio_led_init(void);
void hal_gpio_led_set(uint8_t state);
void hal_delay_ms(uint32_t ms);
```

If a platform implementation needs richer types internally (e.g. `gpio_num_t`, `TickType_t`), those stay inside the `.c` file and never appear in the header.

---

## What This Is Not

This architecture is intentionally minimal. It does not prescribe:

- A component or package manager
- Dynamic configuration via Kconfig across all platforms
- Unit test infrastructure (though the layering makes host-side testing straightforward — mock the HAL, test `app/` natively)
- RTOS abstraction (if both platforms need an RTOS, a `hal_rtos.h` layer follows the same pattern)

Each of these can be added incrementally without changing the core structure.

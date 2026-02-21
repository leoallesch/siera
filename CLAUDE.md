# SIERA - Claude Code Context

## Project Overview

SIERA (Simple Interface for Embedded Runtime Applications) is a modular embedded C firmware library providing:
- HAL interfaces for platform abstraction
- Event-driven architecture (publish-subscribe)
- Data-driven system via a reactive database with change notifications
- Timer controller system
- Finite state machine
- UI management with toolkit-agnostic interfaces (display, view)
- LVGL integration for graphics
- SDL2-based host simulator for development/testing

**Version:** 0.1.0
**Languages:** C11 (production), C++11 (tests)
**Build System:** CMake 3.20+

---

## Build Commands

```bash
make            # Build core only (default)
make core       # Build core module
make ui         # Build with UI module
make tests      # Build and run tests
make examples   # Build examples
make simulator  # Full simulator with LVGL + SDL2
make clean      # Remove build directory
make rebuild    # Clean + build

# Manual CMake
cmake -B build -DSIERA_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build
```

### CMake Options

| Option | Default | Effect |
|--------|---------|--------|
| `SIERA_ENABLE_UI` | OFF | Build display/view interfaces |
| `SIERA_ENABLE_LVGL` | OFF | Fetch LVGL v9.4.0, implies UI |
| `SIERA_BUILD_TESTS` | OFF | Build CppUTest test suite |
| `SIERA_BUILD_EXAMPLES` | OFF | Build examples |
| `SIERA_DRIVER_SIMULATOR` | OFF | SDL2 simulator driver |
| `SIERA_ENABLE_COVERAGE` | OFF | gcov instrumentation; use `make coverage` |

---

## Architecture

```
src/
├── core/                  # always compiled into siera
│   ├── data_structures/   # Intrusive list and queue
│   ├── database/          # RAM datastream, database aggregator, schema macros
│   ├── event/             # Publish-subscribe event system
│   ├── hal/               # HAL interface placeholders
│   ├── state_machine/     # Finite state machine (fsm.c)
│   ├── timer/             # Timer controller + timesource interface
│   └── utils.h            # CONTAINER_OF, NUM_ELEMENTS, WRAP macros
├── driver/
│   └── simulator/         # SDL2 display + mock timesource (SIERA_DRIVER_SIMULATOR)
├── ui/                    # compiled when SIERA_ENABLE_UI or SIERA_ENABLE_LVGL
│   ├── display/           # i_display.h - abstract display update
│   └── view/              # i_view.h - view lifecycle (load/unload)
└── lvgl/                  # compiled when SIERA_ENABLE_LVGL
    └── lv_conf.h          # LVGL v9.4.0 config (RGB565, 64KB heap)

tests/
├── core/                  # CppUTest test files per module
└── doubles/               # Test doubles (e.g. mock timesource)

examples/
└── hello_world/           # Demonstrates database, timers, events
```

---

## Coding Conventions

### Naming
- Struct types: `<name>_t` for internal structs
- Interface types: `i_<name>_t` (e.g., `i_display_t`, `i_timesource_t`)
- Public types: `<name>_t`
- Functions: `module_action()` (e.g., `event_subscribe()`, `database_read()`)
- Macros: `UPPERCASE` for constants, `UPPERCASE()` for function-like macros
- Enum variants: `MODULE_VARIANT` (e.g., `FSM_SIGNAL_ENTER`)

### Formatting (`.clang-format`)
- 2-space indent, no tabs
- No column limit
- Left-aligned pointers (`int* ptr`)
- Function braces on new line; control statement braces same line
- `#pragma once` for header guards

### Memory & Allocation
- **No `malloc()` in core** — static allocation only
- Use intrusive containers: embed `list_node_t` as first member of structs
- Database schema defined at compile time via macros (not runtime)
- LVGL/simulator may use heap (acceptable for host-side code)

### Abstraction Pattern (C-style OOP)
Interfaces use function pointer structs with inline dispatch helpers:
```c
typedef struct i_timesource_t {
  timesource_ticks_t (*get_ticks)(struct i_timesource_t* self);
} i_timesource_t;

static inline timesource_ticks_t timesource_get_ticks(i_timesource_t* self) {
  return self->get_ticks(self);
}
```

---

## Key Patterns

### Database API
`database_init` takes `i_datastream_t** streams, uint8_t count` — an array of stream interface pointers. There is no `database_subscribe_all`; subscribe via `datastream_subscribe_all(&ram_ds.interface, &sub)` directly on the stream.

`ram_datastream_entry_t entries[]` must **not** be `const` — `ram_datastream_init` calls `event_init` on each entry's `entry_on_change` field. Full setup pattern:
```c
static ram_datastream_entry_t entries[] = { MY_DB(DATABASE_EXPAND_AS_ENTRY) };
static const ram_datastream_config_t config = { .entries = entries, .count = NUM_ELEMENTS(entries) };
ram_datastream_init(&ds, &config, &storage);
i_datastream_t* streams[] = { &ds.interface };
database_init(&db, streams, NUM_ELEMENTS(streams));
```

### Database Schema Definition
```c
// Define schema with X-macro
#define DATABASE(ENTRY) \
  ENTRY(Key_Foo, uint16_t) \
  ENTRY(Key_Bar, bool)

DATABASE_ENUM(DATABASE)    // Creates enum with key values
DATABASE_STORAGE(DATABASE) // Creates ram_storage_t struct
```

### Event Subscription
```c
event_subscription_t sub;
event_subscription_init(&sub, my_callback, context_ptr);
event_subscribe(&some_event, &sub);
// Unsubscribe when done
event_unsubscribe(&some_event, &sub);
```

### Timer Usage
```c
s_timer_t timer;
timer_start_repeating(&timer, &controller, 1000, my_callback, context_ptr);
// In main loop:
timer_controller_run(&controller);
```

### FSM State Function
```c
void my_state(fsm_t* fsm, fsm_signal_t signal, const void* data) {
  switch(signal) {
    case FSM_SIGNAL_ENTER: /* ... */ break;
    case FSM_SIGNAL_EXIT:  /* ... */ break;
    case MY_SIGNAL:
      fsm_transition(fsm, other_state);
      break;
  }
}
```

### CONTAINER_OF for Intrusive Containers
```c
typedef struct {
  list_node_t node;  // must be first member
  int value;
} my_item_t;

// Recover parent from node pointer:
my_item_t* item = CONTAINER_OF(node_ptr, my_item_t, node);
```

---

## CMake Conventions

- All modules are compiled into a single `siera` target; link against `siera` directly
- Never use global `include_directories()`, `link_libraries()`, or `add_compile_options()`
- Use `target_*` commands with appropriate visibility (`PUBLIC`/`PRIVATE`/`INTERFACE`)
- Use `$<BUILD_INTERFACE:...>` generator expressions for include paths
- Compiler warnings (`-Wall -Wextra`) are `PRIVATE` — don't leak to consumers
- New platform drivers: add `if(SIERA_DRIVER_<PLATFORM>)` block in `src/driver/CMakeLists.txt`

### Consumer Integration (using siera as a subproject)
```cmake
# Before add_subdirectory(siera):
set(SIERA_CONSUMER_TEST_SOURCES       ${your_test_cpp_files})
set(SIERA_CONSUMER_DOUBLE_SOURCES     ${your_test_double_c_files})
set(SIERA_CONSUMER_TEST_INCLUDE_DIRS  ${your_include_dirs})
set(SIERA_CONSUMER_TEST_LINK_LIBRARIES your-app-library)
set(SIERA_BUILD_TESTS ON CACHE BOOL "" FORCE)

add_subdirectory(siera)
```

---

## Testing

- Framework: **CppUTest** (fetched automatically via FetchContent)
- Test files: `tests/core/**/*.cpp`, one `TEST_GROUP` per module
- Test doubles: `tests/doubles/` — mock/stub implementations of interfaces
- Consumer tests merge into the single `test_core` executable

### Test Pattern
```cpp
TEST_GROUP(TimerTests) {
  s_timer_controller_t controller;
  double_timesource_t timesource;

  void setup() {
    double_timesource_init(&timesource);
    timer_controller_init(&controller, &timesource.interface);
  }
};

TEST(TimerTests, fires_after_interval) {
  // ...
}
```

- Use `mock().expectOneCall(...)` / `mock().checkExpectations()` for callback verification
- Use `double_timesource_set_ticks()` to advance time deterministically
- Run: `make tests` or `ctest --test-dir build`

### Code Coverage
`make coverage` — builds Debug+gcov, runs tests, filters deps, enforces 80% line coverage gate. lcov on GCC 15 requires `--ignore-errors inconsistent,inconsistent` (already in Makefile).
- Arch: `sudo pacman -S lcov`
- Ubuntu: `sudo apt-get install lcov`

---

## Adding New Features

### New HAL Interface
1. Create `src/core/hal/i_<name>.h` with function pointer struct + inline helpers
2. Add implementation in `src/driver/<platform>/`
3. Add test double in `tests/doubles/core/<name>/`

### New Core Module
1. Create `src/core/<module>/` with `.c` and `.h` files
2. Sources are auto-globbed by `src/core/CMakeLists.txt` (`GLOB_RECURSE *.c`)
3. Add new public include directory to `target_include_directories` in `src/core/CMakeLists.txt`
4. Create `tests/core/<module>/test_<module>.cpp`

### New Driver Platform
1. Add `option(SIERA_DRIVER_<PLATFORM> ...)` in `src/driver/CMakeLists.txt`
2. Add `if(SIERA_DRIVER_<PLATFORM>)` block appending sources and include paths

---

## CI

`.github/workflows/ci.yml` — triggers on every PR commit and push to `master`. Steps: configure → build → test → collect coverage → enforce 80% line coverage threshold (hard fail).

---

## Dependencies

| Library | Version | How | Purpose |
|---------|---------|-----|---------|
| LVGL | v9.4.0 | FetchContent | Graphics (optional) |
| CppUTest | master | FetchContent | Unit testing |
| SDL2 | system | pkg-config | Simulator display |

No runtime dependencies in core (fully self-contained, no heap).

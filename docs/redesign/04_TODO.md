# Siera — Implementation TODO

## Phase 1: Foundation

### siera_common
- [ ] `lib/core/siera_common/CMakeLists.txt` (INTERFACE)
- [ ] `lib/core/siera_common/include/siera/common.h`
  - [ ] `SIERA_OFFSET_OF`, `SIERA_CONTAINER_OF`, `SIERA_NUM_ELEMENTS`, `SIERA_CONCAT`

### siera_slist
- [ ] `lib/core/siera_slist/CMakeLists.txt` (INTERFACE, depends on siera_common)
- [ ] `lib/core/siera_slist/include/siera/slist.h`
  - [ ] Types: `siera_slist_node_t`, `siera_slist_head_t`, `SIERA_SLIST_HEAD_INIT`
  - [ ] Functions: init, node_init, is_empty, push, pop, remove
  - [ ] `SIERA_SLIST_FOR_EACH_SAFE`
- [ ] `test/test_slist.cpp` — push/pop, remove, safe iteration removal, empty list

## Phase 2: Event Bus

### siera_event
- [ ] `lib/core/siera_event/CMakeLists.txt` (STATIC, depends on siera_slist)
- [ ] `lib/core/siera_event/include/siera/event.h`
  - [ ] `siera_event_t` (id, data, size, old_data)
  - [ ] `siera_event_handler_fn`
  - [ ] `siera_event_sub_t` (intrusive node + handler + ctx)
  - [ ] `siera_event_bus_t`
  - [ ] bus_init, sub_init, subscribe, unsubscribe, publish
- [ ] `lib/core/siera_event/src/event.c`
- [ ] `test/test_event.cpp` — publish, unsubscribe, self-removal, multiple subs, empty bus

## Phase 3: Timers

### siera_timer
- [ ] `lib/core/siera_timer/CMakeLists.txt` (STATIC, depends on siera_slist)
- [ ] `lib/core/siera_timer/include/siera/timer.h`
  - [ ] `siera_timer_now_fn`, `siera_timer_flags_t`, `siera_timer_cb_fn`
  - [ ] `siera_timer_t`, `siera_timer_mgr_t`
  - [ ] mgr_init, init, start, stop, restart, is_active, tick
- [ ] `lib/core/siera_timer/src/timer.c`
  - [ ] Drift-corrected repeat, wraparound-safe, one-shot remove before callback
- [ ] `test/test_timer.cpp` — oneshot, repeating, drift, stop idempotent, restart, self-restart

## Phase 4: Datasource

### Key generation
- [ ] `lib/core/siera_ds/siera_ds_keys.def` — sample keys
- [ ] `lib/core/siera_ds/src/ds_keys_internal.h`
  - [ ] `siera_ds_flags_t` (NONE, READONLY, SILENT)
  - [ ] `DSK_EXPAND_ENUM` → `siera_ds_key_t` + `DSK_COUNT`
  - [ ] `DSK_EXPAND_STORAGE` → `siera_ds_cache_t` (uint8_t arrays, offsetof)
  - [ ] `SIERA_DS_CACHE_SIZE`
  - [ ] `siera_ds_entry_t` + `DSK_EXPAND_ENTRY` → entry table
  - [ ] `DSK_EXPAND_DEFAULT` → `_siera_ds_apply_defaults()`
  - [ ] `SIERA_DS_MAX_VALUE_SIZE`

### Stream interface
- [ ] `lib/core/siera_ds/include/siera/ds_stream.h`
  - [ ] `siera_ds_stream_api_t` (read, write)
  - [ ] `siera_ds_stream_t` (api + ctx)
  - [ ] `siera_ds_stream_type_t` enum (RAM, PERSIST, GPIO, ADC, PWM, I2C, SPI, COUNT)

### Core
- [ ] `lib/core/siera_ds/include/siera/ds_keys.h`
- [ ] `lib/core/siera_ds/include/siera/ds.h`
  - [ ] `siera_ds_stream_binding_t`, `SIERA_DS_STREAM_END`
  - [ ] `siera_ds_config_t` (streams, events ptr, timers ptr, flush_interval_ms)
  - [ ] `siera_ds_t` (entries, cache, events ptr, timers ptr, flush_interval_ms, dirty, stream_map, flush_timer)
  - [ ] init, read, write, deinit, key_name, key_size, key_stream_type
  - [ ] No tick (app calls siera_timer_tick directly)
  - [ ] No flush (internal only)
- [ ] `lib/core/siera_ds/src/ds.c`
  - [ ] `ds_stream()` — lookup stream_map
  - [ ] `ds_flush()` — internal: walk dirty bits, write persist stream
  - [ ] `ds_flush_cb()` — timer callback
  - [ ] `siera_ds_init` — store borrowed event bus + timer mgr, build stream map, defaults, hydrate, start flush timer if interval > 0
  - [ ] `siera_ds_read` — non-RAM/non-PERSIST sample fresh, memcpy from cache
  - [ ] `siera_ds_write` — change detect, update cache, persist batch or write-through, hw write-through, publish event on borrowed bus
  - [ ] `siera_ds_deinit` — stop flush timer, flush remaining
- [ ] `lib/core/siera_ds/CMakeLists.txt`
- [ ] `test/test_ds.cpp`
  - [ ] Mock streams (persist, gpio, adc)
  - [ ] Fixture: external event bus + timer mgr with fake clock
  - [ ] Defaults, roundtrip, no-event-same-value, event old/new values
  - [ ] Persist write-through (interval=0)
  - [ ] Persist batched flush (interval>0)
  - [ ] Readonly rejects, hw write calls stream, hw read fresh
  - [ ] Deinit flushes remaining dirty

## Phase 5: Build Infrastructure

- [ ] Top-level `CMakeLists.txt`
  - [ ] C11 + CXX17
  - [ ] Options: SIERA_DRV_ESP32, SIERA_DRV_POSIX, SIERA_BUILD_TESTS, SIERA_BUILD_EXAMPLES
  - [ ] FetchContent GoogleTest (gated on tests)
  - [ ] add_subdirectory all core libs
  - [ ] `siera_core` INTERFACE umbrella
  - [ ] Conditional driver subdirs
  - [ ] `siera` INTERFACE umbrella (core + driver)
- [ ] `cmake/SieraTest.cmake` — siera_add_test with gtest_main + gtest_discover_tests
- [ ] Verify: `cmake -B build -DSIERA_BUILD_TESTS=ON && cmake --build build && ctest`

## Phase 6: Drivers

### ESP32
- [ ] `lib/drivers/esp32/CMakeLists.txt`
- [ ] NVS stream: `nvs.h` / `nvs.c` — nvs_get_blob / nvs_set_blob
- [ ] GPIO stream: `gpio.h` / `gpio.c` — pin table dispatch, gpio_get/set_level
- [ ] ADC stream: `adc.h` / `adc.c` — channel table, adc_oneshot_read + cal
- [ ] PWM stream: `pwm.h` / `pwm.c` — ledc_set_duty / ledc_update_duty
- [ ] Time: `time.h` / `time.c` — esp_timer_get_time / 1000

### POSIX
- [ ] `lib/drivers/posix/CMakeLists.txt`
- [ ] FS stream: `fs.h` / `fs.c` — file per key name
- [ ] Time: `time.h` / `time.c` — clock_gettime(CLOCK_MONOTONIC)

## Phase 7: Example
- [ ] `examples/bare_metal/CMakeLists.txt`
- [ ] `examples/bare_metal/main.c`
  - [ ] External event bus + timer mgr
  - [ ] Inline streams (eeprom, gpio, adc)
  - [ ] Config with borrowed pointers
  - [ ] Modules: display, alarm, input, sensor
  - [ ] Main loop: `siera_timer_tick(&g_timers)`
- [ ] Stub HAL headers

## Phase 8: Polish
- [ ] README.md
- [ ] Verify ESP-IDF component integration
- [ ] ASAN / valgrind pass
- [ ] All symbols prefixed `siera_`, all guards `SIERA_*_H`, no platform headers in core

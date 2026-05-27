# Siera — Build System & Drivers

## 1. Layout

```
siera/
├── CMakeLists.txt
├── cmake/
│   └── SieraTest.cmake
│
├── lib/
│   ├── core/
│   │   ├── siera_common/
│   │   │   ├── CMakeLists.txt
│   │   │   └── include/siera/common.h
│   │   ├── siera_slist/
│   │   │   ├── CMakeLists.txt
│   │   │   ├── include/siera/slist.h
│   │   │   └── test/
│   │   │       ├── CMakeLists.txt
│   │   │       └── test_slist.cpp
│   │   ├── siera_event/
│   │   │   ├── CMakeLists.txt
│   │   │   ├── include/siera/event.h
│   │   │   ├── src/event.c
│   │   │   └── test/
│   │   │       ├── CMakeLists.txt
│   │   │       └── test_event.cpp
│   │   ├── siera_timer/
│   │   │   ├── CMakeLists.txt
│   │   │   ├── include/siera/timer.h
│   │   │   ├── src/timer.c
│   │   │   └── test/
│   │   │       ├── CMakeLists.txt
│   │   │       └── test_timer.cpp
│   │   └── siera_ds/
│   │       ├── CMakeLists.txt
│   │       ├── include/siera/
│   │       │   ├── ds.h
│   │       │   ├── ds_keys.h
│   │       │   └── ds_stream.h
│   │       ├── src/
│   │       │   ├── ds.c
│   │       │   └── ds_keys_internal.h
│   │       ├── siera_ds_keys.def
│   │       └── test/
│   │           ├── CMakeLists.txt
│   │           └── test_ds.cpp
│   │
│   └── drivers/
│       ├── esp32/
│       │   ├── CMakeLists.txt
│       │   ├── include/siera/drivers/esp32/
│       │   │   ├── nvs.h
│       │   │   ├── gpio.h
│       │   │   ├── adc.h
│       │   │   ├── pwm.h
│       │   │   └── time.h
│       │   └── src/
│       │       ├── nvs.c
│       │       ├── gpio.c
│       │       ├── adc.c
│       │       ├── pwm.c
│       │       └── time.c
│       └── posix/
│           ├── CMakeLists.txt
│           ├── include/siera/drivers/posix/
│           │   ├── fs.h
│           │   └── time.h
│           └── src/
│               ├── fs.c
│               └── time.c
│
└── examples/
    └── bare_metal/
        ├── CMakeLists.txt
        └── main.c
```

## 2. CMake

### 2.1 Top-Level

```cmake
cmake_minimum_required(VERSION 3.14)
project(siera C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(SIERA_DRV_ESP32      "Build ESP32 drivers"  OFF)
option(SIERA_DRV_POSIX      "Build POSIX drivers"  OFF)
option(SIERA_BUILD_TESTS    "Build tests"          OFF)
option(SIERA_BUILD_EXAMPLES "Build examples"       OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(SIERA_BUILD_TESTS)
    enable_testing()
    include(FetchContent)
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

add_subdirectory(lib/core/siera_common)
add_subdirectory(lib/core/siera_slist)
add_subdirectory(lib/core/siera_event)
add_subdirectory(lib/core/siera_timer)
add_subdirectory(lib/core/siera_ds)

add_library(siera_core INTERFACE)
target_link_libraries(siera_core INTERFACE
    siera_common siera_slist siera_event siera_timer siera_ds)

if(SIERA_DRV_ESP32)
    add_subdirectory(lib/drivers/esp32)
endif()
if(SIERA_DRV_POSIX)
    add_subdirectory(lib/drivers/posix)
endif()

add_library(siera INTERFACE)
target_link_libraries(siera INTERFACE siera_core)
if(SIERA_DRV_ESP32)
    target_link_libraries(siera INTERFACE siera_drv_esp32)
endif()
if(SIERA_DRV_POSIX)
    target_link_libraries(siera INTERFACE siera_drv_posix)
endif()

if(SIERA_BUILD_EXAMPLES)
    add_subdirectory(examples/bare_metal)
endif()
```

### 2.2 cmake/SieraTest.cmake

```cmake
function(siera_add_test TEST_NAME SOURCE_FILE)
    add_executable(${TEST_NAME} ${SOURCE_FILE})
    if(ARGN)
        target_link_libraries(${TEST_NAME} PRIVATE ${ARGN})
    endif()
    target_link_libraries(${TEST_NAME} PRIVATE GTest::gtest_main)
    target_compile_options(${TEST_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    include(GoogleTest)
    gtest_discover_tests(${TEST_NAME})
endfunction()
```

### 2.3 Core Libraries

```cmake
# siera_common
add_library(siera_common INTERFACE)
target_include_directories(siera_common INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

# siera_slist
add_library(siera_slist INTERFACE)
target_include_directories(siera_slist INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(siera_slist INTERFACE siera_common)

# siera_event
add_library(siera_event STATIC src/event.c)
target_include_directories(siera_event PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(siera_event PUBLIC siera_slist)

# siera_timer
add_library(siera_timer STATIC src/timer.c)
target_include_directories(siera_timer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(siera_timer PUBLIC siera_slist)

# siera_ds
add_library(siera_ds STATIC src/ds.c)
target_include_directories(siera_ds PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_include_directories(siera_ds PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(siera_ds PUBLIC siera_slist siera_event siera_timer)
```

Each with `if(SIERA_BUILD_TESTS) add_subdirectory(test) endif()` and:
```cmake
include(SieraTest)
siera_add_test(test_siera_<x> test_<x>.cpp siera_<x>)
```

### 2.4 Drivers

```cmake
# esp32
add_library(siera_drv_esp32 STATIC
    src/nvs.c src/gpio.c src/adc.c src/pwm.c src/time.c)
target_include_directories(siera_drv_esp32 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(siera_drv_esp32 PUBLIC
    siera_core idf::nvs_flash idf::esp_timer idf::driver)

# posix
add_library(siera_drv_posix STATIC src/fs.c src/time.c)
target_include_directories(siera_drv_posix PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(siera_drv_posix PUBLIC siera_core)
```

## 3. Tests (GoogleTest)

### 3.1 test_slist.cpp

```cpp
extern "C" {
#include "siera/slist.h"
}
#include <gtest/gtest.h>

struct Item { int value; siera_slist_node_t node; };

TEST(SieraSlist, PushPop) {
    siera_slist_head_t list = SIERA_SLIST_HEAD_INIT;
    Item a{1}, b{2};
    siera_slist_node_init(&a.node);
    siera_slist_node_init(&b.node);
    siera_slist_push(&list, &a.node);
    siera_slist_push(&list, &b.node);
    EXPECT_EQ(SIERA_CONTAINER_OF(siera_slist_pop(&list), Item, node)->value, 2);
    EXPECT_EQ(SIERA_CONTAINER_OF(siera_slist_pop(&list), Item, node)->value, 1);
    EXPECT_TRUE(siera_slist_is_empty(&list));
}

TEST(SieraSlist, RemoveMiddle) {
    siera_slist_head_t list = SIERA_SLIST_HEAD_INIT;
    Item a{1}, b{2}, c{3};
    siera_slist_node_init(&a.node); siera_slist_node_init(&b.node); siera_slist_node_init(&c.node);
    siera_slist_push(&list, &a.node); siera_slist_push(&list, &b.node); siera_slist_push(&list, &c.node);
    EXPECT_TRUE(siera_slist_remove(&list, &b.node));
    EXPECT_FALSE(siera_slist_remove(&list, &b.node));
}

TEST(SieraSlist, SafeIterationRemoval) {
    siera_slist_head_t list = SIERA_SLIST_HEAD_INIT;
    Item items[5];
    for (int i = 0; i < 5; i++) { items[i].value = i; siera_slist_node_init(&items[i].node); siera_slist_push(&list, &items[i].node); }
    siera_slist_node_t *cur, *tmp;
    SIERA_SLIST_FOR_EACH_SAFE(&list, cur, tmp) {
        if (SIERA_CONTAINER_OF(cur, Item, node)->value % 2 == 0) siera_slist_remove(&list, cur);
    }
    int count = 0;
    SIERA_SLIST_FOR_EACH_SAFE(&list, cur, tmp) { count++; }
    EXPECT_EQ(count, 3);
}
```

### 3.2 test_event.cpp

```cpp
extern "C" {
#include "siera/event.h"
}
#include <gtest/gtest.h>

static int s_count; static uint32_t s_last_id;
static void handler(const siera_event_t *e, void *) { s_count++; s_last_id = e->id; }

TEST(SieraEvent, Publish) {
    siera_event_bus_t bus; siera_event_bus_init(&bus);
    siera_event_sub_t sub; siera_event_sub_init(&sub, handler, nullptr);
    siera_event_subscribe(&bus, &sub);
    s_count = 0;
    siera_event_t e{.id = 7}; siera_event_publish(&bus, &e);
    EXPECT_EQ(s_count, 1); EXPECT_EQ(s_last_id, 7u);
}

TEST(SieraEvent, Unsubscribe) {
    siera_event_bus_t bus; siera_event_bus_init(&bus);
    siera_event_sub_t sub; siera_event_sub_init(&sub, handler, nullptr);
    siera_event_subscribe(&bus, &sub); siera_event_unsubscribe(&bus, &sub);
    s_count = 0; siera_event_t e{.id = 1}; siera_event_publish(&bus, &e);
    EXPECT_EQ(s_count, 0);
}

static siera_event_sub_t g_self;
static void self_remover(const siera_event_t *, void *ctx) {
    s_count++; siera_event_unsubscribe(static_cast<siera_event_bus_t *>(ctx), &g_self);
}

TEST(SieraEvent, SelfRemoval) {
    siera_event_bus_t bus; siera_event_bus_init(&bus);
    siera_event_sub_t other; siera_event_sub_init(&other, handler, nullptr);
    siera_event_subscribe(&bus, &other);
    siera_event_sub_init(&g_self, self_remover, &bus); siera_event_subscribe(&bus, &g_self);
    s_count = 0; siera_event_t e{.id = 99}; siera_event_publish(&bus, &e);
    EXPECT_EQ(s_count, 2);
    s_count = 0; siera_event_publish(&bus, &e); EXPECT_EQ(s_count, 1);
}
```

### 3.3 test_timer.cpp

```cpp
extern "C" {
#include "siera/timer.h"
}
#include <gtest/gtest.h>

static uint32_t s_now; static uint32_t fake_now() { return s_now; }
static int s_fires; static void counter(void *) { s_fires++; }

class SieraTimer : public ::testing::Test {
protected:
    siera_timer_mgr_t mgr; siera_timer_t t;
    void SetUp() override { s_now = 0; s_fires = 0; siera_timer_mgr_init(&mgr, fake_now); siera_timer_init(&t, counter, nullptr); }
};

TEST_F(SieraTimer, Oneshot) {
    siera_timer_start(&mgr, &t, 100, SIERA_TIMER_ONCE);
    s_now = 50; siera_timer_tick(&mgr); EXPECT_EQ(s_fires, 0);
    s_now = 100; siera_timer_tick(&mgr); EXPECT_EQ(s_fires, 1);
    EXPECT_FALSE(siera_timer_is_active(&t));
}

TEST_F(SieraTimer, Repeating) {
    siera_timer_start(&mgr, &t, 100, SIERA_TIMER_REPEAT);
    s_now = 100; siera_timer_tick(&mgr); EXPECT_EQ(s_fires, 1);
    s_now = 200; siera_timer_tick(&mgr); EXPECT_EQ(s_fires, 2);
    siera_timer_stop(&mgr, &t);
    s_now = 300; siera_timer_tick(&mgr); EXPECT_EQ(s_fires, 2);
}

TEST_F(SieraTimer, DriftCorrection) {
    siera_timer_start(&mgr, &t, 100, SIERA_TIMER_REPEAT);
    s_now = 105; siera_timer_tick(&mgr);
    EXPECT_EQ(t.expiry_ms, 200u);
}

TEST_F(SieraTimer, Restart) {
    siera_timer_start(&mgr, &t, 100, SIERA_TIMER_ONCE);
    s_now = 80; siera_timer_restart(&mgr, &t);
    s_now = 100; siera_timer_tick(&mgr); EXPECT_EQ(s_fires, 0);
    s_now = 180; siera_timer_tick(&mgr); EXPECT_EQ(s_fires, 1);
}
```

### 3.4 test_ds.cpp

```cpp
extern "C" {
#include "siera/ds.h"
#include "siera/event.h"
}
#include <gtest/gtest.h>
#include <cstring>

static uint32_t s_now; static uint32_t mock_now() { return s_now; }

struct PEntry { char name[32]; uint8_t data[8]; size_t size; };
static PEntry s_store[32]; static int s_store_n;

static int mock_persist_read(void *, siera_ds_key_t key, void *buf, size_t size) {
    const char *n = siera_ds_key_name(key);
    for (int i = 0; i < s_store_n; i++)
        if (strcmp(s_store[i].name, n) == 0 && s_store[i].size == size)
            { memcpy(buf, s_store[i].data, size); return 0; }
    return -1;
}

static int mock_persist_write(void *, siera_ds_key_t key, const void *buf, size_t size) {
    const char *n = siera_ds_key_name(key);
    for (int i = 0; i < s_store_n; i++)
        if (strcmp(s_store[i].name, n) == 0)
            { memcpy(s_store[i].data, buf, size); s_store[i].size = size; return 0; }
    if (s_store_n < 32) {
        strncpy(s_store[s_store_n].name, n, 31); memcpy(s_store[s_store_n].data, buf, size);
        s_store[s_store_n].size = size; s_store_n++; return 0;
    }
    return -1;
}

static const siera_ds_stream_api_t mock_persist_api = { mock_persist_read, mock_persist_write };

static bool s_gpio; static uint16_t s_adc; static bool s_hw_written;
static int mock_gpio_read(void *, siera_ds_key_t, void *buf, size_t) { memcpy(buf, &s_gpio, sizeof(bool)); return 0; }
static int mock_gpio_write(void *, siera_ds_key_t, const void *, size_t) { s_hw_written = true; return 0; }
static const siera_ds_stream_api_t mock_gpio_api = { mock_gpio_read, mock_gpio_write };
static int mock_adc_read(void *, siera_ds_key_t, void *buf, size_t) { memcpy(buf, &s_adc, sizeof(uint16_t)); return 0; }
static const siera_ds_stream_api_t mock_adc_api = { mock_adc_read, nullptr };

class SieraDs : public ::testing::Test {
protected:
    i_siera_ds_t ds;
    siera_event_bus_t events;
    siera_timer_mgr_t timers;
    siera_ds_stream_t persist_s = { &mock_persist_api, nullptr };
    siera_ds_stream_t gpio_s    = { &mock_gpio_api, nullptr };
    siera_ds_stream_t adc_s     = { &mock_adc_api, nullptr };
    siera_ds_stream_binding_t streams[4] = {
        { SIERA_DS_NVS, &persist_s }, { SIERA_DS_GPIO, &gpio_s },
        { SIERA_DS_ADC, &adc_s }, SIERA_DS_STREAM_END };
    siera_ds_config_t config = { streams, &events, &timers, 0 };

    void SetUp() override {
        s_now = 0; s_store_n = 0; s_gpio = false; s_adc = 3300; s_hw_written = false;
        siera_event_bus_init(&events);
        siera_timer_mgr_init(&timers, mock_now);
        siera_ds_init(&ds, &config);
    }
    void TearDown() override { siera_ds_deinit(&ds); }
};

TEST_F(SieraDs, Defaults) {
    uint8_t h; siera_ds_read(&ds, DSK_ALARM_HOUR, &h); EXPECT_EQ(h, 7);
}

TEST_F(SieraDs, Roundtrip) {
    uint8_t v = 22; siera_ds_write(&ds, DSK_ALARM_HOUR, &v);
    uint8_t out; siera_ds_read(&ds, DSK_ALARM_HOUR, &out); EXPECT_EQ(out, 22);
}

TEST_F(SieraDs, NoEventOnSameValue) {
    int count = 0;
    auto counter = [](const siera_event_t *, void *c) { (*(int *)c)++; };
    siera_event_sub_t sub; siera_event_sub_init(&sub, counter, &count);
    siera_event_subscribe(&events, &sub);
    uint8_t v = 7; siera_ds_write(&ds, DSK_ALARM_HOUR, &v); EXPECT_EQ(count, 0);
    v = 8; siera_ds_write(&ds, DSK_ALARM_HOUR, &v); EXPECT_EQ(count, 1);
}

TEST_F(SieraDs, PersistWriteThrough) {
    uint8_t v = 99; siera_ds_write(&ds, DSK_BRIGHTNESS, &v);
    siera_ds_deinit(&ds);
    siera_event_bus_t ev2; siera_timer_mgr_t tm2;
    siera_event_bus_init(&ev2); siera_timer_mgr_init(&tm2, mock_now);
    i_siera_ds_t ds2; siera_ds_config_t c2 = { streams, &ev2, &tm2, 0 };
    siera_ds_init(&ds2, &c2);
    uint8_t out; siera_ds_read(&ds2, DSK_BRIGHTNESS, &out);
    EXPECT_EQ(out, 99); siera_ds_deinit(&ds2);
}

TEST_F(SieraDs, BatchedFlush) {
    siera_ds_deinit(&ds);
    config.flush_interval_ms = 1000;
    siera_event_bus_init(&events); siera_timer_mgr_init(&timers, mock_now);
    siera_ds_init(&ds, &config);
    uint8_t v = 77; siera_ds_write(&ds, DSK_BRIGHTNESS, &v);

    // Not flushed yet
    s_now = 500; siera_timer_tick(&timers);
    i_siera_ds_t ds2; siera_event_bus_t ev2; siera_timer_mgr_t tm2;
    siera_event_bus_init(&ev2); siera_timer_mgr_init(&tm2, mock_now);
    siera_ds_config_t c2 = { streams, &ev2, &tm2, 0 };
    siera_ds_init(&ds2, &c2);
    uint8_t out; siera_ds_read(&ds2, DSK_BRIGHTNESS, &out);
    EXPECT_EQ(out, 128); siera_ds_deinit(&ds2);

    // Now flushed
    s_now = 1000; siera_timer_tick(&timers);
    siera_event_bus_init(&ev2); siera_timer_mgr_init(&tm2, mock_now);
    siera_ds_init(&ds2, &c2);
    siera_ds_read(&ds2, DSK_BRIGHTNESS, &out);
    EXPECT_EQ(out, 77); siera_ds_deinit(&ds2);
}

TEST_F(SieraDs, ReadonlyRejects) {
    bool v = true; EXPECT_NE(siera_ds_write(&ds, DSK_BUTTON_SET, &v), 0);
}

TEST_F(SieraDs, HwWriteCallsStream) {
    bool led = true; siera_ds_write(&ds, DSK_LED_STATUS, &led); EXPECT_TRUE(s_hw_written);
}

TEST_F(SieraDs, HwReadFresh) {
    s_adc = 4200; uint16_t b; siera_ds_read(&ds, DSK_BATTERY_MV, &b); EXPECT_EQ(b, 4200);
    s_adc = 3100; siera_ds_read(&ds, DSK_BATTERY_MV, &b); EXPECT_EQ(b, 3100);
}
```

## 4. Driver APIs

```c
// ESP32
extern const siera_ds_stream_api_t siera_drv_esp32_nvs_api;
extern const siera_ds_stream_api_t siera_drv_esp32_gpio_api;
extern const siera_ds_stream_api_t siera_drv_esp32_adc_api;
uint32_t siera_drv_esp32_now_ms(void);

// POSIX
extern const siera_ds_stream_api_t siera_drv_posix_fs_api;
uint32_t siera_drv_posix_now_ms(void);
```

## 5. Build & Test

```bash
cmake -B build -DSIERA_BUILD_TESTS=ON
cmake --build build
cd build && ctest --output-on-failure
```

## 6. ESP-IDF Component

```cmake
idf_component_register(
    SRCS
        "lib/core/siera_event/src/event.c"
        "lib/core/siera_timer/src/timer.c"
        "lib/core/siera_ds/src/ds.c"
        "lib/drivers/esp32/src/nvs.c"
        "lib/drivers/esp32/src/gpio.c"
        "lib/drivers/esp32/src/adc.c"
        "lib/drivers/esp32/src/pwm.c"
        "lib/drivers/esp32/src/time.c"
    INCLUDE_DIRS
        "lib/core/siera_common/include"
        "lib/core/siera_slist/include"
        "lib/core/siera_event/include"
        "lib/core/siera_timer/include"
        "lib/core/siera_ds/include"
        "lib/drivers/esp32/include"
    PRIV_INCLUDE_DIRS
        "lib/core/siera_ds"
        "lib/core/siera_ds/src"
    REQUIRES nvs_flash esp_timer driver)
```

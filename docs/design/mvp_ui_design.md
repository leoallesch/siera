# SIERA UI Library Design Document

## 1. Overview

This document describes a reactive UI architecture for the SIERA embedded framework. The design is driven by the database-as-model pattern: data changes flow through the database, and views react to those changes.

### 1.1 Goals

- **View interface**: Abstract view operations (`on_update`, `on_input`) - implementation is platform-specific
- **Reactive updates**: Views subscribe to relevant database keys and update when data changes
- **Bidirectional flow**: Views update display from model; user inputs write back to model
- **Embedded-friendly**: Static allocation, minimal memory footprint
- **No presenter**: Inputs write to database directly; other modules react via on_change

### 1.2 Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              APPLICATION                                     │
│  ┌────────────────────────────────────────────────────────────────────────┐ │
│  │                         MAIN LOOP (infinite)                            │ │
│  │   - Process sensors → database_write()                                  │ │
│  │   - Run timers → database_write()                                       │ │
│  │   - Handle communications → database_write()                            │ │
│  │   - display_tick() (processes inputs, triggers callbacks)               │ │
│  └────────────────────────────────────────────────────────────────────────┘ │
│                                                                              │
│  ┌─────────────────┐     ┌──────────────────────┐                          │
│  │  View Impls     │     │  App Database Keys   │                          │
│  │  (LVGL code)    │     │  (schema macros)     │                          │
│  └────────┬────────┘     └───────────┬──────────┘                          │
└───────────┼──────────────────────────┼──────────────────────────────────────┘
            │                          │
┌───────────┼──────────────────────────┼──────────────────────────────────────┐
│           │               SIERA UI LIBRARY                                   │
│           ▼                          ▼                                       │
│    ┌─────────────┐           ┌─────────────┐                                │
│    │    VIEW     │           │    MODEL    │                                │
│    │  INTERFACE  │           │  (Database) │                                │
│    │             │           │             │                                │
│    │ - on_update │◄──────────│ - on_change │                                │
│    │ - on_input  │           │ - read/write│                                │
│    └──────┬──────┘           └──────┬──────┘                                │
│           │                         │                                        │
│           │    ┌────────────────────┘                                        │
│           │    │                                                             │
│           ▼    ▼                                                             │
│    ┌─────────────────────────────────────────────────────────────────────┐  │
│    │                      SCREEN MANAGER                                  │  │
│    │  - Tracks active view                                                │  │
│    │  - Routes on_change to active view's on_update()                     │  │
│    │  - Handles screen transitions                                        │  │
│    └─────────────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────────────┘
            │
            │ View implementations are platform-specific
            ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                         VIEW IMPLEMENTATIONS                                 │
│    ┌─────────────┐     ┌─────────────┐     ┌─────────────┐                  │
│    │ home_view   │     │ settings    │     │ status_view │                  │
│    │   (LVGL)    │     │   (LVGL)    │     │   (LVGL)    │                  │
│    │             │     │             │     │             │                  │
│    │ knows its   │     │ knows its   │     │ knows its   │                  │
│    │ widgets &   │     │ widgets &   │     │ widgets &   │                  │
│    │ how to      │     │ how to      │     │ how to      │                  │
│    │ update them │     │ update them │     │ update them │                  │
│    └─────────────┘     └─────────────┘     └─────────────┘                  │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 1.3 Data Flow

```
┌──────────────────────────────────────────────────────────────────────────┐
│                    DATA CHANGE FLOW (Model → View)                        │
│                                                                          │
│  [Sensor/Timer/Comms/User Input]                                         │
│       │                                                                  │
│       ▼                                                                  │
│  [database_write(key, data)]                                             │
│       │                                                                  │
│       ▼                                                                  │
│  [on_change event published with key + data]                             │
│       │                                                                  │
│       ├──▶ [Other app modules listening] (business logic)                │
│       │                                                                  │
│       ▼                                                                  │
│  [Screen manager receives on_change]                                     │
│       │                                                                  │
│       ▼                                                                  │
│  [Is key in active view's relevant_keys?]                                │
│       │                                                                  │
│       ▼ (yes)                                                            │
│  [active_view->on_update(key, data)]                                     │
│       │                                                                  │
│       ▼                                                                  │
│  [View impl updates its widgets (LVGL calls, etc.)]                      │
└──────────────────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────────────────┐
│                    INPUT FLOW (View → Model)                              │
│                                                                          │
│  [User touch/click/encoder]                                              │
│       │                                                                  │
│       ▼                                                                  │
│  [display_tick() in main loop - LVGL processes events]                   │
│       │                                                                  │
│       ▼                                                                  │
│  [LVGL callback in view impl fires]                                      │
│       │                                                                  │
│       ▼                                                                  │
│  [View impl calls database_write() or screen_manager_goto()]             │
│       │                                                                  │
│       ├──▶ [database_write triggers on_change] ──▶ other modules react   │
│       │                                                                  │
│       └──▶ [screen_manager loads new view]                               │
└──────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Component Design

B### 2.1 Model Layer (Database)

The Model is the existing SIERA `s_database_t` - a type-safe key-value store with change notifications.

```c
// Database structure (existing)
typedef struct {
    const s_database_config_t* config;
    void* storage;
    event_t on_change;  // Publishes s_database_on_change_args_t
} s_database_t;

// Change notification payload
typedef struct {
    datastream_key_t key;
    const void* data;
} s_database_on_change_args_t;
```

The database is the single source of truth. All state changes (from sensors, timers, communications, or user input) go through `database_write()`. All interested parties (views, business logic modules) subscribe to `on_change`.

---

### 2.2 View Interface

The view interface is minimal. Each view implementation knows its own widgets and how to update them.

#### 2.2.1 View Interface

```c
/**
 * View interface - implemented by each screen
 *
 * The view implementation is platform-specific (LVGL, framebuffer, etc.)
 * and knows its own widgets. The interface just tells it what changed.
 */
typedef struct view_t view_t;

struct view_t {
    /**
     * Keys this view cares about. Screen manager checks this before
     * calling on_update. NULL-terminated array.
     */
    const datastream_key_t* relevant_keys;

    /**
     * Called when a relevant key changes in the database.
     * View implementation updates its widgets based on key and data.
     */
    void (*on_update)(view_t* self, datastream_key_t key, const void* data);

    /**
     * Called when view becomes active. Create/show widgets.
     * db and screen_mgr provided for input handling.
     */
    void (*on_load)(view_t* self, s_database_t* db, struct screen_manager_t* mgr);

    /**
     * Called when view becomes inactive. Destroy/hide widgets.
     */
    void (*on_unload)(view_t* self);

    /**
     * Implementation-specific data (LVGL objects, etc.)
     */
    void* impl;
};
```

#### 2.2.2 View Implementation Example (LVGL)

```c
// home_view.c - LVGL implementation

#include "view.h"
#include "lvgl.h"
#include "app_database.h"

// View-specific state
typedef struct {
    lv_obj_t* screen;
    lv_obj_t* lbl_temperature;
    lv_obj_t* lbl_humidity;
    lv_obj_t* btn_settings;
    s_database_t* db;
    screen_manager_t* mgr;
} home_view_impl_t;

static home_view_impl_t home_impl;

// Keys this view displays
static const datastream_key_t home_keys[] = {
    KEY_TEMPERATURE,
    KEY_HUMIDITY,
    KEY_COUNT  // Sentinel (use KEY_COUNT or define VIEW_KEY_END)
};

// Update widgets when data changes
static void home_on_update(view_t* self, datastream_key_t key, const void* data)
{
    home_view_impl_t* impl = self->impl;

    switch (key) {
    case KEY_TEMPERATURE: {
        int16_t temp = *(const int16_t*)data;
        lv_label_set_text_fmt(impl->lbl_temperature, "%d.%d°C",
                              temp / 10, temp % 10);
        break;
    }
    case KEY_HUMIDITY: {
        uint8_t hum = *(const uint8_t*)data;
        lv_label_set_text_fmt(impl->lbl_humidity, "%d%%", hum);
        break;
    }
    default:
        break;
    }
}

// Button click handler - writes to database or changes screen
static void on_settings_clicked(lv_event_t* e)
{
    home_view_impl_t* impl = lv_event_get_user_data(e);
    // Navigate to settings screen
    screen_manager_goto(impl->mgr, &settings_view);
}

// Create widgets when view loads
static void home_on_load(view_t* self, s_database_t* db, screen_manager_t* mgr)
{
    home_view_impl_t* impl = self->impl;
    impl->db = db;
    impl->mgr = mgr;

    // Create screen
    impl->screen = lv_obj_create(NULL);

    // Temperature label
    impl->lbl_temperature = lv_label_create(impl->screen);
    lv_obj_align(impl->lbl_temperature, LV_ALIGN_CENTER, 0, -30);
    lv_obj_set_style_text_font(impl->lbl_temperature, &lv_font_montserrat_48, 0);

    // Humidity label
    impl->lbl_humidity = lv_label_create(impl->screen);
    lv_obj_align(impl->lbl_humidity, LV_ALIGN_CENTER, 0, 30);

    // Settings button
    impl->btn_settings = lv_btn_create(impl->screen);
    lv_obj_align(impl->btn_settings, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_event_cb(impl->btn_settings, on_settings_clicked,
                        LV_EVENT_CLICKED, impl);

    lv_obj_t* btn_label = lv_label_create(impl->btn_settings);
    lv_label_set_text(btn_label, "Settings");
    lv_obj_center(btn_label);

    // Load the screen
    lv_screen_load(impl->screen);

    // Initial refresh from database
    int16_t temp;
    uint8_t hum;
    database_read(db, KEY_TEMPERATURE, &temp);
    database_read(db, KEY_HUMIDITY, &hum);
    home_on_update(self, KEY_TEMPERATURE, &temp);
    home_on_update(self, KEY_HUMIDITY, &hum);
}

// Destroy widgets when view unloads
static void home_on_unload(view_t* self)
{
    home_view_impl_t* impl = self->impl;
    if (impl->screen) {
        lv_obj_delete(impl->screen);
        impl->screen = NULL;
    }
}

// View instance
view_t home_view = {
    .relevant_keys = home_keys,
    .on_update = home_on_update,
    .on_load = home_on_load,
    .on_unload = home_on_unload,
    .impl = &home_impl,
};
```

#### 2.2.3 Input Handling Pattern

Inputs write directly to the database. Other modules react via on_change:

```c
// In a settings view - slider changed callback
static void on_brightness_changed(lv_event_t* e)
{
    settings_impl_t* impl = lv_event_get_user_data(e);
    lv_obj_t* slider = lv_event_get_target(e);

    uint8_t brightness = (uint8_t)lv_slider_get_value(slider);

    // Write directly to database - triggers on_change for all subscribers
    database_write(impl->db, KEY_BRIGHTNESS, &brightness);

    // A hardware module subscribed to KEY_BRIGHTNESS will receive
    // this change and update the actual display backlight
}
```

---

### 2.3 Screen Manager

The screen manager tracks the active view and routes database changes to it.

#### 2.3.1 Data Structures

```c
typedef struct screen_manager_t {
    view_t* active_view;
    s_database_t* db;
    event_subscription_t db_sub;
} screen_manager_t;
```

#### 2.3.2 Screen Manager API

```c
/**
 * Initialize screen manager with database reference
 */
void screen_manager_init(screen_manager_t* mgr, s_database_t* db);

/**
 * Load a view (unloads current view first if any)
 */
void screen_manager_goto(screen_manager_t* mgr, view_t* view);

/**
 * Get currently active view (or NULL)
 */
view_t* screen_manager_get_active(screen_manager_t* mgr);
```

#### 2.3.3 Implementation

```c
// screen_manager.c

static bool key_is_relevant(const datastream_key_t* keys, datastream_key_t key)
{
    if (!keys) return false;
    for (const datastream_key_t* k = keys; *k != KEY_COUNT; k++) {
        if (*k == key) return true;
    }
    return false;
}

static void on_db_change(void* context, const void* args)
{
    screen_manager_t* mgr = context;
    const s_database_on_change_args_t* change = args;

    if (mgr->active_view &&
        key_is_relevant(mgr->active_view->relevant_keys, change->key)) {
        mgr->active_view->on_update(mgr->active_view, change->key, change->data);
    }
}

void screen_manager_init(screen_manager_t* mgr, s_database_t* db)
{
    mgr->active_view = NULL;
    mgr->db = db;
    mgr->db_sub = (event_subscription_t){
        .callback = on_db_change,
        .context = mgr,
    };
    database_subscribe_all(db, &mgr->db_sub);
}

void screen_manager_goto(screen_manager_t* mgr, view_t* view)
{
    // Unload current view
    if (mgr->active_view && mgr->active_view->on_unload) {
        mgr->active_view->on_unload(mgr->active_view);
    }

    // Load new view
    mgr->active_view = view;
    if (view && view->on_load) {
        view->on_load(view, mgr->db, mgr);
    }
}
```

---

## 3. File Organization

### 3.1 Library Code (`src/ui/`)

```
src/ui/
├── CMakeLists.txt              # Build configuration
├── ui.h                        # Unified include
│
├── view.h                      # View interface definition
│
└── screen_manager/
    ├── screen_manager.h        # Screen manager API
    └── screen_manager.c        # Screen manager implementation
```

### 3.2 Application Code (`examples/ui_demo/`)

```
examples/ui_demo/
├── CMakeLists.txt              # Build configuration
├── main.c                      # Main loop, init
├── app_database.h              # Database schema
│
└── views/
    ├── views.h                 # View declarations
    ├── home_view.c             # Home screen (LVGL impl)
    └── settings_view.c         # Settings screen (LVGL impl)
```

---

## 4. Main Loop Integration

The UI integrates with the existing main loop pattern. No separate UI thread.

### 4.1 Example main.c

```c
#include "lvgl.h"
#include "ui.h"
#include "app_database.h"
#include "views/views.h"

// Database
static s_database_t db;
static app_database_storage_t db_storage;

// Screen manager
static screen_manager_t screen_mgr;

// Simulated sensor (would be real hardware module)
static timer_t sensor_timer;

static void sensor_timer_cb(void* ctx)
{
    s_database_t* db = ctx;

    // Simulate sensor reading
    static int16_t temp = 220;  // 22.0°C
    temp += (rand() % 10) - 5;

    // Write to database - triggers on_change to all subscribers
    // including the active view if it cares about KEY_TEMPERATURE
    database_write(db, KEY_TEMPERATURE, &temp);
}

int main(void)
{
    // Initialize LVGL
    lv_init();
    lv_display_t* disp = lv_sdl_window_create(480, 320);
    lv_indev_t* mouse = lv_sdl_mouse_create();
    lv_indev_set_display(mouse, disp);

    // Initialize database
    database_init(&db, &app_database_config, &db_storage);

    // Initialize screen manager (subscribes to database on_change)
    screen_manager_init(&screen_mgr, &db);

    // Start sensor timer (simulates real sensor module)
    timer_init(&sensor_timer, 1000, sensor_timer_cb, &db);
    timer_start(&sensor_timer);

    // Load initial view
    screen_manager_goto(&screen_mgr, &home_view);

    // Main loop - runs forever
    while (1) {
        // Process LVGL (handles display refresh and input events)
        uint32_t time_till_next = lv_timer_handler();

        // Process your other subsystems here:
        // - timer_tick_all();
        // - comms_process();
        // - sensor_poll();
        // etc.

        // Small delay
        SDL_Delay(time_till_next > 0 ? time_till_next : 1);
    }

    return 0;
}
```

---

## 5. Database Schema Example

### 5.1 app_database.h

```c
#pragma once

#include "database.h"

#define APP_DATABASE(ENTRY) \
    ENTRY(KEY_TEMPERATURE, int16_t)   \
    ENTRY(KEY_HUMIDITY, uint8_t)      \
    ENTRY(KEY_BRIGHTNESS, uint8_t)    \
    ENTRY(KEY_SOUND_ENABLED, bool)    \
    ENTRY(KEY_ACTIVE_SCREEN, uint8_t)

DATABASE_ENUM(APP_DATABASE)
DATABASE_STORAGE_TYPE(APP_DATABASE, app_database_storage_t)

extern const s_database_config_t app_database_config;
```

---

## 6. Build Configuration

### 6.1 CMakeLists.txt (library)

```cmake
# src/ui/CMakeLists.txt

add_library(siera_ui STATIC
    screen_manager/screen_manager.c
)

target_include_directories(siera_ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(siera_ui PUBLIC
    siera_core
)

# LVGL is linked by the application, not the library
# This keeps the library display-agnostic
```

### 6.2 CMakeLists.txt (application)

```cmake
# examples/ui_demo/CMakeLists.txt

add_executable(ui_demo
    main.c
    views/home_view.c
    views/settings_view.c
)

target_link_libraries(ui_demo PRIVATE
    siera_ui
    siera_core
    lvgl
    SDL2
)
```

---

## 7. Testing Strategy

### 7.1 Unit Tests

- **Screen manager**: Verify it routes on_change to active view only
- **View interface**: Mock view, verify lifecycle callbacks called correctly
- **Key filtering**: Verify on_update only called for relevant keys

### 7.2 Integration Tests

- Write to database, verify active view's on_update called
- Change screens, verify old view unloaded, new view loaded
- Verify input callbacks can write to database

### 7.3 Testing Without Hardware

Create a mock/console view implementation for testing:

```c
// test_view.c - Console-based view for testing

static void test_on_update(view_t* self, datastream_key_t key, const void* data)
{
    printf("View update: key=%d\n", key);
}

static void test_on_load(view_t* self, s_database_t* db, screen_manager_t* mgr)
{
    printf("View loaded\n");
}

static void test_on_unload(view_t* self)
{
    printf("View unloaded\n");
}

static const datastream_key_t test_keys[] = { KEY_TEMPERATURE, KEY_COUNT };

view_t test_view = {
    .relevant_keys = test_keys,
    .on_update = test_on_update,
    .on_load = test_on_load,
    .on_unload = test_on_unload,
    .impl = NULL,
};
```

---

## 8. Optional: LVGL XML Integration

LVGL 9.x supports XML view definitions that compile to C at build time. This can reduce boilerplate widget creation code while keeping the reactive architecture.

### 8.1 How It Works

XML defines the widget tree and layout. The compiler generates a `create()` function. Your view implementation:
1. Calls the generated create function
2. Looks up widgets by ID
3. Wires up callbacks and stores widget references

### 8.2 XML View Definition

```xml
<!-- views/xml/home_screen.xml -->
<component>
    <view extends="lv_obj" width="100%" height="100%">

        <lv_label id="lbl_temperature"
                  align="center" y="-30"
                  text="--°C"
                  style_text_font="lv_font_montserrat_48"/>

        <lv_label id="lbl_humidity"
                  align="center" y="30"
                  text="--%"/>

        <lv_button id="btn_settings"
                   align="bottom_mid" y="-20"
                   width="120" height="50">
            <lv_label text="Settings" align="center"/>
        </lv_button>
    </view>
</component>
```

### 8.3 Generated Code (build-time)

```c
// generated/home_screen.c (auto-generated, don't edit)
#include "lvgl.h"

lv_obj_t* home_screen_create(lv_obj_t* parent)
{
    lv_obj_t* root = lv_obj_create(parent);
    lv_obj_set_size(root, lv_pct(100), lv_pct(100));

    lv_obj_t* lbl_temperature = lv_label_create(root);
    lv_obj_set_id(lbl_temperature, "lbl_temperature");
    lv_obj_align(lbl_temperature, LV_ALIGN_CENTER, 0, -30);
    lv_label_set_text(lbl_temperature, "--°C");
    lv_obj_set_style_text_font(lbl_temperature, &lv_font_montserrat_48, 0);

    lv_obj_t* lbl_humidity = lv_label_create(root);
    lv_obj_set_id(lbl_humidity, "lbl_humidity");
    lv_obj_align(lbl_humidity, LV_ALIGN_CENTER, 0, 30);
    lv_label_set_text(lbl_humidity, "--%");

    lv_obj_t* btn_settings = lv_btn_create(root);
    lv_obj_set_id(btn_settings, "btn_settings");
    lv_obj_align(btn_settings, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_size(btn_settings, 120, 50);

    lv_obj_t* lbl = lv_label_create(btn_settings);
    lv_label_set_text(lbl, "Settings");
    lv_obj_center(lbl);

    return root;
}
```

### 8.4 View Implementation Using XML

```c
// views/home_view.c

#include "view.h"
#include "lvgl.h"
#include "app_database.h"

// Generated from XML
extern lv_obj_t* home_screen_create(lv_obj_t* parent);

typedef struct {
    lv_obj_t* screen;
    lv_obj_t* lbl_temperature;
    lv_obj_t* lbl_humidity;
    lv_obj_t* btn_settings;
    s_database_t* db;
    screen_manager_t* mgr;
} home_view_impl_t;

static home_view_impl_t home_impl;

static const datastream_key_t home_keys[] = {
    KEY_TEMPERATURE,
    KEY_HUMIDITY,
    KEY_COUNT
};

static void home_on_update(view_t* self, datastream_key_t key, const void* data)
{
    home_view_impl_t* impl = self->impl;

    switch (key) {
    case KEY_TEMPERATURE: {
        int16_t temp = *(const int16_t*)data;
        lv_label_set_text_fmt(impl->lbl_temperature, "%d.%d°C", temp / 10, temp % 10);
        break;
    }
    case KEY_HUMIDITY: {
        uint8_t hum = *(const uint8_t*)data;
        lv_label_set_text_fmt(impl->lbl_humidity, "%d%%", hum);
        break;
    }
    default:
        break;
    }
}

static void on_settings_clicked(lv_event_t* e)
{
    home_view_impl_t* impl = lv_event_get_user_data(e);
    screen_manager_goto(impl->mgr, &settings_view);
}

static void home_on_load(view_t* self, s_database_t* db, screen_manager_t* mgr)
{
    home_view_impl_t* impl = self->impl;
    impl->db = db;
    impl->mgr = mgr;

    // Create widgets from XML-generated function
    impl->screen = home_screen_create(NULL);

    // Look up widgets by ID (LVGL stores the id string)
    impl->lbl_temperature = lv_obj_get_child_by_id(impl->screen, "lbl_temperature");
    impl->lbl_humidity = lv_obj_get_child_by_id(impl->screen, "lbl_humidity");
    impl->btn_settings = lv_obj_get_child_by_id(impl->screen, "btn_settings");

    // Wire up callbacks
    lv_obj_add_event_cb(impl->btn_settings, on_settings_clicked, LV_EVENT_CLICKED, impl);

    // Load screen
    lv_screen_load(impl->screen);

    // Initial refresh
    int16_t temp;
    uint8_t hum;
    database_read(db, KEY_TEMPERATURE, &temp);
    database_read(db, KEY_HUMIDITY, &hum);
    home_on_update(self, KEY_TEMPERATURE, &temp);
    home_on_update(self, KEY_HUMIDITY, &hum);
}

static void home_on_unload(view_t* self)
{
    home_view_impl_t* impl = self->impl;
    if (impl->screen) {
        lv_obj_delete(impl->screen);
        impl->screen = NULL;
    }
}

view_t home_view = {
    .relevant_keys = home_keys,
    .on_update = home_on_update,
    .on_load = home_on_load,
    .on_unload = home_on_unload,
    .impl = &home_impl,
};
```

### 8.5 CMake Integration

```cmake
# examples/ui_demo/CMakeLists.txt

find_package(Python3 REQUIRED)

# XML source files
set(XML_VIEWS
    ${CMAKE_CURRENT_SOURCE_DIR}/views/xml/home_screen.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/views/xml/settings_screen.xml
)

# Generate C files from XML at build time
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

foreach(XML_FILE ${XML_VIEWS})
    get_filename_component(NAME ${XML_FILE} NAME_WE)
    set(OUT_FILE ${GENERATED_DIR}/${NAME}.c)
    add_custom_command(
        OUTPUT ${OUT_FILE}
        COMMAND ${Python3_EXECUTABLE} ${lvgl_SOURCE_DIR}/scripts/xml_to_c.py
                ${XML_FILE} -o ${OUT_FILE}
        DEPENDS ${XML_FILE}
        COMMENT "Generating ${NAME}.c from XML"
    )
    list(APPEND GENERATED_VIEWS ${OUT_FILE})
endforeach()

add_executable(ui_demo
    main.c
    views/home_view.c
    views/settings_view.c
    ${GENERATED_VIEWS}
)

target_include_directories(ui_demo PRIVATE ${GENERATED_DIR})
```

### 8.6 Trade-offs

| Approach | Pros | Cons |
|----------|------|------|
| **Pure C** | Full control, no build deps | More boilerplate |
| **XML + C** | Less layout code, visual editing | Build-time dependency, still need C for logic |

The XML approach works well when you have complex layouts but simple update logic. The view implementation still owns:
- Which keys to watch
- How to format/display data
- Input handling and navigation

---

## 9. Summary

### Key Differences from Traditional MVP

| Aspect | Traditional MVP | This Design |
|--------|-----------------|-------------|
| Presenter | Coordinates model and view | Not needed - database on_change does this |
| View updates | Presenter pushes to view | View subscribes to relevant keys |
| Input handling | View → Presenter → Model | View writes directly to database |
| Screen transitions | Presenter manages | Screen manager + database key |
| Display abstraction | None or full widget abstraction | View impl is platform-specific |

### Benefits

1. **Simpler**: No presenter layer, fewer abstractions
2. **Reactive**: Views automatically update when data changes
3. **Decoupled**: Views don't know about other modules; database is the bus
4. **Testable**: Mock views by implementing the interface
5. **Flexible**: View implementation can use any display library

### When to Use This Pattern

- Embedded systems with a main loop processing multiple subsystems
- Applications where the database is already the source of truth
- When you want views to react to data rather than be commanded

# Siera — Design Specification

## 1. Overview

| Library | Purpose | Dependencies |
|---|---|---|
| **siera_common** | Shared utility macros | None (header-only) |
| **siera_slist** | Intrusive singly-linked list | siera_common (header-only) |
| **siera_event** | Publish/subscribe event bus | siera_slist |
| **siera_timer** | Software timers | siera_slist |
| **siera_ds** | Compile-time datasource with stream backends | siera_event, siera_timer |

All core libraries always build.

```
  siera_common      (header-only)
       ↑
  siera_slist       (header-only)
       ↑         ↑
  siera_event  siera_timer
       ↑              ↑
       └── siera_ds ──┘
```

The event bus and timer manager are **application-owned**. The application creates them, passes pointers to the datasource, and shares them freely with modules. The datasource does not own these — it borrows them.

---

## 2. Common Utilities

```c
// siera/common.h

#ifndef SIERA_COMMON_H
#define SIERA_COMMON_H

#include <stddef.h>
#include <stdint.h>

#define SIERA_OFFSET_OF(type, member) \
    ((size_t)&(((type *)0)->member))

#define SIERA_CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - SIERA_OFFSET_OF(type, member)))

#define SIERA_NUM_ELEMENTS(array) \
    (sizeof(array) / sizeof((array)[0]))

#define SIERA_CONCAT_(a, b) a##b
#define SIERA_CONCAT(a, b)  SIERA_CONCAT_(a, b)

#endif
```

---

## 3. Intrusive Singly-Linked List

```c
// siera/slist.h

#ifndef SIERA_SLIST_H
#define SIERA_SLIST_H

#include <stdbool.h>
#include "siera/common.h"

typedef struct siera_slist_node {
    struct siera_slist_node *next;
} siera_slist_node_t;

typedef struct {
    siera_slist_node_t *first;
} siera_slist_head_t;

#define SIERA_SLIST_HEAD_INIT { .first = NULL }

static inline void siera_slist_init(siera_slist_head_t *head)       { head->first = NULL; }
static inline void siera_slist_node_init(siera_slist_node_t *node)  { node->next = NULL; }
static inline bool siera_slist_is_empty(const siera_slist_head_t *h){ return h->first == NULL; }

static inline void siera_slist_push(siera_slist_head_t *head,
                                     siera_slist_node_t *node) {
    node->next  = head->first;
    head->first = node;
}

static inline siera_slist_node_t *siera_slist_pop(siera_slist_head_t *head) {
    siera_slist_node_t *node = head->first;
    if (node) { head->first = node->next; node->next = NULL; }
    return node;
}

static inline bool siera_slist_remove(siera_slist_head_t *head,
                                       siera_slist_node_t *node) {
    siera_slist_node_t **pp = &head->first;
    while (*pp) {
        if (*pp == node) { *pp = node->next; node->next = NULL; return true; }
        pp = &(*pp)->next;
    }
    return false;
}

#define SIERA_SLIST_FOR_EACH_SAFE(head, cursor, tmp)       \
    for ((cursor) = (head)->first,                         \
         (tmp) = (cursor) ? (cursor)->next : NULL;         \
         (cursor) != NULL;                                 \
         (cursor) = (tmp),                                 \
         (tmp) = (cursor) ? (cursor)->next : NULL)

#endif
```

---

## 4. Event Bus (siera_event)

### 4.1 Header

```c
// siera/event.h

#ifndef SIERA_EVENT_H
#define SIERA_EVENT_H

#include <stdint.h>
#include <stddef.h>
#include "siera/slist.h"

typedef struct {
    uint32_t     id;
    const void  *data;
    size_t       size;
    const void  *old_data;
} siera_event_t;

typedef void (*siera_event_handler_fn)(const siera_event_t *event, void *ctx);

typedef struct {
    siera_slist_node_t     node;
    siera_event_handler_fn handler;
    void                  *ctx;
} siera_event_sub_t;

typedef struct {
    siera_slist_head_t subscribers;
} siera_event_bus_t;

void siera_event_bus_init(siera_event_bus_t *bus);
void siera_event_sub_init(siera_event_sub_t *sub, siera_event_handler_fn handler, void *ctx);
int  siera_event_subscribe(siera_event_bus_t *bus, siera_event_sub_t *sub);
int  siera_event_unsubscribe(siera_event_bus_t *bus, siera_event_sub_t *sub);
void siera_event_publish(siera_event_bus_t *bus, const siera_event_t *event);

#endif
```

### 4.2 Implementation

```c
// event.c

#include "siera/event.h"
#include "siera/common.h"

void siera_event_bus_init(siera_event_bus_t *bus) {
    siera_slist_init(&bus->subscribers);
}

void siera_event_sub_init(siera_event_sub_t *sub,
                           siera_event_handler_fn handler, void *ctx) {
    siera_slist_node_init(&sub->node);
    sub->handler = handler;
    sub->ctx     = ctx;
}

int siera_event_subscribe(siera_event_bus_t *bus, siera_event_sub_t *sub) {
    if (!bus || !sub || !sub->handler) return -1;
    siera_slist_push(&bus->subscribers, &sub->node);
    return 0;
}

int siera_event_unsubscribe(siera_event_bus_t *bus, siera_event_sub_t *sub) {
    if (!bus || !sub) return -1;
    return siera_slist_remove(&bus->subscribers, &sub->node) ? 0 : -1;
}

void siera_event_publish(siera_event_bus_t *bus, const siera_event_t *event) {
    siera_slist_node_t *node, *tmp;
    SIERA_SLIST_FOR_EACH_SAFE(&bus->subscribers, node, tmp) {
        siera_event_sub_t *sub = SIERA_CONTAINER_OF(node, siera_event_sub_t, node);
        sub->handler(event, sub->ctx);
    }
}
```

---

## 5. Software Timers (siera_timer)

### 5.1 Header

```c
// siera/timer.h

#ifndef SIERA_TIMER_H
#define SIERA_TIMER_H

#include <stdint.h>
#include <stdbool.h>
#include "siera/slist.h"

typedef uint32_t (*siera_timer_now_fn)(void);

typedef enum {
    SIERA_TIMER_ONCE   = 0,
    SIERA_TIMER_REPEAT = (1 << 0),
} siera_timer_flags_t;

typedef void (*siera_timer_cb_fn)(void *ctx);

typedef struct {
    siera_slist_node_t   node;
    siera_timer_cb_fn    callback;
    void                *ctx;
    uint32_t             interval_ms;
    uint32_t             expiry_ms;
    siera_timer_flags_t  flags;
    bool                 active;
} siera_timer_t;

typedef struct {
    siera_slist_head_t  timers;
    siera_timer_now_fn  now;
} siera_timer_mgr_t;

void siera_timer_mgr_init(siera_timer_mgr_t *mgr, siera_timer_now_fn now);
void siera_timer_init(siera_timer_t *t, siera_timer_cb_fn cb, void *ctx);
int  siera_timer_start(siera_timer_mgr_t *mgr, siera_timer_t *t,
                        uint32_t interval_ms, siera_timer_flags_t flags);
int  siera_timer_stop(siera_timer_mgr_t *mgr, siera_timer_t *t);
int  siera_timer_restart(siera_timer_mgr_t *mgr, siera_timer_t *t);
bool siera_timer_is_active(const siera_timer_t *t);
void siera_timer_tick(siera_timer_mgr_t *mgr);

#endif
```

Implementation: drift-corrected repeating timers, 32-bit wraparound-safe, safe self-restart from callback. Same as previous versions.

---

## 6. Datasource (siera_ds)

### 6.1 Stream Interface

```c
// siera/ds_stream.h

#ifndef SIERA_DS_STREAM_H
#define SIERA_DS_STREAM_H

#include <stdint.h>
#include <stddef.h>
#include "siera/ds_keys.h"

typedef struct {
    int (*read)(void *ctx, siera_ds_key_t key, void *buf, size_t size);
    int (*write)(void *ctx, siera_ds_key_t key, const void *buf, size_t size);
} siera_ds_stream_api_t;

typedef struct {
    const siera_ds_stream_api_t *api;
    void                        *ctx;
} siera_ds_stream_t;

typedef enum {
    SIERA_DS_RAM,
    SIERA_DS_PERSIST,
    SIERA_DS_GPIO,
    SIERA_DS_ADC,
    SIERA_DS_PWM,
    SIERA_DS_I2C,
    SIERA_DS_SPI,
    SIERA_DS_STREAM_TYPE_COUNT
} siera_ds_stream_type_t;

#endif
```

### 6.2 Key Registry

```c
// siera_ds_keys.def

SIERA_DS_KEY(SYSTEM_STATE,   uint8_t,    SIERA_DS_RAM,     0,     SIERA_DS_NONE)
SIERA_DS_KEY(UPTIME_SEC,     uint32_t,   SIERA_DS_RAM,     0,     SIERA_DS_NONE)
SIERA_DS_KEY(ALARM_FIRING,   bool,       SIERA_DS_RAM,     false, SIERA_DS_NONE)

SIERA_DS_KEY(ALARM_HOUR,     uint8_t,    SIERA_DS_PERSIST, 7,     SIERA_DS_NONE)
SIERA_DS_KEY(ALARM_MIN,      uint8_t,    SIERA_DS_PERSIST, 0,     SIERA_DS_NONE)
SIERA_DS_KEY(ALARM_ENABLED,  bool,       SIERA_DS_PERSIST, false, SIERA_DS_NONE)
SIERA_DS_KEY(BRIGHTNESS,     uint8_t,    SIERA_DS_PERSIST, 128,   SIERA_DS_NONE)

SIERA_DS_KEY(LED_STATUS,     bool,       SIERA_DS_GPIO,    false, SIERA_DS_NONE)
SIERA_DS_KEY(BUTTON_SET,     bool,       SIERA_DS_GPIO,    false, SIERA_DS_READONLY)

SIERA_DS_KEY(BATTERY_MV,     uint16_t,   SIERA_DS_ADC,     0,     SIERA_DS_READONLY)
SIERA_DS_KEY(LIGHT_SENSOR,   uint16_t,   SIERA_DS_ADC,     0,     SIERA_DS_READONLY)

SIERA_DS_KEY(BACKLIGHT_PWM,  uint8_t,    SIERA_DS_PWM,     0,     SIERA_DS_NONE)
```

### 6.3 Compile-Time Generation

```c
// ds_keys_internal.h

#ifndef SIERA_DS_KEYS_INTERNAL_H
#define SIERA_DS_KEYS_INTERNAL_H

#include "siera/ds_stream.h"
#include "siera/common.h"
#include <stdbool.h>
#include <string.h>

typedef enum {
    SIERA_DS_NONE     = 0,
    SIERA_DS_READONLY = (1 << 0),
    SIERA_DS_SILENT   = (1 << 1),
} siera_ds_flags_t;

#define SIERA_DS_KEY_EXPAND_ENUM(name, type, st, dv, fl) SIERA_DS_KEY_##name,

typedef enum {
    #define SIERA_DS_KEY(...) SIERA_DS_KEY_EXPAND_ENUM(__VA_ARGS__)
    #include "siera_ds_keys.def"
    #undef SIERA_DS_KEY
    SIERA_DS_KEY_COUNT
} siera_ds_key_t;

#define SIERA_DS_KEY_EXPAND_STORAGE(name, type, st, dv, fl) \
    uint8_t name[sizeof(type)];

typedef struct {
    #define SIERA_DS_KEY(...) SIERA_DS_KEY_EXPAND_STORAGE(__VA_ARGS__)
    #include "siera_ds_keys.def"
    #undef SIERA_DS_KEY
} siera_ds_cache_t;

#define SIERA_DS_CACHE_SIZE sizeof(siera_ds_cache_t)

typedef struct {
    const char              *name;
    siera_ds_stream_type_t   stream_type;
    size_t                   size;
    size_t                   offset;
    siera_ds_flags_t         flags;
} siera_ds_entry_t;

#define SIERA_DS_KEY_EXPAND_ENTRY(name, type, st, dv, fl) \
    [SIERA_DS_KEY_##name] = {                              \
        .name        = #name,                              \
        .stream_type = (st),                               \
        .size        = sizeof(type),                       \
        .offset      = offsetof(siera_ds_cache_t, name),   \
        .flags       = (fl),                               \
    },

static const siera_ds_entry_t _siera_ds_entry_table[SIERA_DS_KEY_COUNT] = {
    #define SIERA_DS_KEY(...) SIERA_DS_KEY_EXPAND_ENTRY(__VA_ARGS__)
    #include "siera_ds_keys.def"
    #undef SIERA_DS_KEY
};

#define SIERA_DS_KEY_EXPAND_DEFAULT(name, type, st, dv, fl) \
{                                                            \
    type _tmp = (dv);                                        \
    memcpy((uint8_t *)cache + offsetof(siera_ds_cache_t, name), \
           &_tmp, sizeof(type));                             \
}

static inline void _siera_ds_apply_defaults(siera_ds_cache_t *cache) {
    #define SIERA_DS_KEY(...) SIERA_DS_KEY_EXPAND_DEFAULT(__VA_ARGS__)
    #include "siera_ds_keys.def"
    #undef SIERA_DS_KEY
}

#ifndef SIERA_DS_MAX_VALUE_SIZE
#define SIERA_DS_MAX_VALUE_SIZE 8
#endif

#endif
```

### 6.4 Config

```c
typedef struct {
    siera_ds_stream_type_t  type;
    siera_ds_stream_t      *stream;
} siera_ds_stream_binding_t;

#define SIERA_DS_STREAM_END { .stream = NULL }

typedef struct {
    const siera_ds_stream_binding_t *streams;
    siera_event_bus_t               *events;   // externally owned
    siera_timer_mgr_t               *timers;   // externally owned
    uint32_t                         flush_interval_ms; // 0 = write-through
} siera_ds_config_t;
```

Both the event bus and timer manager are created and owned by the application. The datasource borrows pointers to them. This means:
- Modules subscribe to the same event bus directly
- Modules register timers on the same timer manager directly
- The datasource doesn't own infrastructure — it uses it

### 6.5 Datasource Object

```c
typedef struct {
    const siera_ds_entry_t  *entries;
    siera_ds_cache_t         cache;
    siera_event_bus_t       *events;    // borrowed
    siera_timer_mgr_t       *timers;    // borrowed
    uint32_t                 flush_interval_ms;
    uint32_t                 dirty[(SIERA_DS_KEY_COUNT + 31) / 32];
    siera_ds_stream_t       *stream_map[SIERA_DS_STREAM_TYPE_COUNT];
    siera_timer_t            flush_timer;
} siera_ds_t;
```

### 6.6 Public API

```c
// siera/ds.h

#ifndef SIERA_DS_H
#define SIERA_DS_H

#include "siera/ds_keys.h"
#include "siera/ds_stream.h"
#include "siera/event.h"
#include "siera/timer.h"

int  siera_ds_init(siera_ds_t *ds, const siera_ds_config_t *config);
int  siera_ds_read(const siera_ds_t *ds, siera_ds_key_t key, void *out);
int  siera_ds_write(siera_ds_t *ds, siera_ds_key_t key, const void *in);
void siera_ds_deinit(siera_ds_t *ds);

const char              *siera_ds_key_name(siera_ds_key_t key);
size_t                   siera_ds_key_size(siera_ds_key_t key);
siera_ds_stream_type_t   siera_ds_key_stream_type(siera_ds_key_t key);

#endif
```

No `siera_ds_tick` — the application calls `siera_timer_tick` on the timer manager directly. No `siera_ds_flush` — persistence is internal.

### 6.7 Implementation

```c
// ds.c

#include "siera/ds.h"
#include "ds_keys_internal.h"
#include <string.h>

static inline siera_ds_stream_t *ds_stream(const siera_ds_t *ds,
                                             siera_ds_stream_type_t type) {
    return (type < SIERA_DS_STREAM_TYPE_COUNT) ? ds->stream_map[type] : NULL;
}

static int ds_flush(siera_ds_t *ds) {
    siera_ds_stream_t *s = ds_stream(ds, SIERA_DS_PERSIST);
    if (!s || !s->api->write) return 0;
    int flushed = 0;
    for (int i = 0; i < SIERA_DS_KEY_COUNT; i++) {
        uint32_t word = i / 32, bit = 1u << (i % 32);
        if (ds->dirty[word] & bit) {
            const siera_ds_entry_t *e = &ds->entries[i];
            if (s->api->write(s->ctx, (siera_ds_key_t)i,
                              (uint8_t *)&ds->cache + e->offset, e->size) == 0) {
                ds->dirty[word] &= ~bit;
                flushed++;
            }
        }
    }
    return flushed;
}

static void ds_flush_cb(void *ctx) { ds_flush((siera_ds_t *)ctx); }

int siera_ds_init(siera_ds_t *ds, const siera_ds_config_t *config) {
    memset(ds, 0, sizeof(*ds));
    ds->entries            = _siera_ds_entry_table;
    ds->events             = config->events;
    ds->timers             = config->timers;
    ds->flush_interval_ms  = config->flush_interval_ms;

    if (config->streams) {
        for (const siera_ds_stream_binding_t *b = config->streams;
             b->stream; b++) {
            if (b->type < SIERA_DS_STREAM_TYPE_COUNT)
                ds->stream_map[b->type] = b->stream;
        }
    }

    _siera_ds_apply_defaults(&ds->cache);

    // Hydrate from streams
    for (int i = 0; i < SIERA_DS_KEY_COUNT; i++) {
        const siera_ds_entry_t *e = &ds->entries[i];
        if (e->stream_type == SIERA_DS_RAM) continue;
        siera_ds_stream_t *s = ds_stream(ds, e->stream_type);
        if (s && s->api->read)
            s->api->read(s->ctx, (siera_ds_key_t)i,
                         (uint8_t *)&ds->cache + e->offset, e->size);
    }

    // Auto-flush timer
    if (ds->flush_interval_ms > 0 && ds->timers) {
        siera_timer_init(&ds->flush_timer, ds_flush_cb, ds);
        siera_timer_start(ds->timers, &ds->flush_timer,
                          ds->flush_interval_ms, SIERA_TIMER_REPEAT);
    }

    return 0;
}

int siera_ds_read(const siera_ds_t *ds, siera_ds_key_t key, void *out) {
    if (key >= SIERA_DS_KEY_COUNT) return -1;
    const siera_ds_entry_t *e = &ds->entries[key];

    if (e->stream_type != SIERA_DS_RAM && e->stream_type != SIERA_DS_PERSIST) {
        siera_ds_stream_t *s = ds_stream(ds, e->stream_type);
        if (s && s->api->read)
            s->api->read(s->ctx, key,
                         (uint8_t *)&((siera_ds_t *)ds)->cache + e->offset,
                         e->size);
    }

    memcpy(out, (const uint8_t *)&ds->cache + e->offset, e->size);
    return 0;
}

int siera_ds_write(siera_ds_t *ds, siera_ds_key_t key, const void *in) {
    if (key >= SIERA_DS_KEY_COUNT) return -1;
    const siera_ds_entry_t *e = &ds->entries[key];
    if (e->flags & SIERA_DS_READONLY) return -1;

    uint8_t *cached = (uint8_t *)&ds->cache + e->offset;
    if (memcmp(cached, in, e->size) == 0) return 0;

    uint8_t old[SIERA_DS_MAX_VALUE_SIZE];
    memcpy(old, cached, e->size);
    memcpy(cached, in, e->size);

    if (e->stream_type == SIERA_DS_PERSIST) {
        if (ds->flush_interval_ms > 0) {
            ds->dirty[key / 32] |= (1u << (key % 32));
        } else {
            siera_ds_stream_t *s = ds_stream(ds, SIERA_DS_PERSIST);
            if (s && s->api->write)
                s->api->write(s->ctx, key, in, e->size);
        }
    } else if (e->stream_type != SIERA_DS_RAM) {
        siera_ds_stream_t *s = ds_stream(ds, e->stream_type);
        if (s && s->api->write)
            s->api->write(s->ctx, key, in, e->size);
    }

    if (!(e->flags & SIERA_DS_SILENT) && ds->events) {
        siera_event_t event = {
            .id = (uint32_t)key, .data = in,
            .old_data = old, .size = e->size,
        };
        siera_event_publish(ds->events, &event);
    }
    return 0;
}

void siera_ds_deinit(siera_ds_t *ds) {
    if (ds->flush_interval_ms > 0 && ds->timers)
        siera_timer_stop(ds->timers, &ds->flush_timer);
    ds_flush(ds);
}

const char *siera_ds_key_name(siera_ds_key_t key) {
    return (key < SIERA_DS_KEY_COUNT) ? _siera_ds_entry_table[key].name : "?";
}
size_t siera_ds_key_size(siera_ds_key_t key) {
    return (key < SIERA_DS_KEY_COUNT) ? _siera_ds_entry_table[key].size : 0;
}
siera_ds_stream_type_t siera_ds_key_stream_type(siera_ds_key_t key) {
    return (key < SIERA_DS_KEY_COUNT)
        ? _siera_ds_entry_table[key].stream_type : SIERA_DS_RAM;
}
```

---

## 7. Application Wiring

Everything is externally owned. The datasource borrows.

```c
// Application owns these
static siera_event_bus_t  g_events;
static siera_timer_mgr_t g_timers;
static siera_ds_t        g_ds;

int main(void) {
    siera_event_bus_init(&g_events);
    siera_timer_mgr_init(&g_timers, hal_millis);
    siera_ds_init(&g_ds, &(siera_ds_config_t){
        .streams            = streams,
        .events             = &g_events,
        .timers             = &g_timers,
        .flush_interval_ms  = 5000,
    });

    // Modules get ds pointer — use ds->events and ds->timers
    alarm_init(&g_alarm, &g_ds);
    display_init(&g_display, &g_ds);

    // Main loop drives the timer manager directly
    while (1) {
        siera_timer_tick(&g_timers);
        hal_delay_ms(1);
    }
}
```

Module code:
```c
void alarm_init(alarm_module_t *mod, siera_ds_t *ds) {
    mod->ds = ds;

    // Subscribe to ds events via the borrowed event bus
    siera_event_sub_init(&mod->sub, alarm_on_event, mod);
    siera_event_subscribe(ds->events, &mod->sub);

    // Register timer via the borrowed timer manager
    siera_timer_init(&mod->check_timer, alarm_check_cb, mod);
    siera_timer_start(ds->timers, &mod->check_timer, 1000, SIERA_TIMER_REPEAT);
}
```

# Siera — Bare-Metal Example

```c
// examples/bare_metal/main.c

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "siera/ds.h"
#include "siera/ds_stream.h"
#include "siera/event.h"
#include "siera/timer.h"
#include "siera/common.h"

#include "hal_gpio.h"
#include "hal_adc.h"
#include "hal_eeprom.h"
#include "hal_timer.h"
#include "hal_uart.h"

// ═════════════════════════════════════════════════════════════
//  Streams
// ═════════════════════════════════════════════════════════════

// ── EEPROM persistence ──────────────────────────────────────

static uint16_t eeprom_hash(const char *name) {
    uint16_t h = 0;
    while (*name) h = h * 31 + (uint8_t)*name++;
    return h % 512;
}

static int eeprom_read(void *ctx, siera_ds_key_t key, void *buf, size_t size) {
    (void)ctx;
    return hal_eeprom_read(eeprom_hash(siera_ds_key_name(key)), buf, size);
}

static int eeprom_write(void *ctx, siera_ds_key_t key,
                         const void *buf, size_t size) {
    (void)ctx;
    return hal_eeprom_write(eeprom_hash(siera_ds_key_name(key)), buf, size);
}

static const siera_ds_stream_api_t eeprom_api = { eeprom_read, eeprom_write };
static siera_ds_stream_t eeprom_stream = { &eeprom_api, NULL };

// ── GPIO ────────────────────────────────────────────────────

typedef struct { siera_ds_key_t key; uint8_t pin; bool output; } gpio_pin_t;
typedef struct { const gpio_pin_t *pins; int count; } gpio_ctx_t;

static const gpio_pin_t *gpio_find(const gpio_ctx_t *c, siera_ds_key_t k) {
    for (int i = 0; i < c->count; i++)
        if (c->pins[i].key == k) return &c->pins[i];
    return NULL;
}

static int gpio_read(void *ctx, siera_ds_key_t key, void *buf, size_t size) {
    const gpio_pin_t *p = gpio_find(ctx, key);
    if (!p) return -1;
    bool level = hal_gpio_read(p->pin);
    memcpy(buf, &level, sizeof(bool));
    return 0;
}

static int gpio_write(void *ctx, siera_ds_key_t key,
                       const void *buf, size_t size) {
    const gpio_pin_t *p = gpio_find(ctx, key);
    if (!p || !p->output) return -1;
    bool level; memcpy(&level, buf, sizeof(bool));
    hal_gpio_write(p->pin, level);
    return 0;
}

static const siera_ds_stream_api_t gpio_api = { gpio_read, gpio_write };
static const gpio_pin_t gpio_pins[] = {
    { SIERA_DS_KEY_LED_STATUS, 13, true  },
    { SIERA_DS_KEY_BUTTON_SET,  2, false },
};
static gpio_ctx_t gpio_ctx = { gpio_pins, SIERA_NUM_ELEMENTS(gpio_pins) };
static siera_ds_stream_t gpio_stream = { &gpio_api, &gpio_ctx };

// ── ADC ─────────────────────────────────────────────────────

typedef struct { siera_ds_key_t key; uint8_t channel; } adc_ch_t;
typedef struct { const adc_ch_t *channels; int count; } adc_ctx_t;

static int adc_read(void *ctx, siera_ds_key_t key, void *buf, size_t size) {
    adc_ctx_t *c = ctx;
    for (int i = 0; i < c->count; i++) {
        if (c->channels[i].key == key) {
            uint16_t mv = hal_adc_read_mv(c->channels[i].channel);
            memcpy(buf, &mv, sizeof(uint16_t));
            return 0;
        }
    }
    return -1;
}

static const siera_ds_stream_api_t adc_api = { adc_read, NULL };
static const adc_ch_t adc_channels[] = {
    { SIERA_DS_KEY_BATTERY_MV,   0 },
    { SIERA_DS_KEY_LIGHT_SENSOR, 1 },
};
static adc_ctx_t adc_ctx = { adc_channels, SIERA_NUM_ELEMENTS(adc_channels) };
static siera_ds_stream_t adc_stream = { &adc_api, &adc_ctx };

// ═════════════════════════════════════════════════════════════
//  Config
// ═════════════════════════════════════════════════════════════

static const siera_ds_stream_binding_t streams[] = {
    { SIERA_DS_NVS, &eeprom_stream },
    { SIERA_DS_GPIO,    &gpio_stream   },
    { SIERA_DS_ADC,     &adc_stream    },
    SIERA_DS_STREAM_END
};

// ═════════════════════════════════════════════════════════════
//  Modules
// ═════════════════════════════════════════════════════════════

typedef struct {
    siera_ds_t         *ds;
    siera_event_sub_t   sub;
} display_module_t;

static void display_on_event(const siera_event_t *event, void *ctx) {
    switch ((siera_ds_key_t)event->id) {
    case SIERA_DS_KEY_BRIGHTNESS:
        hal_uart_printf("[disp] brightness=%u\n", *(const uint8_t *)event->data); break;
    case SIERA_DS_KEY_ALARM_FIRING:
        hal_uart_printf("[disp] firing=%d\n", *(const bool *)event->data); break;
    default: break;
    }
}

static void display_init(display_module_t *mod, siera_ds_t *ds) {
    mod->ds = ds;
    siera_event_sub_init(&mod->sub, display_on_event, mod);
    siera_event_subscribe(ds->events, &mod->sub);
}

// ─────────────────────────────────────────────────────────────

typedef struct {
    siera_ds_t         *ds;
    siera_event_sub_t   sub;
    siera_timer_t       check_timer;
    bool armed; uint8_t hour, min;
} alarm_module_t;

static void alarm_on_event(const siera_event_t *event, void *ctx) {
    alarm_module_t *mod = ctx;
    switch ((siera_ds_key_t)event->id) {
    case SIERA_DS_KEY_ALARM_ENABLED: mod->armed = *(const bool *)event->data; break;
    case SIERA_DS_KEY_ALARM_HOUR:    mod->hour = *(const uint8_t *)event->data; break;
    case SIERA_DS_KEY_ALARM_MIN:     mod->min = *(const uint8_t *)event->data; break;
    default: break;
    }
}

static void alarm_check_cb(void *ctx) {
    alarm_module_t *mod = ctx;
    if (!mod->armed) return;
    uint8_t rtc_h = 6, rtc_m = 30;
    bool already;
    siera_ds_read(mod->ds, SIERA_DS_KEY_ALARM_FIRING, &already);
    if (!already && rtc_h == mod->hour && rtc_m == mod->min) {
        bool fire = true;
        siera_ds_write(mod->ds, SIERA_DS_KEY_ALARM_FIRING, &fire);
    }
}

static void alarm_init(alarm_module_t *mod, siera_ds_t *ds) {
    mod->ds = ds;
    siera_ds_read(ds, SIERA_DS_KEY_ALARM_ENABLED, &mod->armed);
    siera_ds_read(ds, SIERA_DS_KEY_ALARM_HOUR,    &mod->hour);
    siera_ds_read(ds, SIERA_DS_KEY_ALARM_MIN,     &mod->min);
    siera_event_sub_init(&mod->sub, alarm_on_event, mod);
    siera_event_subscribe(ds->events, &mod->sub);
    siera_timer_init(&mod->check_timer, alarm_check_cb, mod);
    siera_timer_start(ds->timers, &mod->check_timer, 1000, SIERA_TIMER_REPEAT);
}

// ─────────────────────────────────────────────────────────────

typedef struct { siera_ds_t *ds; siera_timer_t poll_timer; bool last_btn; } input_module_t;

static void input_poll_cb(void *ctx) {
    input_module_t *mod = ctx;
    bool btn; siera_ds_read(mod->ds, SIERA_DS_KEY_BUTTON_SET, &btn);
    if (btn && !mod->last_btn) {
        bool firing; siera_ds_read(mod->ds, SIERA_DS_KEY_ALARM_FIRING, &firing);
        if (firing) { bool d = false; siera_ds_write(mod->ds, SIERA_DS_KEY_ALARM_FIRING, &d); }
    }
    mod->last_btn = btn;
}

static void input_init(input_module_t *mod, siera_ds_t *ds) {
    mod->ds = ds; mod->last_btn = false;
    siera_timer_init(&mod->poll_timer, input_poll_cb, mod);
    siera_timer_start(ds->timers, &mod->poll_timer, 50, SIERA_TIMER_REPEAT);
}

// ─────────────────────────────────────────────────────────────

typedef struct { siera_ds_t *ds; siera_timer_t sample_timer; } sensor_module_t;

static void sensor_sample_cb(void *ctx) {
    sensor_module_t *mod = ctx;
    uint16_t light; siera_ds_read(mod->ds, SIERA_DS_KEY_LIGHT_SENSOR, &light);
    uint8_t brightness = (light > 2000) ? 255 : (uint8_t)(light / 8);
    siera_ds_write(mod->ds, SIERA_DS_KEY_BRIGHTNESS, &brightness);
}

static void sensor_init(sensor_module_t *mod, siera_ds_t *ds) {
    mod->ds = ds;
    siera_timer_init(&mod->sample_timer, sensor_sample_cb, mod);
    siera_timer_start(ds->timers, &mod->sample_timer, 200, SIERA_TIMER_REPEAT);
}

// ═════════════════════════════════════════════════════════════
//  Main
// ═════════════════════════════════════════════════════════════

static siera_event_bus_t  g_events;
static siera_timer_mgr_t  g_timers;
static siera_ds_t         g_ds;
static display_module_t   g_display;
static alarm_module_t     g_alarm;
static input_module_t     g_input;
static sensor_module_t    g_sensor;

static uint32_t now_ms(void) { return hal_millis(); }

int main(void) {
    for (int i = 0; i < (int)SIERA_NUM_ELEMENTS(gpio_pins); i++)
        hal_gpio_init(gpio_pins[i].pin, gpio_pins[i].output);
    for (int i = 0; i < (int)SIERA_NUM_ELEMENTS(adc_channels); i++)
        hal_adc_init(adc_channels[i].channel);
    hal_eeprom_init();
    hal_uart_init(115200);

    // Application owns infrastructure
    siera_event_bus_init(&g_events);
    siera_timer_mgr_init(&g_timers, now_ms);

    // Datasource borrows them
    siera_ds_init(&g_ds, &(siera_ds_config_t){
        .streams           = streams,
        .events            = &g_events,
        .timers            = &g_timers,
        .flush_interval_ms = 5000,
    });

    // Modules get ds pointer — access events and timers through it
    display_init(&g_display, &g_ds);
    alarm_init(&g_alarm, &g_ds);
    input_init(&g_input, &g_ds);
    sensor_init(&g_sensor, &g_ds);

    uint8_t hour = 6, min = 30;
    bool arm = true, led = true;
    siera_ds_write(&g_ds, SIERA_DS_KEY_ALARM_HOUR, &hour);
    siera_ds_write(&g_ds, SIERA_DS_KEY_ALARM_MIN, &min);
    siera_ds_write(&g_ds, SIERA_DS_KEY_ALARM_ENABLED, &arm);
    siera_ds_write(&g_ds, SIERA_DS_KEY_LED_STATUS, &led);

    hal_uart_printf("=== running ===\n");

    // Application drives the timer manager
    while (1) {
        siera_timer_tick(&g_timers);
        hal_delay_ms(1);
    }

    siera_ds_deinit(&g_ds);
    return 0;
}
```

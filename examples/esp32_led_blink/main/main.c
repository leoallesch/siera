#include <stdbool.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "siera/ds.h"
#include "siera/esp32_ds_gpio.h"
#include "siera/esp32_gpio.h"
#include "siera/esp32_timesource.h"
#include "siera/timer.h"

/* ── Pin assignment ─────────────────────────────────────────────────────── */

#define LED_PIN 2 /* GPIO2 — built-in LED on most ESP32 devkits */

/* ── GPIO datastream pin table ──────────────────────────────────────────── */

static const siera_esp32_ds_gpio_pin_t g_gpio_pins[] = {
  { DSK_LED_STATE, LED_PIN, SIERA_GPIO_DIR_OUTPUT, SIERA_GPIO_PULL_NONE },
};

/* ── Timer callback: toggle LED every 500 ms ────────────────────────────── */

static void on_blink(void* ctx)
{
  i_siera_ds_t* ds = ctx;

  bool state;
  siera_ds_read(ds, DSK_LED_STATE, &state);
  state = !state;
  siera_ds_write(ds, DSK_LED_STATE, &state);
}

/* ── Change subscriber: log every datastream write ──────────────────────── */

static void on_change(const void* args, void* ctx)
{
  (void)ctx;
  const siera_ds_on_change_args_t* e = (const siera_ds_on_change_args_t*)args;
  bool state;
  /* The data pointer in the event holds the new value directly */
  state = *(const bool*)e->data;
  printf("[ds] %s → %s\n", siera_ds_key_name(e->key), state ? "ON" : "OFF");
}

/* ── app_main ───────────────────────────────────────────────────────────── */

void app_main(void)
{
  /* 1. HAL: ESP32 GPIO driver */
  siera_hal_gpio_t* gpio_hal = siera_esp32_gpio_init();

  /* 2. GPIO datastream stream */
  static siera_esp32_ds_gpio_t gpio_stream;
  siera_esp32_ds_gpio_init(&gpio_stream, gpio_hal, g_gpio_pins, 1);

  /* 3. Bind the GPIO stream into the datastream */
  static const siera_ds_stream_binding_t bindings[] = {
    { SIERA_DS_GPIO, &gpio_stream.stream },
  };

  /* 4. Timesource */
  siera_timesource_t* timesource = siera_esp32_timesource_init();

  /* 5. Timer manager */
  static siera_timer_mgr_t timers;
  siera_timer_mgr_init(&timers, timesource);

  /* 6. Datastream */
  static i_siera_ds_t ds;
  siera_ds_init(&ds, bindings, 1, &timers, 0);

  /* 7. Subscribe to all changes */
  static siera_event_sub_t sub;
  siera_event_sub_init(&sub, on_change, NULL);
  siera_ds_subscribe_all(&ds, &sub);

  /* 8. Start 500 ms repeating blink timer */
  static siera_timer_t blink_timer;
  siera_timer_start(&timers, &blink_timer, on_blink, &ds, 500, true);

  printf("=== esp32_led_blink ===\n");

  /* 9. Main loop: tick the timer manager */
  while(1) {
    siera_timer_tick(&timers);
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

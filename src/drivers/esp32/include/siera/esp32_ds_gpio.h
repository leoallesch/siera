#ifndef SIERA_ESP32_DS_GPIO_H
#define SIERA_ESP32_DS_GPIO_H

#include <stdbool.h>
#include "siera/ds_stream.h"
#include "siera/hal_gpio.h"
#include "siera/timer.h"

/*
 * GPIO datastream stream for siera_ds.
 *
 * Maps siera_ds keys typed SIERA_DS_GPIO to physical GPIO pins via a
 * compile-time lookup table.  Each entry pairs a key with its pin number,
 * direction, and optional polled mode.
 *
 * Polled mode: when a pin has polled=true, a repeating timer samples the pin
 * at poll_ms intervals and caches the result.  siera_ds_read() returns the
 * cached value without touching the hardware.  Non-polled pins are sampled
 * fresh on every read.
 *
 * A single shared timer fires at the minimum poll_ms across all polled pins.
 * timers may be NULL if no pin has polled=true.
 *
 * Usage:
 *   static const siera_esp32_ds_gpio_pin_t pins[] = {
 *     { DSK_LED_STATUS, GPIO_NUM_2,  SIERA_GPIO_DIR_OUTPUT, SIERA_GPIO_PULL_NONE, false, 0  },
 *     { DSK_BUTTON_SET, GPIO_NUM_0,  SIERA_GPIO_DIR_INPUT,  SIERA_GPIO_PULL_UP,   true,  20 },
 *   };
 *   static bool cache[2];
 *   static siera_esp32_ds_gpio_t gpio_stream;
 *   siera_esp32_ds_gpio_init(&gpio_stream, gpio_hal, pins, 2, cache, &timers);
 *
 *   // Bind to datastream:
 *   siera_ds_stream_binding_t bindings[] = {
 *     { SIERA_DS_GPIO, &gpio_stream.interface },
 *   };
 */

typedef struct {
  siera_dsk_t key;
  uint32_t pin;
  siera_gpio_dir_t dir;
  siera_gpio_pull_t pull;
  bool polled; /* sample on repeating timer */
  uint32_t poll_ms; /* poll interval in ms (0 = use minimum of others) */
} siera_esp32_ds_gpio_pin_t;

typedef struct {
  i_siera_ds_t interface;
  siera_event_t on_change;
  siera_hal_gpio_t* hal;
  const siera_esp32_ds_gpio_pin_t* pins;
  size_t pin_count;
  bool* cache; /* caller-supplied bool[pin_count] */
  siera_timer_t poll_timer;
} siera_esp32_ds_gpio_t;

void siera_esp32_ds_gpio_init(siera_esp32_ds_gpio_t* self,
  siera_hal_gpio_t* hal,
  const siera_esp32_ds_gpio_pin_t* pins,
  size_t pin_count,
  bool* cache,
  siera_timer_mgr_t* timers);

#endif /* SIERA_ESP32_DS_GPIO_H */

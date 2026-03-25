#ifndef SIERA_ESP32_DS_GPIO_H
#define SIERA_ESP32_DS_GPIO_H

#include "siera/ds_stream.h"
#include "siera/hal_gpio.h"

/*
 * GPIO datastream stream for siera_ds.
 *
 * Maps siera_ds keys typed SIERA_DS_GPIO to physical GPIO pins via a
 * compile-time lookup table.  Each entry pairs a key with its pin number and
 * direction; the HAL gpio interface performs the actual read/write.
 *
 * Usage:
 *   static siera_esp32_ds_gpio_pin_t pins[] = {
 *     { DSK_LED_STATUS,  GPIO_NUM_2,  SIERA_GPIO_DIR_OUTPUT, SIERA_GPIO_PULL_NONE },
 *     { DSK_BUTTON_SET,  GPIO_NUM_0,  SIERA_GPIO_DIR_INPUT,  SIERA_GPIO_PULL_UP   },
 *   };
 *   static siera_esp32_ds_gpio_t gpio_stream;
 *   siera_esp32_ds_gpio_init(&gpio_stream, &gpio_hal.interface, pins, 2);
 *
 *   // Bind to datastream:
 *   siera_ds_stream_binding_t bindings[] = {
 *     { SIERA_DS_GPIO, &gpio_stream.stream },
 *   };
 */

typedef struct {
  siera_dsk_t   key;
  uint32_t         pin;
  siera_gpio_dir_t  dir;
  siera_gpio_pull_t pull;
} siera_esp32_ds_gpio_pin_t;

typedef struct {
  siera_ds_stream_t                 stream;
  siera_hal_gpio_t                 *hal;
  const siera_esp32_ds_gpio_pin_t  *pins;
  size_t                            pin_count;
} siera_esp32_ds_gpio_t;

void siera_esp32_ds_gpio_init(siera_esp32_ds_gpio_t           *self,
                               siera_hal_gpio_t                *hal,
                               const siera_esp32_ds_gpio_pin_t *pins,
                               size_t                           pin_count);

#endif /* SIERA_ESP32_DS_GPIO_H */

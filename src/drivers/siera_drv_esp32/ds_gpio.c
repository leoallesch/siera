#include "siera/esp32_ds_gpio.h"

#include <stddef.h>

static const siera_esp32_ds_gpio_pin_t* find_pin(const siera_esp32_ds_gpio_t* drv,
  siera_ds_key_t key)
{
  for(size_t i = 0; i < drv->pin_count; i++) {
    if(drv->pins[i].key == key)
      return &drv->pins[i];
  }
  return NULL;
}

static int _read(void* ctx, siera_ds_key_t key, void* buf, size_t size)
{
  (void)size;
  siera_esp32_ds_gpio_t* drv = (siera_esp32_ds_gpio_t*)ctx;
  const siera_esp32_ds_gpio_pin_t* p = find_pin(drv, key);
  if(!p)
    return -1;

  bool level = false;
  int ret = siera_gpio_read(drv->hal, p->pin, &level);
  if(ret != 0)
    return -1;

  *(bool*)buf = level;
  return 0;
}

static int _write(void* ctx, siera_ds_key_t key, const void* buf, size_t size)
{
  (void)size;
  siera_esp32_ds_gpio_t* drv = (siera_esp32_ds_gpio_t*)ctx;
  const siera_esp32_ds_gpio_pin_t* p = find_pin(drv, key);
  if(!p)
    return -1;

  return siera_gpio_write(drv->hal, p->pin, *(const bool*)buf);
}

static const siera_ds_stream_api_t _api = { _read, _write };

void siera_esp32_ds_gpio_init(siera_esp32_ds_gpio_t* self,
  siera_hal_gpio_t* hal,
  const siera_esp32_ds_gpio_pin_t* pins,
  size_t pin_count)
{
  self->hal = hal;
  self->pins = pins;
  self->pin_count = pin_count;
  self->stream.api = &_api;
  self->stream.ctx = self;

  for(size_t i = 0; i < pin_count; i++)
    siera_gpio_configure(hal, pins[i].pin, pins[i].dir, pins[i].pull);
}

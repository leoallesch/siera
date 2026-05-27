#include "siera/esp32_ds_gpio.h"

#include <stddef.h>

static siera_esp32_ds_gpio_t* self_of(i_siera_ds_t* iface)
{
  return (siera_esp32_ds_gpio_t*)iface;
}

static const siera_esp32_ds_gpio_pin_t* find_pin(const siera_esp32_ds_gpio_t* drv,
  siera_dsk_t key)
{
  for(size_t i = 0; i < drv->pin_count; i++) {
    if(drv->pins[i].key == key)
      return &drv->pins[i];
  }
  return NULL;
}

static void _poll_cb(void* arg)
{
  siera_esp32_ds_gpio_t* drv = (siera_esp32_ds_gpio_t*)arg;
  for(size_t i = 0; i < drv->pin_count; i++) {
    if(drv->pins[i].polled && drv->pins[i].dir == SIERA_GPIO_DIR_INPUT) {
      bool level = false;
      siera_gpio_read(drv->hal, drv->pins[i].pin, &level);
      if(drv->cache[i] != level) {
        drv->cache[i] = level;
        siera_ds_on_change_args_t args = { .key = drv->pins[i].key, .data = &drv->cache[i] };
        siera_event_publish(&drv->on_change, &args);
      }
    }
  }
}

static int _read(i_siera_ds_t* iface, siera_dsk_t key, void* buf, size_t size)
{
  (void)size;
  siera_esp32_ds_gpio_t* drv = self_of(iface);
  const siera_esp32_ds_gpio_pin_t* p = find_pin(drv, key);
  if(!p)
    return -1;

  if(p->polled) {
    size_t idx = (size_t)(p - drv->pins);
    *(bool*)buf = drv->cache[idx];
    return 0;
  }

  bool level = false;
  int ret = siera_gpio_read(drv->hal, p->pin, &level);
  if(ret != 0)
    return -1;

  *(bool*)buf = level;
  return 0;
}

static int _write(i_siera_ds_t* iface, siera_dsk_t key, const void* buf, size_t size)
{
  (void)size;
  siera_esp32_ds_gpio_t* drv = self_of(iface);
  const siera_esp32_ds_gpio_pin_t* p = find_pin(drv, key);
  if(!p)
    return -1;

  int ret = siera_gpio_write(drv->hal, p->pin, *(const bool*)buf);
  if(ret == 0) {
    size_t idx = (size_t)(p - drv->pins);
    bool new_val = *(const bool*)buf;
    if(drv->cache[idx] != new_val) {
      drv->cache[idx] = new_val;
      siera_ds_on_change_args_t args = { .key = key, .data = &drv->cache[idx] };
      siera_event_publish(&drv->on_change, &args);
    }
  }
  return ret;
}

static bool _contains(i_siera_ds_t* iface, siera_dsk_t key)
{
  return find_pin(self_of(iface), key) != NULL;
}

static size_t _size(i_siera_ds_t* iface, siera_dsk_t key)
{
  (void)iface;
  (void)key;
  return sizeof(bool);
}

static siera_event_t* _on_change(i_siera_ds_t* iface)
{
  return &self_of(iface)->on_change;
}

static const i_siera_ds_stream_api_t _api = {
  .read = _read,
  .write = _write,
  .contains = _contains,
  .size = _size,
  .on_change = _on_change,
};

void siera_esp32_ds_gpio_init(siera_esp32_ds_gpio_t* self,
  siera_hal_gpio_t* hal,
  const siera_esp32_ds_gpio_pin_t* pins,
  size_t pin_count,
  bool* cache,
  siera_timer_mgr_t* timers)
{
  self->interface.api = &_api;
  self->hal = hal;
  self->pins = pins;
  self->pin_count = pin_count;
  self->cache = cache;
  siera_event_init(&self->on_change);

  for(size_t i = 0; i < pin_count; i++)
    siera_gpio_configure(hal, pins[i].pin, pins[i].dir, pins[i].pull);

  uint32_t min_ms = 0;
  for(size_t i = 0; i < pin_count; i++) {
    if(pins[i].polled && pins[i].poll_ms > 0) {
      if(min_ms == 0 || pins[i].poll_ms < min_ms)
        min_ms = pins[i].poll_ms;
    }
  }
  if(min_ms > 0 && timers)
    siera_timer_start(timers, &self->poll_timer, _poll_cb, self, min_ms, true);
}

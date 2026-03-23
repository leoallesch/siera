#include "siera/esp32_timesource.h"

#include "esp_timer.h"

static siera_timesource_t s_timesource;

static uint32_t _get_ticks(siera_timesource_t* self)
{
  (void)self;
  return (uint32_t)(esp_timer_get_time() / 1000u); /* µs → ms */
}

siera_timesource_t* siera_esp32_timesource_init(void)
{
  s_timesource.get_ticks = _get_ticks;
  return &s_timesource;
}

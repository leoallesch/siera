#include "siera/sim_timesource.h"

#include <stdint.h>
#include <time.h>

static uint32_t _get_ticks(siera_hal_timesource_t* self)
{
  (void)self;
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint32_t)(ts.tv_sec * 1000u + ts.tv_nsec / 1000000u);
}

siera_hal_timesource_t* siera_sim_timesource_init()
{
  static siera_hal_timesource_t timesource;
  timesource.get_ticks = _get_ticks;
  return &timesource;
}

#include "siera/posix_timesource.h"

#include <stdint.h>
#include <time.h>

static uint32_t _get_ticks(siera_timesource_t* self)
{
  (void)self;
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint32_t)(ts.tv_sec * 1000u + ts.tv_nsec / 1000000u);
}

void posix_timesource_init(posix_timesource_t* self)
{
  self->interface.get_ticks = _get_ticks;
}

#include <time.h>
#include "timesource_simulator.h"

static timesource_ticks_t get_ticks(i_timesource_t* instance)
{
    (void)instance;

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (timesource_ticks_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

i_timesource_t* timesource_simulator(void)
{
  static i_timesource_t simulator = {
    .get_ticks = get_ticks,
  };
  return &simulator;
}
#include <stdio.h>

#include "siera/hal_timesource.h"

#include "app.h"

static uint32_t g_ticks = 0;

static uint32_t get_ticks(siera_timesource_t* self)
{
  (void)self;
  return g_ticks;
}

static siera_timesource_t g_timesource = { get_ticks };

int main(void)
{
  siera_timer_mgr_t timers;
  siera_timer_mgr_init(&timers, &g_timesource);

  app_init(&timers);

  printf("=== multi_target / POSIX ===\n");

  while(g_ticks < 3000) {
    g_ticks++;
    siera_timer_tick(&timers);
  }

  return 0;
}

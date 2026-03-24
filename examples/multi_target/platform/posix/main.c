#include <stdio.h>

#include "siera/sim_timesource.h"

#include "app.h"

int main(void)
{
  siera_timer_mgr_t timers;
  siera_timer_mgr_init(&timers, siera_sim_timesource_init());

  app_init(&timers);

  printf("=== multi_target / simulator ===\n");

  while(1) {
    siera_timer_tick(&timers);
  }

  return 0;
}

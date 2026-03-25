#include <stdio.h>

#include "siera/ds.h"
#include "siera/timer.h"

#include "app.h"

static siera_ds_t g_ds;
static siera_timer_t g_timer;

static void on_tick(void* ctx)
{
  (void)ctx;
  uint32_t counter;
  siera_ds_read(&g_ds, DSK_COUNTER, &counter);
  counter++;
  siera_ds_write(&g_ds, DSK_COUNTER, &counter);
  printf("[app] counter=%u\n", counter);
}

void app_init(siera_timer_mgr_t* timers)
{
  siera_ds_init(&g_ds, NULL, 0, timers, 0);
  siera_timer_start(timers, &g_timer, on_tick, NULL, 500, true);
}

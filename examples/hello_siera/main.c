#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "siera/ds.h"
#include "siera/timer.h"

/* ── Timesource (host: use a simple tick counter driven by the main loop) ── */

static uint32_t g_ticks = 0;
static uint32_t get_ticks(siera_timesource_t* self)
{
  (void)self;
  return g_ticks;
}
static siera_timesource_t g_timesource = { get_ticks };

/* ── Timer callbacks ────────────────────────────────────────────────────── */

static void on_tick(void* ctx)
{
  siera_ds_t* ds = ctx;

  uint32_t counter;
  siera_ds_read(ds, SIERA_DS_KEY_COUNTER, &counter);
  counter++;
  siera_ds_write(ds, SIERA_DS_KEY_COUNTER, &counter);

  bool armed;
  siera_ds_read(ds, SIERA_DS_KEY_ARMED, &armed);
  printf("[tick] counter=%u  armed=%d\n", counter, (int)armed);
}

static void on_arm(void* ctx)
{
  siera_ds_t* ds = ctx;
  bool armed = true;
  siera_ds_write(ds, SIERA_DS_KEY_ARMED, &armed);
}

/* ── Change subscriber ──────────────────────────────────────────────────── */

static void on_change(const void* args, void* ctx)
{
  (void)ctx;
  const siera_ds_on_change_t* e = (const siera_ds_on_change_t*)args;
  printf("[event] key=%s changed\n", siera_ds_key_name(e->key));
}

/* ── Main ───────────────────────────────────────────────────────────────── */

int main(void)
{
  siera_timer_mgr_t timers;
  siera_ds_t ds;

  siera_timer_mgr_init(&timers, &g_timesource);

  siera_ds_init(&ds, NULL, 0, &timers, 0);

  siera_event_sub_t sub;
  siera_event_sub_init(&sub, on_change, NULL);
  siera_ds_subscribe_all(&ds, &sub);

  siera_timer_t tick_timer, arm_timer;
  siera_timer_start(&timers, &tick_timer, on_tick, &ds, 500, true);
  siera_timer_start(&timers, &arm_timer, on_arm, &ds, 1500, false);

  printf("=== hello_siera ===\n");

  /* Simulate 3 seconds */
  while(g_ticks < 3000) {
    g_ticks++;
    siera_timer_tick(&timers);
  }

  siera_ds_deinit(&ds);
  return 0;
}

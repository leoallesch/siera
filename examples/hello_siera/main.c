#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "siera/common.h"
#include "siera/ds_composite.h"
#include "siera/ds_ram.h"
#include "siera/sim_timesource.h"
#include "siera/timer.h"

#define SIERA_DS_KEYS(KEY)   \
  KEY(DSK_COUNTER, uint32_t) \
  KEY(DSK_MESSAGE, uint8_t)  \
  KEY(DSK_ARMED, bool)

#define EXPAND_AS_ENUM(name, type) name,

#define EXPAND_AS_STORAGE(name, type) uint8_t name[sizeof(type)];

#define EXPAND_AS_CONFIG(name, type) { offsetof(ds_storage_t, name), sizeof(type) },

enum {
  SIERA_DS_KEYS(EXPAND_AS_ENUM)
    DSK_COUNT
};

typedef struct {
  SIERA_DS_KEYS(EXPAND_AS_STORAGE)
} ds_storage_t;

static void on_tick(void* ctx)
{
  i_siera_ds_t* ds = ctx;

  uint32_t counter;
  siera_ds_read(ds, DSK_COUNTER, &counter);
  counter++;
  siera_ds_write(ds, DSK_COUNTER, &counter);

  bool armed;
  siera_ds_read(ds, DSK_ARMED, &armed);
  printf("[tick] counter=%u  armed=%d\n", counter, (int)armed);
}

static void on_arm(void* ctx)
{
  i_siera_ds_t* ds = ctx;
  bool armed = true;
  siera_ds_write(ds, DSK_ARMED, &armed);
}

/* ── Change subscriber ──────────────────────────────────────────────────── */

static void on_change(void* ctx, const void* args)
{
  (void)ctx;
  const siera_ds_on_change_args_t* e = (const siera_ds_on_change_args_t*)args;
  printf("[event] key=%d changed\n", e->key);
}

/* ── Main ───────────────────────────────────────────────────────────────── */

int main(void)
{
  static const siera_ram_config_t ram_configs[] = {
    SIERA_DS_KEYS(EXPAND_AS_CONFIG)
  };
  static ds_storage_t ds_storage;

  siera_timer_mgr_t timers;
  siera_ds_ram_t ram_ds;
  siera_ds_composite_t ds_composite;
  i_siera_ds_t* ds = NULL;

  siera_hal_timesource_t* timesource = siera_sim_timesource_init();

  siera_timer_mgr_init(&timers, timesource);

  siera_ds_ram_init(&ram_ds, ram_configs, SIERA_NUM_ELEMENTS(ram_configs), &ds_storage);
  siera_composite_route_t routes[] = {
    { .interface = &ram_ds.interface }
  };
  siera_ds_composite_init(&ds_composite, routes, SIERA_NUM_ELEMENTS(routes));
  ds = &ds_composite.interface;

  siera_event_sub_t sub;
  siera_event_sub_init(&sub, on_change, NULL);
  siera_ds_subscribe_all(ds, &sub);

  siera_timer_t tick_timer, arm_timer;
  siera_timer_start(&timers, &tick_timer, on_tick, ds, 500, true);
  siera_timer_start(&timers, &arm_timer, on_arm, ds, 1500, false);

  printf("=== hello_siera ===\n");

  while(1) {
    siera_timer_tick(&timers);
  }

  return 0;
}

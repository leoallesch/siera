#include <stddef.h>
#include <stdio.h>

#include "siera/common.h"
#include "siera/ds_composite.h"
#include "siera/ds_ram.h"
#include "siera/sim.h"
#include "siera/sim_ds_io.h"
#include "siera/sim_ds_nvs.h"
#include "siera/sim_timesource.h"
#include "siera/timer.h"

#include "lvgl.h"

#include "app.h"
#include "bsp_dsk.h"
#include "system_dsk.h"

/* ── RAM storage and config (generated from DS_RAM-tagged keys) ───────────── */

typedef struct {
  DSK_ENTRIES(EXPAND_AS_RAM_STORAGE)
} ds_ram_storage_t;

static ds_ram_storage_t g_ram_storage;

static const siera_ram_config_t g_ram_cfg[] = {
  DSK_ENTRIES(EXPAND_AS_RAM_CONFIG)
};

/* ── Globals ──────────────────────────────────────────────────────────────── */

static siera_sim_t g_sim;
static siera_sim_ds_nvs_t g_nvs;
static siera_ds_ram_t g_ram;
static siera_ds_composite_t g_ds;

/* ── Simulator widget panel ────────────────────────────────────────────────── */

static const siera_sim_widget_t g_widgets[] = {
  /* BSP inputs — generated from bsp_dsk.h */
  DSK_BSP_ENTRIES(EXPAND_AS_WIDGET_CONFIG)
};

static const siera_sim_config_t g_sim_config = {
  .app_width = 480,
  .app_height = 320,
  .widgets = g_widgets,
  .widget_count = SIERA_NUM_ELEMENTS(g_widgets),
};

int main(void)
{
  lv_init();

  siera_timer_mgr_t timers;
  siera_timer_mgr_init(&timers, siera_sim_timesource_init());

  siera_sim_init(&g_sim, &g_sim_config);

  siera_sim_ds_nvs_init(&g_nvs, "siera_nvs.bin");
  DSK_ENTRIES(EXPAND_AS_NVS_REGISTER)

  siera_ds_ram_init(&g_ram, g_ram_cfg, DSK_RAM_COUNT, &g_ram_storage);

  static siera_composite_route_t routes[3];
  routes[0].interface = siera_sim_get_io(&g_sim);
  routes[1].interface = &g_nvs.interface;
  routes[2].interface = &g_ram.interface;
  siera_ds_composite_init(&g_ds, routes, SIERA_NUM_ELEMENTS(routes));

  /* Output widgets observe the composite so they see RAM/NVS-backed writes. */
  siera_sim_bind_outputs(&g_sim, &g_ds.interface);

  app_init(&g_ds.interface, &timers, siera_sim_get_display(&g_sim));

  printf("=== simulator ===\n");

  while(1) {
    uint32_t time_till_next = lv_timer_handler();
    siera_timer_tick(&timers);
    lv_delay_ms(time_till_next);
  }

  return 0;
}

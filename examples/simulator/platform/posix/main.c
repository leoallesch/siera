#include <stdio.h>

#include "siera/ds.h"
#include "siera/event.h"
#include "siera/sim.h"
#include "siera/sim_ds_nvs.h"
#include "siera/sim_timesource.h"

#include "lvgl.h"

#include "app.h"

static siera_ds_t g_ds;
static siera_sim_t g_sim;
static siera_sim_ds_nvs_t g_nvs;
static siera_event_sub_t g_sim_input_sub;

static const siera_sim_input_t g_sim_inputs[] = {
  { SIERA_DS_KEY_BTN_PAUSE, SIERA_SIM_INPUT_BUTTON },
  { SIERA_DS_KEY_BTN_RESET, SIERA_SIM_INPUT_BUTTON },
};

static const siera_sim_config_t g_sim_config = {
  .app_width = 480,
  .app_height = 320,
  .inputs = g_sim_inputs,
  .input_count = SIERA_NUM_ELEMENTS(g_sim_inputs),
};

static void on_sim_input(const void* args, void* ctx)
{
  const siera_sim_input_event_t* e = args;
  siera_ds_write((siera_ds_t*)ctx, e->key, e->val);
}

int main(void)
{
  lv_init();

  siera_timer_mgr_t timers;
  siera_timer_mgr_init(&timers, siera_sim_timesource_init());
  siera_sim_init(&g_sim, &g_sim_config);

  siera_sim_ds_nvs_init(&g_nvs, "siera_nvs.bin");
  siera_ds_stream_binding_t streams[] = {
    { SIERA_DS_NVS, &g_nvs.stream },
    { SIERA_DS_GPIO, &siera_null_stream },
  };
  siera_ds_init(&g_ds, streams, SIERA_NUM_ELEMENTS(streams), &timers, 0);

  siera_event_sub_init(&g_sim_input_sub, on_sim_input, &g_ds);
  siera_event_subscribe(&g_sim.input_event, &g_sim_input_sub);

  app_init(&g_ds, &timers, siera_sim_get_display(&g_sim));

  printf("=== simulator ===\n");

  while(1) {
    uint32_t time_till_next = lv_timer_handler();
    siera_timer_tick(&timers);
    lv_delay_ms(time_till_next);
  }

  return 0;
}

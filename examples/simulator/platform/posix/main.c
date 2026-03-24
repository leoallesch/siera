#include <stdio.h>

#include "siera/ds.h"
#include "siera/posix_sim.h"
#include "siera/posix_timesource.h"

#include "lvgl.h"

#include "app.h"

static siera_ds_t g_ds;
static siera_posix_sim_t g_sim;

static const siera_posix_sim_input_t g_sim_inputs[] = {
  { SIERA_DS_KEY_BUTTON_1, SIERA_SIM_INPUT_BUTTON },
  { SIERA_DS_KEY_BUTTON_2, SIERA_SIM_INPUT_BUTTON },
  { SIERA_DS_KEY_BUTTON_3, SIERA_SIM_INPUT_BUTTON },
  { SIERA_DS_KEY_BUTTON_4, SIERA_SIM_INPUT_BUTTON },
  { SIERA_DS_KEY_BUTTON_5, SIERA_SIM_INPUT_BUTTON },
};

static const siera_posix_sim_config_t g_sim_config = {
  .app_width = 480,
  .app_height = 320,
  .inputs = g_sim_inputs,
  .input_count = SIERA_NUM_ELEMENTS(g_sim_inputs),
};

int main(void)
{
  siera_timer_mgr_t timers;
  siera_timer_mgr_init(&timers, siera_posix_timesource_init());

  siera_ds_init(&g_ds, NULL, 0, &timers, 0);

  lv_init();

  siera_posix_sim_init(&g_sim, &g_sim_config, &g_ds);

  app_init(&g_ds, &timers, siera_posix_sim_get_display(&g_sim));

  printf("=== simulator / POSIX ===\n");

  while(1) {
    uint32_t time_till_next = lv_timer_handler();
    siera_timer_tick(&timers);
    lv_delay_ms(time_till_next);
  }

  return 0;
}

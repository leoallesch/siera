#pragma once

#include "lvgl.h"
#include "siera/sim.h"

int sim_ui_panel_width(uint8_t input_count);

lv_obj_t* sim_ui_create_screen(
  int                      app_width,
  int                      app_height,
  const siera_sim_input_t* inputs,
  uint8_t                  input_count,
  siera_sim_t*             self,
  lv_obj_t**               out_content_area,
  lv_obj_t**               out_input_panel);

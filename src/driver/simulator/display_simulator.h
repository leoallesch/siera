#pragma once

#include "event_subscription.h"
#include "i_datastream.h"
#include "i_view.h"
#include "lvgl.h"

enum {
  SIMULATOR_INPUT_BUTTON,
  SIMULATOR_INPUT_SWITCH,
  SIMULATOR_INPUT_SLIDER
};
typedef uint8_t simulator_input_t;

typedef struct {
  datastream_key_t key;
  const char* name;
  simulator_input_t input;
} simulator_input_config_t;

typedef struct
{
  lv_display_t* sdl_display;
  lv_indev_t* indev;
  lv_obj_t* content_area;

  i_view_t* current_view;

  i_datastream_t* bsp_stream;
  i_datastream_t* ram_stream;

  event_subscription_t sub;
} display_simulator_t;

void display_simulator_init(
  display_simulator_t* instance,
  int app_width,
  int app_height,
  const simulator_input_config_t* input_config,
  uint8_t input_config_count,
  i_datastream_t* bsp_stream,
  i_datastream_t* ram_stream,
  datastream_key_t current_view);

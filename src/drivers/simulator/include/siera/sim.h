#ifndef SIERA_SIM_H
#define SIERA_SIM_H

#include "lvgl.h"
#include "siera/ds.h"
#include "siera/event.h"
#include "siera/sim_display.h"

#ifndef SIERA_SIM_MAX_INPUTS
#define SIERA_SIM_MAX_INPUTS 16
#endif

typedef enum {
  SIERA_SIM_INPUT_BUTTON,
  SIERA_SIM_INPUT_SWITCH,
  SIERA_SIM_INPUT_SLIDER,
} siera_sim_input_type_t;

typedef struct {
  siera_ds_key_t key;
  siera_sim_input_type_t type;
} siera_sim_input_t;

typedef struct {
  int app_width;
  int app_height;

  const siera_sim_input_t* inputs;
  uint8_t input_count;
} siera_sim_config_t;

typedef struct {
  siera_ds_t* ds;
  siera_ds_key_t key;
} siera_sim_input_ctx_t;

typedef struct {
  lv_display_t* display;
  lv_indev_t* mouse;
  lv_obj_t* content_area;

  siera_sim_display_t sim_display;

  siera_ds_t* ds;

  siera_sim_input_ctx_t input_ctx[SIERA_SIM_MAX_INPUTS];
} siera_sim_t;

void siera_sim_init(siera_sim_t* self,
  const siera_sim_config_t* cfg,
  siera_ds_t* ds);
siera_hal_display_t* siera_sim_get_display(siera_sim_t* self);

#endif /* SIERA_SIM_H */

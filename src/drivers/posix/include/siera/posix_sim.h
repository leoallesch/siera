#ifndef SIERA_POSIX_SIM_H
#define SIERA_POSIX_SIM_H

#include "siera/ds.h"
#include "siera/event.h"
#include "siera/posix_display.h"
#include "lvgl.h"

#ifndef SIERA_POSIX_SIM_MAX_INPUTS
#define SIERA_POSIX_SIM_MAX_INPUTS 16
#endif

typedef enum {
  SIERA_SIM_INPUT_BUTTON,
  SIERA_SIM_INPUT_SWITCH,
  SIERA_SIM_INPUT_SLIDER,
} siera_sim_input_type_t;

typedef struct {
  siera_ds_key_t         key;
  siera_sim_input_type_t type;
} siera_posix_sim_input_t;

typedef struct {
  int app_width;
  int app_height;

  const siera_posix_sim_input_t *inputs;
  uint8_t                        input_count;
} siera_posix_sim_config_t;

typedef struct {
  siera_ds_t    *ds;
  siera_ds_key_t key;
} siera_posix_sim_input_ctx_t;

typedef struct {
  lv_display_t *display;
  lv_indev_t   *mouse;
  lv_obj_t     *content_area;

  siera_posix_display_t posix_display;

  siera_ds_t *ds;

  siera_posix_sim_input_ctx_t input_ctx[SIERA_POSIX_SIM_MAX_INPUTS];
} siera_posix_sim_t;

void                 siera_posix_sim_init(siera_posix_sim_t              *self,
                                          const siera_posix_sim_config_t *cfg,
                                          siera_ds_t                     *ds);
siera_hal_display_t *siera_posix_sim_get_display(siera_posix_sim_t *self);

#endif /* SIERA_POSIX_SIM_H */

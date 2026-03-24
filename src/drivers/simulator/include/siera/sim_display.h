#ifndef SIERA_SIM_DISPLAY_H
#define SIERA_SIM_DISPLAY_H

#include "siera/hal_display.h"
#include "lvgl.h"

typedef struct {
  siera_hal_display_t  interface;    /* must be first */
  lv_obj_t            *content_area;
} siera_sim_display_t;

void siera_sim_display_init(siera_sim_display_t *self, lv_obj_t *content_area);

#endif /* SIERA_SIM_DISPLAY_H */

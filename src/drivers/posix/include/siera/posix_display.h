#ifndef SIERA_POSIX_DISPLAY_H
#define SIERA_POSIX_DISPLAY_H

#include "siera/hal_display.h"
#include "lvgl.h"

typedef struct {
  siera_hal_display_t  interface;    /* must be first */
  lv_obj_t            *content_area;
} siera_posix_display_t;

void siera_posix_display_init(siera_posix_display_t *self, lv_obj_t *content_area);

#endif /* SIERA_POSIX_DISPLAY_H */

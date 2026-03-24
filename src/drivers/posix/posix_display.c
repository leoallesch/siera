#include "siera/posix_display.h"

static void *posix_display_get_canvas(siera_hal_display_t *self)
{
  siera_posix_display_t *d = (siera_posix_display_t *)self;
  return d->content_area;
}

static void posix_display_flush(siera_hal_display_t *self)
{
  (void)self;
  /* LVGL manages its own render loop; nothing to do here */
}

void siera_posix_display_init(siera_posix_display_t *self, lv_obj_t *content_area)
{
  self->interface.get_canvas = posix_display_get_canvas;
  self->interface.flush      = posix_display_flush;
  self->content_area         = content_area;
}

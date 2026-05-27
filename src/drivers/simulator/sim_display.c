#include "siera/sim_display.h"

static void* sim_display_get_canvas(siera_hal_display_t* self)
{
  siera_sim_display_t* d = (siera_sim_display_t*)self;
  return d->content_area;
}

static void sim_display_flush(siera_hal_display_t* self)
{
  (void)self;
  /* LVGL manages its own render loop; nothing to do here */
}

void siera_sim_display_init(siera_sim_display_t* self, lv_obj_t* content_area)
{
  self->interface.get_canvas = sim_display_get_canvas;
  self->interface.flush = sim_display_flush;
  self->content_area = content_area;
}

#include "counter_view.h"

#include <stdio.h>

static void counter_view_load(siera_view_t *base, void *canvas)
{
  counter_view_t *self = (counter_view_t *)base;
  lv_obj_t *parent = (lv_obj_t *)canvas;

  self->label = lv_label_create(parent);
  lv_label_set_text(self->label, "Counter: 0");
  lv_obj_set_style_text_color(self->label, lv_color_hex(0xD4D4E8), 0);
  lv_obj_center(self->label);
}

static void counter_view_unload(siera_view_t *base)
{
  counter_view_t *self = (counter_view_t *)base;
  if(self->label != NULL) {
    lv_obj_delete(self->label);
    self->label = NULL;
  }
}

void counter_view_init(counter_view_t *self)
{
  self->base.load   = counter_view_load;
  self->base.unload = counter_view_unload;
  self->label       = NULL;
}

void counter_view_update(counter_view_t *self, uint32_t value)
{
  if(self->label == NULL)
    return;

  char buf[32];
  snprintf(buf, sizeof(buf), "Counter: %lu", (unsigned long)value);
  lv_label_set_text(self->label, buf);
}

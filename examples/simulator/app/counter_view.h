#ifndef COUNTER_VIEW_H
#define COUNTER_VIEW_H

#include <stdint.h>
#include "lvgl.h"
#include "siera/view.h"

typedef struct {
  siera_view_t base; /* must be first */
  lv_obj_t* label;
} counter_view_t;

void counter_view_init(counter_view_t* self);
void counter_view_update(counter_view_t* self, uint32_t value);

#endif /* COUNTER_VIEW_H */

#pragma once

#include "i_view.h"

typedef struct i_display_t {
  void (*update)(struct i_display_t* self, i_view_t* view);
} i_display_t;

static inline void display_update(i_display_t* self, i_view_t* view)
{
  self->update(self, view);
}
